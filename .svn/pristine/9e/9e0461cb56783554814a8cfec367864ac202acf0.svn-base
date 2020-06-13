//===-- DSPRegisterInfo.h - DSP Register Information Impl -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the DSP implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef DSPREGISTERINFO_H
#define DSPREGISTERINFO_H

#include "DSP.h"
#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "DSPGenRegisterInfo.inc"

namespace llvm {
	class Type;
	class TargetInstrInfo;
	class DSPSubtarget;


class DSPRegisterInfo : public DSPGenRegisterInfo{
	public:
		const DSPSubtarget &Subtarget;
		DSPRegisterInfo(const DSPSubtarget &Subtarget);
	public:
		void eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

		static unsigned getRegisterNumbering(unsigned RegNum);
		unsigned getFrameRegister(const MachineFunction &MF) const override;
		

		/// getCalleeSavedRegs - Return a null-terminated list of all of the
  		/// callee-save registers on this target.
		const uint16_t *getCalleeSavedRegs(const MachineFunction* MF = 0) const override;
  		const uint32_t *getCallPreservedMask(CallingConv::ID) const override;

  		
  		/// getReservedRegs - Returns a bitset indexed by physical register number
  		/// indicating if a register is a special register that has particular uses and
  		/// should be considered unavailable at all times, e.g. SP, RA. This is used by
  		/// register scavenger to determine what registers are free.
  		BitVector getReservedRegs(const MachineFunction &MF) const override;

	virtual const TargetRegisterClass *intRegClass(unsigned Size)const  = 0;
};

}
#endif