//===-- DSPSERegisterInfo.cpp - DSP Register Information ------== -------===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the DSP implementation of the TargetRegisterInfo
// class.
//
//===----------------------------------------------------------------------===//
#ifndef DSPSEREGISTERINFO_H
#define DSPSEREGISTERINFO_H

#include "DSPRegisterInfo.h"

namespace llvm {
	class DSPSEInstrInfo;
	class DSPSERegisterInfo :public DSPRegisterInfo{
	public:
		DSPSERegisterInfo(const DSPSubtarget &STI);
		const TargetRegisterClass *intRegClass(unsigned Size) const override;

	};


}
#endif