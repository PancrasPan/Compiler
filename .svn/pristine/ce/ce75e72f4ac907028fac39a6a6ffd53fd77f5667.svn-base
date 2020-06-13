This is the DSP backend source code.
The copyright is belong to cic.
In order to build this code,you need to:

1.new a dir in llvm_src/lib/Target called DSP
llvm_src/lib/Target/DSP

2.copy all these code to DSP

3.modify llvm_src/lib/Target/LLVMBuild.txt

Add DSP to subdirectories
[common]
subdirectories = ARM AArch64 CppBackend  DSP Hexagon MSP430 NVPTX Mips PowerPC R600 Sparc SystemZ X86 XCore

4.modify llvm_src/CmakeLists.txt

Add DSP to  set(LLVM_ALL_TARGETS

set(LLVM_ALL_TARGETS
  AArch64
  ARM
  CppBackend
  Hexagon
  Mips
  MSP430
  NVPTX
  PowerPC
  R600
  Sparc
  SystemZ
  X86
  XCore
  DSP
  )

 5.modify llvm_src/cmake/config-ix.cmake

locate to line 363

add elseif (LLVM_NATIVE_ARCH MATCHES "dsp")
  set(LLVM_NATIVE_ARCH DSP)

 finally like 
 if (LLVM_NATIVE_ARCH MATCHES "i[2-6]86")
  set(LLVM_NATIVE_ARCH X86)
elseif (LLVM_NATIVE_ARCH STREQUAL "x86")
  set(LLVM_NATIVE_ARCH X86)
elseif (LLVM_NATIVE_ARCH STREQUAL "amd64")
  set(LLVM_NATIVE_ARCH X86)
elseif (LLVM_NATIVE_ARCH STREQUAL "x86_64")
  set(LLVM_NATIVE_ARCH X86)
elseif (LLVM_NATIVE_ARCH MATCHES "sparc")
  set(LLVM_NATIVE_ARCH Sparc)
elseif (LLVM_NATIVE_ARCH MATCHES "powerpc")
  set(LLVM_NATIVE_ARCH PowerPC)
elseif (LLVM_NATIVE_ARCH MATCHES "aarch64")
  set(LLVM_NATIVE_ARCH AArch64)
elseif (LLVM_NATIVE_ARCH MATCHES "arm64")
  set(LLVM_NATIVE_ARCH AArch64)
elseif (LLVM_NATIVE_ARCH MATCHES "arm")
  set(LLVM_NATIVE_ARCH ARM)
elseif (LLVM_NATIVE_ARCH MATCHES "mips")
  set(LLVM_NATIVE_ARCH Mips)
elseif (LLVM_NATIVE_ARCH MATCHES "xcore")
  set(LLVM_NATIVE_ARCH XCore)
elseif (LLVM_NATIVE_ARCH MATCHES "msp430")
  set(LLVM_NATIVE_ARCH MSP430)
elseif (LLVM_NATIVE_ARCH MATCHES "hexagon")
  set(LLVM_NATIVE_ARCH Hexagon)
elseif (LLVM_NATIVE_ARCH MATCHES "s390x")
  set(LLVM_NATIVE_ARCH SystemZ)
elseif (LLVM_NATIVE_ARCH MATCHES "dsp")
  set(LLVM_NATIVE_ARCH DSP)



 6 cmake src code again then  build
