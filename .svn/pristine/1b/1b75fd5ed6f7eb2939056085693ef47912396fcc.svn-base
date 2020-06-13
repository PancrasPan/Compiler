//===-- DSPFixupKinds.h - DSP Specific Fixup Entries ----------*- C++ -*-===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_DSP_DSPFIXUPKINDS_H
#define LLVM_DSP_DSPFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"
namespace llvm {
	namespace DSP{
		enum Fixups{
			// Pure upper 32 bit fixup resulting in - R_DSP_32.
			fixup_DSP_32 = FirstTargetFixupKind,
			// Pure upper 16 bit fixup resulting in - R_DSP_HI16.
			fixup_DSP_HI16,
			// Pure lower 16 bit fixup resulting in - R_DSP_LO16.
			fixup_DSP_LO16,
			// Pure lower 16 bit fixup resulting in - R_DSP_GPREL16.
			fixup_DSP_GPREL16,
			// Global symbol fixup resulting in - R_DSP_GOT16.
			fixup_DSP_GOT_Global,
			// Local symbol fixup resulting in - R_DSP_GOT16.
			fixup_DSP_GOT_Local,
			// resulting in - R_DSP_GOT_HI16
			fixup_DSP_GOT_HI16,
			// resulting in - R_DSP_GOT_LO16
			fixup_DSP_GOT_LO16,

			// PC relative branch fixup resulting in - R_CPU0_PC16.
			// cpu0 PC16, e.g. beq
			//fixup_DSP_PC16,

			//fixup_DSP_PC21,
			

			// resulting in - R_CPU0_CALL16.
			fixup_DSP_CALL,

			//using in Mips linker
			fixup_Mips_PC21_S2,

			fixup_Mips_PC26_S2,

			// PC relative branch fixup resulting in - R_CPU0_PC24.
			// cpu0 PC24, e.g. jeq, jmp
			//fixup_DSP_PC24,
			//For JC, JNC, JMP
			fixup_DSP_PC26,
			//For Loop
			fixup_DSP_PC16,
			// Marker
			LastTargetFixupKind,
			NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind

		};
	}

}
#endif