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

#include "DSPSERegisterInfo.h"
using namespace llvm;
#define DEBUG_TYPE "dsp-reg-info"

DSPSERegisterInfo::DSPSERegisterInfo(const DSPSubtarget &STI) :DSPRegisterInfo(STI){}
const  TargetRegisterClass *DSPSERegisterInfo::intRegClass(unsigned Size)const {
	return &DSP::CPURegsRegClass;
}