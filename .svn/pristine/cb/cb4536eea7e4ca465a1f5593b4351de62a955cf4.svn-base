//===-- DSPSEISelDAGToDAG.h - A Dag to Dag Inst Selector for DSPSE -----===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Subclass of DSPDAGToDAGISel specialized for mips32/64.
//
//===----------------------------------------------------------------------===//

#ifndef DSPSEISELDAGTODAG_H
#define DSPSEISELDAGTODAG_H
#include "DSPISelDAGToDAG.h"
namespace llvm {
	class DSPSEDAGToDAGISel : public DSPDAGToDAGISel {
	public:
		explicit DSPSEDAGToDAGISel(DSPTargetMachine &TM) : DSPDAGToDAGISel(TM) {}
		SDNode* selectADD_FI(SDNode *Node);
	private:
		bool runOnMachineFunction(MachineFunction &MF) override;
		std::pair<bool, SDNode*> selectNode(SDNode *Node) override;
		void processFunctionAfterISel(MachineFunction &MF) override;
		// Insert instructions to initialize the global base register in the
		// first MBB of the function.
		// void initGlobalBaseReg(MachineFunction &MF);
	};
	FunctionPass *createDSPSEISelDag(DSPTargetMachine &TM);
}
#endif
