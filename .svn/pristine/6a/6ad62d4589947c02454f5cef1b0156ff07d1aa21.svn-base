//===-- DSPFrameLowering.cpp - DSP Frame Information --------------------===//
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
#include "DSPFrameLowering.h"
#include "DSPInstrInfo.h"
#include "DSPMachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetOptions.h"
#include <iostream>
#define DEBUG_TYPE "dsp_frame_info"
using namespace llvm;



	const DSPFrameLowering *DSPFrameLowering::create(const DSPSubtarget &ST) {

		return llvm::createDSPSEFrameLowering(ST);
	}
	//- Must have, hasFP() is pure virtual of parent
	// hasFP - Return true if the specified function should have a dedicated frame
	// pointer register. This is true if the function has variable sized allocas or
	// if frame pointer elimination is disabled.
	bool DSPFrameLowering::hasFP(const MachineFunction &MF) const {
		const MachineFrameInfo *MFI = MF.getFrameInfo();
		return MF.getTarget().Options.DisableFramePointerElim(MF) ||
			MFI->hasVarSizedObjects() || MFI->isFrameAddressTaken();
	}

	uint64_t DSPFrameLowering::estimateStackSize(const MachineFunction &MF) const {
		const MachineFrameInfo *MFI = MF.getFrameInfo();
		const TargetRegisterInfo &TRI = *MF.getTarget().getRegisterInfo();
		int64_t Offset = 0;
		// Iterate over fixed sized objects.
		for (int I = MFI->getObjectIndexBegin(); I != 0; ++I)
			Offset = std::max(Offset, -MFI->getObjectOffset(I));
		// Conservatively assume all callee-saved registers will be saved.
		for (const MCPhysReg *R = TRI.getCalleeSavedRegs(&MF); *R; ++R) {
			unsigned Size = TRI.getMinimalPhysRegClass(*R)->getSize();
			Offset = RoundUpToAlignment(Offset + Size, Size);
		}
		unsigned MaxAlign = MFI->getMaxAlignment();
		DEBUG(dbgs() << "max Align" << MaxAlign << "\n");
		std::cout << "max align" << MaxAlign << std::endl;
		// Check that MaxAlign is not zero if there is a stack object that is not a
		// callee-saved spill.
		assert(!MFI->getObjectIndexEnd() || MaxAlign);
		// Iterate over other objects.
		for (unsigned I = 0, E = MFI->getObjectIndexEnd(); I != E; ++I)
			Offset = RoundUpToAlignment(Offset + MFI->getObjectSize(I), MaxAlign);
		// Call frame.
		if (MFI->adjustsStack() && hasReservedCallFrame(MF))
			Offset = RoundUpToAlignment(Offset + MFI->getMaxCallFrameSize(),
			std::max(MaxAlign, getStackAlignment()));
		return RoundUpToAlignment(Offset, getStackAlignment());
	}

