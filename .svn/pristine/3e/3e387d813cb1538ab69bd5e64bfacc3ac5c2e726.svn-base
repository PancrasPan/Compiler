#ifndef DSPSE_FRAMEINFO_H
#define DSPSE_FRAMEINFO_H


#include "DSPFrameLowering.h"
#include "DSPSubtarget.h"

namespace llvm {


	class DSPSEFrameLowering : public DSPFrameLowering{
	public:
		explicit DSPSEFrameLowering(const DSPSubtarget &STI);

		bool hasReservedCallFrame(const MachineFunction &MF) const override;
		void processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
			RegScavenger *RS) const override;

		/// emitProlog/emitEpilog - These methods insert prolog and epilog code into
		/// the function.
		void emitPrologue(MachineFunction &MF) const override;
		void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

			void eliminateCallFramePseudoInstr(MachineFunction &MF,
			MachineBasicBlock &MBB,
			MachineBasicBlock::iterator I) const override;
	
	};
}

#endif