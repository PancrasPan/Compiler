
//===- DSPInstrInfo.td - Target Description for DSP Target -*- tablegen -*-=//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the DSP implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//
// DSP profiles and nodes
//===----------------------------------------------------------------------===//


def SDT_DSPRet : SDTypeProfile<0, 1, [SDTCisInt<0>]>;
//def SDT_DSPVec : SDTypeProfile<1, 4,[SDTCisSameAs<4,1>,SDTCisSameAs<3,1>,SDTCisSameAs<6,1>,SDTCisInt<1>,SDTCisVec<0>]>;

def SDT_DSPVecInsert : SDTypeProfile<1,3,[SDTCisPtrTy<3>,SDTCisSameAs<1,0>,SDTCisVec<0>]>;

def SDTDSPVecOp : SDTypeProfile<1,2,[SDTCisVec<1>,SDTCisVec<0>]>;
def SDT_DSPWrapper :SDTypeProfile<1,1,[SDTCisSameAs<0,1>,SDTCisPtrTy<0>]>;

def SDT_DSPJmpLink      : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;

def SDT_DSPCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_DSPCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

// Call
def DSPJmpLink : SDNode<"DSPISD::JmpLink",SDT_DSPJmpLink,
                         [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                          SDNPVariadic]>;
// Return
def DSPRet : SDNode<"DSPISD::RET", SDTNone,
[SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_DSPCallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_DSPCallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

//def DSPV4 : SDNode<"DSPISD::BUILDVECTOR4",SDT_DSPVec,[SDNPVariadic]>;

def DSPMovGR : SDNode<"DSPISD::MovGR",SDTIntBinOp,[SDNPCommutative]>;
def DSPMAX : SDNode<"DSPISD::MAX",SDTIntBinOp,[SDNPCommutative]>;
def DSPMIN : SDNode<"DSPISD::MIN",SDTIntBinOp,[SDNPCommutative]>;


def DSPVMAX : SDNode<"DSPISD::VMAX",SDTDSPVecOp,[SDNPCommutative]>;
def DSPVMIN : SDNode<"DSPISD::VMIN",SDTDSPVecOp,[SDNPCommutative]>;

def DSPHi : SDNode<"DSPISD::Hi",SDTIntUnaryOp>;
def DSPLo : SDNode<"DSPISD::Lo",SDTIntUnaryOp>;
def DSPGPRel : SDNode<"DSPISD::GPRel",SDTIntUnaryOp>;
def DSPWrapper : SDNode<"DSPISD::Wrapper",SDT_DSPWrapper>;

def DSPVecInsert8: SDNode<"DSPISD::InsertVE8",SDT_DSPVecInsert>;
def DSPVecInsert16: SDNode<"DSPISD::InsertVE16",SDT_DSPVecInsert>;

//ppp
def SDT_DSPVNot:SDTypeProfile<1,1,[SDTCisSameAs<0,1>,SDTCisVec<0>]>;
def SDT_DSPNot:SDTypeProfile<1,1,[SDTCisSameAs<0,1>,SDTCisInt<0>]>;
def SDT_DSPVSEL:SDTypeProfile<1,3,[SDTCisVec<2>,SDTCisVec<3>,SDTCisInt<1>,SDTCisVec<0>]>;
def DSPVAND : SDNode<"DSPISD::VAND",SDTDSPVecOp,[SDNPCommutative]>;
def DSPVXOR : SDNode<"DSPISD::VXOR",SDTDSPVecOp,[SDNPCommutative]>;
def DSPVOR : SDNode<"DSPISD::VOR",SDTDSPVecOp,[SDNPCommutative]>;
def DSPVNOT : SDNode<"DSPISD::VNOT",SDT_DSPVNot,[]>;
def DSPNOT : SDNode<"DSPISD::NOT",SDT_DSPNot,[]>;
def DSPVMOVCV2V : SDNode<"DSPISD::VMOVCV2V",SDTDSPVecOp,[]>;
def DSPVSEL : SDNode<"DSPISD::VSEL",SDT_DSPVSEL,[]>;
//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//
include "DSPInstrFormats.td"

//===----------------------------------------------------------------------===//
// DSP Operand, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//
// Instruction operand types
// Signed Operand

def simm32 : Operand<i32> {
//let EncoderMethod = "getSimm16";
}
def simm16 : Operand<i32> {
//let EncoderMethod = "getSimm16";
}
def calltarget  : Operand<iPTR> {
  let EncoderMethod = "getJumpTargetOpValue";
}
def jmptarget    : Operand<OtherVT> {
  let EncoderMethod = "getJumpTargetOpValue";
  let OperandType = "OPERAND_PCREL";
  let DecoderMethod = "DecodeJumpRelativeTarget";
}
def immZExt8 : ImmLeaf<i32, [{return isUInt<8>(Imm);}]>;
def simm5 : Operand<i32> {
let DecoderMethod= "DecodeSimm5";
}

def simm9 :Operand<i32> {
let DecoderMethod= "DecodeSimm9";//ppp
}

def simm10 :Operand<i32> {
let DecoderMethod= "DecodeSimm11";
}

def simm11 :Operand<i32> {
let DecoderMethod= "DecodeSimm11";
}

// Address operand
def mem : Operand<i32> {
let PrintMethod = "printMemOperandEA";
let MIOperandInfo = (ops CPURegs, simm9);
let EncoderMethod = "getMemEncoding";
}

def memh :Operand<i32> {//ppp
let PrintMethod = "printMemOperandEA";
let MIOperandInfo = (ops CPURegs, simm9);
//let EncoderMethod = "getMemHalfEncoding";
let EncoderMethod = "getMemEncoding";
}

def memb :Operand<i32> {//ppp
let PrintMethod = "printMemOperandEA";
let MIOperandInfo = (ops CPURegs, simm9);
let EncoderMethod = "getMemEncoding";
}

def brtarget : Operand<OtherVT>{
let EncoderMethod = "getBranchTargetOpValue";
let OperandType = "OPERAND_PCREL";
}

// Plus 1.
def Plus1 : SDNodeXForm<imm, [{ return getImm(N, N->getSExtValue() + 1); }]>;

def immSExt32 : PatLeaf<(imm),[{return isInt<32>(N->getSExtValue());}]>;

// Node immediate fits as 16-bit sign extended on target immediate.
// e.g. addi, andi
def immSExt16 : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;
def immSExt10 : PatLeaf<(imm), [{ return isInt<10>(N->getSExtValue()); }]>;
def immSExt11 : PatLeaf<(imm), [{ return isInt<11>(N->getSExtValue()); }]>;
def immSExt9 : PatLeaf<(imm), [{ return isInt<9>(N->getSExtValue()); }]>;
def immSExt5 : PatLeaf<(imm), [{ return isInt<5>(N->getSExtValue()); }]>;
def shamt : Operand<i32>;
// Unsigned Operand
def uimm16 : Operand<i32> {
let PrintMethod = "printUnsignedImm";
}

def uimm11 :Operand<i32>{
let PrintMethod = "prinUnsignedImm";
}
def uimm10 :Operand<i32>{
let PrintMethod = "prinUnsignedImm";
}


// True if (N + 1) fits in 16-bit field.
def immSExt16Plus1 : PatLeaf<(imm), [{
  return isInt<17>(N->getSExtValue()) && isInt<16>(N->getSExtValue() + 1);
}]>;


// True if (N + 1) fits in 16-bit field.
def immSExt9Plus1 : PatLeaf<(imm), [{
  return isInt<10>(N->getSExtValue()) && isInt<10>(N->getSExtValue() + 1);
}]>;


// DSP Address Mode! SDNode frameindex could possibily be a match
// since load and store instructions from stack used it.
def addr : ComplexPattern<iPTR, 2, "SelectAddr", [frameindex], [SDNPWantParent]>;

class AlignedLoad<PatFrag Node> :
PatFrag<(ops node:$ptr), (Node node:$ptr), [{
LoadSDNode *LD = cast<LoadSDNode>(N);
return LD->getMemoryVT().getSizeInBits()/8 <= LD->getAlignment();
}]>;

class AlignedStore<PatFrag Node> :
PatFrag<(ops node:$val, node:$ptr), (Node node:$val, node:$ptr), [{
StoreSDNode *SD = cast<StoreSDNode>(N);
return SD->getMemoryVT().getSizeInBits()/8 <= SD->getAlignment();
}]>;

def loadi16_anyext : PatFrag<(ops node:$ptr), (i32 (unindexedload node:$ptr)),[{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  ISD::LoadExtType ExtType = LD->getExtensionType();
  if (ExtType == ISD::EXTLOAD)
    return LD->getAlignment() >= 2 && !LD->isVolatile();
  return false;
}]>;

// Load/Store PatFrags.
def load_a : AlignedLoad<load>;
def store_a : AlignedStore<store>;



class DSPInstAlias<string Asm, dag Result, bit Emit = 0b1> :
InstAlias<Asm, Result, Emit>;



// Transformation Function - get the lower 16 bits.
def LO16 : SDNodeXForm<imm, [{
return getImm(N, N->getZExtValue() & 0xffff);
}]>;
// Transformation Function - get the higher 16 bits.
def HI16 : SDNodeXForm<imm, [{
return getImm(N, (N->getZExtValue() >> 16) & 0xffff);
}]>;

// Node immediate fits as 16-bit zero extended on target immediate.
// The LO16 param means that only the lower 16 bits of the node
// immediate are caught.
// e.g. addiu, sltiu
def immZExt16 : PatLeaf<(imm), [{
if (N->getValueType(0) == MVT::i32)
return (uint32_t)N->getZExtValue() == (unsigned short)N->getZExtValue();
else
return (uint64_t)N->getZExtValue() == (unsigned short)N->getZExtValue();
}], LO16>;
// Immediate can be loaded with LUi (32-bit int with lower 16-bit cleared).
def immLow16Zero : PatLeaf<(imm), [{
int64_t Val = N->getSExtValue();
return isInt<32>(Val) && !(Val & 0xffff);
}]>;

// shamt field must fit in 5 bits.
def immZExt5 : ImmLeaf<i32, [{return Imm == (Imm & 0x1f);}]>;



//===----------------------------------------------------------------------===//
// Instructions specific format
//===----------------------------------------------------------------------===//

class shift_rotate_imm<bits<3> typeop,bits <3> op,bits<8> inner_op, bits<4> isRotate, string instr_asm,
SDNode OpNode, PatFrag PF, Operand ImmOpnd,
RegisterClass RC, bit isAlu = 0>:
F_3Reg_001<typeop,op,inner_op, (outs CPURegs:$ra), (ins RC:$rb, ImmOpnd:$shamt),
!strconcat(instr_asm, "\t$ra, $rb, $shamt"),
[(set CPURegs:$ra, (OpNode RC:$rb, PF:$shamt))], ALU32_S_SLOT01> {
let rc = 0;
}

class shift_rotate_reg<bits<3> typeop,bits <3> op,bits<8> inner_op, bits<4> isRotate, string instr_asm,
SDNode OpNode, RegisterClass RC>:
F_3Reg_001<typeop,op,inner_op, (outs CPURegs:$ra), (ins RC:$rb, RC:$rc),
!strconcat(instr_asm, "\t$ra, $rb, $rc"),
[(set CPURegs:$ra, (OpNode RC:$rb, RC:$rc))], ALU32_S_SLOT01> {
}

// 32-bit shift instructions.
class shift_rotate_imm32<bits<3> typeop, bits<3> op,bits<8> inner_op, bits<4> isRotate, string instr_asm,
SDNode OpNode>:
shift_rotate_imm<typeop, op, inner_op,isRotate, instr_asm, OpNode, immZExt5, shamt, CPURegs>;


// Arithmetic and logical instructions with 2 register operands.
class ArithLogicI<bits<3> typeop, bits<3> op, bits<3> inner_op,string instr_asm, SDNode OpNode,
Operand Od, PatLeaf imm_type, RegisterClass RC> :
F_2Reg_imm11<typeop,op,inner_op, (outs RC:$ra), (ins RC:$rb, Od:$imm11),
!strconcat(instr_asm, "\t$ra, $rb, $imm11"),
[(set RC:$ra, (OpNode RC:$rb, imm_type:$imm11))],ALU32_S_SLOT01> {
let isReMaterializable = 1;
}



// Arithmetic and logical instructions with 3 register operands.
class ArithLogicR<bits<3> typeop, bits<3> op, bits<8> inner_op,string instr_asm, SDNode OpNode,
InstrItinClass itin, RegisterClass RC, bit isComm = 0,bit isAlu = 1>:
F_3Reg_001<typeop,op,inner_op, (outs RC:$ra), (ins RC:$rb, RC:$rc),
!strconcat(instr_asm, "\t$ra, $rb, $rc"),
[(set RC:$ra, (OpNode RC:$rb, RC:$rc))], itin> {
let isCommutable = isComm; // e.g. add rb rc = add rc rb
let TSFlags{52} = isAlu;
let isReMaterializable = 1;
}

// Arithmetic and logical instructions with 3 register operands.
class VArithLogicR<bits<3> typeop, bits<3> op, bits<8> inner_op,string instr_asm, SDNode OpNode,
InstrItinClass itin, RegisterClass RC, bit isComm = 0>:
F_3Reg_001<typeop,op,inner_op, (outs RC:$ra), (ins RC:$rb, RC:$rc),
!strconcat(instr_asm, "\t$ra, $rb, $rc"),
[(set RC:$ra, (OpNode RC:$rb, RC:$rc))], itin> {
let isCommutable = isComm; // e.g. add rb rc = add rc rb
let isReMaterializable = 1;
}


class Mult<bits<3> typeop,bits<3> op,bits<8> inner_op, string instr_asm, InstrItinClass itin,
RegisterClass RC, list<Register> DefRegs>:
F_3Reg_001<typeop, op,inner_op, (outs), (ins RC:$ra, RC:$rb),
!strconcat(instr_asm, "\t$ra, $rb"), [], itin> {
let isCommutable = 1;
let Defs = DefRegs;
let neverHasSideEffects = 1;
}
class Mult32<bits<3> typeop,bits<3> op,bits<8> inner_op, string instr_asm, InstrItinClass itin>:
Mult<typeop, op,inner_op, instr_asm, itin, CPURegs, [HI, LO]>;


// Load Upper Imediate
class LoadUpper<bits<3> typeop, bits<3> op,bits<5> inner_op, string instr_asm, RegisterClass RC, Operand Imm>:
R2i5<typeop, op,inner_op, (outs RC:$ra), (ins Imm:$imm16),
!strconcat(instr_asm, "\t$ra, $imm16"), [], ALU32_S_SLOT01> {
let rs = 0;
let neverHasSideEffects = 1;
let isReMaterializable = 1;
}


class FMem<bits<3> typeop, bits<3> op, bits<5> inner_op,dag outs, dag ins, string asmstr, list<dag> pattern,
InstrItinClass itin>: F_LS_2Reg_imm9<typeop,op,inner_op, outs, ins, asmstr, pattern, itin> {
//bits<15> addr;//ppp
//let Inst{19-14} = addr{14-9};//ppp
//let Inst{13-5} = addr{8-0};//ppp
}

// Memory Load/Store
let canFoldAsLoad = 1 in
class LoadM<bits<3> typeop,bits<3> op,bits<5> inner_op, string instr_asm, PatFrag OpNode, RegisterClass RC,
Operand MemOpnd, bit Pseudo>:
F_LS_2Reg_imm9<typeop, op,inner_op, (outs RC:$ra), (ins MemOpnd:$addr),//ppp
!strconcat(instr_asm, "\t$ra, $addr"),
[(set RC:$ra, (OpNode addr:$addr))], LD_Ins_SLOT23 > {
let isPseudo = Pseudo;
let mayLoad = 1;
}
class StoreM<bits<3> typeop, bits<3> op,bits<5> inner_op,  string instr_asm, PatFrag OpNode, RegisterClass RC,
Operand MemOpnd, bit Pseudo>:
F_LS_2Reg_imm9<typeop, op,inner_op, (outs), (ins RC:$ra, MemOpnd:$addr),//ppp
!strconcat(instr_asm, "\t$ra, $addr"),
[(OpNode RC:$ra, addr:$addr)], ST_Ins_SLOT23 > {
let isPseudo = Pseudo;
let mayStore = 1;
}
// 32-bit load.
multiclass LoadM32<bits<3> typeop, bits<3> op,bits<5> inner_op,  string instr_asm, PatFrag OpNode,
bit Pseudo = 0> {
def #NAME# : LoadM<typeop, op,inner_op, instr_asm, OpNode, CPURegs, mem, Pseudo>;
}
// 32-bit store.
multiclass StoreM32<bits<3> typeop, bits<3> op,bits<5> inner_op,  string instr_asm, PatFrag OpNode,
bit Pseudo = 0> {
def #NAME# : StoreM<typeop, op,inner_op, instr_asm, OpNode, CPURegs, mem, Pseudo>;
}

// 16-bit load.
multiclass LoadM16<bits<3> typeop, bits<3> op,bits<5> inner_op,  string instr_asm, PatFrag OpNode,
bit Pseudo = 0> {
def #NAME# : LoadM<typeop, op,inner_op, instr_asm, OpNode, CPURegs, memh, Pseudo>;

}
// 16-bit store.
multiclass StoreM16<bits<3> typeop, bits<3> op,bits<5> inner_op,  string instr_asm, PatFrag OpNode,
bit Pseudo = 0> {
def #NAME# : StoreM<typeop, op,inner_op, instr_asm, OpNode, CPURegs, memh, Pseudo>;
}

//ppp
multiclass LoadM8<bits<3> typeop, bits<3> op,bits<5> inner_op,  string instr_asm, PatFrag OpNode,
bit Pseudo = 0> {
def #NAME# : LoadM<typeop, op,inner_op, instr_asm, OpNode, CPURegs, memb, Pseudo>;
}
multiclass StoreM8<bits<3> typeop, bits<3> op,bits<5> inner_op,  string instr_asm, PatFrag OpNode,
bit Pseudo = 0> {
def #NAME# : StoreM<typeop, op,inner_op, instr_asm, OpNode, CPURegs, memb, Pseudo>;
}

let isBranch=1, isTerminator=1, isBarrier=1, hasDelaySlot = 1,
isIndirectBranch = 1 in
class JumpFR<bits<3> typeop,bits<3> op,bits<8> inner_op,  string instr_asm, RegisterClass RC>:
F_Special_0Reg<typeop,op,inner_op,(outs), (ins RC:$ra),
!strconcat(instr_asm, "\t$ra"), [(brind RC:$ra)], ControlFlow_SLOT0> {
}

//mov

class SMovBase<bits<3> typeop,bits<3> op ,bits<4> inner_op, string instr_asm,RegisterClass RC,InstrItinClass itin>:
F_1Reg_imm16<typeop, op, inner_op,(outs RC:$ra), (ins RC:$rb, simm16:$imm16),
!strconcat(instr_asm, "\t$ra,$imm16"),[],itin>{
bits<16> imm16;
let Inst{19-4} = imm16;
//let Constraints = "$ra = $rb";
}

//ppp
class MovBase_c2g_l2v<bits<3> typeop,bits<3> op ,bits<8> inner_op, string instr_asm,RegisterClass RC,InstrItinClass itin>:
F_Special_1Reg_2<typeop, op,inner_op, (outs RC:$rd), (ins),
!strconcat(instr_asm, "\t$rd"),
[],
itin>{
}
class MovBase_g2c_v2l<bits<3> typeop,bits<3> op ,bits<8> inner_op, string instr_asm,RegisterClass RC,InstrItinClass itin>:
F_Special_1Reg_1<typeop, op,inner_op, (outs), (ins RC:$rs),
!strconcat(instr_asm, "\t$rs"),
[],
itin>{
}
class MovBase_vsel<bits<3> typeop,bits<3> op ,bits<2> inner_op, string instr_asm,SDNode OpNode,RegisterClass RC,RegisterClass RD,RegisterClass RE,InstrItinClass itin>:
F_Special_4Reg<typeop, op,inner_op, (outs RC:$rd), (ins RD:$rs,RE:$rt,RE:$rt1),
!strconcat(instr_asm, "\t$rd,$rs,$rt,$rt1"),
[(set RC:$rd ,(OpNode RD:$rs,RE:$rt,RE:$rt1))],
itin>{
let isCommutable = 0;
}
class UncondBranch_jmpr<bits<3> typeop,bits<3> op, bits<8> inner_op, string instr_asm>:
F_Special_1Reg_1<typeop,op,inner_op, (outs), (ins brtarget:$ra),
!strconcat(instr_asm, "\t$ra"), [(br bb:$ra)], ControlFlow_SLOT0> {
let isBranch = 1;
let isTerminator = 1;
let isBarrier = 1;
let hasDelaySlot = 1;
}
class RttBase<RegisterClass RC>: JumpFR<4,0,8, "rtt", RC> {
let isReturn = 1;
let hasDelaySlot = 1;
let isCodeGenOnly = 1;
let hasCtrlDep = 1;
let hasExtraSrcRegAllocReq = 1;
}

class MovBase<bits<3> typeop,bits<3> op ,bits<10> inner_op ,string instr_asm, SDNode Op,RegisterClass RC,RegisterClass RD,InstrItinClass itin>:
F_2Reg_imm4<typeop, op,inner_op, (outs RC:$rd), (ins RC:$ra, RD:$rb,uimm16:$imm),
!strconcat(instr_asm, "\t$ra,$rb,$imm"),[(set RC:$rd ,(Op RC:$ra, RD:$rb, immZExt16:$imm))],itin>{
}

class VMovBase<bits<3> typeop,bits<3> op ,bits<10> inner_op ,string instr_asm, SDNode Op,RegisterClass RC,RegisterClass RD,InstrItinClass itin>:
F_2Reg_imm4<typeop, op,inner_op, (outs RC:$ra), (ins  RD:$rb,uimm16:$imm),
!strconcat(instr_asm, "\t$ra,$rb,$imm"),[(set RC:$ra ,(Op RD:$rb, immZExt16:$imm))],itin>{
}


class RMovBase<bits<3> typeop,bits<3> op ,bits<8> inner_op, string instr_asm,RegisterClass RC,RegisterClass RD,InstrItinClass itin>:
F_2Reg_2<typeop, op,inner_op, (outs), (ins RC:$ra,RD:$rb),
!strconcat(instr_asm, "\t$ra,$rb"),[],itin>{
}


// Return instruction
class RetBase<RegisterClass RC>: JumpFR<4,0,4, "ret", RC> {
let isReturn = 1;
let hasDelaySlot = 1;
let isCodeGenOnly = 1;
let hasCtrlDep = 1;
let hasExtraSrcRegAllocReq = 1;
}


//Unconditional branch 
class UncondBranch<bits<3> typeop,bits<3> op, bits<5> inner_op, string instr_asm>:
F_Special_imm21<typeop,op,inner_op, (outs), (ins brtarget:$imm21),
!strconcat(instr_asm, "\t$imm21"), [(br bb:$imm21)], ControlFlow_SLOT0> {
let isBranch = 1;
let isTerminator = 1;
let isBarrier = 1;
let hasDelaySlot = 1;
}
class CBranch<bits<3> typeop, bits<3> op, bits<5> inner_op, string instr_asm>:
F_Special_imm21<typeop,op,inner_op, (outs), (ins CPURegs:$rt, brtarget:$imm21),
!strconcat(instr_asm, "\t $imm21"), [], ControlFlow_SLOT0> {
let isBranch = 1;
let isTerminator = 1;
let isBarrier = 1;
let hasDelaySlot = 1;
}

//Intrinsic function base

class MMBase<bits<3> typeop,bits<3> op, bits<8> inner_op, string instr_asm, SDNode OpNode ,InstrItinClass itin, RegisterClass RC>
:F_3Reg_001<typeop,op,inner_op,(outs CPURegs:$ra), (ins RC:$rb, RC:$rt),
!strconcat(instr_asm, "\t$ra, $rb, $rt"),
[(set CPURegs:$ra, (OpNode RC:$rb, RC:$rt))],itin>{
let isCommutable = 1;
}

class VMMBase<bits<3> typeop,bits<3> op, bits<8> inner_op, string instr_asm, SDNode OpNode, InstrItinClass itin, RegisterClass RC>
:F_3Reg_001<typeop,op,inner_op,(outs RC:$ra), (ins RC:$rb, RC:$rt),
!strconcat(instr_asm, "\t$ra, $rb, $rt"),
[(set RC:$ra, (OpNode RC:$rb, RC:$rt))],itin>{
let isCommutable = 1;
}
//Logic formats


//Loop base
class LoopBase<bits<3> typeop, bits<3> op, bits<4> inner_op, string instr_asm>:
F_1Reg_imm16<typeop,op,inner_op, (outs), (ins CPURegs:$ra, brtarget:$imm16),
!strconcat(instr_asm, "\t $ra, $imm16"), [], ControlFlow_SLOT0> {
let Inst{3-0} = inner_op;
let isBarrier = 1;
let hasDelaySlot = 1;
}


class SetCC_R<bits<3> typeop,bits<3> op, bits<8> inner_op, string instr_asm, PatFrag cond_op , RegisterClass RC>
:F_3Reg_001<typeop,op,inner_op,(outs RC:$ra), (ins RC:$rb, RC:$rc),
!strconcat(instr_asm, "\t $rb, $rc"),
[(set RC:$ra, (cond_op RC:$rb, RC:$rc))],ALU32_S_SLOT01>{
let Inst{25-20} = 0b000000;
let TSFlags{52} = 1;
let isCommutable = 0;
let isCompare = 1;
}

class SetCC_RI<bits<3> typeop,bits<3> op, bits<5> inner_op, string instr_asm, PatFrag cond_op , RegisterClass RC>
:F_1Reg_imm9<typeop,op,inner_op,(outs RC:$rs), (ins RC:$ra, simm16:$imm9),
!strconcat(instr_asm, "\t $ra, $imm9"),
[/*(set CPURegs:$rs, (cond_op RC:$ra, immSExt16:$imm16))*/],ALU32_S_SLOT01>{
let TSFlags{52} = 1;
let Inst{25-20} = 0b000000;
let isCommutable = 0;
let isCompare = 1;
}

	

class StoreM512<bits<3> typeop,bits<3> op,bits<5> inner_op, string instr_asm, PatFrag OpNode,
 bit Pseudo =0>:StoreM<typeop, op,inner_op, instr_asm, OpNode, VPR4Out, mem, Pseudo>;

 let canFoldAsLoad = 1 in
class LoadM512<bits<3> typeop,bits<3> op,bits<5> inner_op, string instr_asm, PatFrag OpNode,
 bit Pseudo =0>:LoadM<typeop, op,inner_op, instr_asm, OpNode, VPR4Out, mem, Pseudo>;


 class StoreM512_16<bits<3> typeop,bits<3> op,bits<5> inner_op, string instr_asm, PatFrag OpNode,
 bit Pseudo =0>:StoreM<typeop, op,inner_op, instr_asm, OpNode, VPR8Out, mem, Pseudo>;

 let canFoldAsLoad = 1 in
class LoadM512_16<bits<3> typeop,bits<3> op,bits<5> inner_op, string instr_asm, PatFrag OpNode,
 bit Pseudo =0>:LoadM<typeop, op,inner_op, instr_asm, OpNode, VPR8Out, mem, Pseudo>;

 class StoreM512_8<bits<3> typeop,bits<3> op,bits<5> inner_op, string instr_asm, PatFrag OpNode,
 bit Pseudo =0>:StoreM<typeop, op,inner_op, instr_asm, OpNode, VPR16Out, mem, Pseudo>;

 let canFoldAsLoad = 1 in
class LoadM512_8<bits<3> typeop,bits<3> op,bits<5> inner_op, string instr_asm, PatFrag OpNode,
 bit Pseudo =0>:LoadM<typeop, op,inner_op, instr_asm, OpNode, VPR16Out, mem, Pseudo>;


// Jump and Link (Call)
let isCall=1, hasDelaySlot=1 in {
  class JumpLink<bits<3> typeop, bits<3>op, bits<5> funct, string instr_asm>:
    F_Special_imm21<typeop, op, funct, (outs), (ins calltarget:$target, variable_ops),
       !strconcat(instr_asm, "\t$target"), [(DSPJmpLink imm:$target)],
       ControlFlow_SLOT0> {
       }

  class JumpLinkReg<bits<3> typeop, bits<3>op, bits<8> funct, string instr_asm,
                    RegisterClass RC>:
    F_Special_1Reg_1<typeop, op, funct, (outs), (ins RC:$rs, variable_ops),
       !strconcat(instr_asm, "\t$rs"), [(DSPJmpLink RC:$rs)], ControlFlow_SLOT0> {
  }
}






//specific DSP instructions
//******************************************specail instruction *************************************//
/// No operation
def NOP : F_Special_0Reg<4,0,0, (outs), (ins), "nop", [],NOP_SLOT0123>;

//special Nop in vload vstore
def NOP_S : F_Special_0Reg<4,0,0, (outs), (ins), "nops", [],NOP_SLOT0123>;

//***************************************************************************************************//

//*****************************************control instruction***************************************//

let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1 in
def RetLR : DSPPseudo<(outs), (ins), "", [(DSPRet)]>;
let isReturn=1,	isTerminator=1, isBarrier=1 in
def Ret : RetBase<CPURegs>;
def Jmp : UncondBranch<4,0,0b000001,"jmp">;
def JNC : CBranch<4,0,0b01001, "jnc">;
def JC : CBranch<4,0,0b00101, "jc">;

//ppp
def Jmpr : UncondBranch_jmpr<4,0,0b000011,"jmpr">;
def Rtt : RttBase<CPURegs>;

let isTerminator = 1, neverHasSideEffects = 1 in {
def Loop : LoopBase<4,5,0b0000, "loop">;
}
let isTerminator = 1, isBranch = 1, neverHasSideEffects = 1, isCodeGenOnly = 1, isPseudo = 1 in {
def ENDLOOP : F_Special_0Reg<4,0,0, (outs), (ins), "endloop", [],ControlFlow_SLOT0>;
}

//***************************************************************************************************//

//call callr
def CALL : JumpLink<0x4, 0, 0b01101, "call">;
def CALLR : JumpLinkReg<0x4, 0, 0b00000111, "callr", CPURegs>;

//*****************************************data translation instruction******************************//
//def MovVR : DSPPseudo<(outs VPR4Out:$ra), (ins CPURegs:$imm16,CPURegs:$imm2,CPURegs:$imm3,CPURegs:$imm4), "", [(set VPR4Out:$ra,(DSPV4 CPURegs:$imm16,CPURegs:$imm2,CPURegs:$imm3,CPURegs:$imm4))]>;
//set CPURegs:$ra, (DSPMovGR CPURegs:$rs, immSExt16:$in)
def MovGR : DSPPseudo<(outs CPURegs:$ra), (ins CPURegs:$rs,simm32:$in), "", []>;
def LEA   : DSPPseudo<(outs CPURegs:$ra), (ins mem:$addr),"",[(set CPURegs:$ra, addr:$addr)]>;

def MovG2V40 : MovBase<7,6,0b0000100111,"movg2v40",insertelt,VPR4Out,CPURegs,ALU32_V_SLOT01>;
//def MovG2V20 : MovBase<7,6,0b0000101011,"movg2v20",insertelt,CPU512Regs,CPURegs,ALU32_V_SLOT01>;
def MovG2V10 : MovBase<7,6,0b0000101111,"movg2v10",DSPVecInsert8,VPR16Out,CPURegs,ALU32_V_SLOT01>;


def MovV2G40 : VMovBase<7,6,0b0000010111,"movv2g40",extractelt,CPURegs,VPR4Out,ALU32_V_SLOT01>;
def MovV2G20 : VMovBase<7,6,0b0000011011,"movv2g20",vector_extract,CPURegs,VPR8Out,ALU32_V_SLOT01>;
def MovV2G10 : VMovBase<7,6,0b0000011111,"movv2g10",vector_extract,CPURegs,VPR16Out,ALU32_V_SLOT01>;

def MovIGH : SMovBase<7,5,0b0001,"movigh",CPURegs,Data_Transmission_2_SLOT0123>;

let Constraints = "$ra = $rb" in 
def MovIGL : SMovBase<7,5,0b0101,"movigl",CPURegs,Data_Transmission_2_SLOT0123>;
def MovG2G : RMovBase<7,4,0b10000000,"movg2g",CPURegs,CPURegs,Data_Transmission_2_SLOT0123>;

//ppp
def MovC2G : MovBase_c2g_l2v<5,0,0b00001111,"movc2g",CPURegs,ALU32_V_SLOT01>;
def MovL2V : MovBase_c2g_l2v<5,0,0b00011111,"movl2v",VPR16Out,Data_Transmission_0_SLOT0>;
def MovG2C : MovBase_g2c_v2l<5,0,0b00001011,"movg2c",CPURegs,ALU32_V_SLOT01>;
def MovV2L : MovBase_g2c_v2l<5,0,0b00011011,"movv2l",VPR16Out,Data_Transmission_0_SLOT0>;
def MovV2V : RMovBase<7,4,0b10111000,"movv2v",VPR16Out,VPR16Out,Data_Transmission_1_SLOT01>;
def Vsel : MovBase_vsel<5,0,0b10,"vsel",DSPVSEL,VPR16Out,CPURegs,VPR8Out,Data_Transmission_1_SLOT01>;
def VMovG2V10 : RMovBase<7,4,0b10100111,"vmovrg2v10",VPR16Out,CPURegs,Data_Transmission_0_SLOT0>;
def VMovG2V20 : RMovBase<7,4,0b10100110,"vmovrg2v20",VPR8Out,CPURegs,Data_Transmission_0_SLOT0>;
def VMovG2V40 : RMovBase<7,4,0b10100101,"vmovrg2v40",VPR4Out,CPURegs,Data_Transmission_0_SLOT0>;
//***************************************************************************************************//



//**********************************************A&L instruction**************************************//
//ppp
def MUL32 : ArithLogicR<5,1,0b00001100,"mul32", mul, ALU32_S_SLOT01,CPURegs,1>;
def MULU32 : ArithLogicR<5,1,0b00001101,"mulu32", mul, ALU32_S_SLOT01,CPURegs,1>;
def MUL64 : ArithLogicR<5,1,0b00001110,"mul64", mul, ALU32_S_SLOT0,CPURegs,1>;

def ADDiu :ArithLogicI<5,0x6,0b000,"addi",add,simm11,immSExt11, CPURegs>;
def ADDu : ArithLogicR<5,1,0b00000000, "add", add, ALU32_S_SLOT01, CPURegs, 1>;
def SUBu : ArithLogicR<5,1,0b00000010, "sub", sub, ALU32_S_SLOT01, CPURegs, 1>;
//def MUL : ArithLogicR<5,1,0b00001100,"mul32", mul, ALU32_S_SLOT01,CPURegs,1>;
def ORu :ArithLogicR<5,1,0b00000101,"or",or,ALU32_S_SLOT01,CPURegs,1>;
//def SHL : shift_rotate_imm32<0,1,0b00001001, 0x00, "sl", shl>;

def SHL : shift_rotate_reg<5,1,0b00001001, 0x00, "sl", shl,CPURegs>;
def SRA : shift_rotate_reg<5,1,0b00001010, 0x00, "sra", sra,CPURegs>;
def SRL : shift_rotate_reg<5,1,0b00001011,0x00,"srl",srl,CPURegs>;
def AND : ArithLogicR<5,0x1,0b00000100,"and", and, ALU32_S_SLOT01,CPURegs,1>;
def XORu : ArithLogicR<5,1,0b00000110, "xor", xor, ALU32_S_SLOT01, CPURegs,1>;
def EQ   :SetCC_R<5,4,0b00000000,"eq",seteq,CPURegs>;
def NEQ   :SetCC_R<5,4,0b00000001,"neq",setne,CPURegs>;
def GT   :SetCC_R<5,4,0b00000010,"gt",setgt,CPURegs>;
def GTU   :SetCC_R<5,4,0b00001010,"gtu",setugt,CPURegs>;
def GE   :SetCC_R<5,4,0b00000100,"ge",setge,CPURegs>;
def GEU   :SetCC_R<5,4,0b00001100,"geu",setuge,CPURegs>;
def LT   :SetCC_R<5,4,0b00000011,"lt",setlt,CPURegs>;
def LTU   :SetCC_R<5,4,0b00001011,"ltu",setult,CPURegs>;
def LE   :SetCC_R<5,4,0b00000101,"le",setle,CPURegs>;
def LEU   :SetCC_R<5,4,0b00001101,"leu",setule,CPURegs>;

def EQI	:SetCC_RI<5,5,0b00010,"eqi",seteq,CPURegs>;
def NEQI:SetCC_RI<5,5,0b00110,"neqi",setne,CPURegs>;
def GTI :SetCC_RI<5,5,0b01010,"gti",setgt,CPURegs>;
def LTI :SetCC_RI<5,5,0b01110,"lti",setlt,CPURegs>;
def GEI :SetCC_RI<5,5,0b10010,"gei",setge,CPURegs>;
def LEI	:SetCC_RI<5,5,0b10110,"lei",setle,CPURegs>;


def MAX  :MMBase<5,1,0b00000111,"max",DSPMAX,ALU32_S_SLOT01,CPURegs>;
def MIN  :MMBase<5,1,0b00001000,"min",DSPMIN,ALU32_S_SLOT01,CPURegs>;

//***************************************************************************************************//
//ppp-start
def sextloadi16_a : AlignedLoad<sextloadi16>;
def zextloadi16_a : AlignedLoad<zextloadi16>;
def extloadi16_a : AlignedLoad<extloadi16>;

def sextloadi8_a : AlignedLoad<sextloadi8>;
def zextloadi8_a : AlignedLoad<zextloadi8>;
def extloadi8_a : AlignedLoad<extloadi8>;

def truncstorei16_a : AlignedStore<truncstorei16>;
def truncstorei8_a : AlignedStore<truncstorei8>;
//ppp-end

//**************************************memory instruction*******************************************//
/// Load and Store Instructions
let hasDelaySlot = 1 in 
defm LD : LoadM32<defaultOp.Value,2,0b00010, "load32", load_a>;

defm LDHS : LoadM16<defaultOp.Value,2,0b00001, "load16", sextloadi16_a>;
defm LDHZ : LoadM16<defaultOp.Value,2,0b00001, "load16", zextloadi16_a>;
defm LDBS: LoadM8<defaultOp.Value,2,0b00000, "load8", sextloadi8_a>;
defm LDBZ: LoadM8<defaultOp.Value,2,0b00000, "load8", zextloadi8_a>;
defm LDH : LoadM16<defaultOp.Value,2,0b00001, "load16", extloadi16_a>;
defm LDB: LoadM8<defaultOp.Value,2,0b00000, "load8", extloadi8_a>;
defm STI : StoreM8<7,3,0b00000, "store8",truncstorei8_a>;
defm STH : StoreM16<7,3,0b00001, "store16",truncstorei16_a>;

defm ST : StoreM32<7,3,0b00010, "store32", store_a>;
def LUi : LoadUpper<0,2,0b00011, "lui", CPURegs, uimm16>;
def LU : DSPPseudo<(outs CPURegs:$ra), (ins uimm16:$in), "", []>;

def VSTORE32 : StoreM512<7,3,0b10101,"vstore40",store_a>;

let hasDelaySlot = 1 in 
def VLOAD32 : LoadM512<7,2,0b10001,"vload32",load_a>;

def VSTORE16 : StoreM512_16<7,3,0b10110,"vstore20",store_a>;

let hasDelaySlot = 1 in
def VLOAD16 : LoadM512_16<7,2,0b10010,"vload16",load_a>;

def VSTORE8 : StoreM512_8<7,3,0b10111,"vstore10",store_a>;

let hasDelaySlot = 1 in
def VLOAD8 : LoadM512_8<7,2,0b10011,"vload8",load_a>;

//***************************************************************************************************//

//===----------------------------------------------------------------------===//
// Arbitrary patterns that map to one or more instructions
//===----------------------------------------------------------------------===//

def :Pat<(i32 immSExt32:$in),
(MovGR ZERO , imm:$in)>;

//ppp
//def : Pat<(i32 (sextloadi8  addr:$src)), (LDBS addr:$src)>;
//def : Pat<(i32 (zextloadi8  addr:$src)), (LDBZ addr:$src)>;
//def : Pat<(i32 (extloadi8  addr:$src)), (LDB addr:$src)>;
//def : Pat<(i32 (extloadi16 addr:$src)), (LDH addr:$src)>;
//def : Pat<(i32 (sextloadi16 addr:$addr)), (LDHS addr:$addr) >;
//def : Pat<(i32 (zextloadi16 addr:$addr)), (LDHZ addr:$addr) >;
//def : Pat<(truncstorei8 (i32 CPURegs:$src1), addr:$addr),  (STI  CPURegs:$src1,addr:$addr)>;
//def : Pat<(truncstorei16 (i32 CPURegs:$src1), addr:$addr), (STH  CPURegs:$src1,addr:$addr)>;


//def : Pat < sextloadi16 ADDRriS11_1:$addr)),
            //(LDrih addr:$addr) >;
//def : Pat < (i32 (zextloadi16 ADDRriS11_1:$addr)),
           // (LDriuh ADDRriS11_1:$addr) >;

def :Pat<(i32(DSPHi tglobaladdr:$in)),(MovIGH ZERO, tglobaladdr:$in)>;


def :Pat<(add CPURegs:$hi, (DSPLo tglobaladdr:$lo)),
		(MovIGL CPURegs:$hi, tglobaladdr:$lo)>;

def :Pat<(add CPURegs:$gp,(DSPGPRel tglobaladdr:$in)),
		(ADDiu CPURegs:$gp, tglobaladdr:$in)>;


//def :Pat<(i32(DSPWrapper tconstpool:$dst)),(MovGR ZERO, tconstpool:$dst)>;
//def :Pat<(set CPURegs:$ra, addr:$addr) ,(ADDiu SP, addr:$addr)>;



def: Pat<(DSPHi tconstpool:$in),(MovIGH ZERO, tconstpool:$in)>;

def: Pat<(add CPURegs:$hi,(DSPLo tconstpool:$lo)),
			(MovIGL CPURegs:$hi, tconstpool:$lo)>;

// brcond for cmp instruction (brcond CPURegs:$rt, bb:$imm21)
multiclass BrcondPatsCmp<RegisterClass RC, Instruction JNEOp, Instruction JEQOp> {


def : Pat<(brcond (i32 (setgt RC:$lhs, immSExt16Plus1:$rhs)), bb:$dst),
              (JNEOp (LTI RC:$lhs, (Plus1 imm:$rhs)), bb:$dst)>;
def : Pat<(brcond (i32 (setugt RC:$lhs, immSExt16Plus1:$rhs)), bb:$dst),
              (JNEOp (LTI RC:$lhs, (Plus1 imm:$rhs)), bb:$dst)>;



//def : Pat<(brcond (i32( setgt RC:$lhs, RC:$rhs)), bb:$dst),
			//(JEQOp (GT RC:$lhs,RC:$rhs),bb:$dst)>;

//def : Pat<(brcond (i32( setlt RC:$lhs, immSExt16:$imm9)), bb:$dst),
			//(JEQOp (LTI RC:$lhs,immSExt16:$imm9),bb:$dst)>;

//def : Pat<(brcond (i32( setgt RC:$lhs, immSExt9:$imm9)), bb:$dst),
			//(JNEOp (LTI RC:$lhs,immSExt9:$imm9),bb:$dst)>;

def : Pat<(brcond (i32( seteq RC:$lhs, immSExt16:$imm16)), bb:$dst),
			(JEQOp (EQI RC:$lhs,immSExt16:$imm16),bb:$dst)>;
def : Pat<(brcond (i32( setne RC:$lhs, immSExt16:$imm16)), bb:$dst),
			(JEQOp (NEQI RC:$lhs,immSExt16:$imm16),bb:$dst)>;
//def : Pat<(brcond (i32( setgt RC:$lhs, immSExt9:$imm9)), bb:$dst),
			//(JEQOp (GTI RC:$lhs,immSExt9:$imm9),bb:$dst)>;



def : Pat<(brcond (i32( setle RC:$lhs, immSExt9:$imm9)), bb:$dst),
			(JEQOp (LEI RC:$lhs,immSExt9:$imm9),bb:$dst)>;
//def : Pat<(brcond (i32( setge RC:$lhs, immSExt9:$imm9)), bb:$dst),
		//	(JEQOp (GEI RC:$lhs,immSExt9:$imm9),bb:$dst)>;
def : Pat<(brcond RC:$cond, bb:$dst),
         (JEQOp RC:$cond,bb:$dst)>;
}
defm : BrcondPatsCmp<CPURegs, JNC, JC>;



def : Pat<(DSPJmpLink (i32 tglobaladdr:$dst)),
          (CALL tglobaladdr:$dst)>;
def : Pat<(DSPJmpLink  texternalsym:$dst),
          (CALL texternalsym:$dst)>;





multiclass SeteqPats<RegisterClass RC,Instruction EQ, Instruction NEQ> {
// a == b
def : Pat<(seteq RC:$lhs, RC:$rhs),
(EQ RC:$lhs,RC:$rhs)>;
// a != b
def : Pat<(setne RC:$lhs, RC:$rhs),
(NEQ RC:$lhs,RC:$rhs)>;
}


// a < b
multiclass SetltPats<RegisterClass RC> {
def : Pat<(setlt RC:$lhs, RC:$rhs),
(LT RC:$lhs,RC:$rhs)>;
def : Pat<(setult RC:$lhs, RC:$rhs),
(LTU RC:$lhs,RC:$rhs)>;
}

// a <= b
multiclass SetlePats<RegisterClass RC> {
def : Pat<(setle RC:$lhs, RC:$rhs),
(LE RC:$lhs,RC:$rhs)>;
def : Pat<(setule RC:$lhs, RC:$rhs),
(LEU RC:$lhs,RC:$rhs)>;
}
// a > b
multiclass SetgtPats<RegisterClass RC> {
def : Pat<(setgt RC:$lhs, RC:$rhs),
(GT RC:$lhs,RC:$rhs)>;
def : Pat<(setugt RC:$lhs, RC:$rhs),
(GTU RC:$lhs,RC:$rhs)>;
}

// a >= b
multiclass SetgePats<RegisterClass RC> {
def : Pat<(setge RC:$lhs, RC:$rhs),
(GE RC:$lhs,RC:$rhs)>;
def : Pat<(setuge RC:$lhs, RC:$rhs),
(GEU RC:$lhs,RC:$rhs)>;
}

defm : SeteqPats<CPURegs,EQ,NEQ>;
defm : SetltPats<CPURegs>;
defm : SetlePats<CPURegs>;
defm : SetgtPats<CPURegs>;
defm : SetgePats<CPURegs>;


//****************************************vector instruction*****************************************//
//Instrinsic function operation

def VMAX40 :VMMBase<5,1,0b01001001,"vmax40",DSPVMAX,ALU32_V_SLOT01,VPR4Out>;
def VMAX20 :VMMBase<5,1,0b01001010,"vmax20",DSPVMAX,ALU32_V_SLOT01,VPR8Out>;
def VMAX10 :VMMBase<5,1,0b01001011,"vmax10",DSPVMAX,ALU32_V_SLOT01,VPR16Out>;
def VMIN40 :VMMBase<5,1,0b01001101,"vmin40",DSPVMIN,ALU32_V_SLOT01,VPR4Out>;
def VMIN20 :VMMBase<5,1,0b01001110,"vmin20",DSPVMIN,ALU32_V_SLOT01,VPR8Out>;
def VMIN10 :VMMBase<5,1,0b01001111,"vmin10",DSPVMIN,ALU32_V_SLOT01,VPR16Out>;
def VADD32 :VArithLogicR<5,1,0b01000001,"vadd40",add,ALU32_V_SLOT01,VPR4Out,1>;
def VADD16 :VArithLogicR<5,1,0b01000010,"vadd20",add,ALU32_V_SLOT01,VPR8Out,1>;
def VADD8 :VArithLogicR<5,1,0b01000011,"vadd10",add,ALU32_V_SLOT01,VPR16Out,1>;
def VSUB32 :VArithLogicR<5,1,0b01000001,"vsub40",sub,ALU32_V_SLOT01,VPR4Out,1>;
def VSUB16 :VArithLogicR<5,1,0b01000010,"vsub20",sub,ALU32_V_SLOT01,VPR8Out,1>;
def VSUB8 :VArithLogicR<5,1,0b01000011,"vsub10",sub,ALU32_V_SLOT01,VPR16Out,1>;


//***************************************************************************************************//

class Abs_Fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, 
                  InstrItinClass itin, RegisterClass RC, bit isComm = 0>:
  F_2Reg_2<type, op, funct, (outs CPURegs:$ra), (ins RC:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"),
     [(set CPURegs:$ra, (int_dsp_abs_qb RC:$rb))], itin> {
  let isCommutable = isComm;	// e.g. add rb rc =  add rc rb
  let isReMaterializable = 1;
}

def ABS :Abs_Fmt<5,0x4,0x45, "abs", ALU32_S_SLOT01, CPURegs, 0>;

class Test_Fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, 
                  InstrItinClass itin, RegisterClass RC, bit isComm = 0>:
  F_3Reg_001<type, op, funct, (outs CPURegs:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$rb $rc"),
     [(set CPURegs:$ra, (int_dsp_test_qb RC:$rb, RC:$rc))], itin> {
  let isReMaterializable = 1;
}
def TEST :Test_Fmt<5,0x4,0x46, "test", ALU32_S_SLOT01, CPURegs, 0>;

class Testi_Fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, 
                  InstrItinClass itin, Operand Od, PatLeaf imm_type, RegisterClass RC, bit isComm = 0>:
  F_2Reg_2<type, op, funct, (outs CPURegs:$ra), (ins RC:$rb, Od:$imm5),
     !strconcat(instr_asm, "\t$rb $imm5"),
     [(set CPURegs:$ra, (int_dsp_testi_qb RC:$rb, imm_type:$imm5))], itin> {
  let isReMaterializable = 1;
}
def TESTI :Testi_Fmt<5,0x4,0x47, "testi", ALU32_S_SLOT01, simm5, immSExt5, CPURegs, 0>;

class Cbw_Fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, 
                  InstrItinClass itin, RegisterClass RC, bit isComm = 0>:
  F_2Reg_2<type, op, funct, (outs CPURegs:$ra), (ins RC:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"),
     [(set CPURegs:$ra, (int_dsp_cbw_qb RC:$rb))], itin> {
  let isReMaterializable = 1;
}
def CBW :Cbw_Fmt<5,0x4,0x46, "cbw", ALU32_S_SLOT01, CPURegs, 0>;

class Chw_Fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, 
                  InstrItinClass itin, RegisterClass RC, bit isComm = 0>:
  F_2Reg_2<type, op, funct, (outs CPURegs:$ra), (ins RC:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"),
     [(set CPURegs:$ra, (int_dsp_chw_qb RC:$rb))], itin> {
  let isReMaterializable = 1;
}
def CHW :Chw_Fmt<5,0x4,0x47, "chw", ALU32_S_SLOT01, CPURegs, 0>;

class bfext_fmt<bits<3> type, bits<3> op, bits<4> funct, string instr_asm, InstrItinClass itin, Operand Od, PatLeaf imm_type, RegisterClass RC, bit isComm = 0>:
	F_2Reg_2imm5<type, op, funct, (outs CPURegs:$ra), (ins RC:$rb, Od:$imm5_2, Od:$imm5_1),
	   !strconcat(instr_asm, "\t$ra, $rb, $imm5_2, $imm5_1"),
     [(set CPURegs:$ra, (int_dsp_bfext_qb RC:$rb, imm_type:$imm5_2, imm_type:$imm5_1))], itin> {
  
}

def BFEXT : bfext_fmt<5, 0x6, 0x1, "bfext", ALU32_S_SLOT01, simm5, immSExt5,  CPURegs, 0>;

class bfextu_fmt<bits<3> type, bits<3> op, bits<4> funct, string instr_asm, InstrItinClass itin, Operand Od, PatLeaf imm_type, RegisterClass RC, bit isComm = 0>:
	F_2Reg_2imm5<type, op, funct, (outs CPURegs:$ra), (ins RC:$rb, Od:$imm5_2, Od:$imm5_1),
	   !strconcat(instr_asm, "\t$ra, $rb, $imm5_2, $imm5_1"),
     [(set CPURegs:$ra, (int_dsp_bfextu_qb RC:$rb, imm_type:$imm5_2, imm_type:$imm5_1))], itin> {
  
}

def BFEXTU : bfextu_fmt<5, 0x6, 0x1, "bfextu", ALU32_S_SLOT01, simm5, immSExt5,  CPURegs, 0>;

class bfst_fmt<bits<3> type, bits<3> op, bits<4> funct, string instr_asm, InstrItinClass itin, Operand Od, PatLeaf imm_type, RegisterClass RC, bit isComm = 0>:
	F_2Reg_2imm5<type, op, funct, (outs CPURegs:$ro), (ins RC:$ra, RC:$rb, Od:$imm5_2, Od:$imm5_1),
	   !strconcat(instr_asm, "\t$ra, $rb, $imm5_2, $imm5_1"),
     [(set CPURegs:$ro, (int_dsp_bfst_qb RC:$ra, RC:$rb, imm_type:$imm5_2, imm_type:$imm5_1))], itin> {
  
}

def BFST : bfst_fmt<5, 0x6, 0x1, "bfst", ALU32_S_SLOT01, simm5, immSExt5,  CPURegs, 0>;

class bst_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, Operand Od, PatLeaf imm_type, RegisterClass RC, bit isComm = 0>:
	F_1Reg_imm5<type, op, funct, (outs CPURegs:$ra), (ins Od:$imm5),
	   !strconcat(instr_asm, "\t$ra, $imm5"),
     [(set CPURegs:$ra, (int_dsp_bst_qb imm_type:$imm5))], itin> {
  
}

def BST : bfst_fmt<5, 0x6, 0x1, "bst", ALU32_S_SLOT01, simm5, immSExt5,  CPURegs, 0>;

class bclr_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, Operand Od, PatLeaf imm_type, RegisterClass RC, bit isComm = 0>:
	F_1Reg_imm5<type, op, funct, (outs CPURegs:$ra), (ins Od:$imm5),
	   !strconcat(instr_asm, "\t$ra, $imm5"),
     [(set CPURegs:$ra, (int_dsp_bclr_qb imm_type:$imm5))], itin> {
  
}

def BCLR : bfst_fmt<5, 0x6, 0x1, "bclr", ALU32_S_SLOT01, simm5, immSExt5,  CPURegs, 0>;

class veq40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
	F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
	 !strconcat(instr_asm, "\t$ra, $rb, $rc"),
	 [(set RC:$ra, (int_dsp_veq_40 RC:$rb, RC:$rc))], itin> {

	 }

def veq_40 : veq40_fmt<5, 1, 0b00100001, "veq40", ALU32_V_SLOT01, VPR4Out>;

class veq_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_veq_20 RC:$rb, RC:$rc))], itin> {

   }

  def veq_20 : veq_20_fmt<5, 1, 0b00100010, "veq20", ALU32_V_SLOT01, VPR8Out>;

  class veq_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_veq_10 RC:$rb, RC:$rc))], itin> {

   }

  def veq_10 : veq_10_fmt<5, 1, 0b00100011, "veq10", ALU32_V_SLOT01, VPR16Out>;

  class vgt40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vgt_40 RC:$rb, RC:$rc))], itin> {

   }

def vgt_40 : vgt40_fmt<5, 1, 0b00100101, "vgt40", ALU32_V_SLOT01, VPR4Out>;

class vgt_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vgt_20 RC:$rb, RC:$rc))], itin> {

   }

  def vgt_20 : vgt_20_fmt<5, 1, 0b00100110, "vgt20", ALU32_V_SLOT01, VPR8Out>;

  class vgt_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vgt_10 RC:$rb, RC:$rc))], itin> {

   }

  def vgt_10 : vgt_10_fmt<5, 1, 0b00100111, "vgt10", ALU32_V_SLOT01, VPR16Out>;
  //-----------------------------------------------------------------------------//
  class vlt40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vlt_40 RC:$rb, RC:$rc))], itin> {

   }

def vlt_40 : vlt40_fmt<5, 1, 0b00100001, "vlt40", ALU32_V_SLOT01, VPR4Out>;

class vlt_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vlt_20 RC:$rb, RC:$rc))], itin> {

   }

  def vlt_20 : vlt_20_fmt<5, 1, 0b00100010, "vlt20", ALU32_V_SLOT01, VPR8Out>;

  class vlt_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vlt_10 RC:$rb, RC:$rc))], itin> {

   }

  def vlt_10 : vlt_10_fmt<5, 1, 0b00100011, "vlt10", ALU32_V_SLOT01, VPR16Out>;
  //-----------------------------------------------------------------------------//
  class vge40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vge_40 RC:$rb, RC:$rc))], itin> {

   }

def vge_40 : vge40_fmt<5, 1, 0b00100001, "vge40", ALU32_V_SLOT01, VPR4Out>;

class vge_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vge_20 RC:$rb, RC:$rc))], itin> {

   }

  def vge_20 : vge_20_fmt<5, 1, 0b00100010, "vge20", ALU32_V_SLOT01, VPR8Out>;

  class vge_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vge_10 RC:$rb, RC:$rc))], itin> {

   }

  def vge_10 : vge_10_fmt<5, 1, 0b00100011, "vge10", ALU32_V_SLOT01, VPR16Out>;
  //-----------------------------------------------------------------------------//
  class vle40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vle_40 RC:$rb, RC:$rc))], itin> {

   }

def vle_40 : vle40_fmt<5, 1, 0b00100001, "vle40", ALU32_V_SLOT01, VPR4Out>;

class vle_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vle_20 RC:$rb, RC:$rc))], itin> {

   }

  def vle_20 : vle_20_fmt<5, 1, 0b00100010, "vle20", ALU32_V_SLOT01, VPR8Out>;

  class vle_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vle_10 RC:$rb, RC:$rc))], itin> {

   }

  def vle_10 : vle_10_fmt<5, 1, 0b00100011, "vle10", ALU32_V_SLOT01, VPR16Out>;

  //ppp
  class vand_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm,SDNode OpNode, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (OpNode RC:$rb, RC:$rc))], itin> {
   let isCommutable = 1;
   }
   def vand : vand_fmt<5, 1, 0b01000000, "vand",DSPVAND, ALU32_V_SLOT01, VPR16Out>;
  class vxor_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm,SDNode OpNode, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (OpNode RC:$rb, RC:$rc))], itin> {
   let isCommutable = 1;
   }
   def vxor : vxor_fmt<5, 1, 0b01001000, "vxor",DSPVXOR, ALU32_V_SLOT01, VPR16Out>;
  class vor_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm,SDNode OpNode, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (OpNode RC:$rb, RC:$rc))], itin> {
   let isCommutable = 1;
   }
   def vor : vor_fmt<5, 1, 0b01000100, "vor",DSPVOR, ALU32_V_SLOT01, VPR16Out>;
  class vnot_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm,SDNode OpNode, InstrItinClass itin, RegisterClass RC, bit isComm = 0>:
  F_2Reg_2<type, op, funct, (outs RC:$ra), (ins RC:$rb),
   !strconcat(instr_asm, "\t$ra, $rb"),
   [(set RC:$ra, (OpNode RC:$rb))], itin> {
   let isCommutable = 0;
   }
	def vnot_0 : vnot_fmt<5, 4, 0b11110100, "vnot",DSPVNOT, ALU32_V_SLOT01, VPR16Out,0>;
   class not_Fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, SDNode OpNode,InstrItinClass itin, RegisterClass RC, bit isComm = 0>:
  F_2Reg_2<type, op, funct, (outs RC:$ra), (ins RC:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"),
     [(set RC:$ra, (OpNode RC:$rb))], itin> {
	 let isCommutable = 0;
	}
	def not_0 :not_Fmt<5,0x4,0b01000100, "not",DSPNOT, ALU32_S_SLOT01, CPURegs, 0>;
	class vmovcv2v_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm,SDNode OpNode, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (OpNode RC:$rb, RC:$rc))], itin> {
   let isCommutable = 0;
   }
   def vmovcv2v : vmovcv2v_fmt<5, 1, 0b01100000, "vmovcv2v_fmt",DSPVMOVCV2V, Data_Transmission_2_SLOT0123, VPR16Out>;

  //-----------------------------------------------------------------------------//
  class vadd40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vadd_40 RC:$rb, RC:$rc))], itin> {

   }

def vadd_40 : vadd40_fmt<5, 1, 0b00100001, "vadd40", ALU32_V_SLOT01, VPR4Out>;

class vadd_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vadd_20 RC:$rb, RC:$rc))], itin> {

   }

  def vadd_20 : vadd_20_fmt<5, 1, 0b00100010, "vadd20", ALU32_V_SLOT01, VPR8Out>;

  class vadd_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vadd_10 RC:$rb, RC:$rc))], itin> {

   }

  def vadd_10 : vadd_10_fmt<5, 1, 0b00100011, "vadd10", ALU32_V_SLOT01, VPR16Out>;
  //-----------------------------------------------------------------------------//
  class vsub40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsub_40 RC:$rb, RC:$rc))], itin> {

   }

def vsub_40 : vsub40_fmt<5, 1, 0b00100001, "vsub40", ALU32_V_SLOT01, VPR4Out>;

class vsub_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsub_20 RC:$rb, RC:$rc))], itin> {

   }

  def vsub_20 : vsub_20_fmt<5, 1, 0b00100010, "vsub20", ALU32_V_SLOT01, VPR8Out>;

  class vsub_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsub_10 RC:$rb, RC:$rc))], itin> {

   }

  def vsub_10 : vsub_10_fmt<5, 1, 0b00100011, "vsub10", ALU32_V_SLOT01, VPR16Out>;




  //-----------------------------------------------------------------------------//
  class vsl40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsl_40 RC:$rb, RC:$rc))], itin> {

   }

def vsl_40 : vsl40_fmt<5, 1, 0b00100001, "vsl40", ALU32_V_SLOT01, VPR4Out>;

class vsl_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsl_20 RC:$rb, RC:$rc))], itin> {

   }

  def vsl_20 : vsl_20_fmt<5, 1, 0b00100010, "vsl20", ALU32_V_SLOT01, VPR8Out>;

  class vsl_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsl_10 RC:$rb, RC:$rc))], itin> {

   }

  def vsl_10 : vsl_10_fmt<5, 1, 0b00100011, "vsl10", ALU32_V_SLOT01, VPR16Out>;
  //-----------------------------------------------------------------------------//
  class vsls40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsls_40 RC:$rb, RC:$rc))], itin> {

   }

def vsls_40 : vsls40_fmt<5, 1, 0b00100001, "vsls40", ALU32_V_SLOT01, VPR4Out>;

class vsls_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsls_20 RC:$rb, RC:$rc))], itin> {

   }

  def vsls_20 : vsls_20_fmt<5, 1, 0b00100010, "vsls20", ALU32_V_SLOT01, VPR8Out>;

  class vsls_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsls_10 RC:$rb, RC:$rc))], itin> {

   }

  def vsls_10 : vsls_10_fmt<5, 1, 0b00100011, "vsls10", ALU32_V_SLOT01, VPR16Out>;
  //-----------------------------------------------------------------------------//
  class vsra40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsra_40 RC:$rb, RC:$rc))], itin> {

   }

def vsra_40 : vsra40_fmt<5, 1, 0b00100001, "vsra40", ALU32_V_SLOT01, VPR4Out>;

class vsra_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsra_20 RC:$rb, RC:$rc))], itin> {

   }

  def vsra_20 : vsra_20_fmt<5, 1, 0b00100010, "vsra20", ALU32_V_SLOT01, VPR8Out>;

  class vsra_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsra_10 RC:$rb, RC:$rc))], itin> {

   }

  def vsra_10 : vsra_10_fmt<5, 1, 0b00100011, "vsra10", ALU32_V_SLOT01, VPR16Out>;
  //-----------------------------------------------------------------------------//
  class vsrl40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsrl_40 RC:$rb, RC:$rc))], itin> {

   }

def vsrl_40 : vsrl40_fmt<5, 1, 0b00100001, "vsrl.40", ALU32_V_SLOT01, VPR4Out>;

class vsrl_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsrl_20 RC:$rb, RC:$rc))], itin> {

   }

  def vsrl_20 : vsrl_20_fmt<5, 1, 0b00100010, "vsrl20", ALU32_V_SLOT01, VPR8Out>;

  class vsrl_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vsrl_10 RC:$rb, RC:$rc))], itin> {

   }

  def vsrl_10 : vsrl_10_fmt<5, 1, 0b00100011, "vsrl10", ALU32_V_SLOT01, VPR16Out>;
  //-----------------------------------------------------------------------------//
  class vabs40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_2Reg_2<type, op, funct, (outs RC:$ra), (ins RC:$rb),
   !strconcat(instr_asm, "\t$ra, $rb"),
   [(set RC:$ra, (int_dsp_vabs_40 RC:$rb))], itin> {

   }

def vabs_40 : vabs40_fmt<5, 1, 0b00100001, "vabs40", ALU32_V_SLOT01, VPR4Out>;

class vabs_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_2Reg_2<type, op, funct, (outs RC:$ra), (ins RC:$rb),
   !strconcat(instr_asm, "\t$ra, $rb"),
   [(set RC:$ra, (int_dsp_vabs_20 RC:$rb))], itin> {

   }

  def vabs_20 : vabs_20_fmt<5, 1, 0b00100010, "vabs20", ALU32_V_SLOT01, VPR8Out>;

  class vabs_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_2Reg_2<type, op, funct, (outs RC:$ra), (ins RC:$rb),
   !strconcat(instr_asm, "\t$ra, $rb"),
   [(set RC:$ra, (int_dsp_vabs_10 RC:$rb))], itin> {

   }

  def vabs_10 : vabs_10_fmt<5, 1, 0b00100011, "vabs10", ALU32_V_SLOT01, VPR16Out>;

class vsum40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_2Reg_2<type, op, funct, (outs RC:$ra), (ins RC:$rb),
   !strconcat(instr_asm, "\t$ra, $rb"),
   [(set RC:$ra, (int_dsp_vsum_40 RC:$rb))], itin> {

   }

def vsum_40 : vsum40_fmt<5, 1, 0b00100001, "vsum40", ALU32_V_SLOT01, VPR4Out>;

class vsum_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_2Reg_2<type, op, funct, (outs RC:$ra), (ins RC:$rb),
   !strconcat(instr_asm, "\t$ra, $rb"),
   [(set RC:$ra, (int_dsp_vsum_20 RC:$rb))], itin> {

   }

  def vsum_20 : vsum_20_fmt<5, 1, 0b00100010, "vsum20", ALU32_V_SLOT01, VPR8Out>;

  class vsum_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_2Reg_2<type, op, funct, (outs RC:$ra), (ins RC:$rb),
   !strconcat(instr_asm, "\t$ra, $rb"),
   [(set RC:$ra, (int_dsp_vsum_10 RC:$rb))], itin> {

   }

  def vsum_10 : vsum_10_fmt<5, 1, 0b00100011, "vsum10", ALU32_V_SLOT01, VPR16Out>;
  //-----------------------------------------------------------------------------//
  class vfmul40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vfmul_40 RC:$rb, RC:$rc))], itin> {

   }

def vfmul_40 : vfmul40_fmt<5, 1, 0b00100001, "vfmul40", ALU32_V_SLOT01, VPR4Out>;

class vfmul_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vfmul_20 RC:$rb, RC:$rc))], itin> {

   }

  def vfmul_20 : vfmul_20_fmt<5, 1, 0b00100010, "vfmul20", ALU32_V_SLOT01, VPR8Out>;

  class vfmul_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vfmul_10 RC:$rb, RC:$rc))], itin> {

   }

  def vfmul_10 : vfmul_10_fmt<5, 1, 0b00100011, "vfmul10", ALU32_V_SLOT01, VPR16Out>;
  //-----------------------------------------------------------------------------//
  class vfmac40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$dst, RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vfmac_40 RC:$dst, RC:$rb, RC:$rc))], itin> {
	 let Constraints = "$ra = $dst";
   }

def vfmac_40 : vfmac40_fmt<5, 1, 0b00100001, "vfmac40", ALU32_V_SLOT01, VPR4Out>;

class vfmac_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vfmac_20 RC:$rb, RC:$rc))], itin> {

   }

  def vfmac_20 : vfmac_20_fmt<5, 1, 0b00100010, "vfmac20", ALU32_V_SLOT01, VPR8Out>;

  class vfmac_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vfmac_10 RC:$rb, RC:$rc))], itin> {

   }

  def vfmac_10 : vfmac_10_fmt<5, 1, 0b00100011, "vfmac10", ALU32_V_SLOT01, VPR16Out>;
  //-----------------------------------------------------------------------------//
class vcmac_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vcmac_20 RC:$rb, RC:$rc))], itin> {

   }

  def vcmac_20 : vcmac_20_fmt<5, 1, 0b00100010, "vcmac20", ALU32_V_SLOT01, VPR8Out>;

  class vcmac_10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vcmac_10 RC:$rb, RC:$rc))], itin> {

   }

  def vcmac_10 : vcmac_10_fmt<5, 1, 0b00100011, "vcmac10", ALU32_V_SLOT01, VPR16Out>;
  //-----------------------------------------------------------------------------//

class vcmul_20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vcmul_20 RC:$rb, RC:$rc))], itin> {

   }

  def vcmul_20 : vcmul_20_fmt<5, 1, 0b00100010, "vcmul20", ALU32_V_SLOT01, VPR8Out>;

    class vcmulr40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vcmulr_40 RC:$rb, RC:$rc))], itin> {

   }

def vcmulr_40 : vcmulr40_fmt<5, 1, 0b00100001, "vcmulr40", ALU32_V_SLOT01, VPR4Out>;

    class vcmuli40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RC>:
  F_3Reg_001<type, op, funct, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (int_dsp_vcmuli_40 RC:$rb, RC:$rc))], itin> {

   }

def vcmuli_40 : vcmuli40_fmt<5, 1, 0b00100001, "vcmuli40", ALU32_V_SLOT01, VPR4Out>;

class vloadr8_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_vloadr_8 RS:$rb, imm_type:$imm9))], itin> {

   }

def vloadr_8 : vloadr8_fmt<7, 0b010, 0b10111, "vloadr8", LD_Ins_SLOT23, VPR16Out, CPURegs, simm9, immSExt9>;

class vloadr16_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_vloadr_16 RS:$rb, imm_type:$imm9))], itin> {

   }

def vloadr_16 : vloadr16_fmt<7, 0b010, 0b10110, "vloadr16", LD_Ins_SLOT23, VPR8Out, CPURegs, simm9, immSExt9>;

class vloadr32_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_vloadr_32 RS:$rb, imm_type:$imm9))], itin> {

   }

def vloadr_32 : vloadr32_fmt<7, 0b010, 0b10101, "vloadr32", LD_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class loadu8_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_loadu_8 RS:$rb, imm_type:$imm9))], itin> {

   }

def loadu_8 : loadu8_fmt<7, 0b010, 0b01000, "loadu8", LD_Ins_SLOT23, CPURegs, CPURegs, simm9, immSExt9>;

class loadu16_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_loadu_16 RS:$rb, imm_type:$imm9))], itin> {

   }

def loadu_16 : loadu16_fmt<7, 0b010, 0b01001, "loadu16", LD_Ins_SLOT23, CPURegs, CPURegs, simm9, immSExt9>;

class loadu32_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_loadu_32 RS:$rb, imm_type:$imm9))], itin> {

   }

def loadu_32 : loadu32_fmt<7, 0b010, 0b01010, "loadu32", LD_Ins_SLOT23, CPURegs, CPURegs, simm9, immSExt9>;

class vloadu8_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_vloadu_8 RS:$rb, imm_type:$imm9))], itin> {

   }

def vloadu_8 : vloadu8_fmt<7, 0b010, 0b11011, "vloadu8", LD_Ins_SLOT23, VPR16Out, CPURegs, simm9, immSExt9>;

class vloadu16_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_vloadu_16 RS:$rb, imm_type:$imm9))], itin> {

   }

def vloadu_16 : vloadu16_fmt<7, 0b010, 0b11010, "vloadu16", LD_Ins_SLOT23, VPR8Out, CPURegs, simm9, immSExt9>;

class vloadu32_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_vloadu_32 RS:$rb, imm_type:$imm9))], itin> {

   }

def vloadu_32 : vloadu32_fmt<7, 0b010, 0b11001, "vloadu32", LD_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class vloadur8_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_vloadur_8 RS:$rb, imm_type:$imm9))], itin> {

   }

def vloadur_8 : vloadur8_fmt<7, 0b010, 0b11111, "vloadur8", LD_Ins_SLOT23, VPR16Out, CPURegs, simm9, immSExt9>;

class vloadur16_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_vloadur_16 RS:$rb, imm_type:$imm9))], itin> {

   }

def vloadur_16 : vloadur16_fmt<7, 0b010, 0b11110, "vloadur16", LD_Ins_SLOT23, VPR8Out, CPURegs, simm9, immSExt9>;

class vloadur32_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_vloadur_32 RS:$rb, imm_type:$imm9))], itin> {

   }

def vloadur_32 : vloadur32_fmt<7, 0b010, 0b11101, "vloadur32", LD_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class storeu8_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs), (ins  RD:$ra, RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(int_dsp_storeu_8  RD:$ra, RS:$rb, imm_type:$imm9)], itin> {
   }

def storeu_8 : storeu8_fmt<7, 0b011, 0b01000, "storeu8", ST_Ins_SLOT23, CPURegs, CPURegs, simm9, immSExt9>;

class storeu16_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs), (ins  RD:$ra,RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(int_dsp_storeu_16  RD:$ra,RS:$rb, imm_type:$imm9)], itin> {

   }

def storeu_16 : storeu16_fmt<7, 0b011, 0b01001, "storeu16", ST_Ins_SLOT23, CPURegs, CPURegs, simm9, immSExt9>;

class storeu32_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs), (ins  RD:$ra, RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(int_dsp_storeu_32  RD:$ra,  RS:$rb, imm_type:$imm9)], itin> {

   }

def storeu_32 : storeu32_fmt<7, 0b011, 0b01010, "storeu32", ST_Ins_SLOT23, CPURegs, CPURegs, simm9, immSExt9>;

class storeu10_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_storeu_10 RS:$rb, imm_type:$imm9))], itin> {

   }

def storeu_10 : storeu10_fmt<7, 0b011, 0b11011, "storeu10", ST_Ins_SLOT23, VPR16Out, CPURegs, simm9, immSExt9>;

class storeu20_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_storeu_20 RS:$rb, imm_type:$imm9))], itin> {

   }

def storeu_20 : storeu20_fmt<7, 0b011, 0b11010, "storeu20", ST_Ins_SLOT23, VPR8Out, CPURegs, simm9, immSExt9>;

class storeu40_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_storeu_40 RS:$rb, imm_type:$imm9))], itin> {

   }

def storeu_40 : storeu40_fmt<7, 0b011, 0b11001, "storeu40", ST_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class vstoreu10_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_vstoreu_10 RS:$rb, imm_type:$imm9))], itin> {

   }

def vstoreu_10 : vstoreu10_fmt<7, 0b011, 0b11111, "vstoreu10", ST_Ins_SLOT23, VPR16Out, CPURegs, simm9, immSExt9>;

class vstoreu20_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_vstoreu_20 RS:$rb, imm_type:$imm9))], itin> {

   }

def vstoreu_20 : vstoreu20_fmt<7, 0b011, 0b11110, "vstoreu20", ST_Ins_SLOT23, VPR8Out, CPURegs, simm9, immSExt9>;

class vstoreu40_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_vstoreu_40 RS:$rb, imm_type:$imm9))], itin> {

   }

def vstoreu_40 : vstoreu40_fmt<7, 0b011, 0b11101, "vstoreu40", ST_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class loadvh_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_loadvh RS:$rb, imm_type:$imm9))], itin> {

   }

def loadvh : loadvh_fmt<7, 0b011, 0b10000, "loadvh", LD_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class loadvl_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_loadvl RS:$rb, imm_type:$imm9))], itin> {

   }

def loadvl : loadvl_fmt<7, 0b011, 0b10100, "loadvl", LD_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class storevh_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_storevh RS:$rb, imm_type:$imm9))], itin> {

   }

def storevh : storevh_fmt<7, 0b011, 0b10000, "storevh", ST_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class storevl_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_storevl RS:$rb, imm_type:$imm9))], itin> {

   }

def storevl : storevl_fmt<7, 0b011, 0b10100, "storevl", ST_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class loaduvh_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_loaduvh RS:$rb, imm_type:$imm9))], itin> {

   }

def loaduvh : loaduvh_fmt<7, 0b011, 0b11000, "loaduvh", LD_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class loaduvl_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_loaduvl RS:$rb, imm_type:$imm9))], itin> {

   }

def loaduvl : loaduvl_fmt<7, 0b011, 0b11100, "loaduvl", LD_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class storeuvh_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_storeuvh RS:$rb, imm_type:$imm9))], itin> {

   }

def storeuvh : storeuvh_fmt<7, 0b011, 0b11000, "storeuvh", ST_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class storeuvl_fmt<bits<3> type, bits<3> op, bits<5> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS, Operand Od, PatLeaf imm_type>:
  F_LS_2Reg_imm9<type, op, funct, (outs RD:$ra), (ins RS:$rb, Od:$imm9),
   !strconcat(instr_asm, "\t$ra, $rb, $imm9"),
   [(set RD:$ra, (int_dsp_storeuvl RS:$rb, imm_type:$imm9))], itin> {

   }

def storeuvl : storeuvl_fmt<7, 0b011, 0b11100, "storeuvl", ST_Ins_SLOT23, VPR4Out, CPURegs, simm9, immSExt9>;

class loado16_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS>:
  F_3Reg_001<type, op, funct, (outs RD:$ra), (ins RS:$rb, RS:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RD:$ra, (int_dsp_loado_16 RS:$rb, RS:$rc))], itin> {

   }

def loado_16 : loado16_fmt<7, 0b001, 0b11000010, "loado16", LD_Ins_SLOT23, CPURegs, CPURegs>;

class loado32_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS>:
  F_3Reg_001<type, op, funct, (outs RD:$ra), (ins RS:$rb, RS:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RD:$ra, (int_dsp_loado_32 RS:$rb, RS:$rc))], itin> {

   }

def loado_32 : loado32_fmt<7, 0b001, 0b11000001, "loado32", LD_Ins_SLOT23, CPURegs, CPURegs>;

class vloado8_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS>:
  F_3Reg_001<type, op, funct, (outs RD:$ra), (ins RS:$rb, RS:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RD:$ra, (int_dsp_vloado_8 RS:$rb, RS:$rc))], itin> {

   }

def vloado_8 : vloado8_fmt<7, 0b001, 0b11100011, "vloado8", LD_Ins_SLOT23, VPR16Out, CPURegs>;

class vloado16_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS>:
  F_3Reg_001<type, op, funct, (outs RD:$ra), (ins RS:$rb, RS:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RD:$ra, (int_dsp_vloado_16 RS:$rb, RS:$rc))], itin> {

   }

def vloado_16 : vloado16_fmt<7, 0b001, 0b11100010, "vloado16", LD_Ins_SLOT23, VPR8Out, CPURegs>;

class vloado32_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS>:
  F_3Reg_001<type, op, funct, (outs RD:$ra), (ins RS:$rb, RS:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RD:$ra, (int_dsp_vloado_32 RS:$rb, RS:$rc))], itin> {

   }

def vloado_32 : vloado32_fmt<7, 0b001, 0b11100001, "vloado32", LD_Ins_SLOT23, VPR4Out, CPURegs>;

class storeo16_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS>:
  F_3Reg_001<type, op, funct, (outs RD:$ra), (ins RS:$rb, RS:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RD:$ra, (int_dsp_storeo_16 RS:$rb, RS:$rc))], itin> {

   }

def storeo_16 : storeo16_fmt<7, 0b001, 0b11010010, "storeo16", ST_Ins_SLOT23, CPURegs, CPURegs>;

class storeo32_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS>:
  F_3Reg_001<type, op, funct, (outs RD:$ra), (ins RS:$rb, RS:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RD:$ra, (int_dsp_storeo_32 RS:$rb, RS:$rc))], itin> {

   }

def storeo_32 : storeo32_fmt<7, 0b001, 0b11010001, "storeo32", ST_Ins_SLOT23, CPURegs, CPURegs>;

class vstoreo10_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS>:
  F_3Reg_001<type, op, funct, (outs RD:$ra), (ins RS:$rb, RS:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RD:$ra, (int_dsp_vstoreo_10 RS:$rb, RS:$rc))], itin> {

   }

def vstoreo_10 : vstoreo10_fmt<7, 0b001, 0b11110011, "vstoreo10", ST_Ins_SLOT23, VPR16Out, CPURegs>;

class vstoreo20_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS>:
  F_3Reg_001<type, op, funct, (outs RD:$ra), (ins RS:$rb, RS:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RD:$ra, (int_dsp_vstoreo_20 RS:$rb, RS:$rc))], itin> {

   }

def vstoreo_20 : vstoreo20_fmt<7, 0b001, 0b11110010, "vstoreo20", ST_Ins_SLOT23, VPR8Out, CPURegs>;

class vstoreo40_fmt<bits<3> type, bits<3> op, bits<8> funct, string instr_asm, InstrItinClass itin, RegisterClass RD, RegisterClass RS>:
  F_3Reg_001<type, op, funct, (outs RD:$ra), (ins RS:$rb, RS:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RD:$ra, (int_dsp_vstoreo_40 RS:$rb, RS:$rc))], itin> {

   }

def vstoreo_40 : vstoreo40_fmt<7, 0b001, 0b11110001, "vstoreo40", ST_Ins_SLOT23, VPR4Out, CPURegs>;



//===----------------------------------------------------------------------===//
// Pseudo instructions
//===----------------------------------------------------------------------===//


// As stack alignment is always done with addiu, we need a 16-bit immediate
let Defs = [SP], Uses = [SP] in {
def ADJCALLSTACKDOWN : DSPPseudo<(outs), (ins uimm16:$amt),
                                  "!ADJCALLSTACKDOWN $amt",
                                  [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP   : DSPPseudo<(outs), (ins uimm16:$amt1, uimm16:$amt2),
                                  "!ADJCALLSTACKUP $amt1",
                                  [(callseq_end timm:$amt1, timm:$amt2)]>;
}

// When handling PIC code the assembler needs .cpload and .cprestore
// directives. If the real instructions corresponding these directives
// are used, we have the same behavior, but get also a bunch of warnings
// from the assembler.
let neverHasSideEffects = 1 in
def CPRESTORE : DSPPseudo<(outs), (ins i32imm:$loc, CPURegs:$gp),
                           ".cprestore\t$loc", []>;

