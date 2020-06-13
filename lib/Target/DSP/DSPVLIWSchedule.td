//=-DSPVLIWSchedule.td - DSP VILW Scheduling Definitions --*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// There are four SLOTS (four parallel pipelines) in DSP  machine.
// This file describes that machine information.

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
//	  |			  |MOVI.GH、MOV.GL、MOV.G2G、MOV.V2V所有MOV.V2G、    |
//	  |			  |所有VMOVR.G2V； 所有MOV.G2V、					 |   
//    |-----------|--------------------------------------------------|
//    | SLOT3     |   NOP；加载/存储指令；MOVI.GH、MOV.GL、MOV.G2G、 |
//    |           |所有MOV.V2G、所有MOV.G2V、所有VMOVR.G2V；MOV.V2V	 |
//    |===========|==================================================|
//
//



