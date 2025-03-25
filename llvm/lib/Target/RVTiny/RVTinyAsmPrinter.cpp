
#include "MCTargetDesc/RVTinyInstPrinter.h"
#include "MCTargetDesc/RVTinyMCExpr.h"
#include "MCTargetDesc/RVTinyMCTargetDesc.h"
#include "MCTargetDesc/RVTinyTargetStreamer.h"
#include "RVTiny.h"
#include "RVTinyInstrInfo.h"
#include "RVTinyTargetMachine.h"
#include "TargetInfo/RVTinyTargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfoImpls.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {
  class RVTinyAsmPrinter : public AsmPrinter {
    RVTinyTargetStreamer& getTargetStreamer() {
      return static_cast<RVTinyTargetStreamer&>(
          *OutStreamer->getTargetStreamer());
    }
  public:
    explicit RVTinyAsmPrinter(TargetMachine &TM,
                              std::unique_ptr<MCStreamer> Streamer)
        : AsmPrinter(TM, std::move(Streamer)) {}
  
    StringRef getPassName() const override { return "RVTiny Assembly Printer"; }

    void printOperand(const MachineInstr *MI, int opNum, raw_ostream &OS);
    void printMemOperand(const MachineInstr *MI, int opNum, raw_ostream &O);

    void emitFunctionBodyStart() override;
    void emitInstruction(const MachineInstr *MI) override;

    static const char* getRegisterName(MCRegister Reg) {
      return RVTinyInstPrinter::getRegisterName(Reg);
    }

    
    bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                         const char *ExtraCode, raw_ostream &O) override;
    bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                               const char *ExtraCode, raw_ostream &O) override;
  };
} // end of anonymous namespace

static MCOperand createRVTinyMCOperand(RVTinyMCExpr::VariantKind Kind,
                                       MCSymbol *Sym, MCContext &OutContext) {
  const MCSymbolRefExpr *MCSym = MCSymbolRefExpr::create(Sym, OutContext);
  const RVTinyMCExpr *expr = RVTinyMCExpr::create(Kind, MCSym, OutContext);
  return MCOperand::createExpr(expr);
}

void RVTinyAsmPrinter::emitInstruction(const MachineInstr* MI) {

}

void RVTinyAsmPrinter::emitFunctionBodyStart() {

}

void RVTinyAsmPrinter::printOperand(const MachineInstr *MI, int opNum,
                                    raw_ostream &O) {

}

void RVTinyAsmPrinter::printMemOperand(const MachineInstr *MI, int opNum,
                                       raw_ostream &O) {
  printOperand(MI, opNum, O);

  O << "+";
  printOperand(MI, opNum + 1, O);
}

bool RVTinyAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                       const char *ExtraCode, raw_ostream &O) {
  if (ExtraCode && ExtraCode[0]) {
  }

  printOperand(MI, OpNo, O);

  return false;
}


bool RVTinyAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                            unsigned OpNo,
                                            const char *ExtraCode,
                                            raw_ostream &O) {
  if (ExtraCode && ExtraCode[0])
    return true; // Unknown modifier

  O << '[';
  printMemOperand(MI, OpNo, O);
  O << ']';

  return false;
}


// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRVTinyAsmPrinter() {
  RegisterAsmPrinter<RVTinyAsmPrinter> X(getTheRVTinyTarget());
}
