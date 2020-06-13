//=== DSPInstPrinter.h - Convert DSP MCInst to assembly syntax -*- C++ -*-==//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints a DSP MCInst to a .s file.
//
//===----------------------------------------------------------------------===//
#ifndef DSPINSTPRINTER_H
#define DSPINSTPRINTER_H

#include "llvm/MC/MCInstPrinter.h"
namespace llvm {
	namespace DSP{
	}
	class TargetMachine;
	class DSPMCInst;
	class DSPInstPrinter : public MCInstPrinter{
	public:
		DSPInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII, const MCRegisterInfo &MRI)
		:MCInstPrinter(MAI, MII, MRI){}

		//autogenerate by tablegen
		void printInstruction(const MCInst *MI, raw_ostream &O);
		static const char *getRegisterName(unsigned RegNo);
		void printRegName(raw_ostream &OS, unsigned RegNo) const override;
		void printRegName(const MCInst *MI, raw_ostream &OS, unsigned RegNo) const;
		void printInst(const MCInst *MI, raw_ostream &O, StringRef Annot) override;
		void printInst(const DSPMCInst *MI, raw_ostream &O, StringRef Annot);
		bool printAliasInstr(const MCInst *MI, raw_ostream &OS);
		void printCustomAliasOperand(const MCInst *MI, unsigned OpIdx,
			unsigned PrintMethodIdx, raw_ostream &O);
	private:
		void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
		void printUnsignedImm(const MCInst *MI, int opNum, raw_ostream &O);
		void printMemOperand(const MCInst *MI, int opNum, raw_ostream &O);
		void printMemOperandEA(const MCInst *MI, int opNum, raw_ostream &O);
	protected:
	};


}

#endif