//===-- DSPBaseInfo.h - Top level definitions for DSP MC ------*- C++ -*-===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone helper functions and enum definitions for
// the DSP target useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//
#ifndef DSPBASEINFO_H
#define DSPBASEINFO_H
#include "DSPMCTargetDesc.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/ErrorHandling.h"
#include "DSPFixupKinds.h"

namespace llvm {

	namespace DSPII{

		enum TOF {
			//===------------------------------------------------------------------===//
			// DSP Specific MachineOperand flags.
			MO_NO_FLAG,
			/// MO_GOT_CALL - Represents the offset into the global offset table at
			/// which the address of a call site relocation entry symbol resides
			/// during execution. This is different from the above since this flag
			/// can only be present in call instructions.
			MO_GOT_CALL,
			/// MO_GPREL - Represents the offset from the current gp value to be used
			/// for the relocatable object file being produced.
			MO_GPREL,
			/// MO_ABS_HI/LO - Represents the hi or low part of an absolute symbol
			/// address.
			MO_ABS_HI,
			MO_ABS_LO,
			MO_GOT_DISP,
			MO_GOT_PAGE,
			MO_GOT_OFST,
			// N32/64 Flags.
			MO_GPOFF_HI,
			MO_GPOFF_LO,
			/// MO_GOT_HI16/LO16 - Relocations used for large GOTs.
			MO_GOT_HI16,
			MO_GOT_LO16,
			MO_GOT16,
			MO_GOT
		}; // enum TOF {

		enum
		{
			// Pseudo - This represents an instruction that is a pseudo instruction
			// or one that has not been implemented yet. It is illegal to code generate
			// it, but tolerated for intermediate implementation stages.
			Pseudo = 0,
			FrmR0 = 1,
			FrmR1 = 2,
			FrmR2 = 3,
			FrmR3 = 4,
			FrmOther = 5,

			FrmMask = 15,

			FrmPREFIX = 30,
			FrmENDLOOP = 31
		};

		// MCInstrDesc TSFlags
		// *** Must match DSPInstrFormat*.td ***
		enum {
			// This 5-bit field describes the insn type.
			TypePos = 0,
			TypeMask = 0x1f,

			// Solo instructions.
			SoloPos = 5,
			SoloMask = 0x1,
			// Packed only with A or X-type instructions.
			SoloAXPos = 6,
			SoloAXMask = 0x1,
			// Only A-type instruction in first slot or nothing.
			SoloAin1Pos = 7,
			SoloAin1Mask = 0x1,

			// Predicated instructions.
			PredicatedPos = 8,
			PredicatedMask = 0x1,
			PredicatedFalsePos = 9,
			PredicatedFalseMask = 0x1,
			PredicatedNewPos = 10,
			PredicatedNewMask = 0x1,
			PredicateLatePos = 11,
			PredicateLateMask = 0x1,

			// New-Value consumer instructions.
			NewValuePos = 12,
			NewValueMask = 0x1,
			// New-Value producer instructions.
			hasNewValuePos = 13,
			hasNewValueMask = 0x1,
			// Which operand consumes or produces a new value.
			NewValueOpPos = 14,
			NewValueOpMask = 0x7,
			// Stores that can become new-value stores.
			mayNVStorePos = 17,
			mayNVStoreMask = 0x1,
			// New-value store instructions.
			NVStorePos = 18,
			NVStoreMask = 0x1,
			// Loads that can become current-value loads.
			mayCVLoadPos = 19,
			mayCVLoadMask = 0x1,
			// Current-value load instructions.
			CVLoadPos = 20,
			CVLoadMask = 0x1,

			// Extendable insns.
			ExtendablePos = 21,
			ExtendableMask = 0x1,
			// Insns must be extended.
			ExtendedPos = 22,
			ExtendedMask = 0x1,
			// Which operand may be extended.
			ExtendableOpPos = 23,
			ExtendableOpMask = 0x7,
			// Signed or unsigned range.
			ExtentSignedPos = 26,
			ExtentSignedMask = 0x1,
			// Number of bits of range before extending operand.
			ExtentBitsPos = 27,
			ExtentBitsMask = 0x1f,
			// Alignment power-of-two before extending operand.
			ExtentAlignPos = 32,
			ExtentAlignMask = 0x3,

			// Valid subtargets
			validSubTargetPos = 34,
			validSubTargetMask = 0xf,

			// Addressing mode for load/store instructions.
			AddrModePos = 40,
			AddrModeMask = 0x7,
			// Access size for load/store instructions.
			MemAccessSizePos = 43,
			MemAccesSizeMask = 0x7,

			// Branch predicted taken.
			TakenPos = 47,
			TakenMask = 0x1,

			// Floating-point instructions.
			FPPos = 48,
			FPMask = 0x1,
			//using which slot
			SlotPos = 49,
			SlotMask = 0x3,

			//is a VLIW start or not
			StartPos = 50,
			StartMask = 0x1,

			isAluPos = 52,
			isAluMask = 0x1

		};
	}
		inline static unsigned getDSPRegisterNumbering(unsigned RegEnum){

			switch (RegEnum)
			{
			default:llvm_unreachable("unknown register");
				break;
			case DSP::ZERO:
				return 0;
			case DSP::AT:
				return 1;
			case DSP::V0:
				return 2;
			case DSP::V1:
				return 3;
			case DSP::A0:
				return 4;
			case DSP::A1:
				return 5;
			case DSP::A2:
				return 6;
			case DSP::A3:
				return 7;
			case DSP::T0:
				return 8;
			case DSP::T1:
				return 9;
			case DSP::T2:
				return 10;
			case DSP::T3:
				return 11;
			case DSP::T4:
				return 12;
			case DSP::T5:
				return 13;
			case DSP::T6:
				return 14;
			case DSP::T7:
				return 15;
			case DSP::S0:
				return 16;
			case DSP::S1:
				return 17;
			case DSP::S2:
				return 18;
			case DSP::S3:
				return 19;
			case DSP::S4:
				return 20;
			case DSP::S5:
				return 21;
			case DSP::S6:
				return 22;
			case DSP::S7:
				return 23;
			case DSP::T8:
				return 24;
			case DSP::T9:
				return 25;
			case DSP::RA:
				return 26;
			case DSP::SW:
				return 27;
			case DSP::GP:
				return 28;
			case DSP::FP:
				return 29;
			case DSP::SP:
				return 30;
			case DSP::LR:
				return 31;
			case DSP::HI:
				return 32;
			case DSP::LO:
				return 33;
			case DSP::PC:
				return 0;
			case DSP::EPC:
				return 1;
			case DSP::OFF0:
				return 0;
			case DSP::OFF1:
				return 1;
			case DSP::OFF2:
				return 2;
			case DSP::OFF3:
				return 3;
			case DSP::BAR0:
				return 4;
			case DSP::BAR1:
				return 5;
			case DSP::BAR2:
				return 6;
			case DSP::BAR3:
				return 7;
			case DSP::MR0:
				return 8;
			case DSP::MR1:
				return 9;
			case DSP::MR2:
				return 10;
			case DSP::MR3:
				return 11;
			case DSP::VR0:
				return 0;
			case DSP::VR1:
				return 1;
			case DSP::VR2:
				return 2;
			case DSP::VR3:
				return 3;
			case DSP::VR4:
				return 4;
			case DSP::VR5:
				return 5;
			case DSP::VR6:
				return 6;
			case DSP::VR7:
				return 7;
			case DSP::VR8:
				return 8;
			case DSP::VR9:
				return 9;
			case DSP::VR10:
				return 10;
			case DSP::VR11:
				return 11;
			case DSP::VR12:
				return 12;
			case DSP::VR13:
				return 13;
			case DSP::VR14:
				return 14;
			case DSP::VR15:
				return 15;
			}

		}


}

#endif