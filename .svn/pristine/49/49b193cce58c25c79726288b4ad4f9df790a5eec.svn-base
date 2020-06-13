//===-- DSPMCInstLower.cpp - Convert DSP MachineInstr to MCInst ---------===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower DSP MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//
#include "DSPMCInstLower.h"
#include "DSPAsmPrinter.h"
#include "DSPInstrInfo.h"
#include "MCTargetDesc/DSPBaseInfo.h"
#include "MCTargetDesc/DSPMCInst.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Debug.h"
#include <iostream>

using namespace llvm;
#define DEBUG_TYPE "dsp-inst-lower"

DSPMCInstLower::DSPMCInstLower(DSPAsmPrinter &asmprinter) :AsmPrinter(asmprinter){


}

void DSPMCInstLower::Initialize(MCContext *C){
	Ctx = C;
}

static void CreateMCInst(MCInst &Inst, unsigned Opc, const MCOperand &Opnd0,
	const MCOperand &Opnd1,
	const MCOperand &Opnd2 = MCOperand()){

	Inst.setOpcode(Opc);
	Inst.addOperand(Opnd0);
	Inst.addOperand(Opnd1);
	if (Opnd2.isValid())Inst.addOperand(Opnd2);
}


MCOperand DSPMCInstLower::LowerOperand(const MachineOperand &MO, unsigned offset)const {

	MachineOperandType MOTy = MO.getType();
	switch (MOTy)
	{
	default:llvm_unreachable("unknown operand type");
		break;
	case MachineOperand::MO_Register:
		if (MO.isImplicit())break;
		return MCOperand::CreateReg(MO.getReg());
	case MachineOperand::MO_Immediate:
		return MCOperand::CreateImm(MO.getImm() + offset);
	case MachineOperand::MO_RegisterMask:
		break;
	case MachineOperand::MO_ExternalSymbol:
		return LowerSymbolOperand(MO, MOTy, offset);
		break;
	case MachineOperand::MO_GlobalAddress:
		return LowerSymbolOperand(MO, MOTy, offset);
	case MachineOperand::MO_ConstantPoolIndex:
		return LowerSymbolOperand(MO, MOTy, offset);
		break;
	case MachineOperand::MO_CFIIndex:
		return LowerSymbolOperand(MO, MOTy, offset);
		break;
	case MachineOperand::MO_MachineBasicBlock:
		return LowerSymbolOperand(MO, MOTy, offset);
	case MachineOperand::MO_JumpTableIndex:
		return LowerSymbolOperand(MO, MOTy, offset);

	}

	return MCOperand();
}

void DSPMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI)const{
	OutMI.setOpcode(MI->getOpcode());
	for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i){

		const MachineOperand &MO = MI->getOperand(i);
		MCOperand MCOp = LowerOperand(MO);
		if (MCOp.isValid())
		{
			OutMI.addOperand(MCOp);
		}
	}


}

void DSPMCInstLower::Lower(const MachineInstr *MI, DSPMCInst &OutMI, unsigned FU) const {
	OutMI.setOpcode(MI->getOpcode());
	OutMI.setDesc(MI->getDesc());
	OutMI.setFuncUnit(FU);
	for (unsigned i = 0, e = MI->getNumOperands(); i < e; i++){
		const MachineOperand &MO = MI->getOperand(i);
		MCOperand MCOp = LowerOperand(MO);
		if (MCOp.isValid()){
			OutMI.addOperand(MCOp);
		}
	}
}


void DSPMCInstLower::LowerCPLOAD(SmallVector<MCInst, 4>& MCInsts) {
	DEBUG(dbgs() << "-------------------------DSPMCInstLower 1-------------------------\n");
	MCOperand GPReg = MCOperand::CreateReg(DSP::GP);
	MCOperand T9Reg = MCOperand::CreateReg(DSP::T9);
	StringRef SymName("_gp_disp");
	const MCSymbol *Sym = Ctx->GetOrCreateSymbol(SymName);
	const MCSymbolRefExpr *MCSym;

	MCSym = MCSymbolRefExpr::Create(Sym, MCSymbolRefExpr::VK_DSP_ABS_HI, *Ctx);
	MCOperand SymHi = MCOperand::CreateExpr(MCSym);
	MCSym = MCSymbolRefExpr::Create(Sym, MCSymbolRefExpr::VK_DSP_ABS_LO, *Ctx);
	MCOperand SymLo = MCOperand::CreateExpr(MCSym);

	MCInsts.resize(3);

	CreateMCInst(MCInsts[0], DSP::LUi, GPReg, SymHi);
	CreateMCInst(MCInsts[1], DSP::ADDiu, GPReg, GPReg, SymLo);
	CreateMCInst(MCInsts[2], DSP::ADDu, GPReg, GPReg, T9Reg);
	DEBUG(dbgs() << "-------------------------DSPMCInstLower 2-------------------------\n");
}

MCOperand DSPMCInstLower::LowerSymbolOperand(const MachineOperand &MO, MachineOperandType MOTy, unsigned Offset) const {
	MCSymbolRefExpr::VariantKind Kind;
	const MCSymbol *Symbol;
		
	switch (MO.getTargetFlags())
	{
	default:llvm_unreachable("Invalid target flag!");
		break;
	case DSPII::MO_NO_FLAG: Kind = MCSymbolRefExpr::VK_None; break;
	case DSPII::MO_ABS_HI: Kind = MCSymbolRefExpr::VK_DSP_ABS_HI; break;
	case DSPII::MO_ABS_LO: Kind = MCSymbolRefExpr::VK_DSP_ABS_LO; break;
	case DSPII::MO_GPREL: Kind = MCSymbolRefExpr::VK_DSP_GPREL; break;
	case DSPII::MO_GOT_CALL:  Kind = MCSymbolRefExpr::VK_DSP_GOT_CALL; break;
	case DSPII::MO_GOT: Kind = MCSymbolRefExpr::VK_DSP_GOT; break;
	case DSPII::MO_GOT_HI16:  Kind = MCSymbolRefExpr::VK_DSP_GOT_HI16; break;
	case DSPII::MO_GOT_LO16:  Kind = MCSymbolRefExpr::VK_DSP_GOT_LO16; break;
	}
	switch (MOTy){
	case MachineOperand::MO_MachineBasicBlock:
		Symbol = MO.getMBB()->getSymbol();
		break;
	case MachineOperand::MO_GlobalAddress:
		Symbol = AsmPrinter.getSymbol(MO.getGlobal());
		Offset += MO.getOffset();
		break;
	case MachineOperand::MO_ConstantPoolIndex:
		Symbol = AsmPrinter.GetCPISymbol(MO.getIndex());
		Offset = MO.getOffset();
		break;
	case MachineOperand::MO_BlockAddress:
		Symbol = AsmPrinter.GetBlockAddressSymbol(MO.getBlockAddress());
		Offset += MO.getOffset();
		break;
	case MachineOperand::MO_JumpTableIndex:
		Symbol = AsmPrinter.GetJTISymbol(MO.getIndex());
		break;

	case MachineOperand::MO_ExternalSymbol:
		Symbol = AsmPrinter.GetExternalSymbolSymbol(MO.getSymbolName());
		Offset += MO.getOffset();
		break;
	default:
		llvm_unreachable("<unknown operand type>"); break;
	}
	//std::cout << Symbol->getName().str() <<" offset"<<Offset<< std::endl;
	const MCSymbolRefExpr *MCSym = MCSymbolRefExpr::Create(Symbol, Kind, *Ctx);

	if (!Offset)
		return MCOperand::CreateExpr(MCSym);
	assert(Offset > 0);
	//MCConstantExpr - Represent a constant integer expression.
	const MCConstantExpr *OffsetExpr = MCConstantExpr::Create(Offset, *Ctx);
	const MCBinaryExpr *AddExpr = MCBinaryExpr::CreateAdd(MCSym, OffsetExpr, *Ctx);
	return MCOperand::CreateExpr(AddExpr);
}

#ifdef ENABLE_GPRESTORE
// Lower ".cprestore offset" to "st $gp, offset($sp)".
void DSPMCInstLower::LowerCPRESTORE(int64_t Offset,
	SmallVector<MCInst, 4>& MCInsts) {
	assert(isInt<32>(Offset) && (Offset >= 0) &&
		"Imm operand of .cprestore must be a non-negative 32-bit value.");

	MCOperand SPReg = MCOperand::CreateReg(DSP::SP), BaseReg = SPReg;
	MCOperand GPReg = MCOperand::CreateReg(DSP::GP);
	MCOperand ZEROReg = MCOperand::CreateReg(DSP::ZERO);

	if (!isInt<16>(Offset)) {
		unsigned Hi = ((Offset + 0x8000) >> 16) & 0xffff;
		Offset &= 0xffff;
		MCOperand ATReg = MCOperand::CreateReg(DSP::AT);
		BaseReg = ATReg;

		// lui   at,hi
		// add   at,at,sp
		MCInsts.resize(2);
		CreateMCInst(MCInsts[0], DSP::LUi, ATReg, ZEROReg, MCOperand::CreateImm(Hi));
		CreateMCInst(MCInsts[1], DSP::ADDu, ATReg, ATReg, SPReg);
	}

	MCInst St;
	CreateMCInst(St, DSP::ST, GPReg, BaseReg, MCOperand::CreateImm(Offset));
	MCInsts.push_back(St);
}
#endif