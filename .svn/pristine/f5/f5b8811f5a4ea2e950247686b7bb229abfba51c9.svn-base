//===-- DSPMCCodeEmitter.h - Convert DSP Code to Machine Code -----------===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//
// This file defines the DSPMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//

#ifndef MIPS_MC_CODE_EMITTER_H
#define MIPS_MC_CODE_EMITTER_H

#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/Support/DataTypes.h"

using namespace llvm;
namespace llvm {
	class MCContext;
	class MCExpr;
	class MCInst;
	class MCInstrInfo;
	class MCFixup;
	class MCOperand;
	class MCSubtargetInfo;
	class raw_ostream;
	class DSPMCCodeEmitter : public MCCodeEmitter {
		// #define LLVM_DELETED_FUNCTION
		// LLVM_DELETED_FUNCTION - Expands to = delete if the compiler supports it.
		// Use to mark functions as uncallable. Member functions with this should be
		// declared private so that some behavior is kept in C++03 mode.
		// class DontCopy { private: DontCopy(const DontCopy&) LLVM_DELETED_FUNCTION;
		// DontCopy &operator =(const DontCopy&) LLVM_DELETED_FUNCTION; public: ... };
		// Definition at line 79 of file Compiler.h.
		DSPMCCodeEmitter(const DSPMCCodeEmitter &) LLVM_DELETED_FUNCTION;
		void operator=(const DSPMCCodeEmitter &)LLVM_DELETED_FUNCTION;
		const MCInstrInfo &MCII;
		MCContext &Ctx;
		bool IsLittleEndian;
	public:
		DSPMCCodeEmitter(const MCInstrInfo &mcii, MCContext &Ctx_, bool IsLittle)
			: MCII(mcii), Ctx(Ctx_), IsLittleEndian(IsLittle) {}
		~DSPMCCodeEmitter();
		void EmitByte(unsigned char C, raw_ostream &OS) const;
		void EmitInstruction(uint64_t Val, unsigned Size, raw_ostream &OS) const;
		void EncodeInstruction(const MCInst &MI, raw_ostream &OS,
			SmallVectorImpl<MCFixup> &Fixups,
			const MCSubtargetInfo &STI) const override;
		// getBinaryCodeForInstr - TableGen¡¯erated function for getting the
		// binary encoding for an instruction.
		uint64_t getBinaryCodeForInstr(const MCInst &MI,
			SmallVectorImpl<MCFixup> &Fixups,
			const MCSubtargetInfo &STI) const;
		// getBranch16TargetOpValue - Return binary encoding of the branch
		// target operand, such as BEQ, BNE. If the machine operand
		// requires relocation, record the relocation and return zero.
		unsigned getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
			SmallVectorImpl<MCFixup> &Fixups,
			const MCSubtargetInfo &STI) const;
		// getBranch24TargetOpValue - Return binary encoding of the branch
		// target operand, such as JMP #BB01, JEQ, JSUB. If the machine operand
		// requires relocation, record the relocation and return zero.
		unsigned getBranch24TargetOpValue(const MCInst &MI, unsigned OpNo,
			SmallVectorImpl<MCFixup> &Fixups,
			const MCSubtargetInfo &STI) const;
		// getJumpTargetOpValue - Return binary encoding of the jump
		// target operand, such as JSUB #function_addr.
		// If the machine operand requires relocation,
		// record the relocation and return zero.
		unsigned getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
			SmallVectorImpl<MCFixup> &Fixups,
			const MCSubtargetInfo &STI) const;
		// getMachineOpValue - Return binary encoding of operand. If the machin
		// operand requires relocation, record the relocation and return zero.
		unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
			SmallVectorImpl<MCFixup> &Fixups,
			const MCSubtargetInfo &STI) const;
		unsigned getMemEncoding(const MCInst &MI, unsigned OpNo,
			SmallVectorImpl<MCFixup> &Fixups,
			const MCSubtargetInfo &STI) const;

		unsigned getMemHalfEncoding(const MCInst &MI, unsigned OpNo,
			SmallVectorImpl<MCFixup> &Fixups,
			const MCSubtargetInfo &STI) const;
		unsigned getExprOpValue(const MCExpr *Expr, SmallVectorImpl<MCFixup> &Fixups,
			const MCSubtargetInfo &STI) const;
	}; // class DSPMCCodeEmitter
} // namespace llvm.
#endif 
