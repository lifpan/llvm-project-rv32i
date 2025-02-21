
#ifndef LLVM_LIB_TARGET_RVTINY_RVTINY_H
#define LLVM_LIB_TARGET_RVTINY_RVTINY_H

#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class Function;
class FunctionPass;
class PassRegistry;
class RVTinyTargetMachine;
class Target;

Target &getTheRVTinyTarget();

FunctionPass *createRVTinyISelDag(RVTinyTargetMachine &TM);

void initializeRVTinyDAGToDAGISelLegacyPass(PassRegistry &);

} // namespace llvm

#endif