#ifndef DSPMCTARGETDESC_H
#define DSPMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

namespace llvm{
	class Target;
	class MCAsmBackend;
	class MCCodeEmitter;
	class MCContext;
	class MCInstrInfo;
	class MCObjectWriter;
	class MCRegisterInfo;
	class MCSubtargetInfo;
	class StringRef;
	class raw_ostream;
	extern Target TheDSPelTarget;

	MCCodeEmitter *createDSPMCCodeEmitterEB(const MCInstrInfo &MCII,
		const MCRegisterInfo &MRI,
		const MCSubtargetInfo &STI,
		MCContext &Ctx);
	MCCodeEmitter *createDSPMCCodeEmitterEL(const MCInstrInfo &MCII,
		const MCRegisterInfo &MRI,
		const MCSubtargetInfo &STI,
		MCContext &Ctx);
	MCAsmBackend *createDSPAsmBackendEB32(const Target &T, const MCRegisterInfo &MRI,
		StringRef TT, StringRef CPU);
	MCAsmBackend *createDSPAsmBackendEL32(const Target &T, const MCRegisterInfo &MRI,
		StringRef TT, StringRef CPU);

	MCObjectWriter *createDSPELFObjectWriter(raw_ostream &OS,
		uint8_t OSABI,
		bool IsLittleEndian);
} // End llvm namespace



// Defines symbolic names for DSP registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "DSPGenRegisterInfo.inc"

// Defines symbolic names for the DSP instructions.
#define GET_INSTRINFO_ENUM
#include "DSPGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "DSPGenSubtargetInfo.inc"

#endif