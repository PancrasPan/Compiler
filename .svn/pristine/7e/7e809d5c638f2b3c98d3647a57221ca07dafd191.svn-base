//===-- DSPMachineFuctionInfo.h - DSP machine function info -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares DSP-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef DSP_MACHINE_FUNCTION_INFO_H
#define DSP_MACHINE_FUNCTION_INFO_H

#include "llvm/ADT/StringMap.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include <map>
#include <string>
#include <utility>


namespace llvm {

	class DSPCallEntry : public PseudoSourceValue {
	public:
		explicit DSPCallEntry(const StringRef &N);
		explicit DSPCallEntry(const GlobalValue *V);
		bool isConstant(const MachineFrameInfo *) const override;
		bool isAliased(const MachineFrameInfo *) const override;
		bool mayAlias(const MachineFrameInfo *) const override;
	private:
		void printCustom(raw_ostream &O) const override;
		#ifndef NDEBUG
		std::string Name;
		const GlobalValue *Val;
		#endif
	};

	/// DSPMachineFunctionInfo - This class is derived from MachineFunction and
	/// contains private DSP target-specific information for each MachineFunction.
	class DSPFunctionInfo : public MachineFunctionInfo {
	private:
		
		/// SRetReturnReg - Some subtargets require that sret lowering includes
		/// returning the value of the returned struct in a register. This field
		/// holds the virtual register into which the sret argument is passed.
		
		
		

	public:
		DSPFunctionInfo(MachineFunction &MF) :MF(MF),SRetReturnReg(0),EmitNOAT(false),VarArgsFrameIndex(0),MaxCallFrameSize(0),GlobalBaseReg(0){}
		
		~DSPFunctionInfo();

		bool getEmitNOAT() const { return EmitNOAT; }
		void setEmitNOAT()  { EmitNOAT = true; }

		/// \brief Create a MachinePointerInfo that has a Cpu0CallEntr object
		/// representing a GOT entry for an external function.
		MachinePointerInfo callPtrInfo(const StringRef &Name);

		/// \brief Create a MachinePointerInfo that has a Cpu0CallEntr object
		/// representing a GOT entry for a global function.
		MachinePointerInfo callPtrInfo(const GlobalValue *Val);

		unsigned getSRetReturnReg() const { return SRetReturnReg; }
		void setSRetReturnReg(unsigned Reg) { SRetReturnReg = Reg; }
		int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
		void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }
		bool hasByvalArg() const { return HasByvalArg; }
		void setFormalArgInfo(unsigned Size, bool HasByval) {
			IncomingArgSize = Size;
			HasByvalArg = HasByval;
		}
		unsigned getIncomingArgSize() const { return IncomingArgSize; }
		unsigned getMaxCallFrameSize() const { return MaxCallFrameSize; }
		void setMaxCallFrameSize(unsigned S) { MaxCallFrameSize = S; }
		bool globalBaseRegFixed() const;
		bool globalBaseRegSet() const;
		unsigned getGlobalBaseReg();
		/// GlobalBaseReg - keeps track of the virtual register initialized for
		/// use as the global base register. This is used for PIC in some PIC
		/// relocation models.
		unsigned GlobalBaseReg;
		int GPFI; // Index of the frame object for restoring $gp


	private:
		/// SRetReturnReg - Some subtargets require that sret lowering includes
		/// returning the value of the returned struct in a register. This field
		/// holds the virtual register into which the sret argument is passed.
		unsigned SRetReturnReg;
		bool EmitNOAT;
		virtual void  anchor();
		MachineFunction& MF;

		/// VarArgsFrameIndex - FrameIndex for start of var args area.
		int VarArgsFrameIndex;
		unsigned MaxCallFrameSize;
		/// True if function has a byval argument.
		bool HasByvalArg;
		/// Size of incoming argument area.
		unsigned IncomingArgSize;

		/// DSPCallEntry maps.
		StringMap<const DSPCallEntry *> ExternalCallEntries;
		ValueMap<const GlobalValue *, const DSPCallEntry *> GlobalCallEntries;
	};
}

#endif