//===-- DSPTargetMachine.h - Define TargetMachine for DSP ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DSP specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef DSPTARGETMACHINE_H
#define DSPTARGETMACHINE_H

#include "DSPSubtarget.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include <string>

namespace llvm {
	class formatted_raw_ostream;


class DSPTargetMachine : public LLVMTargetMachine {

  DSPSubtarget *Subtarget;
  DSPSubtarget DefaultSubtarget;




public:
  DSPTargetMachine(const Target &T, 
  					StringRef TT, 
  					StringRef CPU,
  					StringRef FS, 
  					const TargetOptions &Options,
                  Reloc::Model RM, 
                  CodeModel::Model CM, 
                  CodeGenOpt::Level OL,
                  bool isLittle);

const DSPSubtarget *getSubtargetImpl() const override {
    if (Subtarget)
      return Subtarget;
    return &DefaultSubtarget;
  }

const DSPInstrInfo *getInstrInfo () const override {return getSubtargetImpl()->getInstrInfo();}

const DSPSelectionDAGInfo *getSelectionDAGInfo () const override {return getSubtargetImpl()->getSelectionDAGInfo();}

const DataLayout *getDataLayout () const  override {return getSubtargetImpl()->getDataLayout();}

const TargetFrameLowering *getFrameLowering () const  override{return getSubtargetImpl()->getFrameLowering();}

const DSPRegisterInfo *getRegisterInfo () const  override {return getSubtargetImpl()->getRegisterInfo();}

const DSPTargetLowering *getTargetLowering () const  override {return getSubtargetImpl()->getTargetLowering();}

const InstrItineraryData *getInstrItineraryData() const override {return &getSubtargetImpl()->getInstrItineraryData();}

/// \brief Reset the subtarget for the DSP target.
void resetSubtarget(MachineFunction *MF);

// Pass Pipeline Configuration
TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
};


//little endian machine
class DSPelTargetMachine : public DSPTargetMachine{
	virtual void anchor();
	public:
		DSPelTargetMachine(const Target &T, StringRef TT,
			StringRef CPU, StringRef FS, const TargetOptions &Options,
			Reloc::Model RM, CodeModel::Model CM, CodeGenOpt::Level OL);
};


} // end namespace llvm




#endif
