//===-- DSPSchedule.td - DSP Scheduling Definitions ------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Functional units across DSP chips sets. Based on GCC/DSP backend files.
//===----------------------------------------------------------------------===//
def ALU     : FuncUnit;
def IMULDIV : FuncUnit;

//===----------------------------------------------------------------------===//
// Instruction Itinerary classes used for DSP
//===----------------------------------------------------------------------===//
def IIAlu              : InstrItinClass;
def II_CLO             : InstrItinClass;
def II_CLZ             : InstrItinClass;
def IILoad             : InstrItinClass;
def IIStore            : InstrItinClass;

def IIHiLo             : InstrItinClass;
def IIImul             : InstrItinClass;
def IIIdiv             : InstrItinClass;

def IIBranch           : InstrItinClass;

def IIPseudo           : InstrItinClass;
def IIVMax			   : InstrItinClass;
def IIVLoad			   : InstrItinClass;
def IIVStore		   : InstrItinClass;



//
//    |===========|==================================================|
//    | PIPELINE  |              Instruction Classes                 |
//    |===========|==================================================|
//    | SLOT0     |  NOP      控制流指令 标量运算指令 矢量运算指令   |
//	  |			  |				数据传送指令                         |
//    |-----------|--------------------------------------------------|
//    | SLOT1     |  NOP   标量运算指令（除MUL64）矢量运算指令		 |
//    |			  |数据传送指令										 |
//	  |			  |（除MOV.G2C、MOV.C2G、MOV.V2L、MOV.L2V）；		 |        
//    |-----------|--------------------------------------------------|
//    | SLOT2     | NOP；加载/存储指令；							 |
//	  |			  |MOVI.GH、MOVI.GL、MOV.G2G、MOV.V2V所有MOV.V2G、    |
//	  |			  |所有VMOVR.G2V； 所有MOV.G2V、					 |   
//    |-----------|--------------------------------------------------|
//    | SLOT3     |   NOP；加载/存储指令；MOVI.GH、MOVI.GL、MOV.G2G、 |
//    |           |所有MOV.V2G、所有MOV.G2V、所有VMOVR.G2V；MOV.V2V	 |
//    |===========|==================================================|
//
//



def SLOT0       : FuncUnit;
def SLOT1       : FuncUnit;
def SLOT2       : FuncUnit;
def SLOT3       : FuncUnit;

//def SLOT_ENDLOOP: FuncUnit;

def PSEUDO      : InstrItinClass;
def NOP_SLOT0123 :InstrItinClass;
def ALU32_S_SLOT0 :InstrItinClass;					//MUL64
def ALU32_S_SLOT01 :InstrItinClass;
def ALU32_V_SLOT01 :InstrItinClass;
def ControlFlow_SLOT0 :InstrItinClass;
def Data_Transmission_0_SLOT0 :InstrItinClass;		//MOV.V2L,MOV.L2V MOV.G2V,MOV.V2G
def Data_Transmission_1_SLOT01 :InstrItinClass;     //VSEL.20 VMOVC.V2V                
def Data_Transmission_2_SLOT0123 :InstrItinClass;
def LD_Ins_SLOT23 :InstrItinClass;
def ST_Ins_SLOT23 :InstrItinClass;


//===----------------------------------------------------------------------===//
// DSP Generic instruction itineraries.
//===----------------------------------------------------------------------===//
// http://llvm.org/docs/doxygen/html/structllvm_1_1InstrStage.html

def DSPGenericItineraries :ProcessorItineraries<[SLOT0, SLOT1, SLOT2, SLOT3],[],[
	  InstrItinData<NOP_SLOT0123      , [InstrStage<1, [SLOT0,SLOT1,SLOT2,SLOT3]>]>,
	  InstrItinData<ALU32_S_SLOT0      , [InstrStage<1, [SLOT0]>]>,
	  InstrItinData<ALU32_S_SLOT01      , [InstrStage<1, [SLOT0,SLOT1]>]>,
	  InstrItinData<ALU32_V_SLOT01      , [InstrStage<1, [SLOT0,SLOT1]>]>,
	  InstrItinData<ControlFlow_SLOT0      , [InstrStage<1, [SLOT0]>]>,
	  InstrItinData<Data_Transmission_0_SLOT0      , [InstrStage<1, [SLOT0]>]>,
	  InstrItinData<Data_Transmission_1_SLOT01      , [InstrStage<1, [SLOT0,SLOT1]>]>,
	  InstrItinData<Data_Transmission_2_SLOT0123      , [InstrStage<1, [SLOT0,SLOT1,SLOT2,SLOT3]>]>,
	  InstrItinData<LD_Ins_SLOT23    , [InstrStage<1, [SLOT2,SLOT3]>]>,
	  InstrItinData<ST_Ins_SLOT23      , [InstrStage<1, [SLOT2,SLOT3]>]>,
	  InstrItinData<PSEUDO , [InstrStage<1, [SLOT0, SLOT1, SLOT2, SLOT3]>]>
]>;

def DSPModel : SchedMachineModel {
  // Max issue per cycle == bundle width.
  let IssueWidth = 4;
  let Itineraries = DSPGenericItineraries;
  let LoadLatency = 1;
}	


