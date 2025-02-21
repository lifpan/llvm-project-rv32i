

#ifndef LLVM_LIB_TARGET_RVTINY_RVTINYINSTRINFO_H
#define LLVM_LIB_TARGET_RVTINY_RVTINYINSTRINFO_H

#include "RVTinyRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "RVTinyGenInstrInfo.inc"

namespace llvm {

class RVTinySubtarget;

class RVTinyInstrInfo : public RVTinyGenInstrInfo {
  const RVTinyRegisterInfo RI;
  const RVTinySubtarget &Subtarget;
  virtual void anchor();

public:
  explicit RVTinyInstrInfo(RVTinySubtarget &ST);

  Register isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;

  Register isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;
  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   const DebugLoc &DL, MCRegister DestReg, MCRegister SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  void storeRegToStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register SrcReg,
      bool isKill, int FrameIndex, const TargetRegisterClass *RC,
      const TargetRegisterInfo *TRI, Register VReg,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  void loadRegFromStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
      Register DestReg, int FrameIndex, const TargetRegisterClass *RC,
      const TargetRegisterInfo *TRI, Register VReg,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  const RVTinyRegisterInfo& getRegisterInfo() const {
      return RI;
  }
};

} // namespace llvm

#endif
