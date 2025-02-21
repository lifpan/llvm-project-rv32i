

#ifndef LLVM_LIB_TARGET_RVTINY_RVTINYTARGETMACHINE_H
#define LLVM_LIB_TARGET_RVTINY_RVTINYTARGETMACHINE_H

#include "RVTinyInstrInfo.h"
#include "RVTinySubtarget.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/Target/TargetMachine.h"
#include <optional>

namespace llvm {

class RVTinyTargetMachine : public CodeGenTargetMachineImpl {
    std::unique_ptr<TargetLoweringObjectFile> TLOF;
    bool is64Bit;
    mutable StringMap<std::unique_ptr<RVTinySubtarget>> SubtargetMap;

  public:
    RVTinyTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                        StringRef FS, const TargetOptions &Options,
                        std::optional<Reloc::Model> RM,
                        std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                        bool JIT, bool is64bit);
    ~RVTinyTargetMachine() override;

    virtual void anchor();

    const RVTinySubtarget *getSubtargetImpl(const Function &F) const override;
  
    // Pass Pipeline Configuration
    TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
    TargetLoweringObjectFile *getObjFileLowering() const override {
      return TLOF.get();
    }
  
    MachineFunctionInfo *
    createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                              const TargetSubtargetInfo *STI) const override;
  };

/// RVTiny 32-bit target machine
///
class RVTiny32ITargetMachine : public RVTinyTargetMachine {
  virtual void anchor();

public:
  RVTiny32ITargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                         StringRef FS, const TargetOptions &Options,
                         std::optional<Reloc::Model> RM,
                         std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                         bool JIT);
};
  
} // namespace llvm

#endif