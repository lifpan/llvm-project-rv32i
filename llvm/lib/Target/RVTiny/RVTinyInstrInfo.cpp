

#include "RVTinyInstrInfo.h"
#include "RVTiny.h"
//#include "RVTinyMachineFunctionInfo.h"
#include "RVTinySubtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "RVTinyGenInstrInfo.inc"

// Pin the vtable to this file.
void RVTinyInstrInfo::anchor() {}

RVTinyInstrInfo::RVTinyInstrInfo(RVTinySubtarget &ST)
    : RVTinyGenInstrInfo(0/*RVTiny::ADJCALLSTACKDOWN*/, 0/*RVTiny::ADJCALLSTACKUP*/),
      RI(),
      Subtarget(ST) {}

Register RVTinyInstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                              int &FrameIndex) const {
  Register R(RVTiny::X0);
  return R;
}

Register RVTinyInstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                             int &FrameIndex) const {
  Register R(RVTiny::X0);
  return R;
}

void RVTinyInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I,
                                  const DebugLoc &DL, MCRegister DestReg,
                                  MCRegister SrcReg, bool KillSrc,
                                  bool RenamableDest,
                                  bool RenamableSrc) const {}

void RVTinyInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register SrcReg,
    bool isKill, int FrameIndex, const TargetRegisterClass *RC,
    const TargetRegisterInfo *TRI, Register VReg,
    MachineInstr::MIFlag Flags) const {}

void RVTinyInstrInfo::loadRegFromStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register DestReg,
    int FrameIndex, const TargetRegisterClass *RC,
    const TargetRegisterInfo *TRI, Register VReg,
    MachineInstr::MIFlag ) const {}
