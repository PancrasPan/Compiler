//===-- DSPRegisterInfo.td - DSP Register defs -----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//===-- DSPRegisterInfo.td - DSP Register defs -----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


let Namespace = "DSP" in {
def sub_32     : SubRegIndex<32>;
def sub_64     : SubRegIndex<64>;
def sub_lo     : SubRegIndex<32>;
def sub_hi     : SubRegIndex<32, 32>;
def sub_128    : SubRegIndex<128>;
}
class Unallocatable {
  bit isAllocatable = 0;
} 


// We have banks of 64 registers each.
class DSPReg<string n> : Register<n> {
  let Namespace = "DSP";
}

class DSPRegWithSubRegs<bits<6> num, string n, list<Register> subregs>
  : RegisterWithSubRegs<n, subregs> {
  let HWEncoding{5-0}= num;
  let Namespace = "DSP";
}

// General 32bit Registers
class DSPGPRReg<bits<6> num, string n> : DSPReg<n> {
  let HWEncoding{5-0} = num;
}

// Vector 128bit Registers
class DSPVRReg<bits<6> num, string n, list<Register> subregs> 
: DSPRegWithSubRegs<num, n, subregs> {
  let HWEncoding{5-0} = num;
  let SubRegIndices = [sub_32];
}


// Co-processor 0 Registers
class DSPC0Reg<bits<6> num, string n> : DSPReg<n> {
  let HWEncoding{5-0} = num;
}

// MOR Registers
class DSPMORReg<bits<6> num, string n> : DSPReg<n> {
  let HWEncoding{5-0} = num;
}

//===----------------------------------------------------------------------===//
//  Registers
//===----------------------------------------------------------------------===//
// The register string, such as "9" or "gp" will show on "llvm-objdump -d"
let Namespace = "DSP" in {
  // General Purpose Registers
  def ZERO : DSPGPRReg<0,  "GR0">, DwarfRegNum<[0]>;
  def AT   : DSPGPRReg<1,  "GR1">,    DwarfRegNum<[1]>;
  def V0   : DSPGPRReg<2,  "GR2">,    DwarfRegNum<[2]>;
  def V1   : DSPGPRReg<3,  "GR3">,    DwarfRegNum<[3]>;
  def A0   : DSPGPRReg<4,  "GR4">,    DwarfRegNum<[4]>;
  def A1   : DSPGPRReg<5,  "GR5">,    DwarfRegNum<[5]>;
  def A2   : DSPGPRReg< 6, "GR6">,    DwarfRegNum<[6]>;
  def A3   : DSPGPRReg< 7, "GR7">,    DwarfRegNum<[7]>;
  def T0   : DSPGPRReg< 8, "GR8">,    DwarfRegNum<[8]>;
  def T1   : DSPGPRReg< 9, "GR9">,    DwarfRegNum<[9]>;
  def T2   : DSPGPRReg< 10, "GR10">,  DwarfRegNum<[10]>;
  def T3   : DSPGPRReg< 11, "GR11">,  DwarfRegNum<[11]>;
  def T4   : DSPGPRReg< 12, "GR12">,  DwarfRegNum<[12]>;
  def T5   : DSPGPRReg< 13, "GR13">,  DwarfRegNum<[13]>;
  def T6   : DSPGPRReg< 14, "GR14">,  DwarfRegNum<[14]>;
  def T7   : DSPGPRReg< 15, "GR15">,  DwarfRegNum<[15]>;
  def S0   : DSPGPRReg< 16, "GR16">,  DwarfRegNum<[16]>;
  def S1   : DSPGPRReg< 17, "GR17">,  DwarfRegNum<[17]>;
  def S2   : DSPGPRReg< 18, "GR18">,  DwarfRegNum<[18]>;
  def S3   : DSPGPRReg< 19, "GR19">,  DwarfRegNum<[19]>;
  def S4   : DSPGPRReg< 20, "GR20">,  DwarfRegNum<[20]>;
  def S5   : DSPGPRReg< 21, "GR21">,  DwarfRegNum<[21]>;
  def S6   : DSPGPRReg< 22, "GR22">,  DwarfRegNum<[22]>;
  def S7   : DSPGPRReg< 23, "GR23">,  DwarfRegNum<[23]>;
  def T8   : DSPGPRReg< 24, "GR24">,  DwarfRegNum<[24]>;
  
  def T9   : DSPGPRReg< 25,  "GR25">, DwarfRegNum<[25]>;
  def RA   : DSPGPRReg< 26, "GR26">,  DwarfRegNum<[26]>;
  def SW   : DSPGPRReg< 27, "GR27">,  DwarfRegNum<[27]>;
  
  def GP   : DSPGPRReg<28, "GR28">,   DwarfRegNum<[28]>;
  def FP   : DSPGPRReg<29, "GR29">,   DwarfRegNum<[29]>;
  def SP   : DSPGPRReg<30, "GR30">,   DwarfRegNum<[30]>;
  def LR   : DSPGPRReg< 31, "GR31">,  DwarfRegNum<[31]>;
  
//  def LR   : DSPGPRReg<14, "lr">,   DwarfRegNum<[14]>;
//  def SW   : DSPGPRReg<15, "sw">,   DwarfRegNum<[15]>;
//  def MAR  : Register< 16, "mar">,  DwarfRegNum<[16]>;
//  def MDR  : Register< 17, "mdr">,  DwarfRegNum<[17]>;


  // Hi/Lo registers number and name
  def HI   : Register<"hi">, DwarfRegNum<[32]>;
  def LO   : Register<"lo">, DwarfRegNum<[33]>;
  def PC   : DSPC0Reg<0, "pc">,  DwarfRegNum<[34]>;
  def EPC  : DSPC0Reg<1, "epc">, DwarfRegNum<[35]>;
 
 def OFF0   : DSPMORReg<0,  "OFF0">, DwarfRegNum<[36]>;
 def OFF1   : DSPMORReg<1,  "OFF1">, DwarfRegNum<[37]>;
 def OFF2   : DSPMORReg<2,  "OFF2">, DwarfRegNum<[38]>;
 def OFF3   : DSPMORReg<3,  "OFF3">, DwarfRegNum<[39]>;
 
 def BAR0  : DSPMORReg<4,  "BAR0">, DwarfRegNum<[40]>;
 def BAR1  : DSPMORReg<5,  "BAR1">, DwarfRegNum<[41]>;
 def BAR2  : DSPMORReg<6,  "BAR2">, DwarfRegNum<[42]>;
 def BAR3  : DSPMORReg<7,  "BAR3">, DwarfRegNum<[43]>;
 
 def MR0  : DSPMORReg<8,  "MR0">, DwarfRegNum<[44]>;
 def MR1  : DSPMORReg<9,  "MR1">, DwarfRegNum<[45]>;
 def MR2  : DSPMORReg<10,  "MR2">, DwarfRegNum<[46]>;
 def MR3  : DSPMORReg<11,  "MR3">, DwarfRegNum<[47]>;


}

 //foreach I = 0-15 in {
 // def VR#I : DSPVRReg<I, "VR"#I, [!cast<DSPGPRReg>("A"#I#)]>,
 //           DwarfRegNum<[!add(I, 48)]>;
 //}
 
 def VR0 : DSPVRReg<0, "VR0", [!cast<DSPGPRReg>("T0")]>,DwarfRegNum<[!add(0, 48)]>;
 def VR1 : DSPVRReg<1, "VR1", [!cast<DSPGPRReg>("T1")]>,DwarfRegNum<[!add(1, 48)]>;
 def VR2 : DSPVRReg<2, "VR2", [!cast<DSPGPRReg>("T2")]>,DwarfRegNum<[!add(2, 48)]>;
 def VR3 : DSPVRReg<3, "VR3", [!cast<DSPGPRReg>("T3")]>,DwarfRegNum<[!add(3, 48)]>;
 def VR4 : DSPVRReg<4, "VR4", [!cast<DSPGPRReg>("T4")]>,DwarfRegNum<[!add(4, 48)]>;
 def VR5 : DSPVRReg<5, "VR5", [!cast<DSPGPRReg>("T5")]>,DwarfRegNum<[!add(5, 48)]>;
 def VR6 : DSPVRReg<6, "VR6", [!cast<DSPGPRReg>("T6")]>,DwarfRegNum<[!add(6, 48)]>;
 def VR7 : DSPVRReg<7, "VR7", [!cast<DSPGPRReg>("T7")]>,DwarfRegNum<[!add(7, 48)]>;
 def VR8 : DSPVRReg<8, "VR8", [!cast<DSPGPRReg>("S0")]>,DwarfRegNum<[!add(8, 48)]>;
 def VR9 : DSPVRReg<9, "VR9", [!cast<DSPGPRReg>("S1")]>,DwarfRegNum<[!add(9, 48)]>;
 def VR10 : DSPVRReg<10, "VR10", [!cast<DSPGPRReg>("S2")]>,DwarfRegNum<[!add(10, 48)]>;
 def VR11 : DSPVRReg<11, "VR11", [!cast<DSPGPRReg>("S3")]>,DwarfRegNum<[!add(11, 48)]>;
 def VR12 : DSPVRReg<12, "VR12", [!cast<DSPGPRReg>("S4")]>,DwarfRegNum<[!add(12, 48)]>;
 def VR13 : DSPVRReg<13, "VR13", [!cast<DSPGPRReg>("S5")]>,DwarfRegNum<[!add(13, 48)]>;
 def VR14 : DSPVRReg<14, "VR14", [!cast<DSPGPRReg>("S6")]>,DwarfRegNum<[!add(14, 48)]>;
 def VR15 : DSPVRReg<15, "VR15", [!cast<DSPGPRReg>("S7")]>,DwarfRegNum<[!add(15, 48)]>;
 
//===----------------------------------------------------------------------===//
// Register Classes
//===----------------------------------------------------------------------===//

def CPURegs : RegisterClass<"DSP", [i32], 32, (add
  // Reserved
  ZERO, AT, 
  // Return Values and Arguments
  V0, V1, A0, A1, A2, A3, 
  // Not preserved across procedure calls
   T0, T1, T2, T3, T4, T5, T6, T7,T8,T9,
  // Callee save
  S0, S1, S2, S3, S4, S5,S6, S7, 
  // Reserved
  GP, FP, LR,
  SP, RA, SW)>;
 // SP, RA, PC, EPC)>;

// Hi/Lo Registers class
def HILO   : RegisterClass<"DSP", [i32], 32, (add HI, LO)>;


//MO Registers class
def MORegs   : RegisterClass<"DSP", [i32], 32, (add 
	
	OFF0, OFF1, OFF2, OFF3,
	BAR0, BAR1, BAR2, BAR3,
	MR0, MR1, MR2, MR3)>;

// Status Registers class
def SR     : RegisterClass<"DSP", [i32], 32, (add SW )>;

// Co-processor 0 Registers class
def C0Regs : RegisterClass<"DSP", [i32], 32, (add PC, EPC)>;

//VR Registers class
def CPU128Regs : RegisterClass<"DSP", [v16i8,v8i16,v4i32], 128, (add 
                      VR0, VR1, VR2, VR3, VR4,
					  VR5, VR6, VR7, VR8, VR9,
					  VR10, VR11,VR12, VR13, 
					  VR14, VR15)>;

//def GPROut :RegisterClass<"DSP",[i32],32,(add(sub CPURegs,SW,PC,EPC))>;
def GPROut :RegisterClass<"DSP",[i32],32,(add(sub CPURegs,SW))>;
def VPR4Out :RegisterClass<"DSP",[v4i32],128,(add CPU128Regs)>;
def VPR8Out :RegisterClass<"DSP",[v8i16],128,(add CPU128Regs)>;
def VPR16Out :RegisterClass<"DSP",[v16i8],128,(add CPU128Regs)>;

def Special_Zero:RegisterClass<"DSP",[i32],32,(add ZERO)>;