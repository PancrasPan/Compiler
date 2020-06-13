//===-- DSPHardwareLoops.cpp - Identify and generate hardware loops ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass identifies loops where we can generate the DSP hardware
// loop instruction.  The hardware loop can perform loop branches with a
// zero-cycle overhead.
//
// The pattern that defines the induction variable can changed depending on
// prior optimizations.  For example, the IndVarSimplify phase run by 'opt'
// normalizes induction variables, and the Loop Strength Reduction pass
// run by 'llc' may also make changes to the induction variable.
// The pattern detected by this phase is due to running Strength Reduction.
//
// Criteria for hardware loops:
//  - Countable loops (w/ ind. var for a trip count)
//  - Assumes loops are normalized by IndVarSimplify
//  - Try inner-most loops first
//  - No nested hardware loops.
//  - No function calls in loops.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/SmallSet.h"
#include "DSP.h"
#include "DSPTargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include <algorithm>
#include <vector>
#include <iostream>

using namespace llvm;

#define DEBUG_TYPE "hwloops"
//#define DSPDEBUG true
#define VREGBASEADDR (2147483648)

cl::opt<bool> DisableHardwareLoops("disable-dsp-hwloops",
	cl::Hidden, cl::desc("Disable Hardware Loops for DSP target"));

#ifndef NDEBUG
static cl::opt<int> HWLoopLimit("max-dsphwloop", cl::Hidden, cl::init(-1));
#endif

cl::opt<bool> ShowDbgInfo("dispdbg",
	cl::Hidden, cl::desc(""));

STATISTIC(NumHWLoops, "Number of loops converted to hardware loops");

namespace llvm {
  void initializeDSPHardwareLoopsPass(PassRegistry&);
}

namespace {
  class CountValue;
  struct DSPHardwareLoops : public MachineFunctionPass {
    MachineLoopInfo            *MLI;
    MachineRegisterInfo        *MRI;
    MachineDominatorTree       *MDT;
    const DSPTargetMachine *TM;
    const DSPInstrInfo     *TII;
    const DSPRegisterInfo  *TRI;
#ifndef NDEBUG
    static int Counter;
#endif

  public:
    static char ID;

    DSPHardwareLoops() : MachineFunctionPass(ID) {
      initializeDSPHardwareLoopsPass(*PassRegistry::getPassRegistry());
    }

    bool runOnMachineFunction(MachineFunction &MF) override;

    const char *getPassName() const override { return "DSP Hardware Loops"; }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<MachineDominatorTree>();
      AU.addRequired<MachineLoopInfo>();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

  private:
    /// Kinds of comparisons in the compare instructions.
    struct Comparison {
      enum Kind {
        EQ  = 0x01,
        NE  = 0x02,
        L   = 0x04, // Less-than property.
        G   = 0x08, // Greater-than property.
        U   = 0x40, // Unsigned property.
        LTs = L,
        LEs = L | EQ,
        GTs = G,
        GEs = G | EQ,
        LTu = L      | U,
        LEu = L | EQ | U,
        GTu = G      | U,
        GEu = G | EQ | U
      };

      static Kind getSwappedComparison(Kind Cmp) {
        assert ((!((Cmp & L) && (Cmp & G))) && "Malformed comparison operator");
        if ((Cmp & L) || (Cmp & G))
          return (Kind)(Cmp ^ (L|G));
        return Cmp;
      }

	  ///Get the comparison relationship between induction variable and bound for compute
	  ///the trip count of loop.
	  ///The loop jumps back to the header block when it is satisfied.
	  static bool getIterationCmpType(unsigned CondOpc, bool Negated, Kind &Cmp) {
		  switch (CondOpc) {
		  case DSP::EQI:
		  case DSP::EQ:
			  Cmp = !Negated ? Comparison::EQ : Comparison::NE;
			  break;
		  case DSP::NEQI:
		  case DSP::NEQ:
			  Cmp = !Negated ? Comparison::NE : Comparison::EQ;
			  break;
		  case DSP::GTI:
		  case DSP::GT:
			  Cmp = !Negated ? Comparison::GTs : Comparison::LEs;
			  break;
		  case DSP::GTU:
			  Cmp = !Negated ? Comparison::GTu : Comparison::LEu;
			  break;
		  case DSP::GEI:
		  case DSP::GE:
			  Cmp = !Negated ? Comparison::GEs : Comparison::LTs;
			  break;
		  case DSP::GEU:
			  Cmp = !Negated ? Comparison::GEu : Comparison::LTu;
			  break;
		  case DSP::LTI:
		  case DSP::LT:
			  Cmp = !Negated ? Comparison::LTs : Comparison::GEs;
			  break;
		  case DSP::LTU:
			  Cmp = !Negated ? Comparison::LTu : Comparison::GEu;
			  break;
		  case DSP::LEI:
		  case DSP::LE:
			  Cmp = !Negated ? Comparison::LEs : Comparison::GTs;
			  break;
		  case DSP::LEU:
			  Cmp = !Negated ? Comparison::LEu : Comparison::GTu;
			  break;
		  default:
			  return 0;
		  }
		  return 1;
	  }
    };

    /// \brief Find the register that contains the loop controlling
    /// induction variable.
    /// If successful, it will return true and set the \p Reg, \p IVBump
    /// and \p IVOp arguments.  Otherwise it will return false.
    /// The returned induction register is the register R that follows the
    /// following induction pattern:
    /// loop:
    ///   R = phi ..., [ R.next, LatchBlock ]
    ///   R.next = R + #bump
    ///   if (R.next < #N) goto loop
    /// IVBump is the immediate value added to R, and IVOp is the instruction
    /// "R.next = R + #bump".
    bool findInductionRegister(MachineLoop *L, unsigned &Reg,
                               int64_t &IVBump, MachineInstr *&IVOp) const;

    /// \brief Analyze the statements in a loop to determine if the loop
    /// has a computable trip count and, if so, return a value that represents
    /// the trip count expression.
    CountValue *getLoopTripCount(MachineLoop *L,
                                 SmallVectorImpl<MachineInstr *> &OldInsts);

    /// \brief Return the expression that represents the number of times
    /// a loop iterates.  The function takes the operands that represent the
    /// loop start value, loop end value, and induction value.  Based upon
    /// these operands, the function attempts to compute the trip count.
    /// If the trip count is not directly available (as an immediate value,
    /// or a register), the function will attempt to insert computation of it
    /// to the loop's preheader.
    CountValue *computeCount(MachineLoop *Loop,
                             const MachineOperand *Start,
                             const MachineOperand *End,
                             unsigned IVReg,
                             int64_t IVBump,
                             Comparison::Kind Cmp) const;

    /// \brief Return true if the instruction is not valid within a hardware
    /// loop.
    bool isInvalidLoopOperation(const MachineInstr *MI) const;

    /// \brief Return true if the loop contains an instruction that inhibits
    /// using the hardware loop.
    bool containsInvalidInstruction(MachineLoop *L) const;

    /// \brief Given a loop, check if we can convert it to a hardware loop.
    /// If so, then perform the conversion and return true.
    bool convertToHardwareLoop(MachineLoop *L);

    /// \brief Return true if the instruction is now dead.
    bool isDead(const MachineInstr *MI,
                SmallVectorImpl<MachineInstr *> &DeadPhis) const;

    /// \brief Remove the instruction if it is now dead.
    void removeIfDead(MachineInstr *MI);

    /// \brief Make sure that the "bump" instruction executes before the
    /// compare.  We need that for the IV fixup, so that the compare
    /// instruction would not use a bumped value that has not yet been
    /// defined.  If the instructions are out of order, try to reorder them.
    bool orderBumpCompare(MachineInstr *BumpI, MachineInstr *CmpI);

    /// \brief Get the instruction that loads an immediate value into \p R,
    /// or 0 if such an instruction does not exist.
    MachineInstr *defWithImmediate(unsigned R);

    /// \brief Get the immediate value referenced to by \p MO, either for
    /// immediate operands, or for register operands, where the register
    /// was defined with an immediate value.
    int64_t getImmediate(MachineOperand &MO);

    /// \brief Reset the given machine operand to now refer to a new immediate
    /// value.  Assumes that the operand was already referencing an immediate
    /// value, either directly, or via a register.
    void setImmediate(MachineOperand &MO, int64_t Val);

    /// \brief Fix the data flow of the induction varible.
    /// The desired flow is: phi ---> bump -+-> comparison-in-latch.
    ///                                     |
    ///                                     +-> back to phi
    /// where "bump" is the increment of the induction variable:
    ///   iv = iv + #const.
    /// Due to some prior code transformations, the actual flow may look
    /// like this:
    ///   phi -+-> bump ---> back to phi
    ///        |
    ///        +-> comparison-in-latch (against upper_bound-bump),
    /// i.e. the comparison that controls the loop execution may be using
    /// the value of the induction variable from before the increment.
    ///
    /// Return true if the loop's flow is the desired one (i.e. it's
    /// either been fixed, or no fixing was necessary).
    /// Otherwise, return false.  This can happen if the induction variable
    /// couldn't be identified, or if the value in the latch's comparison
    /// cannot be adjusted to reflect the post-bump value.
    bool fixupInductionVariable(MachineLoop *L);

    /// \brief Given a loop, if it does not have a preheader, create one.
    /// Return the block that is the preheader.
    MachineBasicBlock *createPreheaderForLoop(MachineLoop *L);

	/// \brief Create a new MBB prelatch right before the latch block, moving
	///the instructions from latch to prelatch except the last one and the 
	///control flow after it.
	///The processed latch block will mark the end address of DSP hardware loop.
	/// It is not valid to replace the loop header with this method.
	MachineBasicBlock * splitLatchBlock(MachineLoop * L);
	
	/// \brief getVRegOriginDef - Return the original machine instr that
	/// defines the specified virtual register or null if none is found
	/// while skipping the COPY instruction.
	MachineInstr * getVRegOriginDef(unsigned Reg) const;
  };

  char DSPHardwareLoops::ID = 0;
#ifndef NDEBUG
  int DSPHardwareLoops::Counter = 0;
#endif

  /// \brief Abstraction for a trip count of a loop. A smaller vesrsion
  /// of the MachineOperand class without the concerns of changing the
  /// operand representation.
  class CountValue {
  public:
    enum CountValueType {
      CV_Register,
      CV_Immediate
    };
  private:
    CountValueType Kind;
    union Values {
      struct {
        unsigned Reg;
        unsigned Sub;
      } R;
      unsigned ImmVal;
    } Contents;

  public:
    explicit CountValue(CountValueType t, unsigned v, unsigned u = 0) {
      Kind = t;
      if (Kind == CV_Register) {
        Contents.R.Reg = v;
        Contents.R.Sub = u;
      } else {
        Contents.ImmVal = v;
      }
    }
    bool isReg() const { return Kind == CV_Register; }
    bool isImm() const { return Kind == CV_Immediate; }

    unsigned getReg() const {
      assert(isReg() && "Wrong CountValue accessor");
      return Contents.R.Reg;
    }
    unsigned getSubReg() const {
      assert(isReg() && "Wrong CountValue accessor");
      return Contents.R.Sub;
    }
    unsigned getImm() const {
      assert(isImm() && "Wrong CountValue accessor");
      return Contents.ImmVal;
    }

    void print(raw_ostream &OS, const TargetMachine *TM = nullptr) const {
      const TargetRegisterInfo *TRI = TM ? TM->getRegisterInfo() : nullptr;
      if (isReg()) { OS << PrintReg(Contents.R.Reg, TRI, Contents.R.Sub); }
      if (isImm()) { OS << Contents.ImmVal; }
    }
  };
} // end anonymous namespace


INITIALIZE_PASS_BEGIN(DSPHardwareLoops, "hwloops",
                      "DSP Hardware Loops", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_END(DSPHardwareLoops, "hwloops",
                    "DSP Hardware Loops", false, false)


/// \brief Returns true if the instruction is a hardware loop instruction.
static bool isHardwareLoop(const MachineInstr *MI) {
	return MI->getOpcode() == DSP::Loop;
}

FunctionPass *llvm::createDSPHardwareLoops() {
  return new DSPHardwareLoops();
}


bool DSPHardwareLoops::runOnMachineFunction(MachineFunction &MF) {
  DEBUG(dbgs() << "\n********* DSP Hardware Loops *********\n");

  bool Changed = false;

  MLI = &getAnalysis<MachineLoopInfo>();
  MRI = &MF.getRegInfo();
  MDT = &getAnalysis<MachineDominatorTree>();
  TM  = static_cast<const DSPTargetMachine*>(&MF.getTarget());
  TII = static_cast<const DSPInstrInfo*>(TM->getInstrInfo());
  TRI = static_cast<const DSPRegisterInfo*>(TM->getRegisterInfo());

  for (MachineLoopInfo::iterator I = MLI->begin(), E = MLI->end();
       I != E; ++I) {
    MachineLoop *L = *I;
	//Find the outermost root loop firstly, then stepping into and processing 
	//its inner-most loop layer. This will avoid converting the same subloop.
    if (!L->getParentLoop())
      Changed |= convertToHardwareLoop(L);
  }

  if (ShowDbgInfo)
	  std::cout << "<--** Totally Complete " << NumHWLoops << 
	  " HardwareLoop conversions**-->" << std::endl;
  if (ShowDbgInfo && Changed)
	DEBUG({ dbgs() << "\n** HWLoops  result **\n";   MF.print(dbgs()); });

  return Changed;
}

/// \brief Return the latch block if it's one of the exiting blocks. Otherwise,
/// return the exiting block. Return 'null' when multiple exiting blocks are
/// present.
static MachineBasicBlock* getExitingBlock(MachineLoop *L) {
	if (MachineBasicBlock *Latch = L->getLoopLatch()) {
		if (L->isLoopExiting(Latch))
			return Latch;
		else
			return L->getExitingBlock();
	}
	return nullptr;
}

bool DSPHardwareLoops::findInductionRegister(MachineLoop *L,
                                                 unsigned &Reg,
                                                 int64_t &IVBump,
                                                 MachineInstr *&IVOp
                                                 ) const {
  MachineBasicBlock *Header = L->getHeader();
  MachineBasicBlock *Preheader = L->getLoopPreheader();
  MachineBasicBlock *Latch = L->getLoopLatch();
  MachineBasicBlock *ExitingBlock = getExitingBlock(L);
  if (!Header || !Preheader || !Latch || !ExitingBlock)
	  return false;

  // This pair represents an induction register together with an immediate
  // value that will be added to it in each loop iteration.
  typedef std::pair<unsigned,int64_t> RegisterBump;

  // Mapping:  R.next -> (R, bump), where R, R.next and bump are derived
  // from an induction operation
  //   R.next = R + bump
  // where bump is an immediate value.
  typedef std::map<unsigned,RegisterBump> InductionMap;

  InductionMap IndMap;

  typedef MachineBasicBlock::instr_iterator instr_iterator;
  for (instr_iterator I = Header->instr_begin(), E = Header->instr_end();
       I != E && I->isPHI(); ++I) {
    MachineInstr *Phi = &*I;

    // Have a PHI instruction.  Get the operand that corresponds to the
    // latch block, and see if is a result of an addition of form "reg+imm",
    // where the "reg" is defined by the PHI node we are looking at.
    for (unsigned i = 1, n = Phi->getNumOperands(); i < n; i += 2) {
      if (Phi->getOperand(i+1).getMBB() != Latch)
        continue;

      unsigned PhiOpReg = Phi->getOperand(i).getReg();
      MachineInstr *DI = getVRegOriginDef(PhiOpReg);
      unsigned UpdOpc = DI->getOpcode();
      bool isAdd = (UpdOpc == DSP::ADDiu);

      if (isAdd) {
        // If the register operand to the add is the PHI we're
        // looking at, this meets the induction pattern.
        unsigned IndReg = DI->getOperand(1).getReg();
        if (getVRegOriginDef(IndReg) == Phi) {
          unsigned UpdReg = DI->getOperand(0).getReg();
          int64_t V = DI->getOperand(2).getImm();
          IndMap.insert(std::make_pair(UpdReg, std::make_pair(IndReg, V)));
        }
      }
    }  // for (i)
  }  // for (instr)

  SmallVector<MachineOperand,2> Cond;
  MachineBasicBlock *TB = nullptr, *FB = nullptr;
  bool NotAnalyzed = TII->AnalyzeBranch(*Latch, TB, FB, Cond, false);
  if (NotAnalyzed)
    return false;

  unsigned CSz = Cond.size();
  //assert (CSz == 0 || CSz == 2);
  //Is latch Block must end with a conditional branch ??
  //For conditional branch, the \p Cond is pushed into <opcode, predicateReg>.
  assert (CSz == 2);
  unsigned PredR = Cond[CSz - 1].getReg();
	if(ShowDbgInfo)
		std::cout << "Pred Register in Cond of AnalyzeBranch(Latch): " 
		<< (PredR-VREGBASEADDR)  << std::endl;

  MachineInstr *PredI = getVRegOriginDef(PredR);
  if (!PredI->isCompare())
    return false;

  unsigned CmpReg1 = 0, CmpReg2 = 0;
  int CmpImm = 0, CmpMask = 0;
  bool CmpAnalyzed = TII->analyzeCompare(PredI, CmpReg1, CmpReg2,
                                         CmpMask, CmpImm);
  // Fail if the compare was not analyzed, or it's not comparing a register
  // with an immediate value.  Not checking the mask here, since we handle
  // the vividual compare opcodes (including CMPb) later on.
  if (!CmpAnalyzed)
    return false;

  // Exactly one of the input registers to the comparison should be among
  // the induction registers.
  InductionMap::iterator IndMapEnd = IndMap.end();
  InductionMap::iterator F = IndMapEnd;
  if (CmpReg1 != 0) {
    InductionMap::iterator F1 = IndMap.find(CmpReg1);
    if (F1 != IndMapEnd)
      F = F1;
  }
  if (CmpReg2 != 0) {
    InductionMap::iterator F2 = IndMap.find(CmpReg2);
    if (F2 != IndMapEnd) {
      if (F != IndMapEnd)
        return false;
      F = F2;
    }
  }
  if (F == IndMapEnd)
    return false;

  Reg = F->second.first;
  IVBump = F->second.second;
  IVOp = getVRegOriginDef(F->first);
  return true;
}


/// \brief Analyze the statements in a loop to determine if the loop has
/// a computable trip count and, if so, return a value that represents
/// the trip count expression.
///
/// This function iterates over the phi nodes in the loop to check for
/// induction variable patterns that are used in the calculation for
/// the number of time the loop is executed.
CountValue *DSPHardwareLoops::getLoopTripCount(MachineLoop *L,
                                    SmallVectorImpl<MachineInstr *> &OldInsts) {
  MachineBasicBlock *TopMBB = L->getTopBlock();
  MachineBasicBlock::pred_iterator PI = TopMBB->pred_begin();
  assert(PI != TopMBB->pred_end() &&
         "Loop must have more than one incoming edge!");
  MachineBasicBlock *Backedge = *PI++;
  if (PI == TopMBB->pred_end())  // dead loop?
    return nullptr;
  MachineBasicBlock *Incoming = *PI++;
  if (PI != TopMBB->pred_end())  // multiple backedges?
    return nullptr;

  // Make sure there is one incoming and one backedge and determine which
  // is which.
  if (L->contains(Incoming)) {
    if (L->contains(Backedge))
      return nullptr;
    std::swap(Incoming, Backedge);
  } else if (!L->contains(Backedge))
    return nullptr;

  if (ShowDbgInfo) {
	  std::cout << "\n**getLoopTripCount**" << std::endl;
	  std::cout << "TOPBlock:" << TopMBB->getFullName() << std::endl;
	  std::cout << "IncomingBlock:" << Incoming->getFullName() << std::endl;
	  std::cout << "BackedgeBlock:" << Backedge->getFullName() << std::endl;
  }
  // Look for the cmp instruction to determine if we can get a useful trip
  // count.  The trip count can be either a register or an immediate.  The
  // location of the value depends upon the type (reg or imm).
  MachineBasicBlock *Latch = L->getLoopLatch();
  if (!Latch)
    return nullptr;

  unsigned IVReg = 0;
  int64_t IVBump = 0;
  MachineInstr *IVOp;
  bool FoundIV = findInductionRegister(L, IVReg, IVBump, IVOp);
  if (ShowDbgInfo) {
	  std::cout << "\n**findInductionRegister**" << std::endl;
	  std::cout << "IVReg:" << (IVReg - VREGBASEADDR) << std::endl;
	  std::cout << "IVBump:" << IVBump << std::endl;
  }
  if (!FoundIV)
    return nullptr;

  MachineBasicBlock *Preheader = L->getLoopPreheader();

  MachineOperand *InitialValue = nullptr;
  MachineInstr *IV_Phi = getVRegOriginDef(IVReg);
  for (unsigned i = 1, n = IV_Phi->getNumOperands(); i < n; i += 2) {
    MachineBasicBlock *MBB = IV_Phi->getOperand(i+1).getMBB();
    if (MBB == Preheader)
      InitialValue = &IV_Phi->getOperand(i);
	else if (MBB == Latch) {
		IVReg = IV_Phi->getOperand(i).getReg();  // Want IV reg after bump.
		//!! Skip the COPY Register to find the bumpReg in ADDiu instruction.
		IVReg = getVRegOriginDef(IVReg)->getOperand(0).getReg();
		if (ShowDbgInfo) 
			std::cout << "UpdateReg:" << (IVReg - VREGBASEADDR) << std::endl;
	}
  }
  if (!InitialValue)
    return nullptr;

  SmallVector<MachineOperand,2> Cond;
  MachineBasicBlock *TB = nullptr, *FB = nullptr;
  bool NotAnalyzed = TII->AnalyzeBranch(*Latch, TB, FB, Cond, false);
  if (NotAnalyzed)
    return nullptr;

  MachineBasicBlock *Header = L->getHeader();
  // TB must be non-null.  If FB is also non-null, one of them must be
  // the header.  Otherwise, branch to TB could be exiting the loop, and
  // the fall through can go to the header.
  assert (TB && "Latch block without a branch?");
  assert ((!FB || TB == Header || FB == Header) && "Branches not to header?");
  if (!TB || (FB && TB != Header && FB != Header))
    return nullptr;

  // Branches of form "if (!P) ..." cause HexagonInstrInfo::AnalyzeBranch
  // to put imm(0), followed by P in the vector Cond.
  // If TB is not the header, it means that the "not-taken" path must lead
  // to the header.
  //However, DSP::AnalyzeBranch just put the opcode of JC or JNC into the 
  //vector Cond, whose form like <opc, predicateReg>. For control logic like:
  //gti Ind, EndV
  //jnc Header
  //jmp exit
  //It means the loop continues when "lei Ind, EndV", so
  //the cmp relationship "gti" must be negated for computing the trip count. 
  
  //bool Negated = (Cond.size() > 1) ^ (TB != Header);
  bool Negated = (Cond[0].getImm() == DSP::JNC) ^ (TB != Header);
  unsigned PredReg = Cond[Cond.size()-1].getReg();
  MachineInstr *CondI = getVRegOriginDef(PredReg);
  unsigned CondOpc = CondI->getOpcode();

  unsigned CmpReg1 = 0, CmpReg2 = 0;
  int Mask = 0, ImmValue = 0;
  bool AnalyzedCmp = TII->analyzeCompare(CondI, CmpReg1, CmpReg2,
                                         Mask, ImmValue);
  if (!AnalyzedCmp)
    return nullptr;

  // The comparison operator type determines how we compute the loop
  // trip count.
  OldInsts.push_back(CondI);
  OldInsts.push_back(IVOp);

  // Sadly, the following code gets information based on the position
  // of the operands in the compare instruction.  This has to be done
  // this way, because the comparisons check for a specific relationship
  // between the operands (e.g. is-less-than), rather than to find out
  // what relationship the operands are in (as on PPC).
  Comparison::Kind Cmp;
  bool isSwapped = false;
  const MachineOperand &Op1 = CondI->getOperand(1);
  const MachineOperand &Op2 = CondI->getOperand(2);
  const MachineOperand *EndValue = nullptr;

  if (Op1.isReg()) {
	  //!! skip COPY
    if (Op2.isImm() || 
		getVRegOriginDef(Op1.getReg())->getOperand(0).getReg() == IVReg)
      EndValue = &Op2;
    else {
      EndValue = &Op1;
      isSwapped = true;
    }
  }

  if (!EndValue)
    return nullptr;
  
  if (!Comparison::getIterationCmpType(CondOpc, Negated, Cmp))
	  return nullptr;

  if (isSwapped)
   Cmp = Comparison::getSwappedComparison(Cmp);

  if (InitialValue->isReg()) {
    unsigned R = InitialValue->getReg();
    MachineBasicBlock *DefBB = getVRegOriginDef(R)->getParent();
	if (ShowDbgInfo) {
		std::cout << "\n**getLoopTripCount**" << std::endl;
		std::cout << "Initial Value DefBB:" << DefBB->getFullName() << std::endl;
		std::cout << "Initial Value VReg:" 
			<<  (InitialValue->getReg()-VREGBASEADDR) << std::endl; 

	}
    if (!MDT->properlyDominates(DefBB, Header))
      return nullptr;
	//The instruction defining the initial value of induction variable.
    OldInsts.push_back(getVRegOriginDef(R));
  }
  if (EndValue->isReg()) {
    unsigned R = EndValue->getReg();
    MachineBasicBlock *DefBB = getVRegOriginDef(R)->getParent();
	if (ShowDbgInfo) {
		std::cout << "End Value DefBB:" << DefBB->getFullName() << std::endl;
		std::cout << "End Value VReg:" 
			<< (EndValue->getReg() - VREGBASEADDR) << std::endl;
	}
    if (!MDT->properlyDominates(DefBB, Header))
      return nullptr;
	OldInsts.push_back(getVRegOriginDef(R));
  }

  return computeCount(L, InitialValue, EndValue, IVReg, IVBump, Cmp);
}

/// \brief Helper function that returns the expression that represents the
/// number of times a loop iterates.  The function takes the operands that
/// represent the loop start value, loop end value, and induction value.
/// Based upon these operands, the function attempts to compute the trip count.
CountValue *DSPHardwareLoops::computeCount(MachineLoop *Loop,
                                               const MachineOperand *Start,
                                               const MachineOperand *End,
                                               unsigned IVReg,
                                               int64_t IVBump,
                                               Comparison::Kind Cmp) const {
  // Cannot handle comparison EQ, i.e. while (A == B).
  if (Cmp == Comparison::EQ)
    return nullptr;

  // Check if either the start or end values are an assignment of an immediate.
  // If so, use the immediate value rather than the register.
  if (Start->isReg()) {
    const MachineInstr *StartValInstr = getVRegOriginDef(Start->getReg());
	//MovGR is a pseudo instruction will be matched as Movigh and Movigl in PostRA Pass.
	//The immediate is in the second operand of MovGR.
    if (StartValInstr && StartValInstr->getOpcode() == DSP::MovGR)
      Start = &StartValInstr->getOperand(2);
  }
  if (End->isReg()) {
    const MachineInstr *EndValInstr = getVRegOriginDef(End->getReg());
    if (EndValInstr && EndValInstr->getOpcode() == DSP::MovGR)
      End = &EndValInstr->getOperand(2);
  }

  assert (Start->isReg() || Start->isImm());
  assert (End->isReg() || End->isImm());
  if (ShowDbgInfo) {
	  std::cout << "\n**computeCount**" << std::endl;
	  if(Start->isImm())
		  std::cout << "Initial Value Imm:" << (Start->getImm()) << std::endl;
	  if (End->isImm())
		  std::cout << "End Value Imm:" << (End->getImm()) << std::endl;
  }

  bool CmpLess =     Cmp & Comparison::L;
  bool CmpGreater =  Cmp & Comparison::G;
  bool CmpHasEqual = Cmp & Comparison::EQ;

  // Avoid certain wrap-arounds.  This doesn't detect all wrap-arounds.
  // If loop executes while iv is "less" with the iv value going down, then
  // the iv must wrap.
  if (CmpLess && IVBump < 0)
    return nullptr;
  // If loop executes while iv is "greater" with the iv value going up, then
  // the iv must wrap.
  if (CmpGreater && IVBump > 0)
    return nullptr;

  if (Start->isImm() && End->isImm()) {
    // Both, start and end are immediates. Figure out trip count in shortcut.
    int64_t StartV = Start->getImm();
    int64_t EndV = End->getImm();
    int64_t Dist = EndV - StartV;
    if (Dist == 0)
      return nullptr;

    bool Exact = (Dist % IVBump) == 0;

    if (Cmp == Comparison::NE) {
      if (!Exact)
        return nullptr;
      if ((Dist < 0) ^ (IVBump < 0))
        return nullptr;
    }

    // For comparisons that include the final value (i.e. include equality
    // with the final value), we need to increase the distance by 1.
    if (CmpHasEqual)
      Dist = Dist > 0 ? Dist+1 : Dist-1;

    // assert (CmpLess => Dist > 0);
    assert ((!CmpLess || Dist > 0) && "Loop should never iterate!");
    // assert (CmpGreater => Dist < 0);
    assert ((!CmpGreater || Dist < 0) && "Loop should never iterate!");

    // "Normalized" distance, i.e. with the bump set to +-1.
	// However, the count in DSP's Loop instruction is the decrement of Count,
	// that is Loop <Count-1, lastbundle>
    int64_t Dist1 = (IVBump > 0) ? (Dist +  (-1)) /   IVBump
                               :  (-Dist + (-1)) / (-IVBump);
    assert (Dist1 > 0 && "Fishy thing.  Both operands have the same sign.");

    uint64_t Count = Dist1;

    if (Count > 0xFFFFFFFFULL)
      return nullptr;

    return new CountValue(CountValue::CV_Immediate, Count);
  }//endif both imm

  // A general case: Start and End are some values, but the actual
  // iteration count may not be available.  If it is not, insert
  // a computation of it into the preheader.

  // If the induction variable bump is not a power of 2, quit.
  // Othwerise we'd need a general integer division.
  if (!isPowerOf2_64(abs64(IVBump)))
    return nullptr;

  MachineBasicBlock *PH = Loop->getLoopPreheader();
  assert (PH && "Should have a preheader by now");
  MachineBasicBlock::iterator InsertPos = PH->getFirstTerminator();
  DebugLoc DL = (InsertPos != PH->end()) ? InsertPos->getDebugLoc()
                                         : DebugLoc();

  // If Start is a register and End is an immediate, the trip count
  // will be "imm - reg".  DSP has no Subir instruction. So we move
  // immediate to a new virtual register, then use Subrr.

  // If the loop IV is going downwards, i.e. if the bump is negative,
  // then the iteration count (computed as End-Start) will need to be
  // negated.  To avoid the negation, just swap Start and End.
  if (IVBump < 0) {
    std::swap(Start, End);
    IVBump = -IVBump;
  }
  // Cmp may now have a wrong direction, e.g.  LEs may now be GEs.
  // Signedness, and "including equality" are preserved.

  bool RegToImm = Start->isReg() && End->isImm(); // for (reg..imm)
  bool RegToReg = Start->isReg() && End->isReg(); // for (reg..reg)

  int64_t StartV = 0, EndV = 0;
  if (Start->isImm())
    StartV = Start->getImm();
  if (End->isImm())
    EndV = End->getImm();

  int64_t AdjV = 0;
  // To compute the iteration count, we would need this computation:
  //   Count = (End - Start + (IVBump-1)) / IVBump
  // or, when CmpHasEqual:
  //   Count = (End - Start + (IVBump-1)+1) / IVBump
  // The "IVBump-1" part is the adjustment (AdjV).  We can avoid
  // generating an instruction specifically to add it if we can adjust
  // the immediate values for Start or End.
  // However, the count in DSP's Loop instruction is the decrement of Count,
  // that is Loop <Count-1, lastbundle>. The actual computation expression is
  //   Count = (End - Start + (-1)) / IVBump
  // or, when CmpHasEqual:
  //   Count = (End - Start + 1 + (-1)) / IVBump
  //Hence, it is more simple to add the adjustment (AdjV). (^ O ^)

  if (CmpHasEqual) {
    // Need to add 1 to the total iteration count.
    if (Start->isImm())
      StartV--;
    else if (End->isImm())
      EndV++;
    else
      AdjV += 1;
  }
  //the NE must ensure the IVBump exact divide Dist, no need for adjustment.
  if (Cmp != Comparison::NE) {
    if (Start->isImm())
      StartV++;
    else if (End->isImm())
      EndV--;
    else
      AdjV--;
  }

  unsigned R = 0, SR = 0;
  if (Start->isReg()) {
	  R = Start->getReg();
	  SR = Start->getSubReg();
  }
  else {
	  R = End->getReg();
	  SR = End->getSubReg();
  }

  const TargetRegisterClass *IntRC = &DSP::CPURegsRegClass;

  // Compute DistR (register with the distance between Start and End).
  unsigned DistR, DistSR;

  // Avoid special case, where the start value is an imm(0).
  if (Start->isImm() && StartV == 0) {
	  DistR = End->getReg();
	  DistSR = End->getSubReg();
  } else {
    const MCInstrDesc &SubD = RegToReg ? TII->get(DSP::SUBu) :
                              (RegToImm ? TII->get(DSP::SUBu) :
                                          TII->get(DSP::ADDiu));
	//No SUBir instruction exists in DSP. Inserting a movGR instruction to
	//move the EndV immediate to GR, then using SUBrr.
	unsigned EndR = MRI->createVirtualRegister(IntRC);
	if (RegToImm) {
		//MovGR is a pseudo instruction which will be matched as Movigh and
		//Movigl later in the PostRA Pass.
		const MCInstrDesc &MoviRD = TII->get(DSP::MovGR);
		MachineInstrBuilder MoviRI =
			BuildMI(*PH, InsertPos, DL, MoviRD, EndR)
			.addReg(DSP::ZERO)// ?? how to set no-use reg number
			.addImm(EndV);
	}

    unsigned SubR = MRI->createVirtualRegister(IntRC);
    MachineInstrBuilder SubIB =
      BuildMI(*PH, InsertPos, DL, SubD, SubR);

    if (RegToReg) {
      SubIB.addReg(End->getReg(), 0, End->getSubReg())
           .addReg(Start->getReg(), 0, Start->getSubReg());
    } else if (RegToImm) {
		SubIB.addReg(EndR)
           .addReg(Start->getReg(), 0, Start->getSubReg());
    } else { // ImmToReg
      SubIB.addReg(End->getReg(), 0, End->getSubReg())
           .addImm(-StartV);//11-bit imm for addi instruction, will it overflow ??
    }
    DistR = SubR;
    DistSR = 0;
  }

  // From DistR, compute AdjR (register with the adjusted distance).
  unsigned AdjR, AdjSR;

  if (AdjV == 0) {
    AdjR = DistR;
    AdjSR = DistSR;
  } else {
    // Generate CountR = ADD DistR, AdjVal
    unsigned AddR = MRI->createVirtualRegister(IntRC);
    const MCInstrDesc &AddD = TII->get(DSP::ADDiu);
    BuildMI(*PH, InsertPos, DL, AddD, AddR)
      .addReg(DistR, 0, DistSR)
      .addImm(AdjV);

    AdjR = AddR;
    AdjSR = 0;
  }

  // From AdjR, compute CountR (register with the final count).
  unsigned CountR, CountSR;

  if (IVBump == 1) {
    CountR = AdjR;
    CountSR = AdjSR;
  } else {
    // The IV bump is a power of two. Log_2(IV bump) is the shift amount.
    unsigned Shift = Log2_32(IVBump);
	unsigned ShiftR = MRI->createVirtualRegister(IntRC);
	const MCInstrDesc &MoviRD = TII->get(DSP::MovGR);
		BuildMI(*PH, InsertPos, DL, MoviRD, ShiftR)
		.addReg(DSP::ZERO)// ?? how to set no-use reg number
		.addImm(Shift);

    // Generate NormR = LSR DistR, Shift.
    unsigned LsrR = MRI->createVirtualRegister(IntRC);
    const MCInstrDesc &LsrD = TII->get(DSP::SRL);
    BuildMI(*PH, InsertPos, DL, LsrD, LsrR)
      .addReg(AdjR, 0, AdjSR)
      .addReg(ShiftR);

    CountR = LsrR;
    CountSR = 0;
  }

  return new CountValue(CountValue::CV_Register, CountR, CountSR);
}


/// \brief Return true if the operation is invalid within hardware loop.
bool DSPHardwareLoops::isInvalidLoopOperation(
      const MachineInstr *MI) const {

  // call is not allowed because the callee may use a hardware loop
  if (MI->getDesc().isCall())
    return true;

  // do not allow nested hardware loops
  if (MI->getOpcode() == DSP::Loop)
    return true;

  return false;
}


/// \brief - Return true if the loop contains an instruction that inhibits
/// the use of the hardware loop function.
bool DSPHardwareLoops::containsInvalidInstruction(MachineLoop *L) const {
  const std::vector<MachineBasicBlock *> &Blocks = L->getBlocks();
  for (unsigned i = 0, e = Blocks.size(); i != e; ++i) {
    MachineBasicBlock *MBB = Blocks[i];
    for (MachineBasicBlock::iterator
           MII = MBB->begin(), E = MBB->end(); MII != E; ++MII) {
      const MachineInstr *MI = &*MII;
      if (isInvalidLoopOperation(MI))
        return true;
    }
  }
  return false;
}


/// \brief Returns true if the instruction is dead.  This was essentially
/// copied from DeadMachineInstructionElim::isDead, but with special cases
/// for inline asm, physical registers and instructions with side effects
/// removed.
bool DSPHardwareLoops::isDead(const MachineInstr *MI,
                              SmallVectorImpl<MachineInstr *> &DeadPhis) const {
  // Examine each operand.
  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);
    if (!MO.isReg() || !MO.isDef())
      continue;

    unsigned Reg = MO.getReg();
    if (MRI->use_nodbg_empty(Reg))
      continue;

    typedef MachineRegisterInfo::use_nodbg_iterator use_nodbg_iterator;

    // This instruction has users, but if the only user is the phi node for the
    // parent block, and the only use of that phi node is this instruction, then
    // this instruction is dead: both it (and the phi node) can be removed.
    use_nodbg_iterator I = MRI->use_nodbg_begin(Reg);
    use_nodbg_iterator End = MRI->use_nodbg_end();

    //!! Skip COPY processing here, going back referring to Hexagon
	if (std::next(I) != End 
		|| (!I->getParent()->isPHI()
		&& (I->getParent()->getOpcode() != DSP::COPY)))
      return false;

	//The Only user is Phi
	if (I->getParent()->isPHI()) {
		MachineInstr *OnePhi = I->getParent();
		for (unsigned j = 0, f = OnePhi->getNumOperands(); j != f; ++j) {
			const MachineOperand &OPO = OnePhi->getOperand(j);
			if (!OPO.isReg() || !OPO.isDef())
				continue;

			unsigned OPReg = OPO.getReg();
			use_nodbg_iterator nextJ;
			for (use_nodbg_iterator J = MRI->use_nodbg_begin(OPReg);
				J != End; J = nextJ) {
				nextJ = std::next(J);
				MachineOperand &Use = *J;
				MachineInstr *UseMI = Use.getParent();

				// If the phi node has a user that is not MI, bail...
				if (MI != UseMI)
					return false;
			}
		}
		DeadPhis.push_back(OnePhi);
	}
	//The only user is COPY. 
	//I->getParent()->getOpcode() == DSP::COPY 
	else {
		//Check if the COPY instruction is dead. 
		MachineInstr *OneCOPY = I->getParent();
		unsigned COPYReg = OneCOPY->getOperand(0).getReg();
		if (MRI->use_nodbg_empty(COPYReg)) {
			//if this Instruction is dead, COPY is too.
			DeadPhis.push_back(OneCOPY);
			continue;
		}
		//Analyze the uses of COPYReg
		use_nodbg_iterator CI = MRI->use_nodbg_begin(COPYReg);
		use_nodbg_iterator CEnd = MRI->use_nodbg_end();
		if (std::next(CI) != End || !CI->getParent()->isPHI())
			return false;
		//Only phi uses the COPYReg 
		//If it is "Instr->COPY->Phi->Instr...", removing these three instructions.
			MachineInstr *OnePhi = CI->getParent();
			for (unsigned j = 0, f = OnePhi->getNumOperands(); j != f; ++j) {
				const MachineOperand &OPO = OnePhi->getOperand(j);
				if (!OPO.isReg() || !OPO.isDef())
					continue;

				unsigned OPReg = OPO.getReg();
				use_nodbg_iterator nextJ;
				for (use_nodbg_iterator J = MRI->use_nodbg_begin(OPReg);
					J != End; J = nextJ) {
					nextJ = std::next(J);
					MachineOperand &Use = *J;
					MachineInstr *UseMI = Use.getParent();

					// If the phi node has a user that is not MI, bail...
					if (MI != UseMI)
						return false;
				}
			}
			//If this Instruction is confirmed as dead at last, erase them all
			//Otherwise, retaining them all.
			DeadPhis.push_back(OneCOPY);
			DeadPhis.push_back(OnePhi);
	}
  }

  // If there are no defs with uses, the instruction is dead.
  return true;
}

void DSPHardwareLoops::removeIfDead(MachineInstr *MI) {
  // This procedure was essentially copied from DeadMachineInstructionElim.

  SmallVector<MachineInstr*, 2> DeadPhis;
  if (isDead(MI, DeadPhis)) {
    DEBUG(dbgs() << "HW looping will remove: " << *MI);

    // It is possible that some DBG_VALUE instructions refer to this
    // instruction.  Examine each def operand for such references;
    // if found, mark the DBG_VALUE as undef (but don't delete it).
    for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
      const MachineOperand &MO = MI->getOperand(i);
      if (!MO.isReg() || !MO.isDef())
        continue;
      unsigned Reg = MO.getReg();
      MachineRegisterInfo::use_iterator nextI;
      for (MachineRegisterInfo::use_iterator I = MRI->use_begin(Reg),
           E = MRI->use_end(); I != E; I = nextI) {
        nextI = std::next(I);  // I is invalidated by the setReg
        MachineOperand &Use = *I;
        MachineInstr *UseMI = I->getParent();
        if (UseMI == MI)
          continue;
        if (Use.isDebug())
          UseMI->getOperand(0).setReg(0U);
        // This may also be a "instr -> phi -> instr" case which can
        // be removed too.
      }
    }

    MI->eraseFromParent();
    for (unsigned i = 0; i < DeadPhis.size(); ++i)
      DeadPhis[i]->eraseFromParent();
  }
}

/// \brief Check if the loop is a candidate for converting to a hardware
/// loop.  If so, then perform the transformation.
///
/// This function works on innermost loops first.  A loop can be converted
/// if it is a counting loop; either a register value or an immediate.
///
/// The code makes several assumptions about the representation of the loop
/// in llvm.
bool DSPHardwareLoops::convertToHardwareLoop(MachineLoop *L) {
  // This is just for sanity.
  assert(L->getHeader() && "Loop without a header?");

  bool Changed = false;
  // Process nested loops first.
  for (MachineLoop::iterator I = L->begin(), E = L->end(); I != E; ++I)
    Changed |= convertToHardwareLoop(*I);

  // If a nested loop has been converted, then we can't convert this loop.
  if (Changed)
    return Changed;

#ifndef NDEBUG
  // Stop trying after reaching the limit (if any).
  int Limit = HWLoopLimit;
  if (Limit >= 0) {
    if (Counter >= HWLoopLimit)
      return false;
    Counter++;
  }
#endif

  // Does the loop contain any invalid instructions?
  if (containsInvalidInstruction(L))
    return false;

  // Is the induction variable bump feeding the latch condition?
  if (!fixupInductionVariable(L))
    return false;

  //!! split the latch block
  // It is not valid to replace the loop header with this method.
  //if (!splitLatchBlock(L))
	 // return false;

  MachineBasicBlock *LastMBB = getExitingBlock(L);
  // Don't generate hw loop if the loop has more than one exit.
  if (!LastMBB)
    return false;

  if (ShowDbgInfo) {
	  std::cout << "\n**convertToHardwareLoop**" << std::endl;
  }
  MachineBasicBlock::iterator LastI = LastMBB->getFirstTerminator();
  if (LastI == LastMBB->end())
    return false;

  // Ensure the loop has a preheader: the loop instruction will be
  // placed there.
  bool NewPreheader = false;
  MachineBasicBlock *Preheader = L->getLoopPreheader();
  if (!Preheader) {
    Preheader = createPreheaderForLoop(L);
    if (!Preheader)
      return false;
    NewPreheader = true;
  }

  if (ShowDbgInfo) {
	  if (Preheader)
		  std::cout << "Preheader:" << Preheader->getFullName() << std::endl;
  }

  //!! Also Tail Merging adjacent Preheaher and Header irrelative of unconditional
  //jump from preheader to header.
  //if (!NewPreheader) {
	 // if (Preheader->getFirstTerminator() == Preheader->end()) {
		//  SmallVector<MachineOperand, 1> EmptyCond;
		//  DebugLoc DL;
		//  MachineBasicBlock *Header = L->getHeader();
		//  TII->InsertBranch(*Preheader, Header, nullptr, EmptyCond, DL);
	 // }
  //}

  //?? DSP Loop End determination may refer to this	from Hexagon determining loop start ??//
  // Determine the loop end block.
  MachineBasicBlock *LoopEnd = LastMBB;
  if (L->getLoopLatch() != LastMBB) {
	  return false;
	  /*
	  // When the exit and latch are not the same, use the latch block as the
	  // end block.
	  // The loop end address is used only after the 1st iteration, and the
	  // loop latch may contains instrs. that need to be executed after the
	  // first iteration.
	  LoopEnd = L->getLoopLatch();
	  LastMBB = LoopEnd;
	  LastI = LoopEnd->getFirstTerminator();
	  // Make sure the latch is a successor of the exit, otherwise it won't work.
	  if (!LastMBB->isSuccessor(LoopEnd))
	  return false;
	  */
  }

  MachineBasicBlock::iterator InsertPos = Preheader->getFirstTerminator();

  ////brief Insert endloop prior to the instruction before terminator,
  ////but skip the COPY, bump, compare instructions being removed.
  ////LastBlock has only terminate instruction, the loop end position 
  ////(the last instruction in this loop body) isn't inside this MBB.
  //if (LastI == LastMBB->begin())
	 // return false;

  //unsigned IVReg = 0;
  //int64_t IVBump = 0;
  //MachineInstr *IVOp;
  //bool FoundIV = findInductionRegister(L, IVReg, IVBump, IVOp);
  //if (!FoundIV)
	 // return nullptr;
  //MachineBasicBlock::iterator EndIntrPos = std::prev(LastI);
  ////!! skip COPY
  //while((EndIntrPos->getOpcode() == DSP::COPY || EndIntrPos->isCompare() || &*EndIntrPos == IVOp)
	 // && EndIntrPos != LastMBB->begin())
	 // EndIntrPos = std::prev(EndIntrPos);
  ////Can't find the non-COPY and non-compare end instruction in LastMBB
  //if (EndIntrPos->getOpcode() == DSP::COPY || EndIntrPos->isCompare() || &*EndIntrPos == IVOp)
	 // return false;

  SmallVector<MachineInstr*, 2> OldInsts;
  // Are we able to determine the trip count for the loop?
  CountValue *TripCount = getLoopTripCount(L, OldInsts);
  if (!TripCount)
    return false;
  
  // Is the trip count available in the preheader?
  if (TripCount->isReg()) {
    // There will be a use of the register inserted into the preheader,
    // so make sure that the register is actually defined at that point.
    MachineInstr *TCDef = getVRegOriginDef(TripCount->getReg());
    MachineBasicBlock *BBDef = TCDef->getParent();
    if (!NewPreheader) {
      if (!MDT->dominates(BBDef, Preheader))
        return false;
    } else {
      // If we have just created a preheader, the dominator tree won't be
      // aware of it.  Check if the definition of the register dominates
      // the header, but is not the header itself.
      if (!MDT->properlyDominates(BBDef, L->getHeader()))
        return false;
    }
  }

  // Convert the loop to a hardware loop.
  DEBUG(dbgs() << "Change to hardware loop at "; L->dump());
  DebugLoc DL;
  if (InsertPos != Preheader->end())
    DL = InsertPos->getDebugLoc();

  if (TripCount->isReg()) {
    // Create a copy of the loop count register.
    unsigned CountReg = MRI->createVirtualRegister(&DSP::CPURegsRegClass);
    BuildMI(*Preheader, InsertPos, DL, TII->get(TargetOpcode::COPY), CountReg)
      .addReg(TripCount->getReg(), 0, TripCount->getSubReg());
    // Add the Loop instruction to the beginning of the loop.
    BuildMI(*Preheader, InsertPos, DL, TII->get(DSP::Loop))
      .addReg(CountReg)
	  .addMBB(LoopEnd);
  }
  else {
	  assert(TripCount->isImm() && "Expecting immediate value for trip count");
	  // No Loop immediate intruction for trip count operand, we need to
	  // create a new virtual register.
	  int64_t CountImm = TripCount->getImm();
	  unsigned CountReg = MRI->createVirtualRegister(&DSP::CPURegsRegClass);
	  BuildMI(*Preheader, InsertPos, DL, TII->get(DSP::MovGR), CountReg)
		  .addReg(DSP::ZERO).addImm(CountImm);
	  BuildMI(*Preheader, InsertPos, DL, TII->get(DSP::Loop))
		  .addReg(CountReg).addMBB(LoopEnd);
  }


  // Make sure the loop end always has a reference in the CFG.  We need
  // to create a BlockAddress operand to get this mechanism to work both the
  // MachineBasicBlock and BasicBlock objects need the flag set.
  LoopEnd->setHasAddressTaken();
  // This line is needed to set the hasAddressTaken flag on the BasicBlock
  // object.
  BlockAddress::get(const_cast<BasicBlock *>(LoopEnd->getBasicBlock()));
  

  // Replace the loop branch with a special sentinel instruction for fixing up
  //end address operand in Loop instruction on later Pass.

  DebugLoc LastIDL = LastI->getDebugLoc();
  //DebugLoc EndIDL = EndIntrPos->getDebugLoc();
  //BuildMI(*LoopEnd, EndIntrPos, EndIDL,
  //        TII->get(DSP::ENDLOOP));
  //MIBundleBuilder(*LoopEnd, EndIntrPos, std::next(EndIntrPos, 1));
  BuildMI(*LoopEnd, LastI, LastIDL,
          TII->get(DSP::ENDLOOP));

  // The loop ends with either:
  //  - a conditional branch followed by an unconditional branch, or
  //  - a conditional branch to the loop start.
  if (LastI->getOpcode() == DSP::JC ||
      LastI->getOpcode() == DSP::JNC) {
    // Delete one and change/add an uncond. branch to out of the loop.
    MachineBasicBlock *BranchTarget = LastI->getOperand(1).getMBB();
    LastI = LastMBB->erase(LastI);
    if (!L->contains(BranchTarget)) {
      if (LastI != LastMBB->end())
        LastI = LastMBB->erase(LastI);
      SmallVector<MachineOperand, 0> Cond;
      TII->InsertBranch(*LastMBB, BranchTarget, nullptr, Cond, LastIDL);
    }
  } else {
    // Conditional branch to loop start; just delete it.
    LastMBB->erase(LastI);
  }
  DEBUG({ dbgs() << "Trip count: \t";  TripCount->print(dbgs()); dbgs() << "\n"; });
  delete TripCount;

  // The induction operation and the comparison may now be
  // unneeded. If these are unneeded, then remove them.
  for (unsigned i = 0; i < OldInsts.size(); ++i)
    removeIfDead(OldInsts[i]);

  ++NumHWLoops;
  if (ShowDbgInfo)
	  std::cout << "<**Complete this HardwareLoop conversion**>"
	  << std::endl;

  return true;
}


bool DSPHardwareLoops::orderBumpCompare(MachineInstr *BumpI,
                                            MachineInstr *CmpI) {
  assert (BumpI != CmpI && "Bump and compare in the same instruction?");

  MachineBasicBlock *BB = BumpI->getParent();
  if (CmpI->getParent() != BB)
    return false;

  typedef MachineBasicBlock::instr_iterator instr_iterator;
  // Check if things are in order to begin with.
  for (instr_iterator I = BumpI, E = BB->instr_end(); I != E; ++I)
    if (&*I == CmpI)
      return true;

  // Out of order.
  unsigned PredR = CmpI->getOperand(0).getReg();
  bool FoundBump = false;
  instr_iterator CmpIt = CmpI, NextIt = std::next(CmpIt);
  for (instr_iterator I = NextIt, E = BB->instr_end(); I != E; ++I) {
    MachineInstr *In = &*I;
    for (unsigned i = 0, n = In->getNumOperands(); i < n; ++i) {
      MachineOperand &MO = In->getOperand(i);
      if (MO.isReg() && MO.isUse()) {
        if (MO.getReg() == PredR)  // Found an intervening use of PredR.
          return false;
      }
    }

    if (In == BumpI) {
      instr_iterator After = BumpI;
      instr_iterator From = CmpI;
      BB->splice(std::next(After), BB, From);
      FoundBump = true;
      break;
    }
  }
  assert (FoundBump && "Cannot determine instruction order");
  return FoundBump;
}


MachineInstr *DSPHardwareLoops::defWithImmediate(unsigned R) {
  MachineInstr *DI = getVRegOriginDef(R);
  unsigned DOpc = DI->getOpcode();
  switch (DOpc) {
    case DSP::MovGR:
      return DI;
  }
  return nullptr;
}


int64_t DSPHardwareLoops::getImmediate(MachineOperand &MO) {
  if (MO.isImm())
    return MO.getImm();
  assert(MO.isReg());
  unsigned R = MO.getReg();
  MachineInstr *DI = defWithImmediate(R);
  assert(DI && "Need an immediate operand");
  // All currently supported "define-with-immediate" instructions 
  // (MOvGR GR, ZERO, imm) have the actual immediate value in the operand(2).
  int64_t v = DI->getOperand(2).getImm();
  return v;
}


void DSPHardwareLoops::setImmediate(MachineOperand &MO, int64_t Val) {
  if (MO.isImm()) {
    MO.setImm(Val);
    return;
  }

  assert(MO.isReg());
  unsigned R = MO.getReg();
  MachineInstr *DI = defWithImmediate(R);
  if (MRI->hasOneNonDBGUse(R)) {
    // If R has only one use, then just change its defining instruction to
    // the new immediate value.
    DI->getOperand(2).setImm(Val);
    return;
  }

  const TargetRegisterClass *RC = MRI->getRegClass(R);
  unsigned NewR = MRI->createVirtualRegister(RC);
  MachineBasicBlock &B = *DI->getParent();
  DebugLoc DL = DI->getDebugLoc();
  BuildMI(B, DI, DL, TII->get(DI->getOpcode()), NewR)
	  .addReg(DSP::ZERO)
	  .addImm(Val);
  MO.setReg(NewR);
}


bool DSPHardwareLoops::fixupInductionVariable(MachineLoop *L) {
  MachineBasicBlock *Header = L->getHeader();
  MachineBasicBlock *Preheader = L->getLoopPreheader();
  MachineBasicBlock *Latch = L->getLoopLatch();
  MachineBasicBlock *ExitingBlock = getExitingBlock(L);

  if (ShowDbgInfo) {
		std::cout << "\nStart HardwareLoop conversion:\n"
			"**fixupInductionVariable**" << std::endl;
	  if (Header)
		  std::cout << "Header:" << Header->getFullName() << std::endl;
	  if (ExitingBlock)
		  std::cout << "ExitingBlock:" << ExitingBlock->getFullName() << std::endl;
	  if (Latch)
		  std::cout << "Latch:" << Latch->getFullName() << std::endl;
  }

  if (!Header || !ExitingBlock || !Latch)
    return false;

  //DSP HardwareLoop instruction loop(GR, endaddr) has no start address of loop
  //It can't process this special case. Otherwise, shall we insert endloop in
  //ExitingBlock or Latch?
  if (ExitingBlock != Latch) {
	  if (ShowDbgInfo)
		  std::cout << "Latch is not one of ExitingBlocks, Can't convert!" 
		  << std::endl;
	  DEBUG(dbgs() << "<**Latch is not one of ExitingBlocks, Can't convert!**>\n");
	  return false;
  }

  // These data structures follow the same concept as the corresponding
  // ones in findInductionRegister (where some comments are).
  typedef std::pair<unsigned,int64_t> RegisterBump;
  typedef std::pair<unsigned,RegisterBump> RegisterInduction;
  typedef std::set<RegisterInduction> RegisterInductionSet;

  // Register candidates for induction variables, with their associated bumps.
  RegisterInductionSet IndRegs;

  // Look for induction patterns:
  //header:
  //   induction = PHI ..., [ latch, update ]
  //   (predicate = cmp induction, bound)
  //   update = ADDiu induction, imm
  //   (predicate = cmp update, bound)
  //   br predicate header, ..., MBBn
  if (ShowDbgInfo)
	  std::cout << "--find Phi operrand derive from Latch--"  << std::endl;
  typedef MachineBasicBlock::instr_iterator instr_iterator;
  for (instr_iterator I = Header->instr_begin(), E = Header->instr_end();
       I != E && I->isPHI(); ++I) {
    MachineInstr *Phi = &*I;

    // Have a PHI instruction.
    for (unsigned i = 1, n = Phi->getNumOperands(); i < n; i += 2) {
		if (ShowDbgInfo)
			std::cout << "MBB operrand:\t" 
			<< (Phi->getOperand(i + 1).getMBB())->getFullName() << std::endl;
      if (Phi->getOperand(i+1).getMBB() != Latch)
        continue;

      unsigned PhiReg = Phi->getOperand(i).getReg();
	  if (ShowDbgInfo)
		  std::cout << "PhiReg from Latch:\t" 
		  << (PhiReg-VREGBASEADDR) << std::endl;
      MachineInstr *DI = getVRegOriginDef(PhiReg);
      unsigned UpdOpc = DI->getOpcode();
      bool isAdd = (UpdOpc == DSP::ADDiu);

      if (isAdd) {
        // If the register operand to the add/sub is the PHI we are looking
        // at, this meets the induction pattern:
		//   induction = PHI ..., [ latch, update ]
		//   update = ADDiu induction, imm
        unsigned IndReg = DI->getOperand(1).getReg();
        if (getVRegOriginDef(IndReg) == Phi) {
          unsigned UpdReg = DI->getOperand(0).getReg();
          int64_t V = DI->getOperand(2).getImm();
          IndRegs.insert(std::make_pair(UpdReg, std::make_pair(IndReg, V)));
        }
      }
    }  // for (i)
  }  // for (instr)

  //Just one vector <update, induction, bump> among the IndRegs Set determine
  //the loop iteration, which is used by the comparison instruction to generate
  //the predicate value.
  if (IndRegs.empty())
    return false;

  MachineBasicBlock *TB = nullptr, *FB = nullptr;
  SmallVector<MachineOperand,2> Cond;
  // AnalyzeBranch returns true if it fails to analyze branch.
  bool NotAnalyzed = TII->AnalyzeBranch(*ExitingBlock, TB, FB, Cond, false);
  if (NotAnalyzed || Cond.empty())
	  return false;

  if (ShowDbgInfo) {
	  std::cout << "--AnalyzeBranch Exiting--" << std::endl;
	  if (Latch)
		  std::cout << "Exiting:" << Latch->getFullName() << std::endl;
	  if (TB)
		  std::cout << "TB:" << TB->getFullName() << std::endl;
	  if (FB)
		  std::cout << "FB:" << FB->getFullName() << std::endl;
	  if (!Cond.empty()) {
		  std::cout << "Opcode:" << Cond[0].getImm() << std::endl;
		  std::cout << "Predicate Reg:" 
			  << (Cond[1].getReg() - VREGBASEADDR) << std::endl;
	  }
  }

  // Now DSP only support the case that Latch is one of ExitingBlocks,
  // Otherwise, this function and conversion will return false before.
  // If extended work would be done in the future, this analysis below
  // would be useful.
  if (!L->isLoopExiting(Latch)){
	  // AnalyzeBranch returns true if it fails to analyze branch.
	  bool NotAnalyzedlatch = TII->AnalyzeBranch(*Latch, TB, FB, Cond, false);
	  if (NotAnalyzedlatch)
		  return false;

	  if (ShowDbgInfo) {
		  std::cout << "--AnalyzeBranch Latch--" << std::endl;
		  if (Latch)
			  std::cout << "Latch:" << Latch->getFullName() << std::endl;
		  if (TB)
			  std::cout << "TB:" << TB->getFullName() << std::endl;
		  if (FB)
			  std::cout << "FB:" << FB->getFullName() << std::endl;
		  if (!Cond.empty()) {
			  std::cout << "Opcode:" << Cond[0].getImm() << std::endl;
			  std::cout << "Predicate Reg:" 
				  << (Cond[1].getReg() - VREGBASEADDR) << std::endl;
		  }
	  }
  }
  else {
	  if (ShowDbgInfo)
		  std::cout << "--Latch is Exiting Block <Normal case>--" << std::endl;
  }

  // Check if the latch branch is unconditional.
  if (Cond.empty())
    return false;

  if (TB != Header && FB != Header)
    // The latch does not go back to the header.  Not a latch we know and love.
    return false;

  // Expecting a predicate register as a condition.  It won't be a hardware
  // predicate register at this point yet, just a vreg.
  // DSPInstrInfo::AnalyzeBranch for conditional branches inserts opcode
  // into Cond, followed by the predicate register.  For negated branches
  // we can test the opcode in Cond[0].
  unsigned CSz = Cond.size();
  if (CSz != 2)
    return false;

  unsigned P = Cond[CSz-1].getReg();
  MachineInstr *PredDef = getVRegOriginDef(P);

  if (!PredDef->isCompare())
    return false;

  SmallSet<unsigned,2> CmpRegs;
  MachineOperand *CmpImmOp = nullptr;

  // Go over all operands to the compare and look for immediate and register
  // operands.  Assume that if the compare has a single register use and a
  // single immediate operand, then the register is being compared with the
  // immediate value.
  for (unsigned i = 0, n = PredDef->getNumOperands(); i < n; ++i) {
    MachineOperand &MO = PredDef->getOperand(i);
    if (MO.isReg()) {
      // Skip all implicit references.  In one case there was:
      //   %vreg140<def> = FCMPUGT32_rr %vreg138, %vreg139, %USR<imp-use>
      if (MO.isImplicit())
        continue;
      if (MO.isUse()) {
        unsigned R = MO.getReg();
        if (!defWithImmediate(R)) {
		//Maybe after COPY ,skip COPY!!
          CmpRegs.insert(getVRegOriginDef(MO.getReg())->getOperand(0).getReg());
          continue;
        }
        // Consider the register to be the "immediate" operand.
        if (CmpImmOp)
          return false;
        CmpImmOp = &MO;
      }
    } else if (MO.isImm()) {
      if (CmpImmOp)    // A second immediate argument?  Confusing.  Bail out.
        return false;
      CmpImmOp = &MO;
    }
  }

  if (CmpRegs.empty())
    return false;

  // Check if the compared register follows the order we want.  Fix if needed.
  for (RegisterInductionSet::iterator I = IndRegs.begin(), E = IndRegs.end();
       I != E; ++I) {
    // This is a success.  If the register used in the comparison is one that
    // we have identified as a bumped (updated) induction register, there is
    // nothing to do.
    if (CmpRegs.count(I->first))
      return true;

    // Otherwise, if the register being compared comes out of a PHI node,
    // and has been recognized as following the induction pattern, and is
    // compared against an immediate, we can fix it.
    const RegisterBump &RB = I->second;
	//That is the update vector determining the predicate.
    if (CmpRegs.count(RB.first)) {
	//The comparison bound is not defined with a constant.
      if (!CmpImmOp)
        return false;

      int64_t CmpImm = getImmediate(*CmpImmOp);
      int64_t V = RB.second;
      if (V > 0 && CmpImm+V < CmpImm)  // Overflow (64-bit).
        return false;
      if (V < 0 && CmpImm+V > CmpImm)  // Overflow (64-bit).
        return false;
	  //bump before comparison, adjusting the comparison bound
      CmpImm += V;
      // Some forms of cmp-immediate allow u9 and s10.  Assume the worst case
      // scenario, i.e. an 8-bit value.
	  //9-bit signed immediate for DSP comparison instructions.
      if (CmpImmOp->isImm() && !isInt<8>(CmpImm))
        return false;

      // Make sure that the compare happens after the bump.  Otherwise,
      // after the fixup, the compare would use a yet-undefined register.
      MachineInstr *BumpI = getVRegOriginDef(I->first);
      bool Order = orderBumpCompare(BumpI, PredDef);
      if (!Order)
        return false;

      // Finally, fix the compare instruction.
      setImmediate(*CmpImmOp, CmpImm);
      for (unsigned i = 0, n = PredDef->getNumOperands(); i < n; ++i) {
        MachineOperand &MO = PredDef->getOperand(i);
        if (MO.isReg() && MO.getReg() == RB.first) {
          MO.setReg(I->first);
          return true;
        }
      }
    }
  }

  if (ShowDbgInfo)
	  std::cout << "Comparison in latch doesn't use induction register."
	  "<**Fail to convert**>" << std::endl;

  return false;
}


/// \brief Create a preheader for a given loop.
MachineBasicBlock *DSPHardwareLoops::createPreheaderForLoop(
      MachineLoop *L) {
  if (MachineBasicBlock *TmpPH = L->getLoopPreheader())
    return TmpPH;

  MachineBasicBlock *Header = L->getHeader();
  MachineBasicBlock *Latch = L->getLoopLatch();
  MachineFunction *MF = Header->getParent();
  DebugLoc DL;

  if (!Latch || Header->hasAddressTaken())
    return nullptr;

  typedef MachineBasicBlock::instr_iterator instr_iterator;

  // Verify that all existing predecessors have analyzable branches
  // (or no branches at all).
  typedef std::vector<MachineBasicBlock*> MBBVector;
  MBBVector Preds(Header->pred_begin(), Header->pred_end());
  SmallVector<MachineOperand,2> Tmp1;
  MachineBasicBlock *TB = nullptr, *FB = nullptr;

  if (TII->AnalyzeBranch(*Latch, TB, FB, Tmp1, false))
    return nullptr;

  for (MBBVector::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
    MachineBasicBlock *PB = *I;
    if (PB != Latch) {
      bool NotAnalyzed = TII->AnalyzeBranch(*PB, TB, FB, Tmp1, false);
      if (NotAnalyzed)
        return nullptr;
    }
  }

  MachineBasicBlock *NewPH = MF->CreateMachineBasicBlock();
  MF->insert(Header, NewPH);

  if (Header->pred_size() > 2) {
    // Ensure that the header has only two predecessors: the preheader and
    // the loop latch.  Any additional predecessors of the header should
    // join at the newly created preheader.  Inspect all PHI nodes from the
    // header and create appropriate corresponding PHI nodes in the preheader.

    for (instr_iterator I = Header->instr_begin(), E = Header->instr_end();
         I != E && I->isPHI(); ++I) {
      MachineInstr *PN = &*I;

      const MCInstrDesc &PD = TII->get(TargetOpcode::PHI);
      MachineInstr *NewPN = MF->CreateMachineInstr(PD, DL);
      NewPH->insert(NewPH->end(), NewPN);

      unsigned PR = PN->getOperand(0).getReg();
      const TargetRegisterClass *RC = MRI->getRegClass(PR);
      unsigned NewPR = MRI->createVirtualRegister(RC);
      NewPN->addOperand(MachineOperand::CreateReg(NewPR, true));

      // Copy all non-latch operands of a header's PHI node to the newly
      // created PHI node in the preheader.
      for (unsigned i = 1, n = PN->getNumOperands(); i < n; i += 2) {
        unsigned PredR = PN->getOperand(i).getReg();
        MachineBasicBlock *PredB = PN->getOperand(i+1).getMBB();
        if (PredB == Latch)
          continue;

        NewPN->addOperand(MachineOperand::CreateReg(PredR, false));
        NewPN->addOperand(MachineOperand::CreateMBB(PredB));
      }

      // Remove copied operands from the old PHI node and add the value
      // coming from the preheader's PHI.
      for (int i = PN->getNumOperands()-2; i > 0; i -= 2) {
        MachineBasicBlock *PredB = PN->getOperand(i+1).getMBB();
        if (PredB != Latch) {
          PN->RemoveOperand(i+1);
          PN->RemoveOperand(i);
        }
      }
      PN->addOperand(MachineOperand::CreateReg(NewPR, false));
      PN->addOperand(MachineOperand::CreateMBB(NewPH));
    }

  } else {
    assert(Header->pred_size() == 2);

    // The header has only two predecessors, but the non-latch predecessor
    // is not a preheader (e.g. it has other successors, etc.)
    // In such a case we don't need any extra PHI nodes in the new preheader,
    // all we need is to adjust existing PHIs in the header to now refer to
    // the new preheader.
    for (instr_iterator I = Header->instr_begin(), E = Header->instr_end();
         I != E && I->isPHI(); ++I) {
      MachineInstr *PN = &*I;
      for (unsigned i = 1, n = PN->getNumOperands(); i < n; i += 2) {
        MachineOperand &MO = PN->getOperand(i+1);
        if (MO.getMBB() != Latch)
          MO.setMBB(NewPH);
      }
    }
  }

  // "Reroute" the CFG edges to link in the new preheader.
  // If any of the predecessors falls through to the header, insert a branch
  // to the new preheader in that place.
  SmallVector<MachineOperand,1> Tmp2;
  SmallVector<MachineOperand,1> EmptyCond;

  TB = FB = nullptr;

  for (MBBVector::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
    MachineBasicBlock *PB = *I;
    if (PB != Latch) {
      Tmp2.clear();
      bool NotAnalyzed = TII->AnalyzeBranch(*PB, TB, FB, Tmp2, false);
      (void)NotAnalyzed; // suppress compiler warning
      assert (!NotAnalyzed && "Should be analyzable!");
      if (TB != Header && (Tmp2.empty() || FB != Header))
        TII->InsertBranch(*PB, NewPH, nullptr, EmptyCond, DL);
      PB->ReplaceUsesOfBlockWith(Header, NewPH);
    }
  }

  // It can happen that the latch block will fall through into the header.
  // Insert an unconditional branch to the header.
  TB = FB = nullptr;
  bool LatchNotAnalyzed = TII->AnalyzeBranch(*Latch, TB, FB, Tmp2, false);
  (void)LatchNotAnalyzed; // suppress compiler warning
  assert (!LatchNotAnalyzed && "Should be analyzable!");
  if (!TB && !FB)
    TII->InsertBranch(*Latch, Header, nullptr, EmptyCond, DL);

  // Finally, the branch from the preheader to the header.
  TII->InsertBranch(*NewPH, Header, nullptr, EmptyCond, DL);
  NewPH->addSuccessor(Header);

  MachineLoop *ParentLoop = L->getParentLoop();
	if (ParentLoop)
		ParentLoop->addBasicBlockToLoop(NewPH, MLI->getBase());

	// Update the dominator information with the new preheader.
   if (MDT) {
       MachineDomTreeNode *HDom = MDT->getNode(Header);
	   MDT->addNewBlock(NewPH, HDom->getIDom()->getBlock());
 	   MDT->changeImmediateDominator(Header, NewPH);
  }

  return NewPH;
}

/// \brief Create a preheader for a given loop.
/// It is not valid to replace the loop header with this method.
/// It is failed when latch block is loop header (only one block
/// in loop).
MachineBasicBlock *DSPHardwareLoops::splitLatchBlock(
	MachineLoop *L) {
	MachineBasicBlock *Latch = L->getLoopLatch();
	MachineFunction *MF = Latch->getParent();
	DebugLoc DL;

	if (!Latch || Latch->hasAddressTaken())
		return nullptr;

	// Verify that all existing predecessors have analyzable branches
	// (or no branches at all).
	typedef std::vector<MachineBasicBlock*> MBBVector;
	MBBVector Preds(Latch->pred_begin(), Latch->pred_end());
	SmallVector<MachineOperand, 2> Tmp1;
	MachineBasicBlock *TB = nullptr, *FB = nullptr;

	if (TII->AnalyzeBranch(*Latch, TB, FB, Tmp1, false))
		return nullptr;

	for (MBBVector::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
		MachineBasicBlock *PB = *I;
			bool NotAnalyzed = TII->AnalyzeBranch(*PB, TB, FB, Tmp1, false);
			if (NotAnalyzed)
				return nullptr;
	}

	MachineBasicBlock *PreLatch = MF->CreateMachineBasicBlock();
	MF->insert(Latch, PreLatch);
	//Update loop information.
	//PreLatch is set to be a new member of the current loop.
	L->addBasicBlockToLoop(PreLatch, MLI->getBase());

	//We expect to leave 1 normal statement and the last control flow in the
	//latch block, marking the endpos of hardware loop.
	//Other instructions prior to them will be moved to the new prelatch block.
	MachineBasicBlock::iterator SplitPos = Latch->getFirstTerminator();
	//No branch or only branch statements exist in the latch block
	if (SplitPos == Latch->end() || SplitPos == Latch->begin())
		return nullptr;
	SplitPos = std::prev(SplitPos);
	//only one non-terminate statement
	if (SplitPos == Latch->begin())
		return Latch;
	PreLatch->splice(PreLatch->end(), Latch,
		Latch->begin(), SplitPos);

	// "Reroute" the CFG edges to link in the new latch.
	// If any of the predecessors falls through to the latch, insert a branch
	// to the new latch in that place.
	SmallVector<MachineOperand, 1> Tmp2;
	SmallVector<MachineOperand, 1> EmptyCond;

	TB = FB = nullptr;

	for (MBBVector::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
		MachineBasicBlock *PB = *I;
			Tmp2.clear();
			bool NotAnalyzed = TII->AnalyzeBranch(*PB, TB, FB, Tmp2, false);
			(void)NotAnalyzed; // suppress compiler warning
			assert(!NotAnalyzed && "Should be analyzable!");
			if (TB != Latch && (Tmp2.empty() || FB != Latch))
				TII->InsertBranch(*PB, PreLatch, nullptr, EmptyCond, DL);
			PB->ReplaceUsesOfBlockWith(Latch, PreLatch);
	}

	// Finally, the branch from the PreLatch to the latch.
	TII->InsertBranch(*PreLatch, Latch, nullptr, EmptyCond, DL);
	PreLatch->addSuccessor(Latch);

	// Update the dominator information with the new prelatch.
	if (MDT) {
		MachineDomTreeNode *HDom = MDT->getNode(Latch);
		MDT->addNewBlock(PreLatch, HDom->getIDom()->getBlock());
		MDT->changeImmediateDominator(Latch, PreLatch);
	}

	return PreLatch;
}

/// getVRegOriginDef - Return the original machine instr that defines the specified virtual
/// register or null if none is found, skipping the COPY instruction.
/// This assumes that the code is in SSA
/// form, so there should only be one original definition.
MachineInstr *DSPHardwareLoops::getVRegOriginDef(unsigned Reg) const {
	// Since the existence of COPY instructions, we must skip them 
	// to find the original definition instruction.
	MachineInstr *DI;

	//do {
	//	DI = MRI->getVRegDef(Reg);
	//	if (!DI)
	//		break;
	//	Reg = DI->getOperand(1).getReg();
	//} while (DI->getOpcode() == DSP::COPY);

	//return DI;

	//\!! Now lihan should fix the redundant COPY instructions derived 
	// from imcompatibility of type of different virtual register in 
	// instruction. 
	// More important, due to COPY is used in function parameter transfer,
	// this funcion wouldn't find the define instruction of parameter 
	// register, so it will throw interrupt. 
	// We temporarily retain this function lest any further fix need,
	// but it is equivalent to getVRegDef() now.
	DI = MRI->getVRegDef(Reg);
	return DI;
}