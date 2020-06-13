//===-- DSPTargetObjectFile.cpp - DSP Object Files ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "DSPTargetObjectFile.h"
#include "DSPSubtarget.h"
#include "llvm\CodeGen\AsmPrinter.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSectionCOFF.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ELF.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/ADT/StringExtras.h"
using namespace llvm;

static cl::opt<unsigned>
SSThreshold("dsp-ssection-threshold",cl::Hidden,
            cl::desc("Small data and bss section threshold size(default=8)"),
            cl::init(8));
void DSPTargetObjectFile::Initialize(MCContext &Ctx,const TargetMachine &TM){
    TargetLoweringObjectFileELF::Initialize(Ctx, TM);


  SmallDataSection =
    getContext().getELFSection(".sdata", ELF::SHT_PROGBITS,
                               ELF::SHF_WRITE |ELF::SHF_ALLOC,
                               SectionKind::getDataRel());

  SmallBSSSection =
    getContext().getELFSection(".sbss", ELF::SHT_NOBITS,
                               ELF::SHF_WRITE |ELF::SHF_ALLOC,
                               SectionKind::getBSS());
}


// A address must be loaded from a small section if its size is less than the
// small section size threshold. Data in this section must be addressed using
// gp_rel operator.
static bool IsInSmallSection(uint64_t Size) {
  return Size > 0 && Size <= SSThreshold;
}

bool DSPTargetObjectFile::IsGlobalInSmallSection(const GlobalValue *GV,
                                                const TargetMachine &TM) const {
  if (GV->isDeclaration() || GV->hasAvailableExternallyLinkage())
    return false;

  return IsGlobalInSmallSection(GV, TM, getKindForGlobal(GV, TM));
}


/// IsGlobalInSmallSection - Return true if this global address should be
/// placed into small data/bss section.
bool DSPTargetObjectFile::
IsGlobalInSmallSection(const GlobalValue *GV, const TargetMachine &TM,
                       SectionKind Kind) const {

  const DSPSubtarget &Subtarget = TM.getSubtarget<DSPSubtarget>();

  // Return if small section is not available.
  if (!Subtarget.useSmallSection())
    return false;

  // Only global variables, not functions.
  const GlobalVariable *GVA = dyn_cast<GlobalVariable>(GV);
  if (!GVA)
    return false;

  // We can only do this for datarel or BSS objects for now.
  if (!Kind.isBSS() && !Kind.isDataRel())
    return false;

  // If this is a internal constant string, there is a special
  // section for it, but not in small data/bss.
  if (Kind.isMergeable1ByteCString())
    return false;

  Type *Ty = GV->getType()->getElementType();
  return IsInSmallSection(TM.getDataLayout()->getTypeAllocSize(Ty));
}



const MCSection *DSPTargetObjectFile::
SelectSectionForGlobal(const GlobalValue *GV, SectionKind Kind,
                       Mangler &Mang, const TargetMachine &TM) const {
  // TODO: Could also support "weak" symbols as well with ".gnu.linkonce.s.*"
  // sections?

  // Handle Small Section classification here.
  if (Kind.isBSS() && IsGlobalInSmallSection(GV, TM, Kind))
    return SmallBSSSection;
  if (Kind.isDataNoRel() && IsGlobalInSmallSection(GV, TM, Kind))
    return SmallDataSection;

  // Otherwise, we work the same as ELF.
  return TargetLoweringObjectFileELF::SelectSectionForGlobal(GV, Kind, Mang,TM);
}
static std::string APIntToHexString(const APInt &AI) {
	unsigned Width = (AI.getBitWidth() / 8) * 2;
	std::string HexString = utohexstr(AI.getLimitedValue(), /*LowerCase=*/true);
	unsigned Size = HexString.size();
	assert(Width >= Size && "hex string is too large!");
	HexString.insert(HexString.begin(), Width - Size, '0');

	return HexString;
}

static std::string scalarConstantToHexString(const Constant *C){
	Type *Ty = C->getType();
	APInt AI;
	if (isa<UndefValue>(C)) {
		AI = APInt(Ty->getPrimitiveSizeInBits(), /*val=*/0);
	}
	else if (Ty->isFloatTy() || Ty->isDoubleTy()) {
		const auto *CFP = cast<ConstantFP>(C);
		AI = CFP->getValueAPF().bitcastToAPInt();
	}
	else if (Ty->isIntegerTy()) {
		const auto *CI = cast<ConstantInt>(C);
		AI = CI->getValue();
	}
	else {
		llvm_unreachable("unexpected constant pool element type!");
	}
	return APIntToHexString(AI);

}

const MCSection *DSPTargetObjectFile::getSectionForConstant(SectionKind Kind, const Constant *C) const {
	
	if (Kind.isReadOnly()){
		if (C){
			Type *Ty = C->getType();
			 SmallString<32> COMDATSymName;
			 const auto *VTy = dyn_cast<VectorType>(Ty);
			COMDATSymName = "__DSPVector@";
			for (int I = VTy->getNumElements() - 1, E = -1; I != E; --I){
				COMDATSymName += scalarConstantToHexString(C->getAggregateElement(I));
			}
			if (!COMDATSymName.empty()){
				unsigned Type = ELF::SHT_PROGBITS;
				unsigned Flags = ELF::SHF_ALLOC|ELF::SHF_WRITE|ELF::SHF_MERGE;
				//unsigned EntrySize =0;
				return getContext().getELFSection(".rodata", Type, Flags, Kind, 0, COMDATSymName);
			}
		}	
	}
	return TargetLoweringObjectFile::getSectionForConstant(Kind, C);
}
