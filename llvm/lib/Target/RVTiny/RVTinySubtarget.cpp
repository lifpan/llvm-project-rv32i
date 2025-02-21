

#include "RVTinySubtarget.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

#define DEBUG_TYPE "rvtiny-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "RVTinyGenSubtargetInfo.inc"

void RVTinySubtarget::anchor() { }

RVTinySubtarget &RVTinySubtarget::initializeSubtargetDependencies(
    StringRef CPU, StringRef TuneCPU, StringRef FS) {
  // Determine default and user specified characteristics
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = (Is64Bit) ? "v9" : "v8";

  if (TuneCPU.empty())
    TuneCPU = CPUName;

  // Parse features string.
  ParseSubtargetFeatures(CPUName, TuneCPU, FS);

  return *this;
}

RVTinySubtarget::RVTinySubtarget(const StringRef &CPU, const StringRef &TuneCPU,
                                 const StringRef &FS, const TargetMachine &TM,
                                 bool is64Bit)
  : RVTinyGenSubtargetInfo(TM.getTargetTriple(), CPU, TuneCPU, FS),
    ReserveRegister(TM.getMCRegisterInfo()->getNumRegs()),
    TargetTriple(TM.getTargetTriple()), Is64Bit(is64Bit),
    InstrInfo(initializeSubtargetDependencies(CPU, TuneCPU, FS)),
    TLInfo(TM, *this), FrameLowering(*this) {}

bool RVTinySubtarget::enableMachineScheduler() const {
  return false;
}
