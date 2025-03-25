
#include "RVTinyTargetStreamer.h"
#include "RVTinyInstPrinter.h"
#include "RVTinyMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

RVTinyTargetStreamer::RVTinyTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

void RVTinyTargetStreamer::anchor() {}

RVTinyTargetAsmStreamer::RVTinyTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS)
    : RVTinyTargetStreamer(S), OS(OS) {}

void RVTinyTargetAsmStreamer::emitRVTinyRegisterIgnore(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(RVTinyInstPrinter::getRegisterName(reg)).lower()
     << ", #ignore\n";
}

void RVTinyTargetAsmStreamer::emitRVTinyRegisterScratch(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(RVTinyInstPrinter::getRegisterName(reg)).lower()
     << ", #scratch\n";
}

RVTinyTargetELFStreamer::RVTinyTargetELFStreamer(MCStreamer& S,
    const MCSubtargetInfo& STI)
    : RVTinyTargetStreamer(S) {
  ELFObjectWriter &W = getStreamer().getWriter();
  unsigned EFlags = W.getELFHeaderEFlags();

  //EFlags |= getEFlagsForFeatureSet(STI);

  W.setELFHeaderEFlags(EFlags);
}

MCELFStreamer& RVTinyTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}
