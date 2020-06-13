//===-- DSPSEInstrInfo.cpp - DSP32/64 Instruction Information -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the DSP32/64 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "DSPSEInstrInfo.h"
#include "DSPTargetMachine.h"
#include "DSPMachineFunction.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"
#include <iostream>


using namespace llvm;

DSPSEInstrInfo::DSPSEInstrInfo(const DSPSubtarget &STI)
	:DSPInstrInfo(STI),RI(STI){}

const DSPRegisterInfo &DSPSEInstrInfo::getRegisterInfo() const {
	return RI;
}

const DSPInstrInfo *llvm::createDSPSEInstrInfo(const DSPSubtarget &STI){
	return new DSPSEInstrInfo(STI);
}

static MachineMemOperand* GetMemOperand(MachineBasicBlock &MBB, int FI,
	unsigned Flag) {
	MachineFunction &MF = *MBB.getParent();
	MachineFrameInfo &MFI = *MF.getFrameInfo();
	unsigned Align = MFI.getObjectAlignment(FI);
	return MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FI), Flag, MFI.getObjectSize(FI), Align);
}

void DSPSEInstrInfo::storeRegToStack(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
unsigned SrcReg, bool isKill, int FI,
const TargetRegisterClass *RC, const TargetRegisterInfo *TRI,
int64_t Offset) const {
	std::cout << "store reg" << std::endl;
	DebugLoc DL;
	if (I != MBB.end()) DL = I->getDebugLoc();
	MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOStore);
	unsigned Opc = 0;
	if (DSP::CPU512RegsRegClass.hasSubClassEq(RC))//ppp
		std::cout << "vector type!!!!!!!!!!" << std::endl;
	Opc = DSP::ST;
	assert(Opc && "Register class not handled!");
	BuildMI(MBB, I, DL, get(Opc)).addReg(SrcReg, getKillRegState(isKill))
		.addFrameIndex(FI).addImm(0).addMemOperand(MMO);
}
void DSPSEInstrInfo::loadRegFromStack(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
unsigned DestReg, int FI, const TargetRegisterClass *RC,
const TargetRegisterInfo *TRI, int64_t Offset) const {
	DebugLoc DL;
	if (I != MBB.end()) DL = I->getDebugLoc();
	MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOLoad);
	unsigned Opc = 0;
	Opc = DSP::LD;
	assert(Opc && "Register class not handled!");
	BuildMI(MBB, I, DL, get(Opc), DestReg).addFrameIndex(FI).addImm(0)
		.addMemOperand(MMO);
}

// DSPInstrInfo::expandPostRAPseudo
/// Expand Pseudo instructions into real backend instructions
bool DSPSEInstrInfo::expandPostRAPseudo(MachineBasicBlock::iterator MI) const {
	MachineBasicBlock &MBB = *MI->getParent();
	switch (MI->getDesc().getOpcode()) {
	default:
		return false;
	case DSP::RetLR:
		ExpandRetLR(MBB, MI, DSP::Ret); break;
	//case DSP::MovVR:
	//	ExpandMovVR(MBB, MI, DSP::MovG2V40);
	//	break;
	case DSP::MovGR:
		ExpandMovGR(MBB, MI, DSP::MovIGH, DSP::MovIGL); break;
	case DSP::LEA:
		ExpandLEA(MBB, MI, DSP::ADDiu); break;
	}
	MBB.erase(MI);
	return true;
}
/// Adjust SP by Amount bytes.
void DSPSEInstrInfo::adjustStackPtr(DSPFunctionInfo *DSPFI, unsigned SP,
	int64_t Amount, MachineBasicBlock &MBB,
	MachineBasicBlock::iterator I) const {
	const DSPSubtarget &STI = Subtarget;
	DebugLoc DL = I != MBB.end() ? I->getDebugLoc() : DebugLoc();
	unsigned ADDu = DSP::ADDu;
	unsigned ADDiu = DSP::ADDiu;
	if (isInt<16>(Amount))// addiu sp, sp, amount
		BuildMI(MBB, I, DL, get(ADDiu), SP).addReg(SP).addImm(Amount);
	else { // Expand immediate that doesn¡¯t fit in 16-bit.
		DSPFI->setEmitNOAT();
		unsigned Reg = loadImmediate(Amount, MBB, I, DL, nullptr);
		BuildMI(MBB, I, DL, get(ADDu), SP).addReg(SP).addReg(Reg);
	}
}

/// This function generates the sequence of instructions needed to get the
/// result of adding register REG and immediate IMM.
unsigned
DSPSEInstrInfo::loadImmediate(int64_t Imm, MachineBasicBlock &MBB,
MachineBasicBlock::iterator II, DebugLoc DL,
unsigned *NewImm) const {
	DSPAnalyzeImmediate AnalyzeImm;
	const DSPSubtarget &STI = Subtarget;
	MachineRegisterInfo &RegInfo = MBB.getParent()->getRegInfo();
	unsigned Size = 32;
	unsigned LUi = DSP::LUi;
	unsigned ZEROReg = DSP::ZERO;
	unsigned ATReg = DSP::AT;
	const TargetRegisterClass *RC = &DSP::CPURegsRegClass;
	bool LastInstrIsADDiu = NewImm;
	const DSPAnalyzeImmediate::InstSeq &Seq =
		AnalyzeImm.Analyze(Imm, Size, LastInstrIsADDiu);
	DSPAnalyzeImmediate::InstSeq::const_iterator Inst = Seq.begin();
	assert(Seq.size() && (!LastInstrIsADDiu || (Seq.size() > 1)));
	// The first instruction can be a LUi, which is different from other
	// instructions (ADDiu, ORI and SLL) in that it does not have a register
	// operand.
	if (Inst->Opc == LUi)
		BuildMI(MBB, II, DL, get(LUi), ATReg).addImm(SignExtend64<16>(Inst->ImmOpnd));
	else
		BuildMI(MBB, II, DL, get(Inst->Opc), ATReg).addReg(ZEROReg)
		.addImm(SignExtend64<16>(Inst->ImmOpnd));
	// Build the remaining instructions in Seq.
	for (++Inst; Inst != Seq.end() - LastInstrIsADDiu; ++Inst)
		BuildMI(MBB, II, DL, get(Inst->Opc), ATReg).addReg(ATReg)
		.addImm(SignExtend64<16>(Inst->ImmOpnd));
	if (LastInstrIsADDiu)
		*NewImm = Inst->ImmOpnd;
	return ATReg;
}


void DSPSEInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
	MachineBasicBlock::iterator MI, DebugLoc DL,
	unsigned DestReg, unsigned SrcReg,
	bool KillSrc) const {
	unsigned Opc = DSP::MovG2G;
    MachineInstrBuilder MIB = BuildMI(MBB, MI, DL, get(Opc));
	std::cout <<"copy phy reg" << std::endl;
	if (DestReg)
		MIB.addReg(DestReg, RegState::Define);

	if (SrcReg)
		MIB.addReg(SrcReg, getKillRegState(KillSrc));
	return;
}

void DSPSEInstrInfo::ExpandRetLR(MachineBasicBlock &MBB,
	MachineBasicBlock::iterator I,
	unsigned Opc) const {

	BuildMI(MBB, I, I->getDebugLoc(), get(Opc),DSP::LR);	
}

void DSPSEInstrInfo::ExpandMovVR(MachineBasicBlock &MBB,
	MachineBasicBlock::iterator I,
	unsigned Opc) const {
	unsigned SrcReg = 0 ;
	unsigned DesReg = 0;
	//judge if four SrcRegs are same
	bool flag = 0; 

	for (int j = 2; j < 5; j++){
		if (I->getOperand(1).getReg() == I->getOperand(j).getReg())
			flag = true;
		else flag = false;
	}
	//std::cout <<"flag is"<< flag << std::endl;
	
	//unsigned num = I->getNumOperands();
	//std::cout << num << std::endl;
	DesReg = I->getOperand(0).getReg();
	if (flag){
		unsigned Op = DSP::VMovG2V40;
		SrcReg = I->getOperand(1).getReg();
		BuildMI(MBB, I, I->getDebugLoc(), get(Op),DesReg).addReg(SrcReg);
	}
	else 
	{
		for (int i = 0; i < 4; i++){
			SrcReg = I->getOperand(i + 1).getReg();
			//std::cout << "srcreg" << SrcReg << std::endl;
			BuildMI(MBB, I, I->getDebugLoc(), get(Opc)).addReg(DesReg).addReg(SrcReg).addImm(i);
		}
	}
	
}

void DSPSEInstrInfo::ExpandMovGR(MachineBasicBlock &MBB,
	MachineBasicBlock::iterator I,
	unsigned Opc1,unsigned Opc2) const {
	unsigned SrcReg = 0;
	unsigned DesReg = 0;
	unsigned num = I->getNumOperands();
	MachineFunction &MF = *MBB.getParent();
	DesReg = I->getOperand(0).getReg();
	SrcReg = I->getOperand(1).getReg();
	unsigned  MOTy = I->getOperand(2).getType();
	int imm = 0;

	//std::cout << "In DSPSEInstrInfo.cpp" << std::endl;

	
	switch (MOTy)
	{
	default:llvm_unreachable("wrong type");
		break;
	case MachineOperand::MO_ConstantPoolIndex:{
												 //std::cout << "constant pool" << std::endl;
												  BuildMI(MBB, I, I->getDebugLoc(), get(Opc2), DesReg).addOperand(I->getOperand(1)).addOperand(I->getOperand(2));
												  //BuildMI(MBB, I, I->getDebugLoc(), get(Opc2), DesReg).addOperand(I->getOperand(2));
												  //std::cout << "Lo is" << imm << std::endl;
												  BuildMI(MBB, I, I->getDebugLoc(), get(Opc1), DesReg).addOperand(I->getOperand(1)).addOperand(I->getOperand(2));
												  //BuildMI(MBB, I, I->getDebugLoc(), get(Opc1), DesReg).addOperand(I->getOperand(2));
												 // std::cout << "Hi is" << imm << std::endl;
	}  break;
	case MachineOperand::MO_Immediate:{
										 // std::cout << "immediate" << std::endl;
										  imm = I->getOperand(2).getImm();
										  short Hi = (imm) >> 16;
										  short Lo = (imm);
										 BuildMI(MBB, I, I->getDebugLoc(), get(Opc2), DesReg).addReg(SrcReg).addImm(Lo);
										  //BuildMI(MBB, I, I->getDebugLoc(), get(Opc2), DesReg).addImm(Lo);
										 // std::cout << "Lo is" << Lo << std::endl;
										  BuildMI(MBB, I, I->getDebugLoc(), get(Opc1), DesReg).addReg(SrcReg).addImm(Hi);
										  //BuildMI(MBB, I, I->getDebugLoc(), get(Opc1), DesReg).addImm(Hi);
										 // std::cout << "Hi is" << Hi << std::endl;
									
	}	  break;
	}
}


void DSPSEInstrInfo::ExpandLEA(MachineBasicBlock &MBB, MachineBasicBlock::iterator I, unsigned int Opc) const {
	unsigned int DesReg = I->getOperand(0).getReg();
	unsigned int SrcReg = I->getOperand(1).getReg();
	unsigned int Imm = I->getOperand(2).getImm();
	BuildMI(MBB, I, I->getDebugLoc(), get(Opc), DesReg).addReg(SrcReg).addImm(Imm);
}