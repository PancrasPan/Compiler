//===-- llvm/Target/DSPTargetObjectFile.h - DSP Object Info ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_DSP_TARGETOBJECTFILE_H
#define LLVM_TARGET_DSP_TARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"


namespace llvm {
	class DSPTargetObjectFile : public TargetLoweringObjectFileELF{
		const MCSection *SmallDataSection; 


        //BSS (Block Start By Symbol Segment),generally refers to the memory that stores global values which haven't  been  initialized
        //This section is like the data section, except that it doesn’t take up space in the executable.
		const MCSection *SmallBSSSection;


	public:
		void Initialize(MCContext &Ctx, const TargetMachine &TM) override;


		 /// IsGlobalInSmallSection - Return true if this global address should be
   	 	/// placed into small data/bss section.
    	bool IsGlobalInSmallSection(const GlobalValue *GV,
                                const TargetMachine &TM, SectionKind Kind)const;
    	bool IsGlobalInSmallSection(const GlobalValue *GV,
                                const TargetMachine &TM) const;

    	const MCSection *SelectSectionForGlobal(const GlobalValue *GV,
                                        SectionKind Kind, Mangler &Mang,
                                       const TargetMachine &TM) const override;
		const MCSection *getSectionForConstant(SectionKind Kind, const Constant *C) const override;
	};
} //end of namespace llvm

#endif
