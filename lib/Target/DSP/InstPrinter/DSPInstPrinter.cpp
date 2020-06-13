//===-- DSPInstPrinter.cpp - Convert DSP MCInst to assembly syntax ------===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an DSP MCInst to a .s file.
//
//===----------------------------------------------------------------------===//
#include "DSPAsmPrinter.h"
#include "DSPInstPrinter.h"
#include "DSPInstrInfo.h"
#include "MCTargetDesc\DSPMCInst.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
using namespace llvm;
#define DEBUG_TYPE "asm-printer"
#define PRINT_ALIAS_INSTR
#include "DSPGenAsmWriter.inc"

const char PacketPadding = '\t';
unsigned short  FUAvaliable = 0x0000;

void DSPInstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const {
		OS  << StringRef(getRegisterName(RegNo)).upper();

}

void DSPInstPrinter::printRegName(const MCInst *MI, raw_ostream &OS, unsigned RegNo) const {
	/*if (MI->getOpcode() == DSP::Ret || MI->getOpcode() == DSP::RetLR)
		OS << "";
	else*/
		printRegName(OS, RegNo);

}

void DSPInstPrinter::printInst(const MCInst *MI, raw_ostream &O, StringRef Annot){
	/*if (!printAliasInstr(MI, O)) printInstruction(MI, O);
	printAnnotation(O, Annot);*/
	printInst((const DSPMCInst*)(MI), O, Annot);
}

void DSPInstPrinter::printInst(const DSPMCInst *MI, raw_ostream &O, StringRef Annot){
	//std::cout << "In DSPInstPrinter.cpp function printInst" << std::endl;
	//std::cout << "MI->isPacketStart" << MI->isPacketStart()<<std::endl;
	
	const char startPacket = '{',
		endPacket = '}';
	const char Separator = '|';
	if (MI->isPacketStart())
		O << startPacket << "\n";
	
	printInstruction(MI, O);
	if (MI->isPacketEnd())
		O << "\n" << endPacket;
		//FUAvaliable = DSP::NO_AVA;
	printAnnotation(O, Annot);

}

static void printExpr(const MCExpr *Expr, raw_ostream &OS){
	int offset = 0;
	const MCSymbolRefExpr *SRE;

	if (const MCBinaryExpr *BE = dyn_cast<MCBinaryExpr>(Expr)){
		SRE = dyn_cast<MCSymbolRefExpr>(BE->getLHS());
		const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(BE->getRHS());
		assert(SRE && CE && "Binary expression must be sym+const.");
		offset = CE->getValue();
	}
	else if (!(SRE = dyn_cast<MCSymbolRefExpr>(Expr)))
		assert(false && "Unexpected MCExpr type.");

	MCSymbolRefExpr::VariantKind Kind = SRE->getKind();
	switch (Kind) {
	default: llvm_unreachable("Invalid kind!");
	case MCSymbolRefExpr::VK_None: break;
	case MCSymbolRefExpr::VK_DSP_ABS_HI: OS << "%hi("; break;
	case MCSymbolRefExpr::VK_DSP_ABS_LO: OS << "%lo("; break;
	case MCSymbolRefExpr::VK_DSP_GOT_CALL:	break;
	case MCSymbolRefExpr::VK_DSP_GPREL: OS << "gp_rel("; break;
	case MCSymbolRefExpr::VK_DSP_GOT: OS << "%rip("; break;
	}
	OS << SRE->getSymbol();
	if (offset) {
		if (offset > 0)
			OS << '+ ';
		OS << offset;
	}
	if ((Kind == MCSymbolRefExpr::VK_DSP_GPOFF_HI) ||
		(Kind == MCSymbolRefExpr::VK_DSP_GPOFF_LO))
		OS << ")))";
	else if (Kind != MCSymbolRefExpr::VK_None)
		OS << ')';
	
}

void DSPInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
	raw_ostream &O) {
	//std::cout << "the OpNo is" << OpNo << std::endl;
	const MCOperand &Op = MI->getOperand(OpNo);
	if (Op.isReg()) {
		printRegName(MI, O, Op.getReg());
		return;
	}

	if (Op.isImm()) {
		O << Op.getImm();
		return;
	}
	assert(Op.isExpr() && "unknown operand kind in printOperand");
	printExpr(Op.getExpr(), O);
}

void DSPInstPrinter::printUnsignedImm(const MCInst *MI, int opNum,
	raw_ostream &O) {
	const MCOperand &MO = MI->getOperand(opNum);
	if (MO.isImm())
		O << (unsigned short int)MO.getImm();
	else
		printOperand(MI, opNum, O);
}
void DSPInstPrinter::
printMemOperand(const MCInst *MI, int opNum, raw_ostream &O) {
	// Load/Store memory operands -- imm($reg)
	// If PIC target the target is loaded as the
	// pattern ld $t9,%call16($gp)
	//printOperand(MI, opNum + 1, O);
	MCOperand MO = MI->getOperand(opNum + 1);
	int Offset = 0;
	if (MO.isImm())
		Offset=(MO.getImm() >> 2);
		//Offset = (MO.getImm());
	else
		assert("wrong offset");
	//std::cout << Offset << std::endl;
	O << "(";
	printOperand(MI, opNum, O);
	O << ")";
}

void DSPInstPrinter::
printMemOperandEA(const MCInst *MI, int opNum, raw_ostream &O) {
	// when using stack locations for not load/store instructions
	// print the same way as all normal 3 operand instructions.
	printOperand(MI, opNum, O);
	O << ", ";
	//printOperand(MI, opNum + 1, O);
	MCOperand MO = MI->getOperand(opNum + 1);
	int Offset = 0;
	if (MO.isImm())
		Offset = (MO.getImm() >> 2);
		//Offset = (MO.getImm());
	else
		assert("wrong offset");
	//std::cout << Offset << std::endl;
	O << Offset;
	return;
}