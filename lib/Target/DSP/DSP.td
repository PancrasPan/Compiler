//===-- DSP.td - Describe the DSP Target Machine ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This is the top level entry point for the DSP target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//



def FeatureO32          : SubtargetFeature<"o32", "DSPABI", "O32",
                               "Enable o32 ABI">;

def FeatureDSPSE		:SubtargetFeature<"dspse", "DSPArchVersion",
											"DSPSE","DSP 1.0 version">;



//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//


include "DSPSchedule.td"
include "DSPInstrInfo.td"
include "DSPCallingConv.td"

def DSPInstrInfo : InstrInfo ;

//===----------------------------------------------------------------------===//
// DSP processors supported.
//===----------------------------------------------------------------------===//

class Proc<string name, SchedMachineModel Model,list<SubtargetFeature> Features>
	:ProcessorModel<name,Model,Features>;

def : Proc<"dspse",DSPModel,[FeatureDSPSE, FeatureO32]	>;
def DSPAsmParser : AsmParser {
	let ShouldEmitMatchRegisterName = 0;
}

def DSPAsmParserVariant : AsmParserVariant {
	int Variant = 0;
	string RegisterPrefix = "$";
}

def DSP : Target {
	let InstructionSet = DSPInstrInfo;
	//let AssemblyParsers = [DSPAsmParser];
	//let AssemblyParserVariants = [DSPAsmParserVariant];
}