
//===-- DSPMCInstLower.h - Lower MachineInstr to MCInst -------*- C++ -*--===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef DSPMCINSTLOWER_H
#define DSPMCINSTLOWER_H
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/Compiler.h"

namespace llvm {

	class  MCContext;
	class  MCInst;
	class  MCOperand;
	class  MachineInstr;
	class  MachineFunction;
	class  DSPAsmPrinter;
	class  DSPMCInst;

	/// DSPMCInstLower - This class is used to lower an MachineInstr into an
	// MCInst.
	class LLVM_LIBRARY_VISIBILITY DSPMCInstLower {
		typedef MachineOperand::MachineOperandType MachineOperandType;
		MCContext *Ctx;
		DSPAsmPrinter &AsmPrinter;
	public:
		DSPMCInstLower(DSPAsmPrinter &asmprinter);
		void Initialize(MCContext* C);
		void Lower(const MachineInstr *MI, MCInst &OutMI) const;
		void Lower(const MachineInstr *MI, DSPMCInst &OutMI, unsigned FU) const;
		MCOperand LowerOperand(const MachineOperand& MO, unsigned offset = 0) const;
		void LowerCPLOAD(SmallVector<MCInst, 4>& MCInsts);


	#ifdef ENABLE_GPRESTORE
		void LowerCPRESTORE(int64_t Offset, SmallVector<MCInst, 4>& MCInsts);
	#endif
		MCOperand LowerSymbolOperand(const MachineOperand &MO, MachineOperandType MOTy, unsigned Offest) const;
	};


}

#endif 


