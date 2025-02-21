
#include "RVTinyMachineFunctionInfo.h"

using namespace llvm;

void RVTinyMachineFunctionInfo::anchor() {}

MachineFunctionInfo *RVTinyMachineFunctionInfo::clone(
    BumpPtrAllocator &Allocator, MachineFunction &DestMF,
    const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
    const {
  return DestMF.cloneInfo<RVTinyMachineFunctionInfo>(*this);
}
