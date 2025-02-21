add_llvm_component_library(LLVMRVTinyDesc
  RVTinyMCTargetDesc.cpp

  LINK_COMPONENTS
  CodeGenTypes
  MC
  Support
  TargetParser

  ADD_TO_COMPONENT
  RVTiny
  )
