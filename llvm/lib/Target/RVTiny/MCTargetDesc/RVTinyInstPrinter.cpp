
#include "RVTinyInstPrinter.h"
#include "RVTiny.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define GET_INSTRUCTION_NAME
#define PRINT_ALIAS_INSTR
#include "RVTinyGenAsmWriter.inc"

void RVTinyInstPrinter::printRegName(raw_ostream& OS, MCRegister Reg) {
  OS << '%' << getRegisterName(Reg);
}

void RVTinyInstPrinter::printRegName(raw_ostream &OS, MCRegister Reg,
                                     unsigned AltIdx) const {
  OS << '%' << getRegisterName(Reg);
}

void RVTinyInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                  StringRef Annot, const MCSubtargetInfo &STI,
                                  raw_ostream &O) {
  if (!printAliasInstr(MI, Address, STI, O))
    printInstruction(MI, Address, STI, O);
  printAnnotation(O, Annot);
}

void RVTinyInstPrinter::printOperand(const MCInst *MI, int opNum,
                                     const MCSubtargetInfo &STI,
                                     raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(opNum);

  if (MO.isReg()) {
    unsigned Reg = MO.getReg();
    printRegName(O, Reg);
    return;
  }

  assert(MO.isExpr() && "Unkown operand kind in printOperand");
  MO.getExpr()->print(O, &MAI);
}
