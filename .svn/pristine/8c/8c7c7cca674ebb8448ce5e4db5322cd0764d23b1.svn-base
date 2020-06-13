//===-- MipsSEISelLowering.cpp - MipsSE DAG Lowering Interface --*- C++ -*-===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Subclass of MipsTargetLowering specialized for mips32/64.
//
//===----------------------------------------------------------------------===//
#include "DSPMachineFunction.h"
#include "DSPSEISelLowering.h"
#include "DSPRegisterInfo.h"
#include "DSPTargetMachine.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
using namespace llvm;
#define DEBUG_TYPE "DSP-isel"


static cl::opt<bool>
EnableDSPTailCalls("enable-DSP-tail-calls", cl::Hidden,
cl::desc("DSP: Enable tail calls."), cl::init(false));


DSPSETargetLowering::DSPSETargetLowering(DSPTargetMachine &TM, const DSPSubtarget &STI)
:DSPTargetLowering(TM, STI){
	addRegisterClass(MVT::i32, &DSP::CPURegsRegClass);
	computeRegisterProperties();
}

const DSPTargetLowering *llvm::createDSPSETargetLowering(DSPTargetMachine &TM, const DSPSubtarget &STI){

	return new DSPSETargetLowering(TM, STI);
}