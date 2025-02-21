
#ifndef LLVM_LIB_TARGET_RVTINY_RVTINYSUBTARGET_H
#define LLVM_LIB_TARGET_RVTINY_RVTINYSUBTARGET_H

#include "MCTargetDesc/RVTinyMCTargetDesc.h"
#include "RVTinyFrameLowering.h"
#include "RVTinyISelLowering.h"
#include "RVTinyInstrInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/TargetParser/Triple.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "RVTinyGenSubtargetInfo.inc"

namespace llvm {
class StringRef;

class RVTinySubtarget : public RVTinyGenSubtargetInfo {
  // ReserveRegister[i] - Register #i is not available as a general purpose
  // register.
  BitVector ReserveRegister;

  Triple TargetTriple;
  virtual void anchor();

  bool Is64Bit;

#define GET_SUBTARGETINFO_MACRO(ATTRIBUTE, DEFAULT, GETTER)                    \
  bool ATTRIBUTE = DEFAULT;
#include "RVTinyGenSubtargetInfo.inc"

  RVTinyInstrInfo InstrInfo;
  RVTinyTargetLowering TLInfo;
  SelectionDAGTargetInfo TSInfo;
  RVTinyFrameLowering FrameLowering;

public:
  RVTinySubtarget(const StringRef &CPU, const StringRef &TuneCPU,
                  const StringRef &FS, const TargetMachine &TM, bool is64bit);

  const RVTinyInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const TargetFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const RVTinyRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }
  const RVTinyTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }

  bool enableMachineScheduler() const override;

#define GET_SUBTARGETINFO_MACRO(ATTRIBUTE, DEFAULT, GETTER)                    \
  bool GETTER() const { return ATTRIBUTE; }
#include "RVTinyGenSubtargetInfo.inc"

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);
  RVTinySubtarget &initializeSubtargetDependencies(StringRef CPU,
                                                   StringRef TuneCPU,
                                                   StringRef FS);
  bool is64Bit() const { return false; }
};

} // namespace llvm

#endif