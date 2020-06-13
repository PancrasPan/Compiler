
#include "DSPMCTargetDesc.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"

#include "DSPMCAsmInfo.h"
#include "InstPrinter/DSPInstPrinter.h"


using namespace llvm;
#define GET_INSTRINFO_MC_DESC
#include "DSPGenInstrInfo.inc"
#define GET_SUBTARGETINFO_MC_DESC
#include "DSPGenSubtargetInfo.inc"
#define GET_REGINFO_MC_DESC
#include "DSPGenRegisterInfo.inc"


/// Select the dsp Architecture Feature for the given triple and cpu name.
/// The function will be called at command ’llvm-objdump -d’ for dsp elf input.
static StringRef selectdspArchFeature(StringRef TT, StringRef CPU) {
std::string dspArchFeature;
if (CPU.empty() || CPU == "generic") {
Triple TheTriple(TT);

if (TheTriple.getArch() == Triple::dsp)
	if (CPU.empty() || CPU == "DSPSE")
		dspArchFeature = "+dspse";
}
return dspArchFeature;
}

static MCInstrInfo *createDSPMCInstrInfo() {
MCInstrInfo *X = new MCInstrInfo();
InitDSPMCInstrInfo(X); // defined in dspGenInstrInfo.inc
return X;
}

static MCRegisterInfo *createDSPMCRegisterInfo(StringRef TT) {
MCRegisterInfo *X = new MCRegisterInfo();
InitDSPMCRegisterInfo(X, DSP::LR); // defined in dspGenRegisterInfo.inc
return X;
}


static MCSubtargetInfo *createDSPMCSubtargetInfo(StringRef TT, StringRef CPU,
StringRef FS) {
std::string ArchFS = selectdspArchFeature(TT,CPU);
if (!FS.empty()) {
if (!ArchFS.empty())
ArchFS = ArchFS + "," + FS.str();
else
ArchFS = FS;
}
MCSubtargetInfo *X = new MCSubtargetInfo();
InitDSPMCSubtargetInfo(X, TT, CPU, ArchFS); // defined in dspGenSubtargetInfo.inc
return X;
}


static MCAsmInfo *createDSPMCAsmInfo(const MCRegisterInfo &MRI, StringRef TT) {
MCAsmInfo *MAI = new DSPMCAsmInfo(TT);
unsigned SP = MRI.getDwarfRegNum(DSP::SP, true);
MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(0, SP, 0);
MAI->addInitialFrameState(Inst);
return MAI;
}


static MCCodeGenInfo *createDSPMCCodeGenInfo(StringRef TT, Reloc::Model RM,
CodeModel::Model CM,
CodeGenOpt::Level OL) {
MCCodeGenInfo *X = new MCCodeGenInfo();
if (CM == CodeModel::JITDefault)
RM = Reloc::Static;
else if (RM == Reloc::Default)
RM = Reloc::PIC_;
X->InitMCCodeGenInfo(RM, CM, OL); // defined in lib/MC/MCCodeGenInfo.cpp
return X;
}

static MCInstPrinter *createDSPMCInstPrinter(const Target &T,
unsigned SyntaxVariant,
const MCAsmInfo &MAI,
const MCInstrInfo &MII,
const MCRegisterInfo &MRI,
const MCSubtargetInfo &STI) {
return new DSPInstPrinter(MAI, MII, MRI);
}

static MCStreamer *createMCStreamer(const Target &T, StringRef TT,
	MCContext &Context, MCAsmBackend &MAB,
	raw_ostream &OS, MCCodeEmitter *Emitter,
	const MCSubtargetInfo &STI,
	bool RelaxAll, bool NoExecStack) {
	return createELFStreamer(Context, MAB, OS, Emitter, RelaxAll,
		NoExecStack);
}
static MCStreamer *
createMCAsmStreamer(MCContext &Ctx, formatted_raw_ostream &OS,
bool isVerboseAsm,
bool useDwarfDirectory, MCInstPrinter *InstPrint,
MCCodeEmitter *CE, MCAsmBackend *TAB, bool ShowInst) {
	return llvm::createAsmStreamer(Ctx, OS, isVerboseAsm,
		useDwarfDirectory, InstPrint, CE, TAB,
		ShowInst);
}

extern "C" void LLVMInitializeDSPTargetMC() {
	// Register the MC Code Emitter
	//TargetRegistry::RegisterMCCodeEmitter(TheDSPTarget,createDSPMCCodeEmitterEB);
	TargetRegistry::RegisterMCCodeEmitter(TheDSPelTarget,
		createDSPMCCodeEmitterEL);
	// Register the object streamer.
	//TargetRegistry::RegisterMCObjectStreamer(TheDSPTarget, createMCStreamer);
	TargetRegistry::RegisterMCObjectStreamer(TheDSPelTarget, createMCStreamer);
	// Register the asm streamer.
	//TargetRegistry::RegisterAsmStreamer(TheDSPTarget, createMCAsmStreamer);
	TargetRegistry::RegisterAsmStreamer(TheDSPelTarget, createMCAsmStreamer);
	// Register the asm backend.
	//TargetRegistry::RegisterMCAsmBackend(TheDSPTarget,createDSPAsmBackendEB32);
	TargetRegistry::RegisterMCAsmBackend(TheDSPelTarget,
		createDSPAsmBackendEL32);


	// Register the MC asm info.

RegisterMCAsmInfoFn Y(TheDSPelTarget, createDSPMCAsmInfo);
// Register the MC codegen info.

TargetRegistry::RegisterMCCodeGenInfo(TheDSPelTarget,
createDSPMCCodeGenInfo);
// Register the MC instruction info.

TargetRegistry::RegisterMCInstrInfo(TheDSPelTarget, createDSPMCInstrInfo);
// Register the MC register info.

TargetRegistry::RegisterMCRegInfo(TheDSPelTarget, createDSPMCRegisterInfo);
// Register the MC subtarget info.

TargetRegistry::RegisterMCSubtargetInfo(TheDSPelTarget,
createDSPMCSubtargetInfo);
// Register the MCInstPrinter.

TargetRegistry::RegisterMCInstPrinter(TheDSPelTarget,
createDSPMCInstPrinter);
}