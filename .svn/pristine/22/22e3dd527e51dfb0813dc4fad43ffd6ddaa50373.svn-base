//===-- DSP.h - Top-level interface for DSP representation ----*- C++ -*-===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM DSP back-end.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_DSP_H
#define TARGET_DSP_H
#include "MCTargetDesc/DSPMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"
namespace llvm {
	namespace DSP{
		enum SlotsState{
			NO_AVA,				//all slots are run out of 0x0000
			SLOT0_ONLY,			//slot0 ava 0x0001
			SLOT1,				//slot1 ava 0x0010
			SLOT1_0,			//slot0 slot1 0x0011
			SLOT2,				//......
			SLOT2_0,
			SLOT2_1,
			SLOT2_1_0,
			SLOT3,
			SLOT3_0,
			SLOT3_1,
			SLOT3_1_0,
			SLOT3_2,
			SLOT3_2_0,
			SLOT3_2_1,
			ALL_AVA				//slot0 slot1 slot2 slot3 0x1111
		};
	}
class DSPTargetMachine;
class FunctionPass;
class DSPMCInst;
FunctionPass *createDSPISelDag(DSPTargetMachine &TM);

#ifdef ENABLE_GPRESTORE
FunctionPass *createDSPEmitGPRestorePass(DSPTargetMachine &TM);
#endif
FunctionPass *createDSPHardwareLoops();
FunctionPass *createDSPVLIWBundlerDrive(TargetMachine &TM);
FunctionPass *createDSPPacketizer();
FunctionPass *createDSPDelaySlotFillerPass(DSPTargetMachine &TM);
FunctionPass *createDSPDelJmpPass(DSPTargetMachine &TM);
FunctionPass *createDSPFixupHwLoops();
FunctionPass *createDSPHandlerCCPass();
FunctionPass *createDSPMemEst();

} // end namespace llvm;
#endif
