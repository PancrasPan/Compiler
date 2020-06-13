//This pass is intented to duel with compound_condition such as a && b && c or a && b || c,because our dsp's SW register is 
//implicit,so LLVM Phi can not use it dirctly.
//
/*
BB#3: derived from LLVM BB %land.rhs
Predecessors according to CFG : BB#2
% vreg13<def> = MovGR %ZERO, 0; CPURegs:%vreg13
%vreg14<def> = LD <fi#3>, 0; mem:LD4[%k] CPURegs : %vreg14
%vreg15<def> = GT %vreg14<kill>, %vreg13<kill>; GPROut:%vreg15 CPURegs : %vreg14, %vreg13
%vreg0<def> = COPY %vreg15; CPURegs:%vreg0 GPROut : %vreg15
Successors according to CFG : BB#4

BB#4: derived from LLVM BB %land.end
Predecessors according to CFG : BB#1 BB#2 BB#3
% vreg1<def> = PHI %vreg5, <BB#1>, %vreg9, <BB#2>, %vreg0, <BB#3>; CPURegs:%vreg1, %vreg5, %vreg9, %vreg0
%vreg16<def> = EQI %vreg1, 0; GPROut:%vreg16 CPURegs : %vreg1
JC %vreg16<kill>, <BB#6>; GPROut:%vreg16
Jmp <BB#5>
*/

// LLVM code after ISel is like this.
//after BB#3
//%vreg15<def> = GT %vreg14<kill>, %vreg13<kill>; GPROut:%vreg15 CPURegs : %vreg14, %vreg13
//we should add, for example
// vreg0 = mov 0
//JNC vreg15, BB#4
//Jmp BB#10

//BB#10
//vreg1 = mov 1
//jmp BB#4
//
//BB#4
// vreg1 = Phi .....,vreg0,BB#3,vreg1,BB#10 or  vreg1 = Phi .....,false,BB#4,true,BB#10,

#include "DSP.h"
#include "MCTargetDesc/DSPBaseInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/PassSupport.h"


#define DEBUG_TYPE "cc handler"
using namespace llvm;



namespace {
	class DSPCCHandler : public MachineFunctionPass {
	public:
		static char ID;
		explicit DSPCCHandler() :MachineFunctionPass(ID) {
		}

		bool runOnMachineFunction(MachineFunction &mf) override {
			DEBUG(dbgs() << "*******************In handler cc pass**********************" << "\n");

			TII = mf.getTarget().getInstrInfo();
			MRI = &mf.getRegInfo();
			MachineInstr* Copy;
			MachineInstr* ConditionCode;
			MachineFunction::iterator RegionNext;
			MachineInstr* Phi;
			for (MachineFunction::iterator MBBI = mf.begin(),MBBe = mf.end(); MBBI != MBBe; MBBI++)
			{
				MachineBasicBlock::iterator CopyI = MBBI->instr_back();
				Copy = &(*CopyI);
				ConditionCode = Copy;
				if (CopyI != MBBI->begin()) {
					if (Copy->isCopy()) {
						ConditionCode = std::prev(CopyI);
					}
					else ConditionCode = &MBBI->instr_back();
				}
					

				RegionNext = std::next(MBBI);

				if (RegionNext == MBBe) break;
				Phi = &RegionNext->instr_front();
				unsigned p = Copy->getOpcode();
				if (Phi&&Phi->isPHI()&&isConditionCode(ConditionCode)) {
					PerformCCHandle(Copy, ConditionCode, Phi, MBBI,RegionNext,mf);
					break;
				}
			}
			mf.dump();
			DEBUG(dbgs() << mf.getFrameInfo()->getNumObjects() << "\n");
			return false;
		}

		const char* getPassName() const override {
			return "DSP handle CC";
		}
	public:
		const TargetInstrInfo* TII;
		MachineRegisterInfo* MRI;
	private:
		bool PerformCCHandle(MachineInstr* Copy, MachineInstr* ConditionCode, MachineInstr* Phi, MachineBasicBlock* MBBI, MachineBasicBlock* RegionNext, MachineFunction &mf);
		bool isConditionCode(MachineInstr* MI);
	};
}
bool DSPCCHandler::isConditionCode(MachineInstr* MI) {
	if (MI == nullptr) return false;
	unsigned Op = MI->getOpcode();
	switch (Op)
	{
	case DSP::LE:
	case DSP::LEI:
	case DSP::LEU:
	case DSP::GE:
	case DSP::GEI:
	case DSP::GEU:
	case DSP::GT:
	case DSP::GTI:
	case DSP::GTU:
	case DSP::LT:
	case DSP::LTI:
	case DSP::LTU:
		return true;
	default: return false;	
	}
	return false;
}

bool DSPCCHandler::PerformCCHandle(MachineInstr * Copy, MachineInstr* ConditionCode, MachineInstr * Phi, MachineBasicBlock * MBBI, MachineBasicBlock* RegionNext, MachineFunction & mf)
{
	unsigned CPR = 0;
	if (Copy->isCopy()) CPR = Copy->getOperand(0).getReg();
	else CPR = ConditionCode->getOperand(0).getReg();
	bool flag = false;
	for (unsigned int i = 0;i < Phi->getNumOperands();i++) {
		if (!Phi->getOperand(i).isReg()) continue;
		unsigned PR = Phi->getOperand(i).getReg();
		if (CPR == PR) {
			flag = true;
			break;
		}
	}
	if (!flag)return flag;
	MachineBasicBlock::succ_iterator SuccBBI = MBBI->succ_begin();
	MachineBasicBlock* SuccBB = *SuccBBI;

	MachineBasicBlock* NewBB = mf.CreateMachineBasicBlock();
	mf.insert(MBBI, NewBB);
	//we need to change CFG for Phi can get correct answer
	MBBI->addSuccessor(NewBB);
	NewBB->addSuccessor(SuccBB);

	//create JNC from MBBI to succBB and erase copy
	unsigned S_reg = ConditionCode->getOperand(1).getReg();
	const TargetRegisterClass *RC = MRI->getRegClass(ConditionCode->getOperand(0).getReg());
	if(Copy->isCopy())
		MBBI->erase(Copy);
	
	// add  new reg = 0, for phi to use
	MachineInstr* NI = mf.CreateMachineInstr(TII->get(DSP::MovGR), DebugLoc());
	MBBI->insert(MBBI->end(), NI);
	unsigned NR = MRI->createVirtualRegister(RC);
	NI->addOperand(MachineOperand::CreateReg(NR,true));
	NI->addOperand(MachineOperand::CreateReg(DSP::ZERO, false));
	NI->addOperand(MachineOperand::CreateImm(0));


	MachineInstr* FJ = mf.CreateMachineInstr(TII->get(DSP::JNC), DebugLoc());
	MBBI->insert(MBBI->end(), FJ);
	FJ->addOperand(MachineOperand::CreateReg(S_reg, false, false, true));
	FJ->addOperand(MachineOperand::CreateMBB(SuccBB));

	
	//create Jmp from MBBI to NewBB
	MachineInstr* Jmp = mf.CreateMachineInstr(TII->get(DSP::Jmp), DebugLoc());
	MBBI->insert(MBBI->end(), Jmp);
	Jmp->addOperand(MachineOperand::CreateMBB(NewBB));


	// create instruction in newBB
	//mov sw ,1
	//jmp succBB
	MachineInstr* NewMI = mf.CreateMachineInstr(TII->get(DSP::MovGR),DebugLoc());
	NewBB->insert(NewBB->end(),NewMI);

	unsigned NewReg = MRI->createVirtualRegister(RC);
	NewMI->addOperand(MachineOperand::CreateReg(NewReg, true));
	NewMI->addOperand(MachineOperand::CreateReg(DSP::ZERO, false));
	NewMI->addOperand(MachineOperand::CreateImm(1));

	MachineInstr* NJmp = mf.CreateMachineInstr(TII->get(DSP::Jmp), DebugLoc());
	NewBB->insert(NewBB->end(), NJmp);
	NJmp->addOperand(MachineOperand::CreateMBB(SuccBB));


	//duel with the Phi
	//unsigned ZeroReg = Phi->getOperand(1).getReg();
	for (unsigned int i = 2;i < Phi->getNumOperands();i += 2) {
		if (!Phi->getOperand(i).isMBB()) continue;
		MachineBasicBlock* FBB = Phi->getOperand(i).getMBB();
		if (FBB != MBBI) continue;
		Phi->getOperand(i - 1).setReg(NR);
		break;
	}
	Phi->addOperand(MachineOperand::CreateReg(NewReg, false));
	Phi->addOperand(MachineOperand::CreateMBB(NewBB));
	return true;
}

char DSPCCHandler::ID = 0;




FunctionPass* llvm::createDSPHandlerCCPass() {
	return new DSPCCHandler();
}