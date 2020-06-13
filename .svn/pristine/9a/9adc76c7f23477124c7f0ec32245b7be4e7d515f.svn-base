//===- DSPMCInst.cpp - DSP sub-class of MCInst --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class extends MCInst to allow some DSP VLIW annotations.
//
//===----------------------------------------------------------------------===//

#include "DSPInstrInfo.h"
#include "MCTargetDesc/DSPBaseInfo.h"
#include "MCTargetDesc/DSPMCInst.h"
#include "MCTargetDesc/DSPMCTargetDesc.h"

using namespace llvm;

// Return the slots used by the insn.
unsigned DSPMCInst::getUnits(const DSPTargetMachine* TM) const {
	const DSPInstrInfo* QII = TM->getInstrInfo();
	const InstrItineraryData* II = TM->getInstrItineraryData();
	const InstrStage*
		IS = II->beginStage(QII->get(this->getOpcode()).getSchedClass());
	return (IS->getUnits());
}



// Return the DSP ISA class for the insn.
unsigned DSPMCInst::getType() const {
	const uint64_t F = MCID->TSFlags;

	return ((F >> DSPII::TypePos) & DSPII::TypeMask);
}

// Return whether the insn is an actual insn.
bool DSPMCInst::isCanon() const {
	return (!MCID->isPseudo() &&
		!isPrefix() &&
		getType() != DSPII::FrmENDLOOP);
}

// Return whether the insn is a prefix.
bool DSPMCInst::isPrefix() const {
	return (getType() == DSPII::FrmPREFIX);
}

// Return whether the insn is solo, i.e., cannot be in a packet.
bool DSPMCInst::isSolo() const {
	const uint64_t F = MCID->TSFlags;
	return ((F >> DSPII::SoloPos) & DSPII::SoloMask);
}

// Return whether the insn is a new-value consumer.
bool DSPMCInst::isNewValue() const {
	const uint64_t F = MCID->TSFlags;
	return ((F >> DSPII::NewValuePos) & DSPII::NewValueMask);
}

// Return whether the instruction is a legal new-value producer.
bool DSPMCInst::hasNewValue() const {
	const uint64_t F = MCID->TSFlags;
	return ((F >> DSPII::hasNewValuePos) & DSPII::hasNewValueMask);
}

// Return the operand that consumes or produces a new value.
const MCOperand& DSPMCInst::getNewValue() const {
	const uint64_t F = MCID->TSFlags;
	const unsigned O = (F >> DSPII::NewValueOpPos) &
		DSPII::NewValueOpMask;
	const MCOperand& MCO = getOperand(O);

	assert((isNewValue() || hasNewValue()) && MCO.isReg());
	return (MCO);
}

// Return whether the instruction needs to be constant extended.
// 1) Always return true if the instruction has 'isExtended' flag set.
//
// isExtendable:
// 2) For immediate extended operands, return true only if the value is
//    out-of-range.
// 3) For global address, always return true.

bool DSPMCInst::isConstExtended(void) const {
	if (isExtended())
		return true;

	if (!isExtendable())
		return false;

	short ExtOpNum = getCExtOpNum();
	int MinValue = getMinValue();
	int MaxValue = getMaxValue();
	const MCOperand& MO = getOperand(ExtOpNum);

	// We could be using an instruction with an extendable immediate and shoehorn
	// a global address into it. If it is a global address it will be constant
	// extended. We do this for COMBINE.
	// We currently only handle isGlobal() because it is the only kind of
	// object we are going to end up with here for now.
	// In the future we probably should add isSymbol(), etc.
	if (MO.isExpr())
		return true;

	// If the extendable operand is not 'Immediate' type, the instruction should
	// have 'isExtended' flag set.
	assert(MO.isImm() && "Extendable operand must be Immediate type");

	int ImmValue = MO.getImm();
	return (ImmValue < MinValue || ImmValue > MaxValue);
}

// Return whether the instruction must be always extended.
bool DSPMCInst::isExtended(void) const {
	const uint64_t F = MCID->TSFlags;
	return (F >> DSPII::ExtendedPos) & DSPII::ExtendedMask;
}

// Return true if the instruction may be extended based on the operand value.
bool DSPMCInst::isExtendable(void) const {
	const uint64_t F = MCID->TSFlags;
	return (F >> DSPII::ExtendablePos) & DSPII::ExtendableMask;
}

// Return number of bits in the constant extended operand.
unsigned DSPMCInst::getBitCount(void) const {
	const uint64_t F = MCID->TSFlags;
	return ((F >> DSPII::ExtentBitsPos) & DSPII::ExtentBitsMask);
}

// Return constant extended operand number.
unsigned short DSPMCInst::getCExtOpNum(void) const {
	const uint64_t F = MCID->TSFlags;
	return ((F >> DSPII::ExtendableOpPos) & DSPII::ExtendableOpMask);
}

// Return whether the operand can be constant extended.
bool DSPMCInst::isOperandExtended(const unsigned short OperandNum) const {
	const uint64_t F = MCID->TSFlags;
	return ((F >> DSPII::ExtendableOpPos) & DSPII::ExtendableOpMask)
		== OperandNum;
}

// Return the min value that a constant extendable operand can have
// without being extended.
int DSPMCInst::getMinValue(void) const {
	const uint64_t F = MCID->TSFlags;
	unsigned isSigned = (F >> DSPII::ExtentSignedPos)
		& DSPII::ExtentSignedMask;
	unsigned bits = (F >> DSPII::ExtentBitsPos)
		& DSPII::ExtentBitsMask;

	if (isSigned) // if value is signed
		return -1 << (bits - 1);
	else
		return 0;
}

// Return the max value that a constant extendable operand can have
// without being extended.
int DSPMCInst::getMaxValue(void) const {
	const uint64_t F = MCID->TSFlags;
	unsigned isSigned = (F >> DSPII::ExtentSignedPos)
		& DSPII::ExtentSignedMask;
	unsigned bits = (F >> DSPII::ExtentBitsPos)
		& DSPII::ExtentBitsMask;

	if (isSigned) // if value is signed
		return ~(-1 << (bits - 1));
	else
		return ~(-1 << bits);
}
