//===-- DSPSubtarget.cpp - DSP Subtarget Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DSP specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//



#include "DSPMachineFunction.h"
#include "DSP.h"
#include "DSPRegisterInfo.h"
#include "DSPSubtarget.h"
#include "DSPTargetMachine.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "dsp-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "DSPGenSubtargetInfo.inc"

static cl::opt<bool> UseSmallSectionOpt
("dsp-use-small-section", cl::Hidden, cl::init(false),
cl::desc("Use small section. Only work when -relocation-model="
"static. pic always not use small section."));

static cl::opt<bool> ReserveGPOpt
("dsp-reserve-gp", cl::Hidden, cl::init(false),
cl::desc("Never allocate $gp to variable"));


static cl::opt<bool> NoCploadOpt
("dsp-no-cpload", cl::Hidden, cl::init(false),
cl::desc("No issue .cpload"));

bool DSPReserveGP;
bool DSPNoCpload;

extern bool FixGlobalBaseReg;

/// Select the DSP CPU for the given triple and cpu name.
/// FIXME: Merge with the copy in DSPMCTargetDesc.cpp
static StringRef selectDSPCPU(Triple TT, StringRef CPU) {
  if (CPU.empty() || CPU == "generic") {
    if (TT.getArch() == Triple::dsp)
      CPU = "dspse";
  }
  return CPU;
}

  void DSPSubtarget::anchor() { }

  static std::string computeDataLayout(const DSPSubtarget &ST) {
  std::string Ret = "";
  // There are both little and big endian mips.
  if (ST.isLittle())
  Ret += "e";
  else
  Ret += "E";
  Ret += "-m:m";
  Ret += "-p:32:32";
  // 8 and 16 bit integers only need no have natural alignment, but try to
  // align them to 32 bits. 64 bit integers have natural alignment.
  Ret += "-i8:8:32-i16:16:32-i64:64";
  // 32 bit registers are always available and the stack is at least 64 bit
  // aligned.
  Ret += "-n32-S64";
  return Ret;
}

DSPSubtarget::DSPSubtarget(const std::string &TT, const std::string &CPU,
                             const std::string &FS, bool little, 
                             Reloc::Model RM,
                             DSPTargetMachine *_TM) :
 DSPGenSubtargetInfo(TT,CPU,FS), DSPArchVersion(DSPSE),DSPABI(UnknownABI),IsLittle(little),
 IsSingleFloat(false),TM(_TM),RM(RM),TargetTriple(TT),
 DL(computeDataLayout(initializeSubtargetDependencies(CPU, FS, TM))),
 TSInfo(DL),InstrInfo(DSPInstrInfo::create(*this)),
 FrameLowering(DSPFrameLowering::create(*this)),
 TLInfo(DSPTargetLowering::create(*TM,*this)){
	DSPABI = O32;
  assert(DSPABI != UnknownABI);

  InstrItins = getInstrItineraryForCPU(CPU);
  UseSmallSection = UseSmallSectionOpt;
  DSPReserveGP = ReserveGPOpt;
  DSPNoCpload = NoCploadOpt;

  if (RM == Reloc::Static && !UseSmallSection && !DSPReserveGP)
	  FixGlobalBaseReg = false;
  else

	  FixGlobalBaseReg = true;


}

  DSPSubtarget &  DSPSubtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS,
                                               const TargetMachine *TM) {
  std::string CPUName = selectDSPCPU(TargetTriple, CPU);

  if(CPUName=="dspse")DSPArchVersion = DSPSE;

  
  
  
  // Parse features string.
  ParseSubtargetFeatures(CPUName, FS);
  // Initialize scheduling itinerary for the specified CPU.
 


  return *this;
  }
  bool DSPSubtarget::abiUsesSoftFloat() const {
	  return true;
  }

  Reloc::Model DSPSubtarget::getRelocationModel() const {
  return TM->getRelocationModel();
  }


 


