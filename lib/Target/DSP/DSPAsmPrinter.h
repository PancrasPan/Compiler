//===-- DSPAsmPrinter.h - DSP LLVM Assembly Printer ----------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// DSP Assembly printer class.
//
//===----------------------------------------------------------------------===//


#ifndef DSPASMPRINTER_H
#define DSPASMPRINTER_H

#include "DSPMachineFunction.h"
#include "DSPSubtarget.h"
#include "DSPMCInstLower.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
	class MCStreamer;
	class MachineInstr;
	class MachineBasicBlock;
	class Module;
	class raw_ostream;

	class LLVM_LIBRARY_VISIBILITY DSPAsmPrinter :public AsmPrinter {
		void EmitInstrWithMacroNoAT(const MachineInstr *MI);
	private:
		// lowerOperand - Convert a MachineOperand into the equivalent MCOperand.
		bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp);
	public:
		const DSPSubtarget *Subtarget;
		const DSPFunctionInfo *DSPFI;
		DSPMCInstLower MCInstLowering;
		explicit DSPAsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
			:AsmPrinter(TM, Streamer), MCInstLowering(*this){
			Subtarget = &TM.getSubtarget<DSPSubtarget>();
		}
		virtual const char *getPassName() const {
			return "dsp assembly printer";
		}

		virtual bool runOnMachineFunction(MachineFunction &MF);
		//- EmitInstruction() must exists or will have run time error.
		void EmitInstruction(const MachineInstr *MI);
		void printSavedRegsBitmask(raw_ostream &O);
		void printHex32(unsigned int Value, raw_ostream &O);
		void emitFrameDirective();
		const char *getCurrentABIString() const;
		void EmitFunctionEntryLabel() override;
		void EmitFunctionBodyStart() override;
		void EmitFunctionBodyEnd() override;
		void EmitConstantPool() override;
		void EmitStartOfAsmFile(Module &M);
		virtual MachineLocation getDebugValueLocation(const MachineInstr *MI) const;
		void PrintDebugValueComment(const MachineInstr *MI, raw_ostream &OS);
		MCSymbol *GetCPISymbol(unsigned CPID) const override;
		//bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,unsigned AsmVariant,const char *ExtraCode, raw_ostream &O) override;
		unsigned getUnit(const MachineInstr* MI);//ppp
		std::string DSPAsmPrinter::PrintAsmSlots(unsigned slot);//ppp




	};

}


#endif