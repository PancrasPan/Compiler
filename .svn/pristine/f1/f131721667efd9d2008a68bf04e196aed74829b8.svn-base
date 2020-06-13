//===-- DSPRegisterInfo.cpp - DSP Register Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the DSP implementation of the TargetRegisterInfo class.
// This file is responsible for the frame pointer elimination optimization
// on DSP.
//
//===----------------------------------------------------------------------===//

#include "DSPRegisterInfo.h"
#include "DSPSubtarget.h"
#include "DSPTargetMachine.h"
#include "DSPSEInstrInfo.h"
#include "DSPMachineFunction.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineValueType.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
 
#define   DEBUG_TYPE "dsp-reg-info"


#define GET_REGINFO_TARGET_DESC
#include "DSPGenRegisterInfo.inc"

DSPRegisterInfo::DSPRegisterInfo(const DSPSubtarget &STI):DSPGenRegisterInfo(DSP::LR),
	Subtarget(STI){
	}

//===----------------------------------------------------------------------===//
// Callee Saved Registers methods
//===----------------------------------------------------------------------===//
/// DSP Callee Saved Registers
// In DSPCallConv.td,
// def CSR_O32 : CalleeSavedRegs<(add LR, FP,V0,V1,(sequence "A%u",3,0),
// (sequence "S%u", 7, 0))>;
// llc create CSR_O32_SaveList and CSR_O32_RegMask from above defined.
const uint16_t *DSPRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
	return CSR_O32_SaveList;
}	

const uint32_t *DSPRegisterInfo::getCallPreservedMask(CallingConv::ID CC) const {
	return CSR_O32_RegMask;
}
unsigned DSPRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();
  return TFI->hasFP(MF) ? (DSP::FP) : (DSP::SP);
}




BitVector DSPRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
	static const uint16_t ReservedCPURegs[] = {
		DSP::ZERO,DSP::AT,DSP::SP,DSP::LR,DSP::PC
	};
	BitVector Reserved(getNumRegs());

	typedef TargetRegisterClass::iterator RegIter;
	for (unsigned I = 0; I < array_lengthof(ReservedCPURegs); ++I)
		Reserved.set(ReservedCPURegs[I]);

	if (Subtarget.useSmallSection()){
		Reserved.set(DSP::GP);
	}

	return Reserved;
}

//- If no eliminateFrameIndex(), it will hang on run.
// pure virtual method
// FrameIndex represent objects inside a abstract stack.
// We must replace FrameIndex with an stack/frame pointer
// direct reference.
void DSPRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
unsigned FIOperandNum, RegScavenger *RS) const {
	MachineInstr &MI = *II;
	MachineFunction &MF = *MI.getParent()->getParent();
	MachineFrameInfo *MFI = MF.getFrameInfo();
	DSPFunctionInfo *DSPFI = MF.getInfo<DSPFunctionInfo>();
	unsigned i = 0;
	while (!MI.getOperand(i).isFI()) {
		++i;
		assert(i < MI.getNumOperands() &&"Instr doesn¡¯t have FrameIndex operand!");
	}
	DEBUG(errs() << "\nFunction : " << MF.getFunction()->getName() << "\n";
	errs() << "<--------->\n" << MI);
	int FrameIndex = MI.getOperand(i).getIndex();
	uint64_t stackSize = MF.getFrameInfo()->getStackSize();
	int64_t spOffset = MF.getFrameInfo()->getObjectOffset(FrameIndex);
	DEBUG(errs() << "FrameIndex : " << FrameIndex << "\n"
		<< "spOffset : " << spOffset << "\n"
		<< "stackSize : " << stackSize << "\n");
	const std::vector<CalleeSavedInfo> &CSI = MFI->getCalleeSavedInfo();
	int MinCSFI = 0;
	int MaxCSFI = -1;
	if (CSI.size()) {
		MinCSFI = CSI[0].getFrameIdx();
		MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
	}
	// The following stack frame objects are always referenced relative to $sp:
	// 1. Outgoing arguments.
	// 2. Pointer to dynamically allocated stack space.
	// 3. Locations for callee-saved registers.
	// Everything else is referenced relative to whatever register
	// getFrameRegister() returns.
	unsigned FrameReg;
	FrameReg = DSP::SP;
	// Calculate final offset.
	// - There is no need to change the offset if the frame object is one of the
	// following: an outgoing argument, pointer to a dynamically allocated
	// stack space or a $gp restore location,
	// - If the frame object is any of the following, its offset must be adjusted
	// by adding the size of the stack:
	// incoming argument, callee-saved register location or local variable.
	int64_t Offset;
	bool IsKill;
	Offset = spOffset + (int64_t)stackSize;
	Offset += MI.getOperand(i + 1).getImm();
	DEBUG(errs() << "Offset : " << Offset << "\n" << "<--------->\n");
	
	// If MI is not a debug value, make sure Offset fits in the 16-bit immediate
	// field.
	if (!MI.isDebugValue()) {
		// Make sure Offset fits within the field available.
		// For MSA instructions, this is a 10-bit signed immediate (scaled by
		// element size), otherwise it is a 16-bit signed immediate.
		//unsigned OffsetBitSize = getLoadStoreOffsetSizeInBits(MI.getOpcode());
		//unsigned OffsetAlign = getLoadStoreOffsetAlign(MI.getOpcode());
		DEBUG(errs() << "debug function"<< "\n" << "<--------->\n");
		unsigned OffsetBitSize = 12;
		unsigned OffsetAlign = 4;

		if (OffsetBitSize < 16 && isInt<16>(Offset) &&
			(!isIntN(OffsetBitSize, Offset) ||
			OffsetToAlignment(Offset, OffsetAlign) != 0)) {
			// If we have an offset that needs to fit into a signed n-bit immediate
			// (where n < 16) and doesn't, but does fit into 16-bits then use an ADDiu
			MachineBasicBlock &MBB = *MI.getParent();
			DebugLoc DL = II->getDebugLoc();
			unsigned ADDiu =  DSP::ADDiu;
			const TargetRegisterClass *RC = &DSP::CPURegsRegClass;
			MachineRegisterInfo &RegInfo = MBB.getParent()->getRegInfo();
			unsigned Reg = RegInfo.createVirtualRegister(RC);
			const DSPSEInstrInfo &TII =
				*static_cast<const DSPSEInstrInfo *>(
				MBB.getParent()->getTarget().getInstrInfo());
			BuildMI(MBB, II, DL, TII.get(ADDiu), Reg).addReg(FrameReg).addImm(Offset);

			FrameReg = Reg;
			Offset = 0;
			IsKill = true;
		}
		else if (!isInt<16>(Offset)) {
			// Otherwise split the offset into 16-bit pieces and add it in multiple
			// instructions.
			MachineBasicBlock &MBB = *MI.getParent();
			DebugLoc DL = II->getDebugLoc();
			unsigned ADDu =DSP::ADDu;
			unsigned NewImm = 0;
			const DSPSEInstrInfo &TII =
				*static_cast<const DSPSEInstrInfo *>(
				MBB.getParent()->getTarget().getInstrInfo());
			unsigned Reg = TII.loadImmediate(Offset, MBB, II, DL,
				OffsetBitSize == 16 ? &NewImm : nullptr);
			BuildMI(MBB, II, DL, TII.get(ADDu), Reg).addReg(FrameReg)
				.addReg(Reg, RegState::Kill);

			FrameReg = Reg;
			Offset = SignExtend64<16>(NewImm);
			IsKill = true;
		}
		//assert("(!MI.isDebugValue() && !isInt<16>(Offset))");
	}
	MI.getOperand(i).ChangeToRegister(FrameReg, false);
	MI.getOperand(i + 1).ChangeToImmediate(Offset);
}