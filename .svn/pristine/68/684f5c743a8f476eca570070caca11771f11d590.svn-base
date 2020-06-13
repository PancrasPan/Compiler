//===---- DSPFixupHwLoops.cpp - Fixup HW loops end bundle by splitting new block ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// The loop end address in the Loop instruction marks the last bundle in the loop,
// but now it is pointed to the latch block.
// Find the end address and split the latch block into two blocks apart according 
// to it, then set the loop end address operand pointing to the block below.
//===----------------------------------------------------------------------===//


#include "llvm/ADT/DenseMap.h"
#include "DSP.h"
#include "DSPVLIWBundler.h"
#include "DSPTargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include <iostream>

using namespace llvm;

extern cl::opt<bool> ShowDbgInfo;
#define DEBUG_TYPE "hwloops fixup"
//#define DSPDEBUG true

STATISTIC(NumHWLoopsFixup, "Number of hardware loops fixup succesfully");

namespace llvm {
	FunctionPass *createDSPFixupHwLoops();
	void initializeDSPFixupHwLoopsPass(PassRegistry&);
}

namespace {
	struct DSPFixupHwLoops : public MachineFunctionPass {
		MachineDominatorTree       *MDT;
		const DSPTargetMachine *TM;
		const DSPInstrInfo     *TII;

	public:
		static char ID;

		DSPFixupHwLoops() : MachineFunctionPass(ID) {
			initializeDSPFixupHwLoopsPass(*PassRegistry::getPassRegistry());
		}

		bool runOnMachineFunction(MachineFunction &MF) override;

		const char *getPassName() const override {
			return "DSP Hardware Loop Fixup";
		}

		void getAnalysisUsage(AnalysisUsage &AU) const override {
			AU.setPreservesCFG();
			AU.addRequired<MachineDominatorTree>();
			MachineFunctionPass::getAnalysisUsage(AU);
		}

	private:
		/// \brief Check the offset between each loop instruction and
		/// the loop basic block to determine if we can use the LOOP instruction
		/// or if we need to set the LC/SA registers explicitly.
		bool fixupLoopInstrs(MachineFunction &MF);

		///Spilt a block to hold the last VLIW bundle as the end of hardware loop body.
		MachineBasicBlock * splitLatchBlock(MachineBasicBlock * Latch, MachineBasicBlock * Preheader);

		///If there exists a inner branch to the latch block, which will make the hardware loop
		///pipeline mechanism fail to jump back to the top of loop body, we need to insert a landing block,
		///just ahead the latch block, as the new target of that branch.
		MachineBasicBlock * splitInnerJmpLanding(MachineBasicBlock * PreLatch, MachineBasicBlock * Latch,
												 MachineBasicBlock * Preheader);

	};

	char DSPFixupHwLoops::ID = 0;
}

INITIALIZE_PASS(DSPFixupHwLoops, "hwloopsfixup",
	"DSP Hardware Loops Fixup", false, false)

	FunctionPass *llvm::createDSPFixupHwLoops() {
	return new DSPFixupHwLoops();
}

bool DSPFixupHwLoops::runOnMachineFunction(MachineFunction &MF) {
	MDT = &getAnalysis<MachineDominatorTree>();
	TM = static_cast<const DSPTargetMachine*>(&MF.getTarget());
	TII = static_cast<const DSPInstrInfo*>(TM->getInstrInfo());
	
	if(ShowDbgInfo)
		DEBUG({ dbgs() << "\n** Before HWLoops  Fixup ** \n"; 
				MF.print(dbgs()); });

	bool Changed = fixupLoopInstrs(MF);

	if (ShowDbgInfo)
		std::cout << "<**-- Totally complete "<< NumHWLoopsFixup 
		<<" HardwwareLoop fixups **>" << std::endl;
	
	if (ShowDbgInfo && Changed)
		DEBUG({ dbgs() << "\n** After HWLoops  Fixup ** \n";   
				MF.print(dbgs()); });
	
	return Changed;
}

/// \brief For DSP, the loop end address in the Loop instruction marks 
/// the last bundle in the loop,but now it is pointed to the latch block.
/// Find the end address and split the latch block into two blocks apart ,
/// then set the loop end address operand pointing to the block below.
/// This function makes two passes over the basic blocks.  The first
/// pass computes the offset of the basic block from the start.
/// The second pass checks all the loop instructions.
bool DSPFixupHwLoops::fixupLoopInstrs(MachineFunction &MF) {
	if (ShowDbgInfo)
		std::cout << "\nStart Fixup HardwareLoops:\n"
		"**fixupLoopInstrs**" << std::endl;
	// Second pass - check each loop instruction to see if it needs to be
	// converted.
	bool Changed = false;

	for (MachineBasicBlock &MBB : MF) {
		// Loop over all the instructions.
		MachineBasicBlock::instr_iterator MII = MBB.instr_begin();
		MachineBasicBlock::instr_iterator MIE = MBB.instr_end();
		while (MII != MIE) {
			if (MII->isDebugValue()) {
				++MII;
				continue;
			}
			if (MII->getOpcode() == DSP::Loop) {
				assert(MII->getOperand(1).isMBB() &&
					"Expect a basic block as loop operand");
				MachineBasicBlock *Latch = MII->getOperand(1).getMBB();
				MachineBasicBlock *Preheader = MII->getParent();
				MachineBasicBlock *NewPreLatch;
				if (ShowDbgInfo) {
					std::cout << "Got Loop instruction" << std::endl;
					std::cout << "Preheader:" << Preheader->getFullName() << std::endl;
					std::cout << "Latch:" << Latch->getFullName() << std::endl;
				}
				NewPreLatch = splitLatchBlock(Latch, Preheader);
				assert(NewPreLatch &&
					"Latch block split failed");

				Changed = true;
				MachineBasicBlock *Landing;
				Landing = splitInnerJmpLanding(NewPreLatch, Latch, Preheader);
				assert(Landing &&
					"Landing block split for inner branch to latch failed.");

				NumHWLoopsFixup++;
				if (ShowDbgInfo)
					std::cout << "<**Complete this HardwwareLoop fixup**>" << std::endl;
			}
				++MII;
		}//while MI
	}//for MBB

	return Changed;
}

/// ReplaceUsesOfBlockWith - Given a machine basic block that branched to
/// 'Old', change the code and CFG so that it branches to 'New' instead.
/// Skip delay slot nop behind branch instruction which is located at 
/// the bottom of Machine Basic Block.
static void ReplaceUsesOfBlockWith(MachineBasicBlock *Old, MachineBasicBlock *New,
	MachineBasicBlock *Use) {
	assert(Old != New && "Cannot replace self with self!");

	MachineBasicBlock::instr_iterator I = Use->instr_end();
	while (I != Use->instr_begin()) {
		--I;
		if (I->getOpcode() == DSP::NOP)
			continue;
		if (!I->isTerminator())
			break;
		// Scan the operands of this machine instruction, replacing any uses of Old
		// with New.
		for (unsigned i = 0, e = I->getNumOperands(); i != e; ++i)
			if (I->getOperand(i).isMBB() &&
				I->getOperand(i).getMBB() == Old)
				I->getOperand(i).setMBB(New);
	}
	// Update the successor information.
	Use->replaceSuccessor(Old, New);
}

/// \brief Create a new prelatch block for a given loop if necessary.
/// It is not valid to replace the loop header with this method.
MachineBasicBlock *DSPFixupHwLoops::splitLatchBlock(
	MachineBasicBlock *Latch, MachineBasicBlock *Preheader) {
	MachineFunction *MF = Latch->getParent();
	DebugLoc DL;
	if (ShowDbgInfo)
		std::cout << "**splitLatchBlock**" << std::endl;

	if (!Latch) 
		return nullptr;

	// Verify that all existing predecessors have analyzable branches
	// (or no branches at all).
	// Latch block may be not analyzable now because the branch at the
	// bottom of block was removed and the endloop instruction could 
	// not be analyzed.So does Preheader block which end with Loop instruction.
	typedef std::vector<MachineBasicBlock*> MBBVector;
	MBBVector Preds(Latch->pred_begin(), Latch->pred_end());
	SmallVector<MachineOperand, 2> Tmp1;
	MachineBasicBlock *TB = nullptr, *FB = nullptr;
	//Preheader is the predecessor of this latch block
	bool PredhasPreheader = false;
	bool PredhasLatch = false;
	for (MBBVector::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
		MachineBasicBlock *PB = *I;
		if (ShowDbgInfo) {
			std::cout << "Predecessor block of Latch:" << PB->getFullName() << std::endl;
		}
		//The Preheader where the loop instruction located in at its bottom
		//is not analyzed when it is the predecessor of Latch block.
		if (PB == Preheader) {
			if (ShowDbgInfo) {
				std::cout << "Preheader is the predecessor of Latch," << std::endl;
			}
			PredhasPreheader = true;
			continue;
		}
		//If latch is one of PB, then latch branch to itself, which means
		//latch is the header of loop. So preheader must be PB of latch 
		//because it only branch to loop header block.
		//That is (PB == latch) <=> (PB == Preheader)
		if (PB == Latch) {
			PredhasLatch = true;
			continue;
		}
		bool NotAnalyzed = TII->AnalyzeBranch(*PB, TB, FB, Tmp1, false);
		if (NotAnalyzed) {
			if (ShowDbgInfo)
				std::cout << "<**Preheader is not analyzed**>" << std::endl;
			return nullptr;
		}
	}
	assert(!(PredhasPreheader^PredhasLatch)
		&& "If there is only one loop block, preheader and latch must be its predecessor.");

	// We expect to leave one bundle in front of endloop and follwing instruction
	//  to remain in the old latch block, marking the end address of hardware loop.
	// Other instructions prior to them will be moved to the new prelatch block.
	MachineBasicBlock::iterator SplitPos = nullptr;
	typedef MachineBasicBlock::reverse_instr_iterator rinstr_iterator;
	//Find the bundle right above endloop which is the split position.
	for (rinstr_iterator MII = Latch->instr_rbegin(), MIE = Latch->instr_rend();
		MII != MIE; ++MII) {
		if (MII->getOpcode() != DSP::ENDLOOP)
			continue;
		else {
			rinstr_iterator BI = std::next(MII);

			if (ShowDbgInfo)
				std::cout << "Instr opcode in the bundle above endloop in reverse order" 
					<< std::endl;
			while (BI != MIE) {
				if (ShowDbgInfo) 
					std::cout << "instr opcode:" 
					<< BI->getOpcode() << std::endl;
				if (!BI->isInsideBundle())
					break;
				BI++;
			}
			assert(BI != MIE && 
				"No bundle above endloop instruction in latch");
			if (ShowDbgInfo)
				std::cout << "Got bundle above endloop" << std::endl;
			MachineBasicBlock::iterator PreEndLoopBundle = &*BI;
			SplitPos = PreEndLoopBundle;
			//Erase the endloop pseudo instruction
			//Latch->erase(std::next(PreEndLoopBundle));
			MachineInstr * EndLoopI = &*MII;
			Latch->remove_instr(EndLoopI);
			break;
		}
	}
	//No endloop instruction in latch block
	if (!SplitPos) {
		if (ShowDbgInfo)
			std::cout << "<**No endloop instruction in latch block**>" << std::endl;
		return nullptr;
	}

	// Split position is just the beginning of latch block, no need split.
	if (SplitPos == Latch->begin()) {
		if (ShowDbgInfo)
			std::cout << "No need splitting latch block" << std::endl;
		//\?? Check comment later for its meaning
		//Keep the last block always has two predecessors contributing to
		//make compiler mark it as potential jump target and generate entry
		//in address label table by which the address field of LOOP instruction could be resloved.
		if(!PredhasPreheader && (Latch->pred_size() < 2))
			Preheader->addSuccessor(Latch);
		return Latch;
	}

	MachineBasicBlock *PreLatch = MF->CreateMachineBasicBlock();
	MF->insert(Latch, PreLatch);
	// Transfer bundles in latch block from the beginning to that above
	// the split position to the new built prelatch block.
	PreLatch->splice(PreLatch->end(), Latch,
		Latch->begin(), SplitPos);

	// "Reroute" the CFG edges to link in the new prelatch.
	// If any of the predecessors falls through to the latch, 
	// it now fall through to prelatch instead. NO NEED TO
	// insert a branch to the new prelatch in that place.
	SmallVector<MachineOperand, 1> Tmp2;
	SmallVector<MachineOperand, 1> EmptyCond;

	TB = FB = nullptr;

	for (MBBVector::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
		MachineBasicBlock *PB = *I;
		//Preheader and Latch may not be analyzed due to no branch instruction lied in them.
		//So their successor list are modified from latch to prelatch later.
		if (PB != Preheader && PB != Latch) {
			Tmp2.clear();
			bool NotAnalyzed = TII->AnalyzeBranch(*PB, TB, FB, Tmp2, false);
			(void)NotAnalyzed; // suppress compiler warning
			assert(!NotAnalyzed && 
				"Predecessor of latch Should be analyzable!");
			////Fall through scenarios:
			////1) No branch fall through: TB == FB == nullptr
			////2) Condition branch and fall through: TB != Latch, FB = nullptr
			//if (TB != Latch && (!FB))
			//	TII->InsertBranch(*PB, PreLatch, nullptr, EmptyCond, DL);
			ReplaceUsesOfBlockWith(Latch, PreLatch, PB);
		}
	}

	// It can happen that header and latch are the same block in loop.
	// In such case, the preheader block will fall through into the latch.
	// Fix up the CFG successor of preheader to the new prelatch.
	if(PredhasPreheader) {
		Preheader->replaceSuccessor(Latch, PreLatch);

		Latch->replaceSuccessor(Latch, PreLatch);
		PreLatch->addSuccessor(Latch);
	}
	else
		PreLatch->addSuccessor(Latch);
	
	// Make sure the latch block has more than one predecessors so that 
	// it generate label in assembler file.
	// \?? Warning: Not sure its potential side effects
	if (Latch->pred_size() < 2)
		Preheader->addSuccessor(Latch);

	// Update the dominator information with the new prelatch.
	if (MDT) {
		MachineDomTreeNode *HDom = MDT->getNode(Latch);
		MDT->addNewBlock(PreLatch, HDom->getIDom()->getBlock());
		MDT->changeImmediateDominator(Latch, PreLatch);
	}

	return PreLatch;
}

///\brief Creat a landing block between the prelatch and latch blocks if there is a branch
///inside loop body exactly right jumping to the latch block.
MachineBasicBlock * DSPFixupHwLoops::splitInnerJmpLanding(MachineBasicBlock * PreLatch, MachineBasicBlock * Latch,
	MachineBasicBlock * Preheader) {
	MachineFunction *MF = Latch->getParent();
	DebugLoc DL;
	if (ShowDbgInfo)
		std::cout << "**figure if splitInnerJmpLanding**" << std::endl;

	if (!Latch || !PreLatch || !Preheader)
		return nullptr;

	// Verify that all existing predecessors have analyzable branches
	// (or no branches at all) except preheader block.
	typedef std::vector<MachineBasicBlock*> MBBVector;
	MBBVector Preds(Latch->pred_begin(), Latch->pred_end());
	SmallVector<MachineOperand, 2> Tmp1;
	MachineBasicBlock *TB = nullptr, *FB = nullptr;
	//If preheader is the predecessor of latch block after spilt, there must be no inner
	//branch to the latch block so that we use preheader as the predecessor of it to
	//complement the requirement of at least two predecessors to generate address label.
	bool PredhasPreheader = false;
	for (MBBVector::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
		MachineBasicBlock *PB = *I;
		if (ShowDbgInfo) {
			std::cout << "Predecessor block of new Latch:" << PB->getFullName() << std::endl;
		}
		//The Preheader where the loop instruction located in at its bottom
		//is not analyzed when it is the predecessor of Latch block.
		if (PB == Preheader) {
			if (ShowDbgInfo) {
				std::cout << "Preheader is the predecessor of new Latch," << std::endl;
			}
			PredhasPreheader = true;
			continue;
		}
		if (PB == Latch) {
			continue;
		}

		bool NotAnalyzed = TII->AnalyzeBranch(*PB, TB, FB, Tmp1, false);
		if (NotAnalyzed) {
			if (ShowDbgInfo)
				std::cout << "<**Predecessor is not analyzed**>" << std::endl;
			return nullptr;
		}
	}
	//No inner jump to latch, getting back.
	if (PredhasPreheader) {
		if (ShowDbgInfo)
			std::cout << "<**No inner jump to latch**>" << std::endl;
		return Latch;
	}

	MachineBasicBlock *Landing = MF->CreateMachineBasicBlock();
	MachineBasicBlock::iterator InsertPos = Landing->end();
	DebugLoc NopDL;
	BuildMI(*Landing, InsertPos, NopDL,
	        TII->get(DSP::NOP));
	MachineInstr *ptrNop = &*Landing->begin();
	DSPVLIWBundler::getBundler()->InsertSlot(ptrNop, 0x9fffffff);
	//MIBundleBuilder(*Landing, Landing->begin(), Landing->end());

	MF->insert(Latch, Landing);
	// "Reroute" the CFG edges to link in the new landing block.
	// If any of the predecessors falls through to the latch, 
	// it now fall through to landing instead. NOTICE NO NEED TO
	// insert a branch to landing in previous block for falling through case.
	SmallVector<MachineOperand, 1> Tmp2;
	TB = FB = nullptr;

	for (MBBVector::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
		MachineBasicBlock *PB = *I;
		Tmp2.clear();
		bool NotAnalyzed = TII->AnalyzeBranch(*PB, TB, FB, Tmp2, false);
		(void)NotAnalyzed; // suppress compiler warning
		assert(!NotAnalyzed &&
			"Predecessor of latch Should be analyzable!");
		ReplaceUsesOfBlockWith(Latch, Landing, PB);
	}
	Landing->addSuccessor(Latch);

	// Make sure the latch block has more than one predecessors so that 
	// it generate label in assembler file.
	// \?? Warning: Not sure its potential side effects
	if (Latch->pred_size() < 2)
		Preheader->addSuccessor(Latch);

	// Update the dominator information with the new prelatch.
	if (MDT) {
		MachineDomTreeNode *HDom = MDT->getNode(Latch);
		MDT->addNewBlock(Landing, HDom->getIDom()->getBlock());
		MDT->changeImmediateDominator(Latch, Landing);
	}

	if (ShowDbgInfo)
		std::cout << "<**A landing block ahead latch has been inserted.**>" << std::endl;
	return Landing;
}
