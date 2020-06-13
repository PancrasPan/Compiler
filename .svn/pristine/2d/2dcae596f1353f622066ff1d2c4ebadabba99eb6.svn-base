//===-- DSPMachineFuctionInfo.cpp - DSP machine function info -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "DSPMachineFunction.h"
#include "DSPInstrInfo.h"
#include "DSPSubtarget.h"
#include "llvm/IR/Function.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/raw_ostream.h"


using namespace llvm;
bool FixGlobalBaseReg;

// class DSPCallEntry.
DSPCallEntry::DSPCallEntry(const StringRef &N) {
#ifndef NDEBUG
	Name = N;
	Val = nullptr;
#endif
}

DSPCallEntry::DSPCallEntry(const GlobalValue *V) {
#ifndef NDEBUG
	Val = V;
#endif
}

bool DSPCallEntry::isConstant(const MachineFrameInfo *) const {
	return false;
}

bool DSPCallEntry::isAliased(const MachineFrameInfo *) const {
	return false;
}

bool DSPCallEntry::mayAlias(const MachineFrameInfo *) const {
	return false;
}

void DSPCallEntry::printCustom(raw_ostream &O) const {
	O << "DSPCallEntry: ";
#ifndef NDEBUG
	if (Val)

		O << Val->getName();
	else
		O << Name;
#endif
}

DSPFunctionInfo::~DSPFunctionInfo() {
	for (StringMap<const DSPCallEntry *>::iterator
		I = ExternalCallEntries.begin(), E = ExternalCallEntries.end(); I != E;
		++I)
		delete I->getValue();
	for (const auto &Entry : GlobalCallEntries)
		delete Entry.second;
}
bool DSPFunctionInfo::globalBaseRegFixed() const {
	return FixGlobalBaseReg;
}

bool DSPFunctionInfo::globalBaseRegSet() const {
	return GlobalBaseReg;
}

unsigned DSPFunctionInfo::getGlobalBaseReg() {
	return GlobalBaseReg = DSP::GP;
}
MachinePointerInfo DSPFunctionInfo::callPtrInfo(const StringRef &Name) {
	const DSPCallEntry *&E = ExternalCallEntries[Name];

	if (!E)
		E = new DSPCallEntry(Name);

	return MachinePointerInfo(E);
}

MachinePointerInfo DSPFunctionInfo::callPtrInfo(const GlobalValue *Val) {
	const DSPCallEntry *&E = GlobalCallEntries[Val];

	if (!E)
		E = new DSPCallEntry(Val);

	return MachinePointerInfo(E);
}

void DSPFunctionInfo::anchor(){}
