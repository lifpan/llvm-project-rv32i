add_llvm_component_library(LLVMRVTinyDesc
  RVTinyInstPrinter.cpp
  RVTinyMCAsmInfo.cpp
  RVTinyMCExpr.cpp
  RVTinyMCTargetDesc.cpp
  RVTinyTargetStreamer.cpp

  LINK_COMPONENTS
  CodeGenTypes
  MC
  Support
  TargetParser

  ADD_TO_COMPONENT
  RVTiny
  )
