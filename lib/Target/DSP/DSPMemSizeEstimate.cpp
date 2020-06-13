
#include "DSP.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include <string>
using namespace llvm;

#define DEBUG_TYPE "ms-estimate"

cl::opt<std::string> outfile("mem_file", cl::init("mem.txt"), cl::Hidden, cl::desc("out file"));
namespace {
	class DSPMemSizeEst :public MachineFunctionPass{
	public:
		static char ID;
		class GlobalCache {
		public:
			bool flag;
			unsigned size;
			GlobalCache():flag(false),size(0){}
		};
		static GlobalCache *GCa;
		explicit DSPMemSizeEst() :MachineFunctionPass(ID) {

		}
		~DSPMemSizeEst();
		bool runOnMachineFunction(MachineFunction &mf) override {
			std::string err = "";
			DEBUG(dbgs() << "*****************************DSP mem size" << "\n");
			DEBUG(dbgs() << outfile << "\n");
			const char * fname = outfile.data();
			raw_fd_ostream out(fname, err, sys::fs::F_RW);

			out << "*******Begin to estimate size of stack********" << "\n";
			if (err !="") {
				return false;
			}
			printFrameInfo(out, mf);
			if (!GCa->flag) printGlobalInfo(out, mf);
			return true;
		}
	public:
		void printFrameInfo(raw_ostream &file,MachineFunction &mf);
		void printGlobalInfo(raw_ostream &file, MachineFunction &mf);
		void printConstantPoolInfo(raw_ostream &file, MachineFunction &mf);
	};
}
DSPMemSizeEst::~DSPMemSizeEst() {
	delete GCa;
}
void DSPMemSizeEst::printFrameInfo(raw_ostream &file,MachineFunction &mf){
	int num = mf.getFrameInfo()->getNumObjects();
	const MachineFrameInfo*  MFI = mf.getFrameInfo();
	file << "stack size \n";
	file << MFI->getStackSize() << "\n";
	file << "frame objects \n";
	for (int i = 0; i < num; i++)
	{
		//file << "object " << i << " size: " << MFI->getObjectSize(i) << "\n";
	}
}

void DSPMemSizeEst::printGlobalInfo(raw_ostream &file, MachineFunction &mf) {
	const Module *m = mf.getFunction()->getParent();
	for (auto gi = m->global_begin(), ge = m->global_end();gi != ge;gi++) {
		file << gi->getName().str() << "\n";
	}
	GCa->flag = true;
}
char DSPMemSizeEst::ID = 0;
DSPMemSizeEst::GlobalCache* DSPMemSizeEst::GCa = new DSPMemSizeEst::GlobalCache();


FunctionPass *llvm::createDSPMemEst() {
	return new DSPMemSizeEst();
}

