add_llvm_component_group(RVTiny)

set(LLVM_TARGET_DEFINITIONS RVTiny.td)

tablegen(LLVM RVTinyGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM RVTinyGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM RVTinyGenCallingConv.inc -gen-callingconv)
tablegen(LLVM RVTinyGenDAGISel.inc -gen-dag-isel)
#tablegen(LLVM RVTinyGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM RVTinyGenInstrInfo.inc -gen-instr-info)
#tablegen(LLVM RVTinyGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM RVTinyGenRegisterInfo.inc -gen-register-info)
#tablegen(LLVM RVTinyGenSearchableTables.inc -gen-searchable-tables)
tablegen(LLVM RVTinyGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(RVTinyCommonTableGen)

add_llvm_target(RVTinyCodeGen
  RVTinyAsmPrinter.cpp
  RVTinyFrameLowering.cpp
  RVTinyInstrInfo.cpp
  RVTinyISelDAGToDAG.cpp
  RVTinyISelLowering.cpp
  RVTinyMachineFunctionInfo.cpp
  RVTinyRegisterInfo.cpp
  RVTinySubtarget.cpp
  RVTinyTargetMachine.cpp
  RVTinyTargetObjectFile.cpp

  LINK_COMPONENTS
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  MC
  SelectionDAG
  RVTinyDesc
  RVTinyInfo
  Support
  Target
  TargetParser

  ADD_TO_COMPONENT
  RVTiny
  )

add_subdirectory(AsmParser)
#add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)