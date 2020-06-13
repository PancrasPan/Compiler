//===-- DSPTargetInfo.cpp - DSP Target Implementation -------------------===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "DSP.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;
Target  llvm::TheDSPelTarget;
extern "C" void LLVMInitializeDSPTargetInfo() {
	RegisterTarget<Triple::dsp,
		/*HasJIT=*/false> X(TheDSPelTarget, "dsp", "DSP");
	
}