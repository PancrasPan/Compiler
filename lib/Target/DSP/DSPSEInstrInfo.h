//===-- DSPSEInstrInfo.h - DSP32/64 Instruction Information ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the DSP32/64 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef DSPSEINSTRUCTIONINFO_H
#define DSPSEINSTRUCTIONINFO_H

#include "DSPInstrInfo.h"
#include "DSPSERegisterInfo.h"
#include "DSPMachineFunction.h"


namespace llvm {

	class DSPSEInstrInfo :public DSPInstrInfo {
		const DSPSERegisterInfo RI;
	public:
		explicit DSPSEInstrInfo(const DSPSubtarget &STI);
		const DSPRegisterInfo &getRegisterInfo()const override;

		void storeRegToStack(MachineBasicBlock &MBB,
			MachineBasicBlock::iterator MI,
			unsigned SrcReg, bool isKill, int FrameIndex,
			const TargetRegisterClass *RC,
			const TargetRegisterInfo *TRI,
			int64_t Offset) const override;
		void loadRegFromStack(MachineBasicBlock &MBB,
			MachineBasicBlock::iterator MI,
			unsigned DestReg, int FrameIndex,
			const TargetRegisterClass *RC,
			const TargetRegisterInfo *TRI,
			int64_t Offset) const override;
		bool expandPostRAPseudo(MachineBasicBlock::iterator MI) const override;
		/// Adjust SP by Amount bytes.
		void adjustStackPtr(DSPFunctionInfo *FI, unsigned SP, int64_t Amount, MachineBasicBlock &MBB,
			MachineBasicBlock::iterator I) const;
		/// Emit a series of instructions to load an immediate. If NewImm is a
		/// non-NULL parameter, the last instruction is not emitted, but instead
		/// its immediate operand is returned in NewImm.
		unsigned loadImmediate(int64_t Imm, MachineBasicBlock &MBB,
			MachineBasicBlock::iterator II, DebugLoc DL,
			unsigned *NewImm) const;

		//?
		void copyPhysReg(MachineBasicBlock &MBB,
			MachineBasicBlock::iterator MI, DebugLoc DL,
			unsigned DestReg, unsigned SrcReg,
			bool KillSrc) const override;
	private:
		void ExpandRetLR(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
			unsigned Opc) const;
		void ExpandMovVR(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
			unsigned Opc) const;
		void ExpandMovGR(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
			unsigned Opc1,unsigned Opc2) const;
		void ExpandLEA(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
			unsigned Opc) const;

	};

}
#endif