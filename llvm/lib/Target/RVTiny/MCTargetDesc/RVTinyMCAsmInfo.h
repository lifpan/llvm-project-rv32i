
#ifndef LLVM_LIB_TARGET_RVTINY_MCTARGETDESC_RVTINYMCASMINFO_H
#define LLVM_LIB_TARGET_RVTINY_MCTARGETDESC_RVTINYMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {

class Triple;

class RVTinyELFMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit RVTinyELFMCAsmInfo(const Triple &TheTriple);

  const MCExpr*
  getExprForPersonalitySymbol(const MCSymbol *Sym, unsigned Encoding,
                              MCStreamer &Streamer) const override;
  const MCExpr* getExprForFDESymbol(const MCSymbol *Sym,
                                    unsigned Encoding,
                                    MCStreamer &Streamer) const override;

};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_RVTINY_MCTARGETDESC_RVTINYMCASMINFO_H
