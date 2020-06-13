//===-- DSPFrameLowering.h - Define frame lowering for DSP ----*- C++ -*-===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DSP_FRAMEINFO_H
#define DSP_FRAMEINFO_H

#include "DSP.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
	class DSPSubtarget;


	class DSPFrameLowering : public TargetFrameLowering{
	protected:
		const DSPSubtarget &STI;
		uint64_t estimateStackSize(const MachineFunction &MF) const;
	public:
		explicit DSPFrameLowering(const DSPSubtarget &STI, unsigned Alignment)
			:TargetFrameLowering(StackGrowsDown, Alignment, 0, Alignment), STI(STI){};

		static const DSPFrameLowering *create(const DSPSubtarget &STI);
		bool hasFP(const MachineFunction &MF) const override;

	};


	//create dsptargetframelowering object
	const DSPFrameLowering  *createDSPSEFrameLowering(const DSPSubtarget &ST);
}


#endif