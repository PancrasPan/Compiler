set(LLVM_TARGET_DEFINITIONS DSPMain.td)

tablegen(LLVM DSPGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM DSPGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM DSPGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM DSPGenPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM DSPGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM DSPGenCallingConv.inc -gen-callingconv)
tablegen(LLVM DSPGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM DSPGenCodeEmitter.inc -gen-emitter)
tablegen(LLVM DSPGenMCCodeEmitter.inc -gen-emitter -mc-emitter)
tablegen(LLVM DSPGenDFAPacketizer.inc -gen-dfa-packetizer)
add_public_tablegen_target(DSPCommonTableGen)

add_llvm_target(DSPCodeGen
	DSPAnalyzeImmediate.cpp
	DSPISelDAGToDAG.cpp
	DSPSEISelDAGToDAG.cpp
	DSPAsmPrinter.cpp
	DSPMemSizeEstimate.cpp
	DSPMCInstLower.cpp
	DSPTargetMachine.cpp
	DSPInstrInfo.cpp
	DSPISelLowering.cpp
	DSPFrameLowering.cpp
	DSPHandleSW.cpp
	DSPMachineFunction.cpp
	DSPRegisterInfo.cpp
	DSPSEFrameLowering.cpp
	DSPSEISelLowering.cpp
	DSPSERegisterInfo.cpp
	DSPSEInstrInfo.cpp
	DSPSubtarget.cpp
	DSPTargetObjectFile.cpp
	DSPSelectionDAGInfo.cpp
	DSPSWLoops.cpp
	DSPVLIWPacketizer.cpp
	DSPVLIWBundler.cpp
	DSPDelaySlotFiller.cpp
	DSPDelUselessJMP.cpp
	DSPMachineScheduler.cpp
	DSPHardwareLoops.cpp
	DSPFixupHwLoops.cpp
	DSPMemSizeEstimate.cpp
  )


add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
add_subdirectory(InstPrinter)

