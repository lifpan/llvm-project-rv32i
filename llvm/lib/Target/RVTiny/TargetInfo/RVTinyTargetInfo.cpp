
#include "TargetInfo/RVTinyTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheRVTinyTarget() {
  static Target TheRVTinyTarget;
  return TheRVTinyTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRVTinyTargetInfo() {
  RegisterTarget<Triple::rvtiny, /*HasJIT=*/false> X(
      getTheRVTinyTarget(), "rvtiny", "RVTiny", "RVTiny");
}
