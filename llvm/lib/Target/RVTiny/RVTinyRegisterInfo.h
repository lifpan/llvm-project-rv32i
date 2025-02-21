
#ifndef LLVM_LIB_TARGET_RVTINY_RVTINYREGISTERINFO_H
#define LLVM_LIB_TARGET_RVTINY_RVTINYREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "RVTinyGenRegisterInfo.inc"

namespace llvm {
  struct RVTinyRegisterInfo : public RVTinyGenRegisterInfo {
    RVTinyRegisterInfo();

    /// Code Generation virtual methods...
    const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
    const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                         CallingConv::ID CC) const override;
  
    const uint32_t* getRTCallPreservedMask(CallingConv::ID CC) const;
  
    BitVector getReservedRegs(const MachineFunction &MF) const override;
    bool isReservedReg(const MachineFunction &MF, MCRegister Reg) const;
  
    const TargetRegisterClass *getPointerRegClass(const MachineFunction &MF,
                                                  unsigned Kind) const override;
  
    bool eliminateFrameIndex(MachineBasicBlock::iterator II,
                             int SPAdj, unsigned FIOperandNum,
                             RegScavenger *RS = nullptr) const override;
  
    Register getFrameRegister(const MachineFunction &MF) const override;

    Register isLoadFromStackSlot(const MachineInstr &MI, int &i) const;

    Register isStoreToStackSlot(const MachineInstr &MI, int &i) const;
  };
  
  } // namespace llvm

#endif