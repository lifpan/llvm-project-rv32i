add_llvm_component_library(LLVMRVTinyInfo
  RVTinyTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  RVTiny
  )
