//===-- DSPAsmBackend.h - DSP Asm Backend ------------------------------===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the DSPAsmBackend class.
//
//===----------------------------------------------------------------------===//
//
#ifndef DSPASMBACKEND_H
#define DSPASMBACKEND_H


#include "MCTargetDesc/DSPFixupKinds.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/ADT/Triple.h"

namespace llvm {
	class MCAssembler;
	struct MCFixupKindInfo;
	class Target;
	class MCObjectWriter;


	class DSPAsmBackend :public MCAsmBackend{
		Triple::OSType OSType;
		bool IsLittle;

	public:
		DSPAsmBackend(const Target &T, Triple::OSType _OSType, bool _IsLittle)
			:MCAsmBackend(), OSType(_OSType), IsLittle(_IsLittle){}

		MCObjectWriter *createObjectWriter(raw_ostream &OS) const override;

		void applyFixup(const MCFixup &Fixup, char *Data, unsigned DataSize,
			uint64_t Value, bool IsPCRel) const override;

		const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override;

		unsigned getNumFixupKinds() const override {
			return DSP::NumTargetFixupKinds;
		}

		/// @name Target Relaxation Interfaces
		/// @{
		/// MayNeedRelaxation - Check whether the given instruction may need
		/// relaxation.
		///
		/// \param Inst - The instruction to test.
		bool mayNeedRelaxation(const MCInst &Inst) const override {
			return false;
		}
		/// fixupNeedsRelaxation - Target specific predicate for whether a given
		/// fixup requires the associated instruction to be relaxed.
		bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
			const MCRelaxableFragment *DF,
			const MCAsmLayout &Layout) const override {
			// FIXME.
			llvm_unreachable("RelaxInstruction() unimplemented");
			return false;
		}

		/// RelaxInstruction - Relax the instruction in the given fragment
		/// to the next wider instruction.
		///
		/// \param Inst - The instruction to relax, which may be the same
		/// as the output.
		/// \param [out] Res On return, the relaxed instruction.
		void relaxInstruction(const MCInst &Inst, MCInst &Res) const override {}

		bool writeNopData(uint64_t Count, MCObjectWriter *OW) const override;
	};
}


#endif