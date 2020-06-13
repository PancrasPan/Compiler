//===----- DSPPacketizer.cpp - vliw packetizer ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This implements a simple VLIW packetizer using DFA. The packetizer works on
// machine basic blocks. For each instruction I in BB, the packetizer consults
// the DFA to see if machine resources are available to execute I. If so, the
// packetizer checks if I depends on any instruction J in the current packet.
// If no dependency is found, I is added to current packet and machine resource
// is marked as taken. If any dependency is found, a target API call is made to
// prune the dependence.
//
//===----------------------------------------------------------------------===//


#include "llvm/CodeGen/DFAPacketizer.h"
#include "DSP.h"
#include "DSPTargetMachine.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineBranchProbabilityInfo.h"
#include "llvm/CodeGen/LatencyPriorityQueue.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/ScheduleDAG.h"
#include "llvm/CodeGen/ScheduleDAGInstrs.h"
#include "llvm/CodeGen/ScheduleHazardRecognizer.h"
#include "llvm/CodeGen/SchedulerRegistry.h"
#include "llvm/MC/MCInstrItineraries.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include <map>
#include <vector>
#include <iostream>
using namespace llvm;


#define DEBUG_TYPE "packets"

cl::opt<bool> DisablePacketizer("nopack",
	cl::Hidden, cl::init(false), cl::desc("Disable Packetizer for DSP target"));

static cl::opt<bool> PacketizeVolatiles("dsp-packetize-volatiles",
	cl::ZeroOrMore, cl::Hidden, cl::init(true),
	cl::desc("Allow non-solo packetization of volatile memory references"));

namespace llvm {
	void initializeDSPPacketizerPass(PassRegistry&);	
}

static unsigned int  judgeSlot = 0;

namespace {
	class DSPPacketizer : public MachineFunctionPass{
	public:
		static char ID;
		DSPPacketizer() :MachineFunctionPass(ID){
			initializeDSPPacketizerPass(*PassRegistry::getPassRegistry());
		}

		void getAnalysisUsage(AnalysisUsage &AU) const override {
			AU.setPreservesCFG();
			AU.addRequired<MachineDominatorTree>();
			AU.addRequired<MachineBranchProbabilityInfo>();
			AU.addPreserved<MachineDominatorTree>();
			AU.addRequired<MachineLoopInfo>();
			AU.addPreserved<MachineLoopInfo>();
			MachineFunctionPass::getAnalysisUsage(AU);
		}

		const char *getPassName() const override {
			return "DSP Packetizer";
		}

		bool runOnMachineFunction(MachineFunction &Fn) override;
	};
	char DSPPacketizer::ID = 0;

	class DSPPacketizerList : public VLIWPacketizerList {

	private:

		// Has the instruction been promoted to a dot-new instruction.
		bool PromotedToDotNew;

		// Has the instruction been glued to allocframe.
		bool GlueAllocframeStore;

		// Has the feeder instruction been glued to new value jump.
		bool GlueToNewValueJump;

		// Check if there is a dependence between some instruction already in this
		// packet and this instruction.
		bool Dependence;

		// Only check for dependence if there are resources available to
		// schedule this instruction.
		bool FoundSequentialDependence;

		/// \brief A handle to the branch probability pass.
		const MachineBranchProbabilityInfo *MBPI;

		// Track MIs with ignored dependece.
		std::vector<MachineInstr*> IgnoreDepMIs;

	public:
		// Ctor.
		DSPPacketizerList(MachineFunction &MF, MachineLoopInfo &MLI,
			MachineDominatorTree &MDT,
			const MachineBranchProbabilityInfo *MBPI);

		// initPacketizerState - initialize some internal flags.
		void initPacketizerState() override;

		// ignorePseudoInstruction - Ignore bundling of pseudo instructions.
		bool ignorePseudoInstruction(MachineInstr *MI,MachineBasicBlock *MBB) override;

		// isSoloInstruction - return true if instruction MI can not be packetized
		// with any other instruction, which means that MI itself is a packet.
		bool isSoloInstruction(MachineInstr *MI) override;

		// isLegalToPacketizeTogether - Is it legal to packetize SUI and SUJ
		// together.
		bool isLegalToPacketizeTogether(SUnit *SUI, SUnit *SUJ) override;

		// isLegalToPruneDependencies - Is it legal to prune dependece between SUI
		// and SUJ.
		bool  isLegalToPruneDependencies(SUnit *SUI, SUnit *SUJ) override;


		MachineBasicBlock::iterator addToPacket(MachineInstr *MI) override;
	private:
		bool IsCallDependent(MachineInstr* MI, SDep::Kind DepType, unsigned DepReg);
		bool PromoteToDotNew(MachineInstr* MI, SDep::Kind DepType,
			MachineBasicBlock::iterator &MII,
			const TargetRegisterClass* RC);
		bool CanPromoteToDotNew(MachineInstr* MI, SUnit* PacketSU,
			unsigned DepReg,
			std::map <MachineInstr*, SUnit*> MIToSUnit,
			MachineBasicBlock::iterator &MII,
			const TargetRegisterClass* RC);
		bool CanPromoteToNewValue(MachineInstr* MI, SUnit* PacketSU,
			unsigned DepReg,
			std::map <MachineInstr*, SUnit*> MIToSUnit,
			MachineBasicBlock::iterator &MII);
		bool CanPromoteToNewValueStore(MachineInstr* MI, MachineInstr* PacketMI,
			unsigned DepReg,
			std::map <MachineInstr*, SUnit*> MIToSUnit);
		bool DemoteToDotOld(MachineInstr* MI);
		bool ArePredicatesComplements(MachineInstr* MI1, MachineInstr* MI2,
			std::map <MachineInstr*, SUnit*> MIToSUnit);
		bool RestrictingDepExistInPacket(MachineInstr*,
			unsigned, std::map <MachineInstr*, SUnit*>);
		bool isNewifiable(MachineInstr* MI);
		bool isCondInst(MachineInstr* MI);
		bool tryAllocateResourcesForConstExt(MachineInstr* MI);
		bool canReserveResourcesForConstExt(MachineInstr *MI);
		void reserveResourcesForConstExt(MachineInstr* MI);
		bool isNewValueInst(MachineInstr* MI);
		void judgeSlots(MachineBasicBlock *MBB,MachineInstr *MI);
	};
}

	

	INITIALIZE_PASS_BEGIN(DSPPacketizer, "packets", "DSP Packetizer",
		false, false)
	INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
	INITIALIZE_PASS_DEPENDENCY(MachineBranchProbabilityInfo)
	INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
	INITIALIZE_AG_DEPENDENCY(AliasAnalysis)
	INITIALIZE_PASS_END(DSPPacketizer, "packets", "DSP Packetizer",false, false)


	DSPPacketizerList::DSPPacketizerList(MachineFunction &MF, MachineLoopInfo &MLI, MachineDominatorTree &MDT,
	const MachineBranchProbabilityInfo *MBPI)
	:VLIWPacketizerList(MF, MLI, MDT, true){
		this->MBPI = MBPI;
	}

	static bool IsControlFlow(MachineInstr *MI){
		return (MI->getDesc().isTerminator() || MI->getDesc().isCall());
	}

	static bool IsNop(MachineInstr *MI){
		return (MI->getOpcode()==DSP::NOP);
	}

	static bool IsCJorUJ(MachineInstr *MI) {
		return MI->getDesc().isBranch();
	}

	//isLegalToPacketizeTogether:
	// SUI is the current instruction that is out side of the current packet.
	// SUJ is the current instruction inside the current packet against which that
	// SUI will be packetized.
	bool DSPPacketizerList::isLegalToPacketizeTogether(SUnit *SUI, SUnit *SUJ) {
		MachineInstr *I = SUI->getInstr();
		MachineInstr *J = SUJ->getInstr();
		assert(I && J && "Unable to packetize null instruction!");

		const MCInstrDesc &MCIDI = I->getDesc();
		const MCInstrDesc &MCIDJ = J->getDesc();

		MachineBasicBlock::iterator MBBI = I;

		const unsigned FrameSize = MF.getFrameInfo()->getStackSize();

		const DSPRegisterInfo *RI = (const DSPRegisterInfo *)TM.getRegisterInfo();
		const DSPInstrInfo *II = (const DSPInstrInfo*)TII;

		if (I->getOpcode() == DSP::INLINEASM)
			llvm_unreachable("Should not meet inline asm here!");

		if (isSoloInstruction(I))
			llvm_unreachable("Should not meet solo instr here!");

		// Two control flow instructions cannot go in the same packet.
		if (IsControlFlow(I) && IsControlFlow(J)) {
			Dependence = true;
			return false;
		}

		if (IsNop(I) || IsNop(J))
		{
			Dependence = true;
			return false;
		}

		if (IsCJorUJ(I) || IsCJorUJ(J)) {
			Dependence = true;
			return false;
		}
		//check if SUI is the successor of SUJ
		if (SUJ->isSucc(SUI)){
			for (unsigned i = 0; (i < SUJ->Succs.size()) && !FoundSequentialDependence; ++i){
				if (SUJ->Succs[i].getSUnit() != SUI){
					continue;
				}

				SDep::Kind DepType = SUJ->Succs[i].getKind();
				//std::cout << "Dep Type" << DepType << std::endl;



				unsigned DepReg = 0;
				const TargetRegisterClass *RC = nullptr;
				if (DepType == SDep::Data){
					DepReg = SUJ->Succs[i].getReg();
					RC = RI->getMinimalPhysRegClass(DepReg);
				}

				if (DepType == SDep::Data) {
					FoundSequentialDependence = true;
				}
				// Ignore output dependences due to superregs. We can
				// write to two different subregisters of R1:0 for instance
				// in the same cycle
				//

				//
				// Let the
				// If neither I nor J defines DepReg, then this is a
				// superfluous output dependence. The dependence must be of the
				// form:
				//  R0 = ...
				//  R1 = ...
				// and there is an output dependence between the two instructions
				// with
				// DepReg = D0
				// We want to ignore these dependences.
				// Ideally, the dependence constructor should annotate such
				// dependences. We can then avoid this relatively expensive check.
				//

				else if (DepType == SDep::Output){
					unsigned DepReg = SUJ->Succs[i].getReg();
					// Check if I and J really defines DepReg.
					if (I->definesRegister(DepReg) ||
						J->definesRegister(DepReg)) {
						FoundSequentialDependence = true;
						break;

					}
				}

				else if ((DepType == SDep::Order) &&
					!I->hasOrderedMemoryRef() &&
					!J->hasOrderedMemoryRef()){
					FoundSequentialDependence = true;
					break;
				}
				else if (DepType != SDep::Anti) {
					FoundSequentialDependence = true;
					break;
				}
				/*else if (DepType == SDep::Anti){
					unsigned DepReg = SUJ->Succs[i].getReg();
					if (I->definesRegister(DepReg) ||
						J->definesRegister(DepReg)) {
						FoundSequentialDependence = true;
						break;

					}
				}*/
			}
			if (FoundSequentialDependence) {
				Dependence = true;
				return false;
			}
			
		}
		
		return true;
	}

	// isLegalToPruneDependencies
	bool DSPPacketizerList::isLegalToPruneDependencies(SUnit *SUI, SUnit *SUJ) {
		MachineInstr *I = SUI->getInstr();
		assert(I && SUJ->getInstr() && "Unable to packetize null instruction!");

		const unsigned FrameSize = MF.getFrameInfo()->getStackSize();

		if (Dependence) {

			// Check if the instruction was promoted to a dot-new. If so, demote it
			// back into a dot-old.
			

			// Check if the instruction (must be a store) was glued with an Allocframe
			// instruction. If so, restore its offset to its original value, i.e. use
			// curent SP instead of caller's SP.
			/*if (GlueAllocframeStore) {
				I->getOperand(1).setImm(I->getOperand(1).getImm() +
					FrameSize + 8);
			}*/

			return false;
		}
		return true;
	}

	bool DSPPacketizerList::isSoloInstruction(MachineInstr *MI){
		if (MI->isInlineAsm())
			return true;

		if (MI->isEHLabel())
			return true;

		if (MI->isReturn())
			return true;
		return false;
	}

	bool DSPPacketizerList::ignorePseudoInstruction(MachineInstr *MI, MachineBasicBlock *MBB){
		if (MI->isDebugValue())
			return true;

		// We must print out inline assembly
		if (MI->isInlineAsm())
			return false;
		const MCInstrDesc& TID = MI->getDesc();
		unsigned SchedClass = TID.getSchedClass();
		const InstrStage* IS =
			ResourceTracker->getInstrItins()->beginStage(SchedClass);
		unsigned FuncUnits = IS->getUnits();
		return !FuncUnits;
	}


	// initPacketizerState - Initialize packetizer flags
	void DSPPacketizerList::initPacketizerState() {

		Dependence = false;
		PromotedToDotNew = false;
		GlueToNewValueJump = false;
		GlueAllocframeStore = false;
		FoundSequentialDependence = false;

		return;
	}

	bool DSPPacketizer::runOnMachineFunction(MachineFunction &MF){
		const TargetInstrInfo *TII = MF.getTarget().getInstrInfo();
		MachineLoopInfo &MLI = getAnalysis<MachineLoopInfo>();
		MachineDominatorTree &MDT = getAnalysis<MachineDominatorTree>();
		const MachineBranchProbabilityInfo *MBPI = &getAnalysis<MachineBranchProbabilityInfo>();

		//Instantiate the packetizer.
		DSPPacketizerList Packetizer(MF, MLI, MDT, MBPI);


		// DFA state table should not be empty.
		assert(Packetizer.getResourceTracker() && "Empty DFA table!");

		//
		// Loop over all basic blocks and remove KILL pseudo-instructions
		// These instructions confuse the dependence analysis. Consider:
		// D0 = ...   (Insn 0)
		// R0 = KILL R0, D0 (Insn 1)
		// R0 = ... (Insn 2)
		// Here, Insn 1 will result in the dependence graph not emitting an output
		// dependence between Insn 0 and Insn 2. This can lead to incorrect
		// packetization
		//
		for (MachineFunction::iterator MBB = MF.begin(), MBBe = MF.end();
			MBB != MBBe; ++MBB) {
			MachineBasicBlock::iterator End = MBB->end();
			MachineBasicBlock::iterator MI = MBB->begin();
			while (MI != End) {
				if (MI->isKill()) {
					MachineBasicBlock::iterator DeleteMI = MI;
					++MI;
					MBB->erase(DeleteMI);
					End = MBB->end();
					continue;
				}
				++MI;
			}
		}

		// Loop over all of the basic blocks.
		for (MachineFunction::iterator MBB = MF.begin(), MBBe = MF.end();
			MBB != MBBe; ++MBB) {
			// Find scheduling regions and schedule / packetize each region.
			unsigned RemainingCount = MBB->size();

			for (MachineBasicBlock::iterator RegionEnd = MBB->end();
				RegionEnd != MBB->begin();) {
				// The next region starts above the previous region. Look backward in the
				// instruction stream until we find the nearest boundary.
				MachineBasicBlock::iterator I = RegionEnd;
				for (; I != MBB->begin(); --I, --RemainingCount) {
					if (TII->isSchedulingBoundary(std::prev(I), MBB, MF))
						break;
				}
				I = MBB->begin();

				// Skip empty scheduling regions.
				if (I == RegionEnd) {
					RegionEnd = std::prev(RegionEnd);
					--RemainingCount;
					continue;
				}
				// Skip regions with one instruction.
				if (I == std::prev(RegionEnd)) {
					RegionEnd = std::prev(RegionEnd);
					continue;
				}
				//std::cout << "In DSPVLIWPacketizer.cpp" << std::endl;
				//std::cout <<" start MI packetizer" << std::endl;
				Packetizer.PacketizeMIs(MBB, I, RegionEnd);
				RegionEnd = I;
			}
		}

		return true;

	}

	void DSPPacketizerList::judgeSlots(MachineBasicBlock *MBB,MachineInstr *MI){
		const MCInstrDesc MID = MI->getDesc();
		unsigned InsnClass = MID.getSchedClass();
		const InstrItineraryData *InstrItins = getResourceTracker()->getInstrItins();
		const llvm::InstrStage *IS = InstrItins->beginStage(InsnClass);
		unsigned FuncUnits = IS->getUnits();
		if (judgeSlot >FuncUnits){ 
			judgeSlot = 0; endPacket(MBB, MI); 
			return;
		}

		switch (FuncUnits)
		{
		default:llvm_unreachable("unknow FU");
			break;
		case 1:{
				   judgeSlot = DSP::SLOT0_ONLY | judgeSlot; break;
		}
		case 3:{
				   if (judgeSlot&&DSP::SLOT0_ONLY)judgeSlot = judgeSlot | DSP::SLOT1;
				   else judgeSlot = judgeSlot | DSP::SLOT0_ONLY;
				   break;
		}
		case 12:{
					if (judgeSlot&&DSP::SLOT2)judgeSlot = judgeSlot | DSP::SLOT3;
					else judgeSlot = judgeSlot | DSP::SLOT2;
					break;
		}
		case 15:{
					if (!(judgeSlot&&DSP::SLOT0_ONLY))judgeSlot = judgeSlot | DSP::SLOT0_ONLY;
					else if (!(judgeSlot&&DSP::SLOT1))judgeSlot = judgeSlot | DSP::SLOT1;
					else if (!(judgeSlot&&DSP::SLOT2))judgeSlot = judgeSlot | DSP::SLOT2;
					else if (!(judgeSlot&&DSP::SLOT3))judgeSlot = judgeSlot | DSP::SLOT3;
					break;
		}
		}
		
	}

	MachineBasicBlock::iterator
		DSPPacketizerList::addToPacket(MachineInstr *MI) {

			//std::cout << "this is add to packet function" << std::endl;
			//std::cout << "opcode is" << MI->getOpcode() << std::endl;
			MachineBasicBlock::iterator MII = MI;
			MachineBasicBlock *MBB = MI->getParent();
			const DSPInstrInfo *QII = (const DSPInstrInfo *)TII;
			
			if (GlueToNewValueJump) {
				std::cout << "glue to new value" << std::endl;
				++MII;
				MachineInstr *nvjMI = MII;
				assert(ResourceTracker->canReserveResources(MI));
				ResourceTracker->reserveResources(MI);
				
				// Here, we are sure that "reserveResources" would succeed.
				ResourceTracker->reserveResources(nvjMI);
				CurrentPacketMIs.push_back(MI);
				CurrentPacketMIs.push_back(nvjMI);
			}
			else {
				MachineBasicBlock::iterator MII = MI;
				++MII;

				ResourceTracker->reserveResources(MI);
				CurrentPacketMIs.push_back(MI);
			}
			return MII;
		}



	//===----------------------------------------------------------------------===//
	//                         Public Constructor Functions
	//===----------------------------------------------------------------------===//

	FunctionPass *llvm::createDSPPacketizer() {
		return new DSPPacketizer();
	}