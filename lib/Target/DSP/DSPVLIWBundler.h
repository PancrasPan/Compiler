#ifndef DSPVLIWBUNDLER
#define DSPVLIWBUNDLER
#include "DSP.h"
#include "DSPSubtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/ADT/DenseMap.h"
#include <iostream>
#include <vector>
#include <map>
#include <memory>
namespace llvm {
	
	
	class VLIWBundler{
		virtual void anchor();
	public:
		VLIWBundler(){
			std::cout << "abstract" << std::endl;
		};

		//virtual void PerformBundle(MachineBasicBlock *MBB);
		//virtual void PerformBundle(MachineInstr *MI);
	};
	namespace DSPVLIW{
		enum 
		{
			VLIWEND = 0xffffffff,
			VLIWINSIDE = 0x7fffffff,

		};
	}
	struct  slot
	{
		uint16_t mask;
		uint32_t SlotMask;
	};

	class DSPVLIWBundler :public VLIWBundler{
		void anchor() override;
	private:
		static DSPVLIWBundler* UniqueBundler;
		DSPVLIWBundler();
		DSPVLIWBundler(const DSPVLIWBundler&);
		DSPVLIWBundler& operator = (const DSPVLIWBundler&);

	public:
		std::map<const MachineInstr *, unsigned> InstrToSlot;
		void InsertSlot(MachineInstr* MI, unsigned slot);
		std::vector<slot> Slots;
		uint16_t usedSlot;
		static DSPVLIWBundler* getBundler();
		~DSPVLIWBundler();
		void initializeResource();
		std::vector<int> getResource();
		void reset();
		//void PerformBundle(MachineBasicBlock *MBB) override;
		//void PerformBundle(MachineInstr *MI) override;
		void PerformBundle(DSPMCInst *MI,uint32_t* Binary);
		void doTSFlagsFixup(std::vector<const MachineInstr*> Package);
	};


}

#endif