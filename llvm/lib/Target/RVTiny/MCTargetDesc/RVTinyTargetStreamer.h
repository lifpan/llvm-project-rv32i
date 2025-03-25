
#ifndef LLVM_LIB_TARGET_RVTINY_MCTARGETDESC_RVTINYTARGETSTREAMER_H
#define LLVM_LIB_TARGET_RVTINY_MCTARGETDESC_RVTINYTARGETSTREAMER_H

#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {

class formatted_raw_ostream;

class RVTinyTargetStreamer : public MCTargetStreamer {
  virtual void anchor();

public:
  RVTinyTargetStreamer(MCStreamer &S);

  virtual void emitRVTinyRegisterIgnore(unsigned reg) {};

  virtual void emitRVTinyRegisterScratch(unsigned reg) {};
};

// This part is for ascii assembly output
class RVTinyTargetAsmStreamer : public RVTinyTargetStreamer {
  formatted_raw_ostream &OS;

public:
  RVTinyTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);
  void emitRVTinyRegisterIgnore(unsigned reg) override;
  void emitRVTinyRegisterScratch(unsigned reg) override;
};

// This part is for ELF object output
class RVTinyTargetELFStreamer : public RVTinyTargetStreamer {
public:
  RVTinyTargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);
  MCELFStreamer &getStreamer();
  void emitRVTinyRegisterIgnore(unsigned reg) override {}
  void emitRVTinyRegisterScratch(unsigned reg) override {}
};

} // end namespace llvm

#endif
