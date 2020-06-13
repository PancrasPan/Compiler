//===-- DSPSEFrameLowering.cpp - DSP Frame Information ------------------===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the DSP implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//


#include "DSPAnalyzeImmediate.h"
#include "DSPSEFrameLowering.h"
#include "DSPMachineFunction.h"
#include "DSPSEInstrInfo.h"
#include "DSPSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"
#include <iostream>

using namespace llvm;

DSPSEFrameLowering::DSPSEFrameLowering(const DSPSubtarget &STI)
:DSPFrameLowering(STI, STI.stackAlignment()){}




const DSPFrameLowering *llvm::createDSPSEFrameLowering(const DSPSubtarget &ST) {
	return new DSPSEFrameLowering(ST);
}

void DSPSEFrameLowering::emitPrologue(MachineFunction &MF) const {
	MachineBasicBlock &MBB = MF.front();
	MachineFrameInfo *MFI = MF.getFrameInfo();
	DSPFunctionInfo *DSPFI = MF.getInfo<DSPFunctionInfo>();
	const DSPSEInstrInfo &TII =
		*static_cast<const DSPSEInstrInfo*>(MF.getTarget().getInstrInfo());
	MachineBasicBlock::iterator MBBI = MBB.begin();
	DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
	unsigned SP = DSP::SP;
	// First, compute final stack size.
	uint64_t StackSize = MFI->getStackSize();
	// No need to allocate space on the stack.
	if (StackSize == 0 && !MFI->adjustsStack()) return;
	MachineModuleInfo &MMI = MF.getMMI();
	const MCRegisterInfo *MRI = MMI.getContext().getRegisterInfo();
	MachineLocation DstML, SrcML;
	// Adjust stack.

	//DSP doesn't support the minus address, inital sp with 0x000fffff in main block.
	uint64_t Data_Address_Low = 0x0000;
	uint64_t Data_Address_High = 0x0010;
	//std::cout << MBB.getFullName() << std::endl;
	if (MBB.getFullName() == "_main:entry"){
		BuildMI(MBB, MBBI,dl, TII.get(DSP::MovIGL), SP).addReg(SP).addImm(Data_Address_Low);
		BuildMI(MBB, MBBI, dl, TII.get(DSP::MovIGH), SP).addReg(SP).addImm(Data_Address_High);
		BuildMI(MBB, MBBI, dl, TII.get(DSP::MovIGH), DSP::LR).addReg(DSP::LR).addImm(0x0017);
		BuildMI(MBB, MBBI, dl, TII.get(DSP::MovIGL), DSP::LR).addReg(DSP::LR).addImm(0xfff0);
	}
		
	TII.adjustStackPtr(DSPFI, SP, -StackSize, MBB, MBBI);
	// emit ".cfi_def_cfa_offset StackSize"
	unsigned CFIIndex = MMI.addFrameInst(MCCFIInstruction::createDefCfaOffset(nullptr, -StackSize));
	BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
		.addCFIIndex(CFIIndex);
	const std::vector<CalleeSavedInfo> &CSI = MFI->getCalleeSavedInfo();
	if (CSI.size()) {
		// Find the instruction past the last instruction that saves a callee-saved
		// register to the stack.
		for (unsigned i = 0; i < CSI.size(); ++i)
			++MBBI;
		// Iterate over list of callee-saved registers and emit .cfi_offset
		// directives.
		for (std::vector<CalleeSavedInfo>::const_iterator I = CSI.begin(),
			E = CSI.end(); I != E; ++I) {
			int64_t Offset = MFI->getObjectOffset(I->getFrameIdx());
			unsigned Reg = I->getReg();
			{
				// Reg is in CPURegs.
				unsigned CFIIndex = MMI.addFrameInst(MCCFIInstruction::createOffset(
					nullptr, MRI->getDwarfRegNum(Reg, 1), Offset));
				BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
					.addCFIIndex(CFIIndex);
			}
		}
	}
}

void DSPSEFrameLowering::emitEpilogue(MachineFunction &MF,
	MachineBasicBlock &MBB) const {
	MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
	MachineFrameInfo *MFI = MF.getFrameInfo();
	DSPFunctionInfo *DSPFI = MF.getInfo<DSPFunctionInfo>();
	const DSPSEInstrInfo &TII =
		*static_cast<const DSPSEInstrInfo*>(MF.getTarget().getInstrInfo());
	DebugLoc dl = MBBI->getDebugLoc();
	unsigned SP = DSP::SP;
	// Get the number of bytes from FrameInfo
	uint64_t StackSize = MFI->getStackSize();
	if (!StackSize)
		return;
	// Adjust stack.
	TII.adjustStackPtr(DSPFI, SP, StackSize, MBB, MBBI);
}
bool
DSPSEFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
	const MachineFrameInfo *MFI = MF.getFrameInfo();
	// Reserve call frame if the size of the maximum call frame fits into 16-bit
	// immediate field and there are no variable sized objects on the stack.// Make sure the second register scavenger spill slot can be accessed with one
// instruction.
	return isInt<16>(MFI->getMaxCallFrameSize() + getStackAlignment()) &&
		!MFI->hasVarSizedObjects();
}

void DSPSEFrameLowering::
processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
RegScavenger *RS) const {
	MachineRegisterInfo& MRI = MF.getRegInfo();
	// FIXME: remove this code if register allocator can correctly mark
	// $fp and $ra used or unused.
	// The register allocator might determine $ra is used after seeing
	// instruction "jr $ra", but we do not want PrologEpilogInserter to insert
	// instructions to save/restore $ra unless there is a function call.
	// To correct this, $ra is explicitly marked unused if there is no
	// function call.
	if (MF.getFrameInfo()->hasCalls())
		MRI.setPhysRegUsed(DSP::LR);
	else {
		MRI.setPhysRegUnused(DSP::LR);
	}
	return;
}

// Eliminate ADJCALLSTACKDOWN, ADJCALLSTACKUP pseudo instructions
void DSPSEFrameLowering::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
MachineBasicBlock::iterator I) const {
	// Simply discard ADJCALLSTACKDOWN, ADJCALLSTACKUP instructions.
	MBB.erase(I);
	//std::cout << "yes!!" << std::endl;
}