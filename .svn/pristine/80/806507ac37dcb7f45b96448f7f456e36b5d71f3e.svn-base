//===-- DSPISelLowering.h - DSP DAG Lowering Interface ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that DSP uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef DSPISELLOWERING_H
#define DSPISELLOWERING_H

#include "DSP.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/IR/Function.h"
#include <deque>


namespace llvm {
	namespace DSPISD {
		//DSP Specific DAG Nodes
		enum NodeType {
			FIRST_NUMBER = ISD::BUILTIN_OP_END,

			// scalar node type  
			MAX,
			MIN,

	//		Hi,
			// Get the Lower 16 bits from a 32-bit immediate
			// No relation with Cpu0 Lo register
	//		Lo,

			// vector  node type with two operand
			VMAX,
			VMIN,
			VSUM,

//			VSUM,

			//
			BUILDVECTOR4,
			BUILDVECTOR8,
			BUILDVECTOR16,


			//


			RET,
			RET_FLAG,

			//
			MovGR,
			MOVIGH,
			MOVIGL,
			GPRel,
			JmpLink,
			Wrapper,
			Hi,
			Lo,
			InsertVE8,
			InsertVE16,
			ExtractVE8,
			ExtractVE16,

			//ppp-start
			NOT,
			VNOT,
			VAND,
			VXOR,
			VOR,
			VSEL,
			VMOVCV2V
			//ppp-end

		};
	}
	//class DSPFunctionInfo;
	class DSPSubtarget;
	class DSPTargetMachine;

	class DSPTargetLowering : public TargetLowering{
	protected:

		//getGlobalReg
		SDValue getGlobalReg(SelectionDAG &DAG, EVT Ty) const;

		// This method creates the following nodes, which are necessary for
		// computing a local symbol's address:
		//
		// (add (load (wrapper $gp, %got(sym)), %lo(sym))
		template<class NodeTy>
		SDValue getAddrLocal(NodeTy *N, EVT Ty, SelectionDAG &DAG) const {
			SDLoc DL(N);
			unsigned GOTFlag = DSPII::MO_GOT;
			SDValue GOT = DAG.getNode(DSPISD::Wrapper, DL, Ty, getGlobalReg(DAG, Ty),
				getTargetNode(N, Ty, DAG, GOTFlag));
			SDValue Load = DAG.getLoad(Ty, DL, DAG.getEntryNode(), GOT,
				MachinePointerInfo::getGOT(), false, false,
				false, 0);
			unsigned LoFlag = DSPII::MO_ABS_LO;
			SDValue Lo = DAG.getNode(DSPISD::Lo, DL, Ty,
				getTargetNode(N, Ty, DAG, LoFlag));
			return DAG.getNode(ISD::ADD, DL, Ty, Load, Lo);
		}

		// This method creates the following nodes, which are necessary for
		// computing a global symbol's address:
		//
		// (load (wrapper $gp, %got(sym)))
		template<class NodeTy>
		SDValue getAddrGlobal(NodeTy *N, EVT Ty, SelectionDAG &DAG,
			unsigned Flag, SDValue Chain,
			const MachinePointerInfo &PtrInfo) const {
			SDLoc DL(N);
			SDValue Tgt = DAG.getNode(DSPISD::Wrapper, DL, Ty, getGlobalReg(DAG, Ty),
				getTargetNode(N, Ty, DAG, Flag));
			return DAG.getLoad(Ty, DL, Chain, Tgt, PtrInfo, false, false, false, 0);
		}

		// This method creates the following nodes, which are necessary for
		// computing a global symbol's address in large-GOT mode:
		//
		// (load (wrapper (add %hi(sym), $gp), %lo(sym)))
		template<class NodeTy>
		SDValue getAddrGlobalLargeGOT(NodeTy *N, EVT Ty, SelectionDAG &DAG,
			unsigned HiFlag, unsigned LoFlag,
			SDValue Chain,
			const MachinePointerInfo &PtrInfo) const {
			SDLoc DL(N);
			SDValue Hi = DAG.getNode(DSPISD::Hi, DL, Ty,
				getTargetNode(N, Ty, DAG, HiFlag));
			Hi = DAG.getNode(ISD::ADD, DL, Ty, Hi, getGlobalReg(DAG, Ty));
			SDValue Wrapper = DAG.getNode(DSPISD::Wrapper, DL, Ty, Hi,
				getTargetNode(N, Ty, DAG, LoFlag));
			return DAG.getLoad(Ty, DL, Chain, Wrapper, PtrInfo, false, false, false,
				0);
		}

		// This method creates the following nodes, which are necessary for
		// computing a symbol's address in non-PIC mode:
		//
		// (add %hi(sym), %lo(sym))
		template<class NodeTy>
		SDValue getAddrNonPIC(NodeTy *N, EVT Ty, SelectionDAG &DAG) const {
			SDLoc DL(N);
			SDValue Hi = getTargetNode(N, Ty, DAG, DSPII::MO_ABS_HI);
			SDValue Lo = getTargetNode(N, Ty, DAG, DSPII::MO_ABS_LO);
			return DAG.getNode(ISD::ADD, DL, Ty,
				DAG.getNode(DSPISD::Hi, DL, Ty, Hi),
				DAG.getNode(DSPISD::Lo, DL, Ty, Lo));
		}


		/// ByValArgInfo - Byval argument information.
		struct ByValArgInfo {
			unsigned FirstIdx; // Index of the first register used.
			unsigned NumRegs; // Number of registers used for this argument.
			unsigned Address; // Offset of the stack area used to pass this argument.
			ByValArgInfo() : FirstIdx(0), NumRegs(0), Address(0) {}
		};

		/// This function fills Ops, which is the list of operands that will later
		/// be used when a function call node is created. It also generates
		/// copyToReg nodes to set up argument registers.
		virtual void
			getOpndList(SmallVectorImpl<SDValue> &Ops,
			std::deque< std::pair<unsigned, SDValue> > &RegsToPass,
			bool IsPICCall, bool GlobalOrExternal, bool InternalLinkage,
			CallLoweringInfo &CLI, SDValue Callee, SDValue Chain) const;
		/// DSPCC - This class provides methods used to analyze formal and call
		/// arguments and inquire about calling convention information.
		class DSPCC {
		public:
			enum SpecialCallingConvType {
				NoSpecialCallingConv
			};
			DSPCC(CallingConv::ID CallConv, bool IsO32, CCState &Info,
				SpecialCallingConvType SpecialCallingConv = NoSpecialCallingConv);
			void analyzeCallOperands(const SmallVectorImpl<ISD::OutputArg> &Outs,
				bool IsVarArg, bool IsSoftFloat,
				const SDNode *CallNode,
				std::vector<ArgListEntry> &FuncArgs);
			void analyzeFormalArguments(const SmallVectorImpl<ISD::InputArg> &Ins,
				bool IsSoftFloat,
				Function::const_arg_iterator FuncArg);
			void analyzeCallResult(const SmallVectorImpl<ISD::InputArg> &Ins,
				bool IsSoftFloat, const SDNode *CallNode,
				const Type *RetTy) const;
			void analyzeReturn(const SmallVectorImpl<ISD::OutputArg> &Outs,
				bool IsSoftFloat, const Type *RetTy) const;
			const CCState &getCCInfo() const { return CCInfo; }
			/// hasByValArg - Returns true if function has byval arguments.
			bool hasByValArg() const { return !ByValArgs.empty(); }
			/// regSize - Size (in number of bits) of integer registers.
			unsigned regSize() const { return IsO32 ? 4 : 4; }
			/// numIntArgRegs - Number of integer registers available for calls.
			unsigned numIntArgRegs() const;
			/// reservedArgArea - The size of the area the caller reserves for
			/// register arguments. This is 16-byte if ABI is O32.
			unsigned reservedArgArea() const;
			/// Return pointer to array of integer argument registers.
			const MCPhysReg *intArgRegs() const;
			typedef SmallVectorImpl<ByValArgInfo>::const_iterator byval_iterator;
			byval_iterator byval_begin() const { return ByValArgs.begin(); }
			byval_iterator byval_end() const { return ByValArgs.end(); }
		private:
			/// Return the type of the register which is used to pass an argument or
			/// return a value. This function returns f64 if the argument is an i64
			/// value which has been generated as a result of softening an f128 value.
			/// Otherwise, it just returns VT.


			/// useRegsForByval - Returns true if the calling convention allows the
			/// use of registers to pass byval arguments.
			bool useRegsForByval() const { return CallConv != CallingConv::Fast; }

			/// Return the function that analyzes fixed argument list functions.
			llvm::CCAssignFn *fixedArgFn() const;
			void allocateRegs(ByValArgInfo &ByVal, unsigned ByValSize,
				unsigned Align);
			void handleByValArg(unsigned ValNo, MVT ValVT, MVT LocVT,
				CCValAssign::LocInfo LocInfo,
				ISD::ArgFlagsTy ArgFlags);
			MVT getRegVT(MVT VT, const Type *OrigTy, const SDNode *CallNode,
				bool IsSoftFloat) const;
			template<typename Ty>
			void analyzeReturn(const SmallVectorImpl<Ty> &RetVals, bool IsSoftFloat,
				const SDNode *CallNode, const Type *RetTy) const;
			CCState &CCInfo;
			CallingConv::ID CallConv;
			bool IsO32;
			SpecialCallingConvType SpecialCallingConv;
			SmallVector<ByValArgInfo, 2> ByValArgs;
		};
	protected:
		const DSPSubtarget &Subtarget;
	private:
		// Create a TargetGlobalAddress node.
		SDValue getTargetNode(GlobalAddressSDNode *N, EVT Ty, SelectionDAG &DAG,
			unsigned Flag) const;

		// Create a TargetExternalSymbol node.
		SDValue getTargetNode(ExternalSymbolSDNode *N, EVT Ty, SelectionDAG &DAG,
			unsigned Flag) const;

		SDValue getTargetNode(BlockAddressSDNode *N, EVT Ty, SelectionDAG &DAG,
			unsigned Flag) const;

		SDValue getTargetNode(JumpTableSDNode *N, EVT Ty, SelectionDAG &DAG,
			unsigned Flag) const;

		SDValue getTargetNode(ConstantPoolSDNode *N, EVT Ty, SelectionDAG &DAG,
			unsigned Flag) const;
		DSPCC::SpecialCallingConvType getSpecialCallingConv(SDValue Callee) const;

		// Lower Operand helpers
		SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
			CallingConv::ID CallConv, bool isVarArg,
			const SmallVectorImpl<ISD::InputArg> &Ins,
			SDLoc dl, SelectionDAG &DAG,
			SmallVectorImpl<SDValue> &InVals,
			const SDNode *CallNode, const Type *RetTy) const;
	public:
		explicit DSPTargetLowering(DSPTargetMachine &TM, const DSPSubtarget &STI);


		static const DSPTargetLowering *create(DSPTargetMachine &TM, const DSPSubtarget &STI);




		/// getTargetNodeName - This method returns the name of a target specific
		/// DAG node.
		const char *getTargetNodeName(unsigned Opcode) const override;


	private:
		SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
		SDValue LowerGlobalTLSAddress(SDValue Op, SelectionDAG &DAG)const;
		SDValue LowerJumpTable(SDValue Op, SelectionDAG &DAG) const;
		SDValue LowerConstantPool(SDValue Op, SelectionDAG &DAG) const;
		SDValue LowerSELECT(SDValue Op, SelectionDAG &DAG) const;
		SDValue LowerBRCOND(SDValue Op, SelectionDAG &DAG) const;
		
		SDValue LowerShiftLeftParts(SDValue Op, SelectionDAG& DAG) const;
		SDValue LowerShiftRightParts(SDValue Op, SelectionDAG& DAG,
			bool IsSRA) const;

		SDValue LowerInsertVectorElt(SDValue Op, SelectionDAG &DAG) const;
		/// isEligibleForTailCallOptimization - Check whether the call is eligible
		/// for tail call optimization.
		/*virtual bool
		isEligibleForTailCallOptimization(const DSPCC &Cpu0CCInfo,
		unsigned NextStackOffset,
		const DSPFunctionInfo &FI) const = 0;*/
		/// copyByValArg - Copy argument registers which were used to pass a byval
		/// argument to the stack. Create a stack frame object for the byval
		/// argument.
		void copyByValRegs(SDValue Chain, SDLoc DL,
			std::vector<SDValue> &OutChains, SelectionDAG &DAG,
			const ISD::ArgFlagsTy &Flags,
			SmallVectorImpl<SDValue> &InVals,
			const Argument *FuncArg,
			const DSPCC &CC, const ByValArgInfo &ByVal) const;
		/// passByValArg - Pass a byval argument in registers or on stack.
		void passByValArg(SDValue Chain, SDLoc DL,
			std::deque< std::pair<unsigned, SDValue> > &RegsToPass,
			SmallVectorImpl<SDValue> &MemOpChains, SDValue StackPtr,
			MachineFrameInfo *MFI, SelectionDAG &DAG, SDValue Arg,
			const DSPCC &CC, const ByValArgInfo &ByVal,
			const ISD::ArgFlagsTy &Flags, bool isLittle) const;
		SDValue passArgOnStack(SDValue StackPtr, unsigned Offset, SDValue Chain,
			SDValue Arg, SDLoc DL, bool IsTailCall,
			SelectionDAG &DAG) const;

		bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
			bool isVarArg,
			const SmallVectorImpl<ISD::OutputArg> &Outs,
			LLVMContext &Context) const override;
		SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
			SmallVectorImpl<SDValue> &InVals) const override;

		SDValue LowerFormalArguments(SDValue Chain,
			CallingConv::ID CallConv,
			bool isVarArg,
			const SmallVectorImpl<ISD::InputArg> &Ins,
			SDLoc dl,
			SelectionDAG &DAG,
			SmallVectorImpl<SDValue> &InVals) const override;



		//SDValue LowerCall(CallLoweringInfo &CLI,SmallVectorImpl<SDValue> &InVals) const override;

		SDValue LowerReturn(SDValue Chain,
			CallingConv::ID CallConv, bool isVarArg,
			const SmallVectorImpl<ISD::OutputArg> &Outs,
			const SmallVectorImpl<SDValue> &OutVals,
			SDLoc dl, SelectionDAG &DAG) const;

		SDValue LowerOperation(SDValue Op, SelectionDAG &DAG)const override;

		bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA)const override;

		bool isFPImmLegal(const APFloat &Imm, EVT VT) const override;

	};
	const DSPTargetLowering *createDSPSETargetLowering(DSPTargetMachine &TM, const  DSPSubtarget &STI);
}
#endif