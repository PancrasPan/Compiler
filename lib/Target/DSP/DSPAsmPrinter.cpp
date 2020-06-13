//===-- DSPAsmPrinter.cpp - DSP LLVM Assembly Printer -------------------===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format DSP assembly language.
//

#include "DSPAsmPrinter.h"
#include "InstPrinter/DSPInstPrinter.h"
#include "MCTargetDesc/DSPBaseInfo.h"
#include "DSP.h"
#include "DSPInstrInfo.h"
#include "DSPVLIWBundler.h"
#include "DSPMachineFunction.h"
#include "MCTargetDesc\DSPMCInst.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetOptions.h"
#include <iostream>
#include "llvm/Support/Debug.h"
using namespace llvm;
int SlotMaskToOrder(uint32_t SlotMask) {//ppp
	switch (SlotMask) {
	case 0x9fffffff:return 0;
	case 0xbfffffff:return 1;
	case 0xdfffffff:return 2;
	case 0xffffffff:return 3;
	default:return 10;
	}
}
unsigned DSPAsmPrinter::getUnit(const MachineInstr* MI) {//ppp
	InstrItineraryData DSPInstrItins = Subtarget->getInstrItineraryForCPU("dspse");
	unsigned InsnClass = MI->getDesc().getSchedClass();
	const llvm::InstrStage *IS = DSPInstrItins.beginStage(InsnClass);
	return IS->getUnits();
}
std::string DSPAsmPrinter::PrintAsmSlots(unsigned slot) {//ppp
	OutStreamer.AddComment(Twine(SlotMaskToOrder(slot)));
	OutStreamer.EmitRawText("Slot:	");
	return "Slots allocation has been generated in the .s file!";
}
#define DEBUG_TYPE "dsp-asm-printer"

bool DSPAsmPrinter::runOnMachineFunction(MachineFunction &MF){
	DSPFI = MF.getInfo<DSPFunctionInfo>();
	AsmPrinter::runOnMachineFunction(MF);
	return true;

}

//- EmitInstruction() must exists or will have run time error.
/*void DSPAsmPrinter::EmitInstruction(const MachineInstr *MI) {
	if (MI->isDebugValue()) {
		SmallString<128> Str;
		raw_svector_ostream OS(Str);
		PrintDebugValueComment(MI, OS);
		return;
	}
	MachineBasicBlock::const_instr_iterator I = MI;
	MachineBasicBlock::const_instr_iterator E = MI->getParent()->instr_end();
	MCInst TmpInst0;
	do {
		MCInstLowering.Lower(I, TmpInst0);
		OutStreamer.EmitInstruction(TmpInst0, getSubtargetInfo());
	} while ((++I != E) && I->isInsideBundle()); // Delay slot check
}*/

void DSPAsmPrinter::EmitInstruction(const MachineInstr *MI){
	//std::cout << "Start DSPAsmPrinter.cpp EmitInstruction" << std::endl;
	DSPVLIWBundler *BD = DSPVLIWBundler::getBundler();
	if (MI->isBundle()) {
		//std::cout << "is bundled" << std::endl;
		std::vector<const MachineInstr *> BundleMIs;
		const MachineBasicBlock *MBB = MI->getParent();
		MachineBasicBlock::const_instr_iterator MII = MI;
		++MII;
		unsigned int IgnoreCount = 0;
		while (MII != MBB->end() && MII->isInsideBundle()) {
			const MachineInstr *MInst = MII;
			if (MInst->getOpcode() == TargetOpcode::DBG_VALUE ||
				MInst->getOpcode() == TargetOpcode::IMPLICIT_DEF
				|| MInst->getOpcode() == TargetOpcode::CFI_INSTRUCTION//ppp
				) {
				IgnoreCount++;
				++MII;
				continue;
			}
			BundleMIs.push_back(MInst);
			++MII;
		}
		unsigned Size = BundleMIs.size();
		assert((Size + IgnoreCount) == MI->getBundleSize() && "Corrupt Bundle!");

		//sort the package according to the FuncUnit using bubble sort because of the size is small
		/*for (unsigned int i = 0; i < Size; i++)//ppp
			for (unsigned j = 0; j < Size - i - 1; j++)
				if (getUnit(BundleMIs[j]) > getUnit(BundleMIs[j + 1])) {
					const MachineInstr * tmp = BundleMIs[j + 1];
					BundleMIs[j + 1] = BundleMIs[j];
					BundleMIs[j] = tmp;
				}*/

		for (unsigned Index = 0; Index < Size; Index++) {
			if (BundleMIs[Index]->isCFIInstruction())continue;//ppp
			DSPMCInst MCI;
			//get the FuncUnits for InstPrinter
			InstrItineraryData DSPInstrItins = Subtarget->getInstrItineraryForCPU("dspse");
			unsigned InsnClass = BundleMIs[Index]->getDesc().getSchedClass();

			const llvm::InstrStage *IS = DSPInstrItins.beginStage(InsnClass);
			MCI.setIS(IS);
			unsigned FuncUnits = IS->getUnits();
			//std::cout << "asmprinter" << FuncUnits << std::endl;
			MCI.setPacketStart(Index == 0);
			MCI.setPacketEnd(Index == (Size - 1));
			unsigned slot = BD->InstrToSlot[BundleMIs[Index]];
			//std::cout << "asmprinter" << slot << std::endl;
			MCI.setPos(slot);
			//ppp
			/*unsigned num = BundleMIs[Index]->getNumOperands();
			bool isCFI = false;
			for (unsigned i = 0; i < num; i++){
				isCFI = BundleMIs[Index]->getOperand(i).isCFIIndex();
				if (isCFI)break;//
			}
			if (isCFI){
				AsmPrinter::emitCFIInstruction(*BundleMIs[Index]);
			}
			else*/
			{
				MCInstLowering.Lower(BundleMIs[Index], MCI, FuncUnits);
				DEBUG(PrintAsmSlots(slot));
				EmitToStreamer(OutStreamer, MCI);

			}

		}
	}
	else{
		if (MI->isInsideBundle())
			//std::cout << "isInsideBundle 2" << std::endl;//ppp
			DEBUG(dbgs() << "isInsideBundle 2" << "\n");
		DSPMCInst MCI;
		InstrItineraryData DSPInstrItins = Subtarget->getInstrItineraryForCPU("dspse");
		unsigned InsnClass = MI->getDesc().getSchedClass();
		const llvm::InstrStage *IS = DSPInstrItins.beginStage(InsnClass);
		MCI.setIS(IS);
		MCI.setPacketStart(true);
		MCI.setPacketEnd(true);
		unsigned slot = BD->InstrToSlot[MI];
		MCI.setPos(slot);
		MCInstLowering.Lower(MI, MCI);
		DEBUG(PrintAsmSlots(slot));
		EmitToStreamer(OutStreamer, MCI);
		
	}
	//std::cout << "End DSPAsmPrinter.cpp EmitInstruction" << std::endl;
}


void DSPAsmPrinter::printSavedRegsBitmask(raw_ostream &O) {
	// CPU and FPU Saved Registers Bitmasks
	/*unsigned CPUBitmask = 0;
	int CPUTopSavedRegOff;
	// Set the CPU and FPU Bitmasks
	const MachineFrameInfo *MFI = MF->getFrameInfo();
	const std::vector<CalleeSavedInfo> &CSI = MFI->getCalleeSavedInfo();
	// size of stack area to which FP callee-saved regs are saved.
	unsigned CPURegSize = DSP::CPURegsRegClass.getSize();
	unsigned i = 0, e = CSI.size();
	// Set CPU Bitmask.
	for (; i != e; ++i) {
		unsigned Reg = CSI[i].getReg();
		unsigned RegNum = getDSPRegisterNumbering(Reg);
		CPUBitmask |= (1 << RegNum);
	}
	CPUTopSavedRegOff = CPUBitmask ? -CPURegSize : 0;
	// Print CPUBitmask
	O << "\t.mask \t"; printHex32(CPUBitmask, O);
	O << ', ' << CPUTopSavedRegOff << '\n';
	*/
}



// Print a 32 bit hex number with all numbers.
void DSPAsmPrinter::printHex32(unsigned Value, raw_ostream &O) {
	O << "0x";
	for (int i = 7; i >= 0; i--)
		O.write_hex((Value & (0xF << (i * 4))) >> (i * 4));
}
//===----------------------------------------------------------------------===//
// Frame and Set directives
//===----------------------------------------------------------------------===//
//-> .frame $sp,8,$lr
// .mask 0x00000000,0
// .set noreorder
// .set nomacro
/// Frame Directive
void DSPAsmPrinter::emitFrameDirective() {
	const TargetRegisterInfo &RI = *TM.getRegisterInfo();
	unsigned stackReg = RI.getFrameRegister(*MF);
	unsigned returnReg = RI.getRARegister();
	unsigned stackSize = MF->getFrameInfo()->getStackSize();
	/*
	if (OutStreamer.hasRawTextSupport())
		OutStreamer.EmitRawText("\t.frame\t$" +
		StringRef(DSPInstPrinter::getRegisterName(stackReg)).upper() +
		"," + Twine(stackSize) + ",$" +
		StringRef(DSPInstPrinter::getRegisterName(returnReg)).upper());
		*/
}
/// Emit Set directives.
const char *DSPAsmPrinter::getCurrentABIString() const {
	switch (Subtarget->getTargetABI()) {
	case DSPSubtarget::O32: return "abi32";
	default: llvm_unreachable("Unknown DSP ABI");;
	}
}
// .type main,@function
//-> .ent main # @main
// main:
void DSPAsmPrinter::EmitFunctionEntryLabel() {
	/*
	if (OutStreamer.hasRawTextSupport())
		OutStreamer.EmitRawText("\t.ent\t" + Twine(CurrentFnSym->getName()));
		*/
	OutStreamer.EmitLabel(CurrentFnSym);
	
}

void DSPAsmPrinter::EmitConstantPool(){
	AsmPrinter::EmitConstantPool();
}
// .frame $sp,8,$pc
// .mask 0x00000000,0
//-> .set noreorder
//-> .set nomacro
/// EmitFunctionBodyStart - Targets can override this to emit stuff before
/// the first basic block in the function.
void DSPAsmPrinter::EmitFunctionBodyStart() {
	MCInstLowering.Initialize(&MF->getContext());
	emitFrameDirective();
	/*if (OutStreamer.hasRawTextSupport()) {
		SmallString<128> Str;
		raw_svector_ostream OS(Str);
		printSavedRegsBitmask(OS);
		OutStreamer.EmitRawText(OS.str());
		OutStreamer.EmitRawText(StringRef("\t.set\tnoreorder"));
		OutStreamer.EmitRawText(StringRef("\t.set\tnomacro"));
		if (DSPFI->getEmitNOAT())
			OutStreamer.EmitRawText(StringRef("\t.set\tnoat"));
			
	}*/
	
	//global address 

}
//-> .set macro
//-> .set reorder
//-> .end main
/// EmitFunctionBodyEnd - Targets can override this to emit stuff after
/// the last basic block in the function.
void DSPAsmPrinter::EmitFunctionBodyEnd() {
	// There are instruction for this macros, but they must
	// always be at the function end, and we can¡¯t emit and
	// break with BB logic.
	/*if (OutStreamer.hasRawTextSupport()) {
		if (DSPFI->getEmitNOAT())
			OutStreamer.EmitRawText(StringRef("\t.set\tat"));
		OutStreamer.EmitRawText(StringRef("\t.set\tmacro"));
		OutStreamer.EmitRawText(StringRef("\t.set\treorder"));
		OutStreamer.EmitRawText("\t.end\t" + Twine(CurrentFnSym->getName()));
	}*/
}
// .section .mdebug.abi32
		  // .previous
void DSPAsmPrinter::EmitStartOfAsmFile(Module &M) {
 // FIXME: Use SwitchSection.
 // Tell the assembler which ABI we are using
	/* if (OutStreamer.hasRawTextSupport())
		 OutStreamer.EmitRawText("\t.section .mdebug." + Twine(getCurrentABIString()));
			  // return to previous section
	if (OutStreamer.hasRawTextSupport())
		OutStreamer.EmitRawText(StringRef("\t.previous"));*/
}
MachineLocation
DSPAsmPrinter::getDebugValueLocation(const MachineInstr *MI) const {
	// Handles frame addresses emitted in DSPInstrInfo::emitFrameIndexDebugValue.
	assert(MI->getNumOperands() == 4 && "Invalid no. of machine operands!");
	assert(MI->getOperand(0).isReg() && MI->getOperand(1).isImm() &&
		"Unexpected MachineOperand types");
	return MachineLocation(MI->getOperand(0).getReg(),
		MI->getOperand(1).getImm());
}
void DSPAsmPrinter::PrintDebugValueComment(const MachineInstr *MI,
	raw_ostream &OS) {
	// TODO: implement
	OS << "PrintDebugValueComment()";

}


  MCSymbol *DSPAsmPrinter::GetCPISymbol(unsigned CPID) const  {
	const MachineConstantPoolEntry &CPE = MF->getConstantPool()->getConstants()[CPID];
	if (!CPE.isMachineConstantPoolEntry()){
		SectionKind Kind = CPE.getSectionKind(TM.getDataLayout());
		//std::cout << "this is the section kind "<< Kind.isMergeableConst()<< std::endl;
		//std::cout << Kind.isMergeableConst16()<< std::endl;

		const Constant *C = CPE.Val.ConstVal;
		const MCSectionELF *S = cast<MCSectionELF>(getObjFileLowering().getSectionForConstant(Kind, C));
		 MCSymbol *Sym =S->getGroup();
		return Sym;
	}
	return AsmPrinter::GetCPISymbol(CPID);
}


  static MCInstPrinter *createDSPMCInstPrinter(const Target &T,
	  unsigned SyntaxVariant,
	  const MCAsmInfo &MAI,
	  const MCInstrInfo &MII,
	  const MCRegisterInfo &MRI,
	  const MCSubtargetInfo &STI) {
	  if (SyntaxVariant == 0)
		  return(new DSPInstPrinter(MAI, MII, MRI));
	  else
		  return nullptr;
  }

// Force static initialization.
extern "C" void LLVMInitializeDSPAsmPrinter() {
	RegisterAsmPrinter<DSPAsmPrinter> Y(TheDSPelTarget);
	TargetRegistry::RegisterMCInstPrinter(TheDSPelTarget,
		createDSPMCInstPrinter);
}