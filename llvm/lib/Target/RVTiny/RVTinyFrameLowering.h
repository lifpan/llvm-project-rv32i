
#ifndef LLVM_LIB_TARGET_RVTINY_RVTINYFRAMELOWERING_H
#define LLVM_LIB_TARGET_RVTINY_RVTINYFRAMELOWERING_H

#include "RVTiny.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Support/TypeSize.h"

namespace llvm {

  class RVTinySubtarget;
  class RVTinyFrameLowering : public TargetFrameLowering {
  public:
    explicit RVTinyFrameLowering(const RVTinySubtarget &ST);
  
    /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
    /// the function.
    void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
    void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

    bool hasFPImpl(const MachineFunction &MF) const { return false; }
  };

} // namespace llvm

#endif