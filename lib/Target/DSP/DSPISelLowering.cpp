//===-- DSPISelLowering.cpp - DSP DAG Lowering Implementation -------------===//
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

#include "DSPISelLowering.h"
#include "DSPTargetMachine.h"
#include "DSPTargetObjectFile.h"
#include "DSPMachineFunction.h"
#include "DSPSubtarget.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/VariadicFunction.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Target/TargetOptions.h"
#include "MCTargetDesc/DSPBaseInfo.h"
#include <bitset>
#include <numeric>
#include <cctype>

#include <iostream>

using namespace llvm;


#define DEBUG_TYPE "dsp-lower"

STATISTIC(NumTailCalls, "Number of tail calls");

SDValue DSPTargetLowering::getGlobalReg(SelectionDAG &DAG, EVT Ty) const {
	DSPFunctionInfo *FI = DAG.getMachineFunction().getInfo<DSPFunctionInfo>();
	return DAG.getRegister(FI->getGlobalBaseReg(), Ty);
}

SDValue DSPTargetLowering::getTargetNode(GlobalAddressSDNode *N, EVT Ty,
	SelectionDAG &DAG,
	unsigned Flag) const {
	return DAG.getTargetGlobalAddress(N->getGlobal(), SDLoc(N), Ty, 0, Flag);
}

SDValue DSPTargetLowering::getTargetNode(ExternalSymbolSDNode *N, EVT Ty,
	SelectionDAG &DAG,
	unsigned Flag) const {
	return DAG.getTargetExternalSymbol(N->getSymbol(), Ty, Flag);
}

SDValue DSPTargetLowering::getTargetNode(BlockAddressSDNode *N, EVT Ty,
	SelectionDAG &DAG,
	unsigned Flag) const {
	return DAG.getTargetBlockAddress(N->getBlockAddress(), Ty, 0, Flag);
}

SDValue DSPTargetLowering::getTargetNode(JumpTableSDNode *N, EVT Ty,
	SelectionDAG &DAG,
	unsigned Flag) const {
	return DAG.getTargetJumpTable(N->getIndex(), Ty, Flag);
}

SDValue DSPTargetLowering::getTargetNode(ConstantPoolSDNode *N, EVT Ty, SelectionDAG &DAG, unsigned Flag) const{
	return DAG.getTargetConstantPool(N->getConstVal(), Ty, N->getAlignment(),N->getOffset(),Flag);
}

//DAG representation
const char *DSPTargetLowering::getTargetNodeName(unsigned Opcode) const {
	switch (Opcode){
	default: return nullptr;
	case DSPISD::MAX:  return "DSPISD::MAX"; break;
	case DSPISD::MIN:  return "DSPISD::MIN"; break;
	case DSPISD::VMAX: return "DSPISD::VMAX"; break;
	case DSPISD::VMIN: return "DSPISD::VMIN"; break;
	case DSPISD::VSUM: return "DSPISD::VSUM"; break;
	case DSPISD::RET:  return "DSPISD::RET"; break;
	case DSPISD::RET_FLAG: return "DSPISD::RET_FLAG"; break;
	case DSPISD::MOVIGH: return"DSPISD::MOVIGH"; break;
	case DSPISD::MOVIGL: return"DSPISD::MOVIGL"; break;
	case DSPISD::BUILDVECTOR4: return"DSPISD::BUILDVECTOR4"; break;
	case DSPISD::GPRel: return "DSPISD::GPRel"; break;
	case DSPISD::Wrapper: return "DSPISD::Wrapper"; break;
	case DSPISD::Hi: return "DSPISD::Hi"; break;
	case DSPISD::Lo: return "DSPISD::Lo"; break;
	case DSPISD::InsertVE8: return "DSPISD::InsertVE8"; break;
	case DSPISD::InsertVE16: return "DSPISD::InsertVE16"; break;
	case DSPISD::JmpLink: return "DSPISD::JmpLink"; break;
	//ppp-start
	case DSPISD::VAND: return "DSPISD::VAND"; break;
	case DSPISD::VNOT: return "DSPISD::VNOT"; break;
	case DSPISD::NOT: return "DSPISD::NOT"; break;
	case DSPISD::VXOR: return "DSPISD::VXOR"; break;
	case DSPISD::VOR: return "DSPISD::VOR"; break;
	case DSPISD::VSEL: return "DSPISD::VSEL"; break;
	case DSPISD::VMOVCV2V: return "DSPISD::VMOVCV2V"; break;
	//ppp-end
	}

}


DSPTargetLowering::DSPTargetLowering(DSPTargetMachine &TM, const DSPSubtarget &STI)
	:TargetLowering(TM, new DSPTargetObjectFile()), Subtarget(STI){
	// Set up the register classes
	addRegisterClass(MVT::i32, &DSP::CPURegsRegClass);
	addRegisterClass(MVT::v64i8, &DSP::CPU512RegsRegClass);//ppp
	addRegisterClass(MVT::v32i16, &DSP::CPU512RegsRegClass);//ppp
	addRegisterClass(MVT::v16i32, &DSP::CPU512RegsRegClass);//ppp
	//- Set .align 2
	// It will emit .align 2 later

	setMinFunctionAlignment(2);
	setMinStackArgumentAlignment(16);
	// must, computeRegisterProperties - Once all of the register classes are
	// added, this allows us to compute derived properties we expose.
	computeRegisterProperties();

	setOperationAction(ISD::INTRINSIC_WO_CHAIN, MVT::Other, Custom);
	setOperationAction(ISD::STORE, MVT::v16i32, Legal);


	// Load extented operations for i1 types must be promoted
	setLoadExtAction(ISD::EXTLOAD, MVT::i1, Promote);
	setLoadExtAction(ISD::ZEXTLOAD, MVT::i1, Promote);
	setLoadExtAction(ISD::SEXTLOAD, MVT::i1, Promote);


	setTruncStoreAction(MVT::f64, MVT::f32, Expand);

	AddPromotedToType(ISD::SETCC, MVT::i1, MVT::i32);
	//DSP does not have i1 type, so use i32 for
  // setcc operations results (slt, sgt, ...). 

	setBooleanContents(ZeroOrOneBooleanContent);
	setOperationAction(ISD::BUILD_VECTOR, MVT::v16i32, Custom);
	setOperationAction(ISD::BUILD_VECTOR, MVT::v32i16, Custom);
	setOperationAction(ISD::BUILD_VECTOR, MVT::v64i8, Custom);
	//setTargetDAGCombine

	setOperationAction(ISD::INSERT_VECTOR_ELT, MVT::v64i8, Custom);
	setOperationAction(ISD::INSERT_VECTOR_ELT, MVT::v32i16, Custom);

	//setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::i16, Custom);
	//setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::i8, Custom);


	setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
	setOperationAction(ISD::GlobalTLSAddress, MVT::i32, Custom);
	setOperationAction(ISD::ConstantPool, MVT::i32, Custom);


	setOperationAction(ISD::BR_CC, MVT::i32, Promote);
	AddPromotedToType(ISD::BR_CC, MVT::i1, MVT::i32);
	setOperationAction(ISD::BRCOND, MVT::Other, Custom);
	setOperationAction(ISD::SELECT, MVT::i32, Expand);//ppp Custom->Expand

	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i64, Expand);//ppp
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32, Expand);//ppp
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);//ppp
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);//ppp
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::Other, Expand);//ppp

	setOperationAction(ISD::SELECT_CC, MVT::i32, Expand);
	setOperationAction(ISD::SELECT_CC, MVT::Other, Expand);
	setOperationAction(ISD::INSERT_VECTOR_ELT, MVT::Other, Expand);
	setOperationAction(ISD::UINT_TO_FP, MVT::i32, Expand);
	setOperationAction(ISD::UINT_TO_FP, MVT::i64, Expand);
	setOperationAction(ISD::FP_TO_UINT, MVT::i32, Expand);
	setOperationAction(ISD::FP_TO_UINT, MVT::i64, Expand);

	//DSP doesn't support div rem  and float ,we need to use libcall
	setLibcallName(RTLIB::SDIV_I32, "__dsp_i32sdiv");
	setOperationAction(ISD::SDIV, MVT::i32, Expand);
	setLibcallName(RTLIB::UDIV_I32, "__dsp_i32udiv");
	setOperationAction(ISD::UDIV, MVT::i32, Expand);
	setLibcallName(RTLIB::UREM_I32, "__dsp_i32urem");
	setOperationAction(ISD::UREM, MVT::i32, Expand);
	setLibcallName(RTLIB::SREM_I32, "_dsp_i32srem");
	setOperationAction(ISD::UREM, MVT::i32, Expand);
	//setLibcallName(RTLIB::SINTTOFP_I32_F64, "__dsp_floatsidf");
	setOperationAction(ISD::FP_TO_SINT, MVT::i32, Expand);
	setOperationAction(ISD::SINT_TO_FP, MVT::i32, Expand);
	setLibcallName(RTLIB::UINTTOFP_I32_F64, "__dsp_floatunsidf");
	setLibcallName(RTLIB::SINTTOFP_I32_F64, "__dsp_floatsidf");

	setOperationAction(ISD::CTTZ, MVT::i32, Expand);
	setOperationAction(ISD::CTTZ, MVT::i64, Expand);
	setOperationAction(ISD::CTTZ_ZERO_UNDEF, MVT::i32, Expand);
	setOperationAction(ISD::CTTZ_ZERO_UNDEF, MVT::i64, Expand);
	setOperationAction(ISD::CTLZ_ZERO_UNDEF, MVT::i32, Expand);
	setOperationAction(ISD::CTLZ_ZERO_UNDEF, MVT::i64, Expand);
	setOperationAction(ISD::ROTR, MVT::i32, Expand);//ppp
	setOperationAction(ISD::ROTR, MVT::i64, Expand);//ppp
	setOperationAction(ISD::ROTL, MVT::i32, Expand);
	setOperationAction(ISD::ROTL, MVT::i64, Expand);
	setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32, Expand);
	setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i64, Expand);

	setOperationAction(ISD::FSIN, MVT::f32, Expand);
	setOperationAction(ISD::FSIN, MVT::f64, Expand);
	setOperationAction(ISD::FCOS, MVT::f32, Expand);
	setOperationAction(ISD::FCOS, MVT::f64, Expand);
	setOperationAction(ISD::FSINCOS, MVT::f32, Expand);
	setOperationAction(ISD::FSINCOS, MVT::f64, Expand);
	setOperationAction(ISD::FPOWI, MVT::f32, Expand);
	setOperationAction(ISD::FPOW, MVT::f32, Expand);
	setOperationAction(ISD::FPOW, MVT::f64, Expand);
	setOperationAction(ISD::FLOG, MVT::f32, Expand);
	setOperationAction(ISD::FLOG2, MVT::f32, Expand);
	setOperationAction(ISD::FLOG10, MVT::f32, Expand);
	setOperationAction(ISD::FEXP, MVT::f32, Expand);
	setOperationAction(ISD::FMA, MVT::f32, Expand);
	setOperationAction(ISD::FMA, MVT::f64, Expand);
	setOperationAction(ISD::FREM, MVT::f32, Expand);
	setOperationAction(ISD::FREM, MVT::f64, Expand);

	setOperationAction(ISD::EH_RETURN, MVT::Other, Custom);

	setOperationAction(ISD::VAARG, MVT::Other, Expand);
	setOperationAction(ISD::VACOPY, MVT::Other, Expand);
	setOperationAction(ISD::VAEND, MVT::Other, Expand);

	// Use the default for now
	setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
	setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);

	setOperationAction(ISD::ATOMIC_LOAD, MVT::i32, Expand);
	setOperationAction(ISD::ATOMIC_LOAD, MVT::i64, Expand);
	setOperationAction(ISD::ATOMIC_STORE, MVT::i32, Expand);
	setOperationAction(ISD::ATOMIC_STORE, MVT::i64, Expand);

	setInsertFencesForAtomic(true);
}

const DSPTargetLowering *DSPTargetLowering::create(DSPTargetMachine &TM, const DSPSubtarget &STI){
	return llvm::createDSPSETargetLowering(TM, STI);
}


// intrinsic with no side-effect
static SDValue LowerINTRINSIC_WO_CHAIN(SDValue Op, SelectionDAG &DAG) {
	SDLoc dl(Op);
	unsigned IntNo = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
	switch (IntNo){
	default: return SDValue();
	case Intrinsic::dsp_max:
		return DAG.getNode(DSPISD::MAX, dl, Op.getValueType(),
			Op.getOperand(1), Op.getOperand(2));
	case Intrinsic::dsp_min:
		return DAG.getNode(DSPISD::MIN, dl, Op.getValueType(),
			Op.getOperand(1), Op.getOperand(2));
	case Intrinsic::dsp_vmax_10:
	case Intrinsic::dsp_vmax_20:
	case Intrinsic::dsp_vmax_40:
		return DAG.getNode(DSPISD::VMAX, dl, Op.getValueType(),
			Op.getOperand(1), Op.getOperand(2));
	case Intrinsic::dsp_vmin_10:
	case Intrinsic::dsp_vmin_20:
	case Intrinsic::dsp_vmin_40:
		return DAG.getNode(DSPISD::VMIN, dl, Op.getValueType(),
			Op.getOperand(1), Op.getOperand(2));
	//ppp-start
	case Intrinsic::dsp_vand:
		return DAG.getNode(DSPISD::VAND, dl, Op.getValueType(),
			Op.getOperand(1), Op.getOperand(2));
	case Intrinsic::dsp_vxor:
		return DAG.getNode(DSPISD::VXOR, dl, Op.getValueType(),
			Op.getOperand(1), Op.getOperand(2));
	case Intrinsic::dsp_vor:
		return DAG.getNode(DSPISD::VOR, dl, Op.getValueType(),
			Op.getOperand(1), Op.getOperand(2));
	case Intrinsic::dsp_vnot:
		return DAG.getNode(DSPISD::VNOT, dl, Op.getValueType(),
			Op.getOperand(1));
	case Intrinsic::dsp_not:
		return DAG.getNode(DSPISD::NOT, dl, Op.getValueType(),
			Op.getOperand(1));
	case Intrinsic::dsp_vmovcv2v:
		return DAG.getNode(DSPISD::VMOVCV2V, dl, Op.getValueType(),
			Op.getOperand(1), Op.getOperand(2));
	case Intrinsic::dsp_vsel:
		return DAG.getNode(DSPISD::VSEL, dl, Op.getValueType(),
			Op.getOperand(1), Op.getOperand(2),Op.getOperand(3));
	//ppp-end
	}
}


SDValue DSPTargetLowering::
LowerJumpTable(SDValue Op, SelectionDAG &DAG) const
{
	JumpTableSDNode *N = cast<JumpTableSDNode>(Op);
	EVT Ty = Op.getValueType();

	if (getTargetMachine().getRelocationModel() != Reloc::PIC_)
		return getAddrNonPIC(N, Ty, DAG);

	return getAddrLocal(N, Ty, DAG);
}


/// isZero - Returns true if Elt is a constant integer zero
static bool isZero(SDValue V) {
	ConstantSDNode *C = dyn_cast<ConstantSDNode>(V);
	return C && C->isNullValue();
}

bool DSPTargetLowering::isFPImmLegal(const APFloat &Imm, EVT VT) const {
	std::cout << "fp legal" << std::endl;
	if (VT != MVT::f32 || VT != MVT::f64)
		return false;
	if (Imm.isNegZero())
		return false;
	return Imm.isZero();
}

static SDValue LowerBuildVector(SDValue Op, SelectionDAG &DAG){
	SDLoc dl(Op);
	MVT VT = Op.getSimpleValueType();
	MVT ExtVT = VT.getVectorElementType();
	unsigned NumElems = Op.getNumOperands();
	BuildVectorSDNode* BVN = dyn_cast<BuildVectorSDNode>(Op);
	SDValue Chain = Op.getOperand(0);

	unsigned NumZero = 0;
	unsigned NumNonZero = 0;
	unsigned NonZeros = 0;
	bool IsAllConstants = true;
	SmallSet<SDValue, 8> Values;
	for (unsigned i = 0; i < NumElems; ++i) {
		SDValue Elt = Op.getOperand(i);
		if (Elt.getOpcode() == ISD::UNDEF)
			continue;
		Values.insert(Elt);
		if (Elt.getOpcode() != ISD::Constant &&
			Elt.getOpcode() != ISD::ConstantFP)
			IsAllConstants = false;
		if (isZero(Elt))
			NumZero++;
		else {
			NonZeros |= (1 << i);
			NumNonZero++;
		}
	}
	DEBUG(dbgs() << "is all constant?  " << IsAllConstants << "\n");

	return SDValue();
}

SDValue DSPTargetLowering::LowerConstantPool(SDValue Op, SelectionDAG &DAG)const {
	DEBUG(dbgs() << "this is before lower constant pool" << "\n");
	ConstantPoolSDNode *CP = cast<ConstantPoolSDNode>(Op);
	//unsigned char OpFlag = DSPII::MO_ABS_HI;
	unsigned WrapperKind = DSPISD::Wrapper;
	ConstantPoolSDNode *N = cast<ConstantPoolSDNode>(Op);
	CodeModel::Model M = DAG.getTarget().getCodeModel();
	EVT Ty = Op.getValueType();
	SDLoc DL(CP);
	SDValue HI = getTargetNode(N,Ty,DAG, DSPII::MO_ABS_HI);
	SDValue LO = getTargetNode(N, Ty, DAG, DSPII::MO_ABS_LO);
	SDValue Result = getAddrNonPIC(N, Ty, DAG);
	//Result = DAG.getNode(WrapperKind, DL, getPointerTy(), Result);
	DEBUG(dbgs() << "this is after lower constant pool" << "\n");
	//if (getTargetMachine().getRelocationModel() == Reloc::PIC_)
		return Result;

}

SDValue DSPTargetLowering::LowerGlobalAddress(SDValue Op,
	SelectionDAG &DAG) const {
	// FIXME there isn't actually debug info here
	SDLoc DL(Op);
	DSPTargetObjectFile &TLOF = (DSPTargetObjectFile&)getObjFileLowering();
	EVT Ty = Op.getValueType();
	GlobalAddressSDNode *N = cast<GlobalAddressSDNode>(Op);
	const GlobalValue *GV = N->getGlobal();

	if (getTargetMachine().getRelocationModel() != Reloc::PIC_) {
		const DSPTargetObjectFile &TLOF =
			(const DSPTargetObjectFile&)getObjFileLowering();

		// %gp_rel relocation
		if (TLOF.IsGlobalInSmallSection(GV, getTargetMachine())) {
			SDValue GA = DAG.getTargetGlobalAddress(GV, DL, MVT::i32, 0,
				DSPII::MO_GPREL);
			SDValue GPRelNode = DAG.getNode(DSPISD::GPRel, DL,
				DAG.getVTList(MVT::i32), GA);
			SDValue GPReg = DAG.getRegister(DSP::GP, MVT::i32);
			return DAG.getNode(ISD::ADD, DL, MVT::i32, GPReg, GPRelNode);
		}

		// %hi/%lo relocation
			return getAddrNonPIC(N, Ty, DAG);
	}

	if (GV->hasInternalLinkage() || (GV->hasLocalLinkage() && !isa<Function>(GV)))
		return getAddrLocal(N, Ty, DAG);

	if (!TLOF.IsGlobalInSmallSection(GV, getTargetMachine()))
		return getAddrGlobalLargeGOT(N, Ty, DAG, DSPII::MO_GOT_HI16,
		DSPII::MO_GOT_LO16, DAG.getEntryNode(),
		MachinePointerInfo::getGOT());
	return getAddrGlobal(N, Ty, DAG, DSPII::MO_GOT16, DAG.getEntryNode(),
		MachinePointerInfo::getGOT());
}


SDValue DSPTargetLowering::LowerGlobalTLSAddress(SDValue Op,
	SelectionDAG &DAG) const {
	SDLoc DL(Op);
	DSPTargetObjectFile &TLOF = (DSPTargetObjectFile&)getObjFileLowering();
	EVT Ty = Op.getValueType();
	GlobalAddressSDNode *N = cast<GlobalAddressSDNode>(Op);
	const GlobalValue *GV = N->getGlobal();
	SDValue GAHi = DAG.getTargetGlobalAddress(GV, DL, MVT::i32, 0,
		DSPII::MO_ABS_HI);
	SDValue GALo = DAG.getTargetGlobalAddress(GV, DL, MVT::i32, 0,
		DSPII::MO_ABS_LO);
	SDValue HiPart = DAG.getNode(DSPISD::Hi, DL, Ty,  GAHi);
	SDValue Lo = DAG.getNode(DSPISD::Lo, DL, MVT::i32, GALo);
	return DAG.getNode(ISD::ADD, DL, MVT::i32, HiPart, Lo);
}

SDValue DSPTargetLowering::LowerInsertVectorElt(SDValue Op, SelectionDAG &DAG) const {
	std::cout << "vector insert" << std::endl;
	SDLoc DL(Op);
	EVT Ty = Op.getValueType();
	EVT ScalarTy = Ty.getScalarType();
	std::cout << "size " << ScalarTy.getSizeInBits() << std::endl;
	SDValue Imm = Op.getOperand(2);
	SDValue Value = Op.getOperand(1);
	if (ScalarTy == MVT::i16)
		return DAG.getNode(DSPISD::InsertVE16, DL, Ty, Op.getOperand(0), Value, Imm);

	if (ScalarTy == MVT::i8)
		return DAG.getNode(DSPISD::InsertVE8, DL, Ty, Op.getOperand(0), Value, Imm);

	return SDValue();
}

static SDValue LowerExtractVectorElt(SDValue Op,SelectionDAG &DAG){
	std::cout << "vector extract" << std::endl;
	SDLoc DL(Op);
	EVT Ty = Op.getValueType();
	EVT ScalarTy = Ty.getScalarType();
	std::cout << "size " << ScalarTy.getSizeInBits() << std::endl;
	SDValue Imm = Op.getOperand(2);
	SDValue Value = Op.getOperand(1);
	if (ScalarTy == MVT::i16)
		return DAG.getNode(DSPISD::ExtractVE16, DL, Ty, Op.getOperand(0), Value, Imm);

	if (ScalarTy == MVT::i8)
		return DAG.getNode(DSPISD::ExtractVE8, DL, Ty, Op.getOperand(0), Value, Imm);

	return Op;
}

SDValue DSPTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
	std::cout << "op code is " << Op.getOpcode() << std::endl;
	switch (Op.getOpcode())
	{
	default: llvm_unreachable("Should not custom lower this ");
	case ISD::INTRINSIC_WO_CHAIN: return LowerINTRINSIC_WO_CHAIN(Op, DAG);
	case ISD::BUILD_VECTOR: return LowerBuildVector(Op, DAG);
	case ISD::INSERT_VECTOR_ELT: return LowerInsertVectorElt(Op, DAG);
	//case ISD::EXTRACT_VECTOR_ELT: return LowerExtractVectorElt(Op, DAG);
	//case ISD::SELECT: return LowerSELECT(Op, DAG);//ppp
	case ISD::JumpTable: return LowerJumpTable(Op, DAG);
	case ISD::BRCOND:             return LowerBRCOND(Op, DAG);
	case ISD::ConstantPool: return LowerConstantPool(Op, DAG);
	case ISD::GlobalAddress:      return LowerGlobalAddress(Op, DAG);
	case ISD::GlobalTLSAddress:      return LowerGlobalTLSAddress(Op, DAG);
	case ISD::SHL_PARTS:          return LowerShiftLeftParts(Op, DAG);
	case ISD::SRA_PARTS:          return LowerShiftRightParts(Op, DAG, true);
	case ISD::SRL_PARTS:          return LowerShiftRightParts(Op, DAG, false);
	}
	return SDValue();
}

SDValue DSPTargetLowering::
LowerBRCOND(SDValue Op, SelectionDAG &DAG) const
{
	SDLoc DL(Op);
	SDValue XOr = Op.getOperand(1);
	SDValue SetCCV = XOr.getOperand(0);
	DEBUG(dbgs() << "xor op" << XOr.getOpcode() << "\n");
	return Op;
}

SDValue DSPTargetLowering::
LowerSELECT(SDValue Op, SelectionDAG &DAG) const
{
	return Op;
}
//leilei begin1
// addLiveIn - This helper function adds the specified physical register to the
// MachineFunction as a live in value.  It also creates a corresponding
// virtual register for it.
static unsigned
addLiveIn(MachineFunction &MF, unsigned PReg, const TargetRegisterClass *RC)
{
	unsigned VReg = MF.getRegInfo().createVirtualRegister(RC);
	MF.getRegInfo().addLiveIn(PReg, VReg);
	return VReg;
}

SDValue DSPTargetLowering::LowerShiftLeftParts(SDValue Op,
	SelectionDAG &DAG) const {
	SDLoc DL(Op);
	SDValue Lo = Op.getOperand(0), Hi = Op.getOperand(1);
	SDValue Shamt = Op.getOperand(2);

	// if shamt < 32:
	//  lo = (shl lo, shamt)
	//  hi = (or (shl hi, shamt) (srl (srl lo, 1), ~shamt))
	// else:
	//  lo = 0
	//  hi = (shl lo, shamt[4:0])
	SDValue Not = DAG.getNode(ISD::XOR, DL, MVT::i32, Shamt,
		DAG.getConstant(-1, MVT::i32));
	SDValue ShiftRight1Lo = DAG.getNode(ISD::SRL, DL, MVT::i32, Lo,
		DAG.getConstant(1, MVT::i32));
	SDValue ShiftRightLo = DAG.getNode(ISD::SRL, DL, MVT::i32, ShiftRight1Lo,
		Not);
	SDValue ShiftLeftHi = DAG.getNode(ISD::SHL, DL, MVT::i32, Hi, Shamt);
	SDValue Or = DAG.getNode(ISD::OR, DL, MVT::i32, ShiftLeftHi, ShiftRightLo);
	SDValue ShiftLeftLo = DAG.getNode(ISD::SHL, DL, MVT::i32, Lo, Shamt);
	SDValue Cond = DAG.getNode(ISD::AND, DL, MVT::i32, Shamt,
		DAG.getConstant(0x20, MVT::i32));
	Lo = DAG.getNode(ISD::SELECT, DL, MVT::i32, Cond,
		DAG.getConstant(0, MVT::i32), ShiftLeftLo);
	Hi = DAG.getNode(ISD::SELECT, DL, MVT::i32, Cond, ShiftLeftLo, Or);

	SDValue Ops[2] = { Lo, Hi };
	return DAG.getMergeValues(Ops, DL);
}

SDValue DSPTargetLowering::LowerShiftRightParts(SDValue Op, SelectionDAG &DAG,
	bool IsSRA) const {
	SDLoc DL(Op);
	SDValue Lo = Op.getOperand(0), Hi = Op.getOperand(1);
	SDValue Shamt = Op.getOperand(2);

	// if shamt < 32:
	//  lo = (or (shl (shl hi, 1), ~shamt) (srl lo, shamt))
	//  if isSRA:
	//    hi = (sra hi, shamt)
	//  else:
	//    hi = (srl hi, shamt)
	// else:
	//  if isSRA:
	//   lo = (sra hi, shamt[4:0])
	//   hi = (sra hi, 31)
	//  else:
	//   lo = (srl hi, shamt[4:0])
	//   hi = 0
	SDValue Not = DAG.getNode(ISD::XOR, DL, MVT::i32, Shamt,
		DAG.getConstant(-1, MVT::i32));
	SDValue ShiftLeft1Hi = DAG.getNode(ISD::SHL, DL, MVT::i32, Hi,
		DAG.getConstant(1, MVT::i32));
	SDValue ShiftLeftHi = DAG.getNode(ISD::SHL, DL, MVT::i32, ShiftLeft1Hi, Not);
	SDValue ShiftRightLo = DAG.getNode(ISD::SRL, DL, MVT::i32, Lo, Shamt);
	SDValue Or = DAG.getNode(ISD::OR, DL, MVT::i32, ShiftLeftHi, ShiftRightLo);
	SDValue ShiftRightHi = DAG.getNode(IsSRA ? ISD::SRA : ISD::SRL, DL, MVT::i32,
		Hi, Shamt);
	SDValue Cond = DAG.getNode(ISD::AND, DL, MVT::i32, Shamt,
		DAG.getConstant(0x20, MVT::i32));
	SDValue Shift31 = DAG.getNode(ISD::SRA, DL, MVT::i32, Hi,
		DAG.getConstant(31, MVT::i32));
	Lo = DAG.getNode(ISD::SELECT, DL, MVT::i32, Cond, ShiftRightHi, Or);
	Hi = DAG.getNode(ISD::SELECT, DL, MVT::i32, Cond,
		IsSRA ? Shift31 : DAG.getConstant(0, MVT::i32),
		ShiftRightHi);

	SDValue Ops[2] = { Lo, Hi };
	return DAG.getMergeValues(Ops, DL);
}

//===----------------------------------------------------------------------===//
// TODO: Implement a generic logic using tblgen that can support this.
// Cpu0 32 ABI rules:
// ---
//===----------------------------------------------------------------------===//

// Passed in stack only.
static bool CC_DSPS32(unsigned ValNo, MVT ValVT, MVT LocVT,
	CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
	CCState &State) {
	// Do not process byval args here.
	if (ArgFlags.isByVal())
		return true;

	// Promote i8 and i16
	if (LocVT == MVT::i8 || LocVT == MVT::i16) {
		LocVT = MVT::i32;
		if (ArgFlags.isSExt())
			LocInfo = CCValAssign::SExt;
		else if (ArgFlags.isZExt())
			LocInfo = CCValAssign::ZExt;
		else
			LocInfo = CCValAssign::AExt;
	}

	unsigned OrigAlign = ArgFlags.getOrigAlign();
	unsigned Offset = State.AllocateStack(ValVT.getSizeInBits() >> 3,
		OrigAlign);
	State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
	return false;
}

// Passed first two i32 arguments in registers and others in stack.
static bool CC_DSPO32(unsigned ValNo, MVT ValVT, MVT LocVT,
	CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
	CCState &State) {

	//number of pass var registers in scalar size
	static const unsigned IntRegsSize = 2;

	static const unsigned VectorRegsSize = 2;

	static const MCPhysReg IntRegs[] = { DSP::A0, DSP::A1 };

	static const MCPhysReg VectorRegs[] = { DSP::VR2, DSP::VR3 };
	// Do not process byval args here.
	if (ArgFlags.isByVal())
		return true;

	// Promote i8 and i16
	if (LocVT == MVT::i8 || LocVT == MVT::i16) {
		LocVT = MVT::i32;
		if (ArgFlags.isSExt())
			LocInfo = CCValAssign::SExt;
		else if (ArgFlags.isZExt())
			LocInfo = CCValAssign::ZExt;
		else
			LocInfo = CCValAssign::AExt;
	}

	unsigned Reg;

	// f32 and f64 are allocated in A0, A1 when either of the following
	// is true: function is vararg, argument is 3rd or higher, there is previous
	// argument which is not f32 or f64.
	bool AllocateFloatsInIntReg = true;
	unsigned OrigAlign = ArgFlags.getOrigAlign();
	bool isI64 = (ValVT == MVT::i32 && OrigAlign == 8);

	if (ValVT == MVT::i32 || (ValVT == MVT::f32 && AllocateFloatsInIntReg)) {
		Reg = State.AllocateReg(IntRegs, IntRegsSize);
		// If this is the first part of an i64 arg,
		// the allocated register must be A0.
		if (isI64 && (Reg == DSP::A1))
			Reg = State.AllocateReg(IntRegs, IntRegsSize);
		LocVT = MVT::i32;
	}
	else if (ValVT == MVT::f64 && AllocateFloatsInIntReg) {
		// Allocate int register. If first
		// available register is Cpu0::A1, shadow it too.
		Reg = State.AllocateReg(IntRegs, IntRegsSize);
		if (Reg == DSP::A1)
			Reg = State.AllocateReg(IntRegs, IntRegsSize);
		State.AllocateReg(IntRegs, IntRegsSize);
		LocVT = MVT::i32;
	}
	else if (ValVT == MVT::v16i32||ValVT == MVT::v32i16){
		Reg = State.AllocateReg(VectorRegs, VectorRegsSize);	
	}
	else
		llvm_unreachable("Cannot handle this ValVT.");

	if (!Reg) {
		unsigned Offset = State.AllocateStack(ValVT.getSizeInBits() >> 3,
			OrigAlign);
		State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
	}
	else
		State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));

	return false;
}

#include "DSPGenCallingConv.inc"

//===----------------------------------------------------------------------===//
//                  Call Calling Convention Implementation
//===----------------------------------------------------------------------===//

static const uint16_t O32IntRegs[] = {
	DSP::A0, DSP::A1
};
//LowerCall
/// LowerCall - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
SDValue
DSPTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
SmallVectorImpl<SDValue> &InVals) const {

	SelectionDAG &DAG = CLI.DAG;
	SDLoc DL = CLI.DL;
	SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
	SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
	SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
	SDValue Chain = CLI.Chain;
	SDValue Callee = CLI.Callee;
	bool &IsTailCall = CLI.IsTailCall;
	CallingConv::ID CallConv = CLI.CallConv;
	bool IsVarArg = CLI.IsVarArg;

	MachineFunction &MF = DAG.getMachineFunction();
	MachineFrameInfo *MFI = MF.getFrameInfo();
	const TargetFrameLowering *TFL = MF.getTarget().getFrameLowering();
	DSPFunctionInfo *FuncInfo = MF.getInfo<DSPFunctionInfo>();
	bool IsPIC = getTargetMachine().getRelocationModel() == Reloc::PIC_;
	DSPFunctionInfo *DSPFI = MF.getInfo<DSPFunctionInfo>();

	// Analyze operands of the call, assigning locations to each operand.
	SmallVector<CCValAssign, 16> ArgLocs;
	CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(),
		getTargetMachine(), ArgLocs, *DAG.getContext());
	DSPCC::SpecialCallingConvType SpecialCallingConv =
		getSpecialCallingConv(Callee);
	DSPCC DSPCCInfo(CallConv, Subtarget.isABI_O32(),
		CCInfo, SpecialCallingConv);

	DSPCCInfo.analyzeCallOperands(Outs, IsVarArg,
		Subtarget.abiUsesSoftFloat(),
		Callee.getNode(), CLI.getArgs());

	// Get a count of how many bytes are to be pushed on the stack.
	unsigned NextStackOffset = CCInfo.getNextStackOffset();



	// Check if it's really possible to do a tail call.
	/*if (IsTailCall)
	IsTailCall =
	isEligibleForTailCallOptimization(Cpu0CCInfo, NextStackOffset,
	*MF.getInfo<DSPFunctionInfo>());
	*/
	if (!IsTailCall && CLI.CS && CLI.CS->isMustTailCall())
		report_fatal_error("failed to perform tail call elimination on a call "
		"site marked musttail");

	if (IsTailCall)
		++NumTailCalls;

	// Chain is the output chain of the last Load/Store or CopyToReg node.
	// ByValChain is the output chain of the last Memcpy node created for copying
	// byval arguments to the stack.
	unsigned StackAlignment = TFL->getStackAlignment();
	NextStackOffset = RoundUpToAlignment(NextStackOffset, StackAlignment);
	SDValue NextStackOffsetVal = DAG.getIntPtrConstant(NextStackOffset, true);

	if (!IsTailCall)
		Chain = DAG.getCALLSEQ_START(Chain, NextStackOffsetVal, DL);

	SDValue StackPtr = DAG.getCopyFromReg(
		Chain, DL, DSP::SP,
		getPointerTy());

	// With EABI is it possible to have 16 args on registers.
	std::deque< std::pair<unsigned, SDValue> > RegsToPass;
	SmallVector<SDValue, 8> MemOpChains;
	DSPCC::byval_iterator ByValArg = DSPCCInfo.byval_begin();

	// Walk the register/memloc assignments, inserting copies/loads.
	for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
		SDValue Arg = OutVals[i];
		CCValAssign &VA = ArgLocs[i];
		MVT ValVT = VA.getValVT(), LocVT = VA.getLocVT();
		ISD::ArgFlagsTy Flags = Outs[i].Flags;

		// ByVal Arg.
		if (Flags.isByVal()) {
			assert(Flags.getByValSize() &&
				"ByVal args of size 0 should have been ignored by front-end.");
			assert(ByValArg != DSPCCInfo.byval_end());
			assert(!IsTailCall &&
				"Do not tail-call optimize if there is a byval argument.");
			passByValArg(Chain, DL, RegsToPass, MemOpChains, StackPtr, MFI, DAG, Arg,
				DSPCCInfo, *ByValArg, Flags, Subtarget.isLittle());
			++ByValArg;
			continue;
		}

		// Promote the value if needed.
		switch (VA.getLocInfo()) {
		default: llvm_unreachable("Unknown loc info!");
		case CCValAssign::Full:
			break;
		case CCValAssign::SExt:
			Arg = DAG.getNode(ISD::SIGN_EXTEND, DL, LocVT, Arg);
			break;
		case CCValAssign::ZExt:
			Arg = DAG.getNode(ISD::ZERO_EXTEND, DL, LocVT, Arg);
			break;
		case CCValAssign::AExt:
			Arg = DAG.getNode(ISD::ANY_EXTEND, DL, LocVT, Arg);
			break;
		}

		// Arguments that can be passed on register must be kept at
		// RegsToPass vector
		if (VA.isRegLoc()) {
			RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
			continue;
		}

		// Register can't get to this point...
		assert(VA.isMemLoc());

		// emit ISD::STORE whichs stores the
		// parameter value to a stack Location
		MemOpChains.push_back(passArgOnStack(StackPtr, VA.getLocMemOffset(),
			Chain, Arg, DL, IsTailCall, DAG));
	}

	// Transform all store nodes into one single node because all store
	// nodes are independent of each other.
	if (!MemOpChains.empty())
		Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

	// If the callee is a GlobalAddress/ExternalSymbol node (quite common, every
	// direct call is) turn it into a TargetGlobalAddress/TargetExternalSymbol
	// node so that legalize doesn't hack it.
	bool IsPICCall = IsPIC; // true if calls are translated to
	// jalr $t9
	bool GlobalOrExternal = false, InternalLinkage = false;
	SDValue CalleeLo;
	EVT Ty = Callee.getValueType();

	if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
		if (IsPICCall) {
			const GlobalValue *Val = G->getGlobal();
			InternalLinkage = Val->hasInternalLinkage();

			if (InternalLinkage)
				Callee = getAddrLocal(G, Ty, DAG);
			else
				Callee = getAddrGlobal(G, Ty, DAG, DSPII::MO_GOT_CALL, Chain,
				FuncInfo->callPtrInfo(Val));
		}
		else
			Callee = DAG.getTargetGlobalAddress(G->getGlobal(), DL, getPointerTy(), 0,
			DSPII::MO_NO_FLAG);
		GlobalOrExternal = true;
	}
	else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
		const char *Sym = S->getSymbol();

		if (!IsPIC) // static
			Callee = DAG.getTargetExternalSymbol(Sym, getPointerTy(),
			DSPII::MO_NO_FLAG);
		else // PIC
			Callee = getAddrGlobal(S, Ty, DAG, DSPII::MO_GOT_CALL, Chain,
			FuncInfo->callPtrInfo(Sym));

		GlobalOrExternal = true;
	}

	SmallVector<SDValue, 8> Ops(1, Chain);
	SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);

	getOpndList(Ops, RegsToPass, IsPICCall, GlobalOrExternal, InternalLinkage,
		CLI, Callee, Chain);

	//if (IsTailCall)
	//	return DAG.getNode(DSPISD::TailCall, DL, MVT::Other, Ops);

	Chain = DAG.getNode(DSPISD::JmpLink, DL, NodeTys, Ops);
	SDValue InFlag = Chain.getValue(1);

	// Create the CALLSEQ_END node.
	Chain = DAG.getCALLSEQ_END(Chain, NextStackOffsetVal,
		DAG.getIntPtrConstant(0, true), InFlag, DL);
	InFlag = Chain.getValue(1);

	// Handle result values, copying them out of physregs into vregs that we
	// return.
	return LowerCallResult(Chain, InFlag, CallConv, IsVarArg,
		Ins, DL, DAG, InVals, CLI.Callee.getNode(), CLI.RetTy);


}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
SDValue
DSPTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
CallingConv::ID CallConv, bool IsVarArg,
const SmallVectorImpl<ISD::InputArg> &Ins,
SDLoc DL, SelectionDAG &DAG,
SmallVectorImpl<SDValue> &InVals,
const SDNode *CallNode,
const Type *RetTy) const {
	// Assign locations to each value returned by this call.
	SmallVector<CCValAssign, 16> RVLocs;
	CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(),
		getTargetMachine(), RVLocs, *DAG.getContext());

	DSPCC DSPCCInfo(CallConv, Subtarget.isABI_O32(), CCInfo);

	DSPCCInfo.analyzeCallResult(Ins, Subtarget.abiUsesSoftFloat(),
		CallNode, RetTy);

	// Copy all of the result registers out of their specified physreg.
	for (unsigned i = 0; i != RVLocs.size(); ++i) {
		SDValue Val = DAG.getCopyFromReg(Chain, DL, RVLocs[i].getLocReg(),
			RVLocs[i].getLocVT(), InFlag);
		Chain = Val.getValue(1);
		InFlag = Val.getValue(2);

		if (RVLocs[i].getValVT() != RVLocs[i].getLocVT())
			Val = DAG.getNode(ISD::BITCAST, DL, RVLocs[i].getValVT(), Val);

		InVals.push_back(Val);
	}

	return Chain;
}
//leilei end1
//===----------------------------------------------------------------------===//
// Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//

SDValue
DSPTargetLowering::passArgOnStack(SDValue StackPtr, unsigned Offset,
SDValue Chain, SDValue Arg, SDLoc DL,
bool IsTailCall, SelectionDAG &DAG) const {
	if (!IsTailCall) {
		SDValue PtrOff = DAG.getNode(ISD::ADD, DL, getPointerTy(), StackPtr,
			DAG.getIntPtrConstant(Offset));
		return DAG.getStore(Chain, DL, Arg, PtrOff, MachinePointerInfo(), false,
			false, 0);
	}

	MachineFrameInfo *MFI = DAG.getMachineFunction().getFrameInfo();
	int FI = MFI->CreateFixedObject(Arg.getValueSizeInBits() / 8, Offset, false);
	SDValue FIN = DAG.getFrameIndex(FI, getPointerTy());
	return DAG.getStore(Chain, DL, Arg, FIN, MachinePointerInfo(),
		/*isVolatile=*/ true, false, 0);
}

void DSPTargetLowering::
getOpndList(SmallVectorImpl<SDValue> &Ops,
std::deque< std::pair<unsigned, SDValue> > &RegsToPass,
bool IsPICCall, bool GlobalOrExternal, bool InternalLinkage,
CallLoweringInfo &CLI, SDValue Callee, SDValue Chain) const {
	// T9 should contain the address of the callee function if
	// -reloction-model=pic or it is an indirect call.
	if (IsPICCall || !GlobalOrExternal) {
		unsigned T9Reg = DSP::T9;
		RegsToPass.push_front(std::make_pair(T9Reg, Callee));
	}
	else
		Ops.push_back(Callee);

	// Insert node "GP copy globalreg" before call to function.
	//
	// R_CPU0_CALL* operators (emitted when non-internal functions are called
	// in PIC mode) allow symbols to be resolved via lazy binding.
	// The lazy binding stub requires GP to point to the GOT.
	if (IsPICCall && !InternalLinkage) {
		unsigned GPReg = DSP::GP;
		EVT Ty = MVT::i32;
		RegsToPass.push_back(std::make_pair(GPReg, getGlobalReg(CLI.DAG, Ty)));
	}

	// Build a sequence of copy-to-reg nodes chained together with token
	// chain and flag operands which copy the outgoing args into registers.
	// The InFlag in necessary since all emitted instructions must be
	// stuck together.
	SDValue InFlag;
	std::cout << "RegsToPass.size()=" << RegsToPass.size() << std::endl;
	for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
		Chain = CLI.DAG.getCopyToReg(Chain, CLI.DL, RegsToPass[i].first,
			RegsToPass[i].second, InFlag);
		InFlag = Chain.getValue(1);
	}

	// Add argument registers to the end of the list so that they are
	// known live into the call.
	for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
		Ops.push_back(CLI.DAG.getRegister(RegsToPass[i].first,
		RegsToPass[i].second.getValueType()));

	// Add a register mask operand representing the call-preserved registers.
	const TargetRegisterInfo *TRI = getTargetMachine().getRegisterInfo();
	const uint32_t *Mask = TRI->getCallPreservedMask(CLI.CallConv);
	assert(Mask && "Missing call preserved mask for calling convention");
	Ops.push_back(CLI.DAG.getRegisterMask(Mask));

	if (InFlag.getNode())
		Ops.push_back(InFlag);
}


/// LowerFormalArguments - transform physical registers into virtual registers
/// and generate load operations for arguments places on the stack.
SDValue DSPTargetLowering::LowerFormalArguments(SDValue Chain,
	CallingConv::ID CallConv,
	bool IsVarArg,
	const SmallVectorImpl<ISD::InputArg> &Ins,
	SDLoc DL, SelectionDAG &DAG,
	SmallVectorImpl<SDValue> &InVals)const {


	MachineFunction &MF = DAG.getMachineFunction();
	MachineFrameInfo *MFI = MF.getFrameInfo();
	DSPFunctionInfo *DSPFI = MF.getInfo<DSPFunctionInfo>();
	DSPFI->setVarArgsFrameIndex(0);
	// Assign locations to all of the incoming arguments.
	SmallVector<CCValAssign, 16> ArgLocs;
	CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(),
		getTargetMachine(), ArgLocs, *DAG.getContext());
	DSPCC DSPCCInfo(CallConv, Subtarget.isABI_O32(),
		CCInfo);//leilei begin2
	DSPFI->setFormalArgInfo(CCInfo.getNextStackOffset(),
		DSPCCInfo.hasByValArg());


	// 函数参数的开始
	Function::const_arg_iterator FuncArg =
		DAG.getMachineFunction().getFunction()->arg_begin();
	bool UseSoftFloat = Subtarget.abiUsesSoftFloat();

	DSPCCInfo.analyzeFormalArguments(Ins, UseSoftFloat, FuncArg);

	// Used with vargs to acumulate store chains.
	std::vector<SDValue> OutChains;

	unsigned CurArgIdx = 0;
	DSPCC::byval_iterator ByValArg = DSPCCInfo.byval_begin();

	for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
		CCValAssign &VA = ArgLocs[i];
		std::advance(FuncArg, Ins[i].OrigArgIndex - CurArgIdx);
		CurArgIdx = Ins[i].OrigArgIndex;
		EVT ValVT = VA.getValVT();
		ISD::ArgFlagsTy Flags = Ins[i].Flags;
		bool IsRegLoc = VA.isRegLoc();

		
		if (Flags.isByVal()) {
			assert(Flags.getByValSize() &&
				"ByVal args of size 0 should have been ignored by front-end.");
			assert(ByValArg != DSPCCInfo.byval_end());
			copyByValRegs(Chain, DL, OutChains, DAG, Flags, InVals, &*FuncArg,
				DSPCCInfo, *ByValArg);
			//std::cout << "val1,result!!" << std::endl;
			++ByValArg;
			continue;
		}
		// Arguments stored on registers
		if (Subtarget.isABI_O32() && IsRegLoc) {
			MVT RegVT = VA.getLocVT();
			unsigned ArgReg = VA.getLocReg();
			const TargetRegisterClass *RC = getRegClassFor(RegVT);
			std::cout << "val1,result!!" << VA.getValNo() <<" ArgReg= "<< ArgReg << std::endl;
			// Transform the arguments stored on
			// physical registers into virtual ones
			unsigned Reg = addLiveIn(DAG.getMachineFunction(), ArgReg, RC);
			SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegVT);

			// If this is an 8 or 16-bit value, it has been passed promoted
			// to 32 bits.  Insert an assert[sz]ext to capture this, then
			// truncate to the right size.
			if (VA.getLocInfo() != CCValAssign::Full) {
				unsigned Opcode = 0;
				if (VA.getLocInfo() == CCValAssign::SExt)
					Opcode = ISD::AssertSext;
				else if (VA.getLocInfo() == CCValAssign::ZExt)
					Opcode = ISD::AssertZext;
				if (Opcode)
					ArgValue = DAG.getNode(Opcode, DL, RegVT, ArgValue,
					DAG.getValueType(ValVT));
				ArgValue = DAG.getNode(ISD::TRUNCATE, DL, ValVT, ArgValue);
				//std::cout << "val2,result!!" << std::endl;
			}

			// Handle floating point arguments passed in integer registers.
			if ((RegVT == MVT::i32 && ValVT == MVT::f32) ||
				(RegVT == MVT::i64 && ValVT == MVT::f64))
				ArgValue = DAG.getNode(ISD::BITCAST, DL, ValVT, ArgValue);
			InVals.push_back(ArgValue);
			//std::cout << "val3,result!!" << ArgValue.getResNo() << std::endl;
		}
		else { // VA.isRegLoc()

			// sanity check
			
			assert(VA.isMemLoc());

			// The stack pointer offset is relative to the caller stack frame.
			int FI = MFI->CreateFixedObject(ValVT.getSizeInBits() / 8,
				VA.getLocMemOffset(), true);

			// Create load nodes to retrieve arguments from the stack
			SDValue FIN = DAG.getFrameIndex(FI, getPointerTy());
			SDValue Load = DAG.getLoad(ValVT, DL, Chain, FIN,
				MachinePointerInfo::getFixedStack(FI),
				false, false, false, 0);
			InVals.push_back(Load);
			//std::cout << "val4,result!!" << Load.getResNo() << std::endl;
			OutChains.push_back(Load.getValue(1));
			//std::cout << "val2,result!!" << Load.getValue(1).getResNo() << std::endl;
			std::cout << "val2,result!!" << VA.getValNo() << " FI= " << FI << std::endl;
		}
	}

	for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
		// The cpu0 ABIs for returning structs by value requires that we copy
		// the sret argument into $v0 for the return. Save the argument into
		// a virtual register so that we can access it from the return points.
		if (Ins[i].Flags.isSRet()) {
			unsigned Reg = DSPFI->getSRetReturnReg();
			if (!Reg) {
				Reg = MF.getRegInfo().createVirtualRegister(
					getRegClassFor(MVT::i32));
				DSPFI->setSRetReturnReg(Reg);
			}
			SDValue Copy = DAG.getCopyToReg(DAG.getEntryNode(), DL, Reg, InVals[i]);
			Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, Copy, Chain);
			//std::cout << "val5,result!!" << std::endl;
			break;
		}
	}


	// All stores are grouped in one node to allow the matching between
	// the size of Ins and InVals. This only happens when on varg functions
	if (!OutChains.empty()) {
		OutChains.push_back(Chain);
		Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, OutChains);
		std::cout << "val6,result!!" << OutChains[0].getResNo() <<std::endl;
	}//leilei end2
	return Chain;
}

bool DSPTargetLowering::isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const {
	return false;
}
//===----------------------------------------------------------------------===//
// Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//
bool
DSPTargetLowering::CanLowerReturn(CallingConv::ID CallConv,
MachineFunction &MF, bool IsVarArg,
const SmallVectorImpl<ISD::OutputArg> &Outs,
LLVMContext &Context) const {
	SmallVector<CCValAssign, 16> RVLocs;
	CCState CCInfo(CallConv, IsVarArg, MF, getTargetMachine(),
		RVLocs, Context);
	return CCInfo.CheckReturn(Outs, RetCC_DSP);
}


SDValue DSPTargetLowering::LowerReturn(SDValue Chain,
	CallingConv::ID CallConv,
	bool isVarArg,
	const SmallVectorImpl<ISD::OutputArg> &Outs,
	const SmallVectorImpl<SDValue> &OutVals,
	SDLoc DL, SelectionDAG &DAG) const {
	// CCValAssign - represent the assignment of
	// the return value to a location
	SmallVector<CCValAssign, 16> RVLocs;
	MachineFunction &MF = DAG.getMachineFunction();
	// CCState - Info about the registers and stack slot.
	CCState CCInfo(CallConv, isVarArg, MF, getTargetMachine(), RVLocs,
		*DAG.getContext());
	DSPCC DSPCCInfo(CallConv, Subtarget.isABI_O32(),
		CCInfo);
	// Analyze return values.
	DSPCCInfo.analyzeReturn(Outs, Subtarget.abiUsesSoftFloat(), MF.getFunction()->getReturnType());
	SDValue Flag;
	SmallVector<SDValue, 4> RetOps(1, Chain);
	// Copy the result values into the output registers.
	for (unsigned i = 0; i != RVLocs.size(); ++i) {
		SDValue Val = OutVals[i];
		CCValAssign &VA = RVLocs[i];
		assert(VA.isRegLoc() && "Can only return in registers!");
		if (RVLocs[i].getValVT() != RVLocs[i].getLocVT())
			Val = DAG.getNode(ISD::BITCAST, DL, RVLocs[i].getLocVT(), Val);
		Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), Val, Flag);
		// Guarantee that all emitted copies are stuck together with flags.
		Flag = Chain.getValue(1);
		RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
	}
	// The DSP ABIs for returning structs by value requires that we copy
	// the sret argument into $v0 for the return. We saved the argument into
	// a virtual register in the entry block, so now we copy the value out
	// and into $v0.
	if (MF.getFunction()->hasStructRetAttr()) {
		DSPFunctionInfo *DSPFI = MF.getInfo<DSPFunctionInfo>();
		unsigned Reg = DSPFI->getSRetReturnReg();
		if (!Reg)
			llvm_unreachable("sret virtual register not created in the entry block");
		SDValue Val = DAG.getCopyFromReg(Chain, DL, Reg, getPointerTy());
		unsigned V0 = DSP::V0;
		Chain = DAG.getCopyToReg(Chain, DL, V0, Val, Flag);
		Flag = Chain.getValue(1);
		RetOps.push_back(DAG.getRegister(V0, getPointerTy()));
	}
	RetOps[0] = Chain; // Update chain.
	// Add the flag if we have it.
	if (Flag.getNode())
		RetOps.push_back(Flag);
	// Return on DSP is always a "ret $lr"
	return DAG.getNode(DSPISD::RET, DL, MVT::Other, RetOps);
}
DSPTargetLowering::DSPCC::DSPCC(
	CallingConv::ID CC, bool IsO32_, CCState &Info,
	DSPCC::SpecialCallingConvType SpecialCallingConv_)
	: CCInfo(Info), CallConv(CC), IsO32(IsO32_),
	SpecialCallingConv(SpecialCallingConv_) {
	// Pre-allocate reserved argument area.
	CCInfo.AllocateStack(reservedArgArea(), 1);
}

template<typename Ty>
void DSPTargetLowering::DSPCC::
analyzeReturn(const SmallVectorImpl<Ty> &RetVals, bool IsSoftFloat,
const SDNode *CallNode, const Type *RetTy) const {
	CCAssignFn *Fn;
	Fn = RetCC_DSP;
	for (unsigned I = 0, E = RetVals.size(); I < E; ++I) {
		MVT VT = RetVals[I].VT;
		ISD::ArgFlagsTy Flags = RetVals[I].Flags;
		MVT RegVT = this->getRegVT(VT, RetTy, CallNode, IsSoftFloat);
		if (Fn(I, VT, RegVT, CCValAssign::Full, Flags, this->CCInfo)) {
#ifndef NDEBUG
			dbgs() << "Call result #" << I << " has unhandled type "
				<< EVT(VT).getEVTString() << '\n';
#endif
			llvm_unreachable(nullptr);
		}
	}
}
////leilei begin3
//analyzeFormalArguments
void DSPTargetLowering::DSPCC::
analyzeFormalArguments(const SmallVectorImpl<ISD::InputArg> &Args,
bool IsSoftFloat, Function::const_arg_iterator FuncArg) {
	unsigned NumArgs = Args.size();
	llvm::CCAssignFn *FixedFn = fixedArgFn();
	unsigned CurArgIdx = 0;

	for (unsigned I = 0; I != NumArgs; ++I) {
		MVT ArgVT = Args[I].VT;
		ISD::ArgFlagsTy ArgFlags = Args[I].Flags;
		std::advance(FuncArg, Args[I].OrigArgIndex - CurArgIdx);
		CurArgIdx = Args[I].OrigArgIndex;

		if (ArgFlags.isByVal()) {
			handleByValArg(I, ArgVT, ArgVT, CCValAssign::Full, ArgFlags);
			continue;
		}

		MVT RegVT = getRegVT(ArgVT, FuncArg->getType(), nullptr, IsSoftFloat);

		if (!FixedFn(I, ArgVT, RegVT, CCValAssign::Full, ArgFlags, CCInfo))
			continue;


		dbgs() << "Formal Arg #" << I << " has unhandled type "
			<< EVT(ArgVT).getEVTString();

		llvm_unreachable(nullptr);
	}
}

void DSPTargetLowering::DSPCC::allocateRegs(ByValArgInfo &ByVal,
	unsigned ByValSize,
	unsigned Align) {
	unsigned RegSize = regSize(), NumIntArgRegs = numIntArgRegs();
	const uint16_t *IntArgRegs = intArgRegs();
	assert(!(ByValSize % RegSize) && !(Align % RegSize) &&
		"Byval argument's size and alignment should be a multiple of"
		"RegSize.");

	ByVal.FirstIdx = CCInfo.getFirstUnallocated(IntArgRegs, NumIntArgRegs);

	// If Align > RegSize, the first arg register must be even.
	if ((Align > RegSize) && (ByVal.FirstIdx % 2)) {
		CCInfo.AllocateReg(IntArgRegs[ByVal.FirstIdx]);
		++ByVal.FirstIdx;
	}

	// Mark the registers allocated.
	for (unsigned I = ByVal.FirstIdx; ByValSize && (I < NumIntArgRegs);
		ByValSize -= RegSize, ++I, ++ByVal.NumRegs)
		CCInfo.AllocateReg(IntArgRegs[I]);
}
DSPTargetLowering::DSPCC::SpecialCallingConvType
DSPTargetLowering::getSpecialCallingConv(SDValue Callee) const {
	DSPCC::SpecialCallingConvType SpecialCallingConv =
		DSPCC::NoSpecialCallingConv;
	return SpecialCallingConv;
}
//handleByValArg
void DSPTargetLowering::DSPCC::handleByValArg(unsigned ValNo, MVT ValVT,
	MVT LocVT,
	CCValAssign::LocInfo LocInfo,
	ISD::ArgFlagsTy ArgFlags) {
	assert(ArgFlags.getByValSize() && "Byval argument's size shouldn't be 0.");

	struct ByValArgInfo ByVal;
	unsigned RegSize = regSize();
	unsigned ByValSize = RoundUpToAlignment(ArgFlags.getByValSize(), RegSize);
	unsigned Align = std::min(std::max(ArgFlags.getByValAlign(), RegSize),
		RegSize * 2);

	if (useRegsForByval())
		allocateRegs(ByVal, ByValSize, Align);

	// Allocate space on caller's stack.
	ByVal.Address = CCInfo.AllocateStack(ByValSize - RegSize * ByVal.NumRegs,
		Align);
	CCInfo.addLoc(CCValAssign::getMem(ValNo, ValVT, ByVal.Address, LocVT,
		LocInfo));
	ByValArgs.push_back(ByVal);
}

unsigned DSPTargetLowering::DSPCC::numIntArgRegs() const {
	return IsO32 ? array_lengthof(O32IntRegs) : 0;
}

//intArgRegs
const MCPhysReg *DSPTargetLowering::DSPCC::intArgRegs() const {
	return O32IntRegs;
}

llvm::CCAssignFn *DSPTargetLowering::DSPCC::fixedArgFn() const {
	if (IsO32)
		return CC_DSPO32;
	else // IsS32
		return CC_DSPS32;
}
void DSPTargetLowering::DSPCC::
analyzeCallOperands(const SmallVectorImpl<ISD::OutputArg> &Args,
bool IsVarArg, bool IsSoftFloat, const SDNode *CallNode,
std::vector<ArgListEntry> &FuncArgs) {
	assert((CallConv != CallingConv::Fast || !IsVarArg) &&
		"CallingConv::Fast shouldn't be used for vararg functions.");

	unsigned NumOpnds = Args.size();
	llvm::CCAssignFn *FixedFn = fixedArgFn();


	for (unsigned I = 0; I != NumOpnds; ++I) {
		MVT ArgVT = Args[I].VT;
		ISD::ArgFlagsTy ArgFlags = Args[I].Flags;
		bool R;

		if (ArgFlags.isByVal()) {
			handleByValArg(I, ArgVT, ArgVT, CCValAssign::Full, ArgFlags);
			continue;
		}


		{
			MVT RegVT = getRegVT(ArgVT, FuncArgs[Args[I].OrigArgIndex].Ty, CallNode,
				IsSoftFloat);
			R = FixedFn(I, ArgVT, RegVT, CCValAssign::Full, ArgFlags, CCInfo);
		}

		if (R) {
#ifndef NDEBUG
			dbgs() << "Call operand #" << I << " has unhandled type "
				<< EVT(ArgVT).getEVTString();
#endif
			llvm_unreachable(nullptr);
		}
	}
}
//copyByValRegs
void DSPTargetLowering::
copyByValRegs(SDValue Chain, SDLoc DL, std::vector<SDValue> &OutChains,
SelectionDAG &DAG, const ISD::ArgFlagsTy &Flags,
SmallVectorImpl<SDValue> &InVals, const Argument *FuncArg,
const DSPCC &CC, const ByValArgInfo &ByVal) const {
	MachineFunction &MF = DAG.getMachineFunction();
	MachineFrameInfo *MFI = MF.getFrameInfo();
	unsigned RegAreaSize = ByVal.NumRegs * CC.regSize();
	unsigned FrameObjSize = std::max(Flags.getByValSize(), RegAreaSize);
	int FrameObjOffset;

	if (RegAreaSize)
		FrameObjOffset = (int)CC.reservedArgArea() -
		(int)((CC.numIntArgRegs() - ByVal.FirstIdx) * CC.regSize());
	else
		FrameObjOffset = ByVal.Address;

	// Create frame object.
	EVT PtrTy = getPointerTy();
	int FI = MFI->CreateFixedObject(FrameObjSize, FrameObjOffset, true);
	SDValue FIN = DAG.getFrameIndex(FI, PtrTy);
	InVals.push_back(FIN);

	if (!ByVal.NumRegs)
		return;

	// Copy arg registers.
	MVT RegTy = MVT::getIntegerVT(CC.regSize() * 8);
	const TargetRegisterClass *RC = getRegClassFor(RegTy);

	for (unsigned I = 0; I < ByVal.NumRegs; ++I) {
		unsigned ArgReg = CC.intArgRegs()[ByVal.FirstIdx + I];
		unsigned VReg = addLiveIn(MF, ArgReg, RC);
		unsigned Offset = I * CC.regSize();
		SDValue StorePtr = DAG.getNode(ISD::ADD, DL, PtrTy, FIN,
			DAG.getConstant(Offset, PtrTy));
		SDValue Store = DAG.getStore(Chain, DL, DAG.getRegister(VReg, RegTy),
			StorePtr, MachinePointerInfo(FuncArg, Offset),
			false, false, 0);
		OutChains.push_back(Store);
	}
}

//passByValArg
// Copy byVal arg to registers and stack.
void DSPTargetLowering::
passByValArg(SDValue Chain, SDLoc DL,
std::deque< std::pair<unsigned, SDValue> > &RegsToPass,
SmallVectorImpl<SDValue> &MemOpChains, SDValue StackPtr,
MachineFrameInfo *MFI, SelectionDAG &DAG, SDValue Arg,
const DSPCC &CC, const ByValArgInfo &ByVal,
const ISD::ArgFlagsTy &Flags, bool isLittle) const {

	unsigned ByValSizeInBytes = Flags.getByValSize();
	unsigned OffsetInBytes = 0; // From beginning of struct
	unsigned RegSizeInBytes = CC.regSize();
	unsigned Alignment = std::min(Flags.getByValAlign(), RegSizeInBytes);
	EVT PtrTy = getPointerTy(), RegTy = MVT::getIntegerVT(RegSizeInBytes * 8);

	if (ByVal.NumRegs) {
		const MCPhysReg *ArgRegs = CC.intArgRegs();
		bool LeftoverBytes = (ByVal.NumRegs * RegSizeInBytes > ByValSizeInBytes);
		unsigned I = 0;

		// Copy words to registers.
		for (; I < ByVal.NumRegs - LeftoverBytes;
			++I, OffsetInBytes += RegSizeInBytes) {
			SDValue LoadPtr = DAG.getNode(ISD::ADD, DL, PtrTy, Arg,
				DAG.getConstant(OffsetInBytes, PtrTy));
			SDValue LoadVal = DAG.getLoad(RegTy, DL, Chain, LoadPtr,
				MachinePointerInfo(), false, false, false,
				Alignment);
			MemOpChains.push_back(LoadVal.getValue(1));
			unsigned ArgReg = ArgRegs[ByVal.FirstIdx + I];
			RegsToPass.push_back(std::make_pair(ArgReg, LoadVal));
		}

		// Return if the struct has been fully copied.
		if (ByValSizeInBytes == OffsetInBytes)
			return;

		// Copy the remainder of the byval argument with sub-word loads and shifts.
		if (LeftoverBytes) {
			assert((ByValSizeInBytes > OffsetInBytes) &&
				(ByValSizeInBytes < OffsetInBytes + RegSizeInBytes) &&
				"Size of the remainder should be smaller than RegSizeInBytes.");
			SDValue Val;

			for (unsigned LoadSizeInBytes = RegSizeInBytes / 2, TotalBytesLoaded = 0;
				OffsetInBytes < ByValSizeInBytes; LoadSizeInBytes /= 2) {
				unsigned RemainingSizeInBytes = ByValSizeInBytes - OffsetInBytes;

				if (RemainingSizeInBytes < LoadSizeInBytes)
					continue;

				// Load subword.
				SDValue LoadPtr = DAG.getNode(ISD::ADD, DL, PtrTy, Arg,
					DAG.getConstant(OffsetInBytes, PtrTy));
				SDValue LoadVal = DAG.getExtLoad(
					ISD::ZEXTLOAD, DL, RegTy, Chain, LoadPtr, MachinePointerInfo(),
					MVT::getIntegerVT(LoadSizeInBytes * 8), false, false, Alignment);
				MemOpChains.push_back(LoadVal.getValue(1));

				// Shift the loaded value.
				unsigned Shamt;

				if (isLittle)
					Shamt = TotalBytesLoaded * 8;
				else
					Shamt = (RegSizeInBytes - (TotalBytesLoaded + LoadSizeInBytes)) * 8;

				SDValue Shift = DAG.getNode(ISD::SHL, DL, RegTy, LoadVal,
					DAG.getConstant(Shamt, MVT::i32));

				if (Val.getNode())
					Val = DAG.getNode(ISD::OR, DL, RegTy, Val, Shift);
				else
					Val = Shift;

				OffsetInBytes += LoadSizeInBytes;
				TotalBytesLoaded += LoadSizeInBytes;
				Alignment = std::min(Alignment, LoadSizeInBytes);
			}

			unsigned ArgReg = ArgRegs[ByVal.FirstIdx + I];
			RegsToPass.push_back(std::make_pair(ArgReg, Val));
			return;
		}
	}

	// Copy remainder of byval arg to it with memcpy.
	unsigned MemCpySize = ByValSizeInBytes - OffsetInBytes;
	SDValue Src = DAG.getNode(ISD::ADD, DL, PtrTy, Arg,
		DAG.getConstant(OffsetInBytes, PtrTy));
	SDValue Dst = DAG.getNode(ISD::ADD, DL, PtrTy, StackPtr,
		DAG.getIntPtrConstant(ByVal.Address));
	Chain = DAG.getMemcpy(Chain, DL, Dst, Src, DAG.getConstant(MemCpySize, PtrTy),
		Alignment, /*isVolatile=*/false, /*AlwaysInline=*/false,
		MachinePointerInfo(), MachinePointerInfo());
	MemOpChains.push_back(Chain);
}
//leilei end3
void DSPTargetLowering::DSPCC::
analyzeCallResult(const SmallVectorImpl<ISD::InputArg> &Ins, bool IsSoftFloat,
const SDNode *CallNode, const Type *RetTy) const {
	analyzeReturn(Ins, IsSoftFloat, CallNode, RetTy);
}
void DSPTargetLowering::DSPCC::
analyzeReturn(const SmallVectorImpl<ISD::OutputArg> &Outs, bool IsSoftFloat,
const Type *RetTy) const {
	analyzeReturn(Outs, IsSoftFloat, nullptr, RetTy);
}
unsigned DSPTargetLowering::DSPCC::reservedArgArea() const {
	return (IsO32 && (CallConv != CallingConv::Fast)) ? 8 : 0;
}


MVT DSPTargetLowering::DSPCC::getRegVT(MVT VT, const Type *OrigTy,
	const SDNode *CallNode,
	bool IsSoftFloat) const {
	if (IsSoftFloat || IsO32)
		return VT;
	return VT;
}




