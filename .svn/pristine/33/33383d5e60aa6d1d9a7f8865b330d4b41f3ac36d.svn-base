//===-- DSPMCAsmInfo.h - DSP Asm Info ------------------------*- C++ -*--===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the DSPMCAsmInfo class.
//
//===----------------------------------------------------------------------===//
#ifndef DSPTARGETASMINFO_H
#define DSPTARGETASMINFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
class StringRef;
class Target;

class DSPMCAsmInfo : public MCAsmInfo {
virtual void anchor();
public:
explicit DSPMCAsmInfo(StringRef TT);
};
} // namespace llvm
#endif