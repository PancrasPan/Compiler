//===-- DSPELFObjectWriter.cpp - DSP ELF Writer -------------------------===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/DSPBaseInfo.h"
#include "MCTargetDesc/DSPFixupKinds.h"
#include "MCTargetDesc/DSPMCTargetDesc.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include <list>
#include <iostream>
namespace llvm {

	namespace {
		class DSPELFObjectWriter : public MCELFObjectTargetWriter {
		public:
			DSPELFObjectWriter(uint8_t OSABI);
			virtual ~DSPELFObjectWriter();
			unsigned GetRelocType(const MCValue &Target, const MCFixup &Fixup,
				bool IsPCRel) const override;
			bool needsRelocateWithSymbol(const MCSymbolData &SD,
				unsigned Type) const override;
		};
	}
	DSPELFObjectWriter::DSPELFObjectWriter(uint8_t OSABI)
		: MCELFObjectTargetWriter(/*_is64Bit=false*/ false, OSABI, ELF::EM_DSP,
		/*HasRelocationAddend*/ false) {}
	DSPELFObjectWriter::~DSPELFObjectWriter() {}
	unsigned DSPELFObjectWriter::GetRelocType(const MCValue &Target,
		const MCFixup &Fixup,
		bool IsPCRel) const {
		// determine the type of the relocation
		unsigned Type = (unsigned)ELF::R_DSP_NONE;
		unsigned Kind = (unsigned)Fixup.getKind();
		//std::cout << "roc" << " " << Kind << std::endl;
		switch (Kind) {
		default:
			llvm_unreachable("invalid fixup kind!");
		case FK_Data_1:
			Type = ELF::R_DSP_32;
			break;
		case FK_Data_2:
			Type = ELF::R_DSP_32;
			break;
		case FK_Data_4:
			Type = ELF::R_DSP_32;
			break;
		case FK_GPRel_4:
			Type = ELF::R_DSP_GPREL32;
			break;
		case DSP::fixup_DSP_32:
			Type = ELF::R_DSP_32;
			break;
		case DSP::fixup_DSP_GPREL16:
			Type = ELF::R_DSP_GPREL16;
			break;
		case DSP::fixup_DSP_GOT_Global:
		case DSP::fixup_DSP_GOT_Local:
			Type = ELF::R_DSP_GOT16;
			break;
		case DSP::fixup_DSP_HI16:
			Type = ELF::R_DSP_HI16;
			break;
		case DSP::fixup_DSP_LO16:
			Type = ELF::R_DSP_LO16;
			break;
		case DSP::fixup_DSP_GOT_HI16:
			Type = ELF::R_DSP_GOT_HI16;
			break;
		case DSP::fixup_DSP_GOT_LO16:
			Type = ELF::R_DSP_GOT_LO16;
			break;
		case DSP::fixup_DSP_CALL:
			Type = ELF::R_CPU0_CALL16;
			break;
		/*case DSP::fixup_DSP_PC16:
			Type = ELF::R_DSP_PC16;
			break;
		case DSP::fixup_DSP_PC21:
			Type = ELF::R_DSP_PC21;
			break;
		case DSP::fixup_DSP_PC24:
			Type = ELF::R_DSP_PC24;
			break;*/
		case DSP::fixup_DSP_PC26:
			Type = ELF::R_DSP_PC26;
			break;
		case DSP::fixup_Mips_PC21_S2:
			Type = ELF::R_MIPS_PC21_S2;
			break;
		case DSP::fixup_Mips_PC26_S2:
			Type = ELF::R_MIPS_PC26_S2;
			break;
		}
		return Type;
	}

	bool DSPELFObjectWriter::needsRelocateWithSymbol(const MCSymbolData &SD,
		unsigned Type) const {// FIXME: This is extremelly conservative. This really needs to use a
// whitelist with a clear explanation for why each realocation needs to
// point to the symbol, not to the section.
			switch (Type) {
			default:
				return true;
			case ELF::R_DSP_GOT16:
				// For DSP pic mode, I think it¡¯s OK to return true but I didn¡¯t confirm.
				// llvm_unreachable("Should have been handled already");
				return true;
				// These relocations might be paired with another relocation. The pairing is
				// done by the static linker by matching the symbol. Since we only see one
				// relocation at a time, we have to force them to relocate with a symbol to
				// avoid ending up with a pair where one points to a section and another
				// points to a symbol.
			case ELF::R_DSP_HI16:
			case ELF::R_DSP_LO16:
				// R_DSP_32 should be a relocation record, I don¡¯t know why Mips set it to
				// false.
			case ELF::R_DSP_32:
				return true;
			case ELF::R_DSP_GPREL16:
				return false;
			}
		}
	MCObjectWriter *llvm::createDSPELFObjectWriter(raw_ostream &OS,
		uint8_t OSABI,
		bool IsLittleEndian) {
		MCELFObjectTargetWriter *MOTW = new DSPELFObjectWriter(OSABI);
		return createELFObjectWriter(MOTW, OS, IsLittleEndian);
	}
}