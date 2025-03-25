#include "RVTiny.h"
#include "RVTinyMCTargetDesc.h"
#include "RVTinyInstPrinter.h"
#include "RVTinyMCAsmInfo.h"
#include "RVTinyTargetStreamer.h"
#include "TargetInfo/RVTinyTargetInfo.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "RVTinyGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "RVTinyGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "RVTinyGenRegisterInfo.inc"

using namespace llvm;

static MCAsmInfo *createRVTinyMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT,
                                       const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new RVTinyELFMCAsmInfo(TT);
  unsigned Reg = MRI.getDwarfRegNum(RVTiny::X6, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, Reg, 0);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCInstrInfo *createRVTinyMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitRVTinyMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createRVTinyMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  return X;
}

static MCSubtargetInfo *
createRVTinyMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createRVTinyMCSubtargetInfoImpl(TT, CPU, CPU, FS);
}

static MCTargetStreamer *createTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &,
                                                 MCInstPrinter *) {
  return new RVTinyTargetStreamer(S);
}

static MCInstPrinter *createRVTinyMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  assert(SyntaxVariant == 0);
  return new RVTinyInstPrinter(MAI, MII, MRI);
}

namespace {

class RVTinyMCInstrAnalysis : public MCInstrAnalysis {
public:
  explicit RVTinyMCInstrAnalysis(const MCInstrInfo *Info)
      : MCInstrAnalysis(Info) {}
};

} // end anonymous namespace

static MCInstrAnalysis *createRVTinyInstrAnalysis(const MCInstrInfo *Info) {
  return new RVTinyMCInstrAnalysis(Info);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRVTinyTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(getTheRVTinyTarget(), createRVTinyMCAsmInfo);

  for (Target *T : {&getTheRVTinyTarget()}) {
    TargetRegistry::RegisterMCInstrInfo(*T, createRVTinyMCInstrInfo);
    TargetRegistry::RegisterMCRegInfo(*T, createRVTinyMCRegisterInfo);
    TargetRegistry::RegisterMCSubtargetInfo(*T, createRVTinyMCSubtargetInfo);
    TargetRegistry::RegisterMCInstPrinter(*T, createRVTinyMCInstPrinter);
    //TargetRegistry::RegisterMCInstrAnalysis(*T, createRVTinyInstrAnalysis);
    //TargetRegistry::RegisterMCCodeEmitter(*T, createRVTinyMCCodeEmitter);
    //TargetRegistry::RegisterMCAsmBackend(*T, createRVTinyAsmBackend);
    TargetRegistry::RegisterAsmTargetStreamer(*T, createTargetAsmStreamer);
  }
}
