class Format<bits<5> Val>{
	bits<5> Value = Val;
}

def Pseudo : Format<0>;
def FrmR0 : Format<1>;
def FrmR1 : Format<2>;
def FrmR2 : Format<3>;
def FrmR3 : Format<4>;
def Frm5      : Format<5>;
def Frm6      : Format<6>;
def Frm7      : Format<7>;
def Frm8      : Format<8>;
def Frm9      : Format<9>;
def Frm10      : Format<10>;
def Frm11      : Format<11>;
def Frm12      : Format<12>;
def Frm13      : Format<13>;
def Frm14      : Format<14>;
def FrmOther : Format<15>; // Instruction w/a custom format
def FrmENDLOOP :Format<31>;

class OpFormat<bits<3> Op>{
	bits<3> Value = Op;
}

def defaultOp : OpFormat<7>;

class SlotMode<bits<2> value>{
	bits<2> Value = value;
}

def Slot0 :SlotMode<0>;
def Slot1 :SlotMode<1>;
def Slot2 :SlotMode<2>;
def Slot3 :SlotMode<3>;


class InstDSP<dag outs, dag ins, string asmstr, list<dag> pattern,InstrItinClass itin>
          : Instruction {
  field bits<32> Inst;

  let Namespace = "DSP";
  let Size = 4;
  bits<3> opcode = 0 ;
  bits<3> typecode = 0;

  let Inst{31-29} = typecode;   // top 3 is typecode 
  let Inst{28-26} = opcode;     

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString   = asmstr;
  let Pattern = pattern;
  let Itinerary = itin;

  // *** Must match MCTargetDesc/DSPBaseInfo.h ***

  // Instruction type according to the ISA.
  //IType Type = type;
  bits<5> Type = 0;
  let TSFlags{4-0} = Type;

  // Solo instructions, i.e., those that cannot be in a packet with others.
  bits<1> isSolo = 0;
  let TSFlags{5} = isSolo;
  // Packed only with A or X-type instructions.
  bits<1> isSoloAX = 0;
  let TSFlags{6} = isSoloAX;
  // Only A-type instruction in first slot or nothing.
  bits<1> isSoloAin1 = 0;
  let TSFlags{7} = isSoloAin1;

  // Predicated instructions.
  bits<1> isPredicated = 0;
  let TSFlags{8} = isPredicated;
  bits<1> isPredicatedFalse = 0;
  let TSFlags{9} = isPredicatedFalse;
  bits<1> isPredicatedNew = 0;
  let TSFlags{10} = isPredicatedNew;
  bits<1> isPredicateLate = 0;
  let TSFlags{11} = isPredicateLate; // Late predicate producer insn.

  // New-value insn helper fields.
  bits<1> isNewValue = 0;
  let TSFlags{12} = isNewValue; // New-value consumer insn.
  bits<1> hasNewValue = 0;
  let TSFlags{13} = hasNewValue; // New-value producer insn.
  bits<3> opNewValue = 0;
  let TSFlags{16-14} = opNewValue; // New-value produced operand.
  bits<1> isNVStorable = 0;
  let TSFlags{17} = isNVStorable; // Store that can become new-value store.
  bits<1> isNVStore = 0;
  let TSFlags{18} = isNVStore; // New-value store insn.
  bits<1> isCVLoadable = 0;
  let TSFlags{19} = isCVLoadable; // Load that can become cur-value load.
  bits<1> isCVLoad = 0;
  let TSFlags{20} = isCVLoad; // Cur-value load insn.

  // Immediate extender helper fields.
  bits<1> isExtendable = 0;
  let TSFlags{21} = isExtendable; // Insn may be extended.
  bits<1> isExtended = 0;
  let TSFlags{22} = isExtended; // Insn must be extended.
  bits<3> opExtendable = 0;
  let TSFlags{25-23} = opExtendable; // Which operand may be extended.
  bits<1> isExtentSigned = 0;
  let TSFlags{26} = isExtentSigned; // Signed or unsigned range.
  bits<5> opExtentBits = 0;
  let TSFlags{31-27} = opExtentBits; //Number of bits of range before extending.
  bits<2> opExtentAlign = 0;
  let TSFlags{33-32} = opExtentAlign; // Alignment exponent before extending.

  // If an instruction is valid on a subtarget (v2-v5), set the corresponding
  // bit from validSubTargets. v2 is the least significant bit.
  // By default, instruction is valid on all subtargets.
  bits<4> validSubTargets =0xf ;
  let TSFlags{37-34} = validSubTargets;

  // Addressing mode for load/store instructions.
  bits<3> addrMode = 0;
  let TSFlags{42-40} = addrMode;

  // Memory access size for mem access instructions (load/store)
  bits<3> accessSize = 0;
  let TSFlags{45-43} = accessSize;

  bits<1> isTaken = 0;
  let TSFlags {47} = isTaken; // Branch prediction.

  bits<1> isFP = 0;
  let TSFlags {48} = isFP; // Floating-point.

  SlotMode Slots = Slot3;
  let TSFlags{49-50} = Slots.Value;

  bits<1> Start = 1;
  let TSFlags{51} = Start;

  let DecoderNamespace = "DSP";
  field bits<32> SoftFail = 0;

}

// DSP Pseudo Instructions Format
class DSPPseudo<dag outs, dag ins, string asmstr, list<dag> pattern>:
InstDSP<outs, ins, asmstr, pattern, PSEUDO> {
let isCodeGenOnly = 1;
let isPseudo = 1;
}





//two registers
/*

/*================================================
typecode|opcode|rd|rs|imm|inner_op<10>
================================================*/
class R2i10< bits<3> typecode, bits<3> opcode,bits<10> inner_op,dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin>
   :InstDSP<outs, ins, asmstr, pattern, itin>{
  bits<6> rd;
  bits<6> rs;
  bits<4> imm;
  bits<10> inner_op;

  let Inst{31-29} = typecode;
  let Inst{28-26} = opcode;
  let Inst{25-20}=rd;
  let Inst{19-14}=rs;
  let Inst{13-10}=imm;
  }

*/

/*================================================
typecode|opcode|rd|rs|imm|inner_op<5>
================================================*/
class R2i5<bits<3>typecode,bits<3>opcode, bits<5> inner_op, dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin>
   :InstDSP<outs, ins, asmstr, pattern, itin>{
  bits<6> rd;
  bits<6> rs;
  bits<9> imm;
  let Inst{31-29} = typecode;
  let Inst{28-26} = opcode;
  let Inst{25-20}=rd;
  let Inst{19-14}=rs;
  let Inst{13-5}=imm;
  let Inst{4-0}=inner_op;
  }





//three registers




/*================================================
typecode|opcode|rd|rs|rt|inner_op<8>
=================================================*/
class R3<bits<3>typecode, bits<3>opcode, bits<8> inner_op,dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin>
   :InstDSP<outs, ins, asmstr, pattern, itin>{
  bits<6> rd;
  bits<6> rs;
  bits<6> rt;
  let Inst{31-29} = typecode;
  let Inst{28-26} = opcode;
  let Inst{25-20}=rd;
  let Inst{19-14}=rs;
  let Inst{13-8}=rt;
  let Inst{7-0}=inner_op;
  }



//special


/*================================================
   four register
  typecode |opcode|rd|rs|rt|rt1|inner_op<2>
 ================================================*/
   
class R4i2<bits<3>typecode, bits<3>opcode,bits<2> inner_op, dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin>
   :InstDSP<outs, ins, asmstr, pattern, itin>{
  bits<6> rd;
  bits<6> rs;
  bits<6> rt;
  bits<6> rt1;
  let Inst{31-29} = typecode;
  let Inst{28-26} = opcode;
  let Inst{25-20}=rd;
  let Inst{19-14}=rs;
  let Inst{13-8}=rt;
  let Inst{7-2}=rt1;
  let Inst{1-0}=inner_op;
  }




  
  //===----------------------------------------------------------------------===//
// For NOP RET RTT SYNCH 
//===----------------------------------------------------------------------===//

class F_Special_0Reg<bits<3> type,bits<3> op, bits<8> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = 0x0;
  let Inst{19-14} = 0x0;
  let Inst{13-8} = 0x0;
  let Inst{7-0}  = funct;
}
//===----------------------------------------------------------------------===//
// For JMP JC JNC CALL
//===----------------------------------------------------------------------===//

class F_Special_imm21<bits<3> type,bits<3> op, bits<5> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<21> imm21;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-5} = imm21;
  let Inst{4-0}  = funct;
}

//===----------------------------------------------------------------------===//
//For VSEL.20
//===----------------------------------------------------------------------===//

class F_Special_4Reg<bits<3> type,bits<3> op, bits<2> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  rd;
  bits<6>  rs;
  bits<6>  rt;
  bits<6>  rt1;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = rd;
  let Inst{19-14} = rs;
  let Inst{13-8} = rt;
  let Inst{7-2}  = rt1;
  let Inst{1-0}  = funct;
}
//===----------------------------------------------------------------------===//
//For  JMPR  CALLR  MOV.G2C  MOV.V2L
//===----------------------------------------------------------------------===//

class F_Special_1Reg_1<bits<3> type,bits<3> op, bits<8> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  rs;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = 0x0;
  let Inst{19-14} = rs;
  let Inst{13-8} = 0x0;
  let Inst{7-0}  = funct;
}
//===----------------------------------------------------------------------===//
//For  MOV.C2G  MOV.L2V
//===----------------------------------------------------------------------===//

class F_Special_1Reg_2<bits<3> type,bits<3> op, bits<8> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  rd;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = rd;
  let Inst{19-14} = 0x0;
  let Inst{13-8} = 0x0;
  let Inst{7-0}  = funct;
}
//===----------------------------------------------------------------------===//
//For ADD, ADDC, SUB, SUBC, AND, OR, XOR, MAX, MIN, SL, SRA, SRL, MUL32, MULU32
//    MUL64, VEQ10, VGT10, VLT10, VGE10, VLE10, VEQ20, VGT20, VLT20, VGE20, VLE20
//    VEQ40, VGT40, VLT40, VGE40, VLE40, VMOVC.V2V, VADD10, VSUB10, VMAX10, VMIN10
//    VSL10, VSLS10, VSRA10, VSRL10, VADD20, VSUB20, VMAX20, VMIN20, VSL20, VSLS20
//    VSRA20, VSRL20, VADD40, VSUB40, VMAX40, VMIN40, VSL40, VSLS40, VSRA40, VSRL40
//    VAND, VOR, VXOR, VFMUL10, VFMUL20, VFMUL40, VFMAC10, VFMAC20, VFMAC40, VCMUL20
//    VCMULR40, VCMULI40, VCMAC10, VCMAC20, VLOADO8, VLOADO16, VLOADO32, LOADO16
//    LOADO32, VSTOREO10, VSTOREO20, VSTOREO40, STOREO16, STOREO32
//===----------------------------------------------------------------------===//

class F_3Reg_001<bits<3> type,bits<3> op, bits<8> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  ra;
  bits<6>  rb;
  bits<6>  rc;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = ra;
  let Inst{19-14} = rb;
  let Inst{13-8} = rc;
  let Inst{7-0}  = funct;
}
//===----------------------------------------------------------------------===//
//For LOAD8, LOAD16, LOAD32, LOADU8, LOADU16, LOADU32, VLOAD8, VLOAD16, VLOAD32
//    VLOADR8, VLOADR16, VLOADR32, LOAD.VH, LOAD.VL, VLOADU8, VLOADU16, VLOADU32
//    VLOADUR8, VLOADUR16, VLOADUR32, LOADU.VH, LOADU.VL
//----------------------------------------------------------------------------//
//For STORE8, STORE16, STORE32, STOREU8, STOREU16, STOREU32, STORE10, STORE20
//    STORE40, VSTORE10, VSTORE20, VSTORE40, STORE.VH, STORE.VL, STOREU10, STOREU20
//    STOREU40, VSTOREU10, VSTOREU20, VSTOREU40, STOREU.VH, STOREU.VL
//===----------------------------------------------------------------------===//

class F_LS_2Reg_imm9<bits<3> type,bits<3> op, bits<5> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  ra;
  bits<15>  addr;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = ra;
  let Inst{19-5} = addr;
  let Inst{4-0}  = funct;
}


//Intrinsic format operand type
class IF_LS_2Reg_imm9<bits<3> type,bits<3> op, bits<5> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  ra;
  bits<6>  rb;
  bits<9>  imm9;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = ra;
  let Inst{19-14} = rb;
  let Inst{13-5} = imm9;
  let Inst{4-0}  = funct;
}
//===----------------------------------------------------------------------===//
//For EQ, NEQ, GT, LT, GE, LE, GTU, LTU, GEU, LEU, 
//===----------------------------------------------------------------------===//

class F_2Reg<bits<3> type,bits<3> op, bits<8> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  rs;
  bits<6>  rt;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = 0x0;
  let Inst{19-14} = rs;
  let Inst{13-8} = rt;
  let Inst{7-0}  = funct;
}
//===----------------------------------------------------------------------===//
//For NOT, ABS, CBW, CHW, MOV.G2V, VMOVR.G2V.10, VMOVR.G2V.20, VMOVR.G2V.40
//    MOV.V2V, VNOT, VABS.10, VSUM.10, VABS.20, VSUM.20, VABS.40, VSUM.40, MOV.G2G
//===----------------------------------------------------------------------===//

class F_2Reg_2<bits<3> type,bits<3> op, bits<8> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  ra;
  bits<6>  rb;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = ra;
  let Inst{19-14} = rb;
  let Inst{13-8} = 0x0;
  let Inst{7-0}  = funct;
}
//===----------------------------------------------------------------------===//
//For LOOP, MOVI.GH, MOVI.GL
//===----------------------------------------------------------------------===//


class F_1Reg_imm16<bits<3> type,bits<3> op, bits<4> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  ra;
  bits<16> imm16;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = ra;
  let Inst{19-4} = imm16;
  let Inst{3-0}  = funct;
}
//===----------------------------------------------------------------------===//
//For EQI, NEQI, GTI, LTI, GEI, LEI, TESTI
//===----------------------------------------------------------------------===//

class F_1Reg_imm9<bits<3> type,bits<3> op, bits<5> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  ra;
  bits<9> imm9;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = 0x0;
  let Inst{19-14} = ra;
  let Inst{13-5} = imm9;
  let Inst{4-0}  = funct;
}
//===----------------------------------------------------------------------===//
//For BST, BCLR
//===----------------------------------------------------------------------===//

class F_1Reg_imm5<bits<3> type,bits<3> op, bits<5> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  ra;
  bits<5> imm5;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = ra;
  let Inst{19-14} = 0x0;
  let Inst{13-9} = imm5;
  let Inst{8-5}  = 0x0;
  let Inst{4-0}  = funct;
}
//===----------------------------------------------------------------------===//
//For ADDI, ADDIC
//===----------------------------------------------------------------------===//

class F_2Reg_imm11<bits<3> type,bits<3> op, bits<3> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  ra;
  bits<6>  rb;
  bits<11> imm11;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = ra;
  let Inst{19-14} = rb;
  let Inst{13-3} = imm11;
  let Inst{2-0}  = funct;
}
//===----------------------------------------------------------------------===//
//For BFEXT, BFEXTU, BFST
//===----------------------------------------------------------------------===//

class F_2Reg_2imm5<bits<3> type,bits<3> op, bits<4> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  ra;
  bits<6>  rb;
  bits<5> imm5_2;
  bits<5> imm5_1;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = ra;
  let Inst{19-14} = rb;
  let Inst{13-9} = imm5_2;
  let Inst{8-4} = imm5_1;
  let Inst{3-0}  = funct;
}
//===----------------------------------------------------------------------===//
//For MOV.V2G.10 , MOV.V2G.20 , MOV.V2G.40
//    MOV.G2V.10 , MOV.G2V.20 , MOV.G2V.40
//===----------------------------------------------------------------------===//

class F_2Reg_imm4<bits<3> type,bits<3> op, bits<10> funct, dag outs, dag ins, string asmstr,
         list<dag> pattern, InstrItinClass itin>:
      InstDSP<outs, ins, asmstr, pattern, itin>
{
  bits<6>  ra;
  bits<6>  rb;
  bits<4> imm4;

  let Inst{31-29} = type;
  let Inst{28-26} = op;
  let Inst{25-20} = ra;
  let Inst{19-14} = rb;
  let Inst{13-10} = imm4;
  let Inst{9-0}  = funct;
}