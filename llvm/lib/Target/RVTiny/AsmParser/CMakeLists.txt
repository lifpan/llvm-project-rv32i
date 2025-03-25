add_llvm_component_library(LLVMRVTinyAsmParser
  RVTinyAsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  RVTinyDesc
  RVTinyInfo
  Support
  TargetParser

  ADD_TO_COMPONENT
  RVTiny
  )
