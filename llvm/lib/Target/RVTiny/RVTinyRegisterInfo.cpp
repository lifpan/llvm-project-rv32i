
#include "RVTinyRegisterInfo.h"
#include "RVTiny.h"
#include "RVTinySubtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "RVTinyGenRegisterInfo.inc"

RVTinyRegisterInfo::RVTinyRegisterInfo() : RVTinyGenRegisterInfo(0) {}

const MCPhysReg*
RVTinyRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return nullptr;
}

const uint32_t*
RVTinyRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                         CallingConv::ID CC) const {
  return nullptr;
}

const uint32_t*
RVTinyRegisterInfo::getRTCallPreservedMask(CallingConv::ID CC) const {
  return nullptr;
}

BitVector
RVTinyRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector RR;
  return RR;
}

bool RVTinyRegisterInfo::isReservedReg(const MachineFunction &MF,
                                       MCRegister Reg) const {
  return false;
}

const TargetRegisterClass*
RVTinyRegisterInfo::getPointerRegClass(const MachineFunction &MF,
                                       unsigned Kind) const {
  return nullptr;
}

bool
RVTinyRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                        int SPAdj, unsigned FIOperandNum,
                                        RegScavenger *RS) const {
  return false;
}

Register RVTinyRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  Register Reg(RVTiny::X0);
  return Reg;
}

Register RVTinyRegisterInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                                 int &i) const {
  Register R(RVTiny::X0);
  return R;
}

Register RVTinyRegisterInfo::isStoreToStackSlot(const MachineInstr &MI,
                                                int &i) const {
  Register R(RVTiny::X0);
  return R;
}