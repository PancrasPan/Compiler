# RUN: not llvm-mc %s -triple=mipsel-unknown-linux -mcpu=mips32r2 -arch=mips -mattr=+micromips 2>&1 -filetype=obj | FileCheck %s
#
# CHECK: LLVM ERROR: out of range PC16 fixup

.text
  b foo
  .space 65536 - 8, 1   # -8 = size of b instr plus size of automatically inserted nop
  nop                   # This instr makes the branch too long to fit into a 17-bit offset
foo:
  add $0,$0,$0
