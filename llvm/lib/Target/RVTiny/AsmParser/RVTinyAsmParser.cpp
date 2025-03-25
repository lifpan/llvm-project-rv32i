
#include "MCTargetDesc/RVTinyMCExpr.h"
#include "MCTargetDesc/RVTinyMCTargetDesc.h"
#include "TargetInfo/RVTinyTargetInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCAsmMacro.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/SMLoc.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>

using namespace llvm;

namespace {

class RVTinyOperand;

class RVTinyAsmParser : public MCTargetAsmParser {
  MCAsmParser &Parser;
  const MCRegisterInfo &MRI;

  enum class TailRelocKind { Load_GOT, Add_TLS, Load_TLS, Call_TLS };

  /// @name Auto-generated Match Functions
  /// {

#define GET_ASSEMBLER_HEADER
#include "RVTinyGenAsmMatcher.inc"

  /// }

  // public interface of the MCTargetAsmParser.
  bool matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;
  bool parseRegister(MCRegister &Reg, SMLoc &StartLoc, SMLoc &EndLoc) override;
  ParseStatus tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                               SMLoc &EndLoc) override;
  bool parseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;
  ParseStatus parseDirective(AsmToken DirectiveID) override;

  unsigned validateTargetOperandClass(MCParsedAsmOperand &Op,
                                      unsigned Kind) override;

  // Custom parse functions for RVTiny specific operands.
  ParseStatus parseMEMOperand(OperandVector &Operands);

  ParseStatus parseMembarTag(OperandVector &Operands);

  ParseStatus parseASITag(OperandVector &Operands);

  ParseStatus parsePrefetchTag(OperandVector &Operands);

  template <TailRelocKind Kind>
  ParseStatus parseTailRelocSym(OperandVector &Operands);

  template <unsigned N> ParseStatus parseShiftAmtImm(OperandVector &Operands);

  ParseStatus parseCallTarget(OperandVector &Operands);

  ParseStatus parseOperand(OperandVector &Operands, StringRef Name);

  ParseStatus parseRVTinyAsmOperand(std::unique_ptr<RVTinyOperand> &Operand,
                                   bool isCall = false);

  ParseStatus parseBranchModifiers(OperandVector &Operands);

  ParseStatus parseExpression(int64_t &Val);

  // Helper function for dealing with %lo / %hi in PIC mode.
  const RVTinyMCExpr *adjustPICRelocation(RVTinyMCExpr::VariantKind VK,
                                         const MCExpr *subExpr);

  // Helper function to see if current token can start an expression.
  bool isPossibleExpression(const AsmToken &Token);

  // Check if mnemonic is valid.
  MatchResultTy mnemonicIsValid(StringRef Mnemonic, unsigned VariantID);

  // returns true if Tok is matched to a register and returns register in RegNo.
  MCRegister matchRegisterName(const AsmToken &Tok, unsigned &RegKind);

  bool matchRVTinyAsmModifiers(const MCExpr *&EVal, SMLoc &EndLoc);

  bool is64Bit() const {
    return false;
  }

  bool expandSET(MCInst &Inst, SMLoc IDLoc,
                 SmallVectorImpl<MCInst> &Instructions);

  bool expandSETSW(MCInst &Inst, SMLoc IDLoc,
                   SmallVectorImpl<MCInst> &Instructions);

  bool expandSETX(MCInst &Inst, SMLoc IDLoc,
                  SmallVectorImpl<MCInst> &Instructions);

  SMLoc getLoc() const { return getParser().getTok().getLoc(); }

public:
  RVTinyAsmParser(const MCSubtargetInfo &sti, MCAsmParser &parser,
                 const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, sti, MII), Parser(parser),
        MRI(*Parser.getContext().getRegisterInfo()) {
    Parser.addAliasForDirective(".half", ".2byte");
    Parser.addAliasForDirective(".uahalf", ".2byte");
    Parser.addAliasForDirective(".word", ".4byte");
    Parser.addAliasForDirective(".uaword", ".4byte");
    Parser.addAliasForDirective(".nword", is64Bit() ? ".8byte" : ".4byte");
    if (is64Bit())
      Parser.addAliasForDirective(".xword", ".8byte");

    // Initialize the set of available features.
    setAvailableFeatures(ComputeAvailableFeatures(getSTI().getFeatureBits()));
  }
};

} // end anonymous namespace

namespace {

/// RVTinyOperand - Instances of this class represent a parsed RVTiny machine
/// instruction.
class RVTinyOperand : public MCParsedAsmOperand {
public:
  enum RegisterKind {
    rk_None,
    rk_IntReg,
    rk_IntPairReg,
    rk_FloatReg,
    rk_DoubleReg,
    rk_QuadReg,
    rk_CoprocReg,
    rk_CoprocPairReg,
    rk_Special,
  };

private:
  enum KindTy {
    k_Token,
    k_Register,
    k_Immediate,
    k_MemoryReg,
    k_MemoryImm,
    k_ASITag,
    k_PrefetchTag,
    k_TailRelocSym, // Special kind of immediate for TLS relocation purposes.
  } Kind;

  SMLoc StartLoc, EndLoc;

  struct Token {
    const char *Data;
    unsigned Length;
  };

  struct RegOp {
    unsigned RegNum;
    RegisterKind Kind;
  };

  struct ImmOp {
    const MCExpr *Val;
  };

  struct MemOp {
    unsigned Base;
    unsigned OffsetReg;
    const MCExpr *Off;
  };

  union {
    struct Token Tok;
    struct RegOp Reg;
    struct ImmOp Imm;
    struct MemOp Mem;
    unsigned ASI;
    unsigned Prefetch;
  };

public:
  RVTinyOperand(KindTy K) : Kind(K) {}

  bool isToken() const override { return Kind == k_Token; }
  bool isReg() const override { return Kind == k_Register; }
  bool isImm() const override { return Kind == k_Immediate; }
  bool isMem() const override { return isMEMrr() || isMEMri(); }
  bool isMEMrr() const { return Kind == k_MemoryReg; }
  bool isMEMri() const { return Kind == k_MemoryImm; }
  bool isMembarTag() const { return Kind == k_Immediate; }
  bool isASITag() const { return Kind == k_ASITag; }
  bool isPrefetchTag() const { return Kind == k_PrefetchTag; }
  bool isTailRelocSym() const { return Kind == k_TailRelocSym; }

  bool isCallTarget() const {
    if (!isImm())
      return false;

    if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Imm.Val))
      return CE->getValue() % 4 == 0;

    return true;
  }

  bool isShiftAmtImm5() const {
    if (!isImm())
      return false;

    if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Imm.Val))
      return isUInt<5>(CE->getValue());

    return false;
  }

  bool isShiftAmtImm6() const {
    if (!isImm())
      return false;

    if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Imm.Val))
      return isUInt<6>(CE->getValue());

    return false;
  }

  bool isIntReg() const {
    return (Kind == k_Register && Reg.Kind == rk_IntReg);
  }

  bool isFloatReg() const {
    return (Kind == k_Register && Reg.Kind == rk_FloatReg);
  }

  bool isFloatOrDoubleReg() const {
    return (Kind == k_Register && (Reg.Kind == rk_FloatReg
                                   || Reg.Kind == rk_DoubleReg));
  }

  bool isCoprocReg() const {
    return (Kind == k_Register && Reg.Kind == rk_CoprocReg);
  }

  StringRef getToken() const {
    assert(Kind == k_Token && "Invalid access!");
    return StringRef(Tok.Data, Tok.Length);
  }

  MCRegister getReg() const override {
    assert((Kind == k_Register) && "Invalid access!");
    return Reg.RegNum;
  }

  const MCExpr *getImm() const {
    assert((Kind == k_Immediate) && "Invalid access!");
    return Imm.Val;
  }

  unsigned getMemBase() const {
    assert((Kind == k_MemoryReg || Kind == k_MemoryImm) && "Invalid access!");
    return Mem.Base;
  }

  unsigned getMemOffsetReg() const {
    assert((Kind == k_MemoryReg) && "Invalid access!");
    return Mem.OffsetReg;
  }

  const MCExpr *getMemOff() const {
    assert((Kind == k_MemoryImm) && "Invalid access!");
    return Mem.Off;
  }

  unsigned getASITag() const {
    assert((Kind == k_ASITag) && "Invalid access!");
    return ASI;
  }

  unsigned getPrefetchTag() const {
    assert((Kind == k_PrefetchTag) && "Invalid access!");
    return Prefetch;
  }

  const MCExpr *getTailRelocSym() const {
    assert((Kind == k_TailRelocSym) && "Invalid access!");
    return Imm.Val;
  }

  /// getStartLoc - Get the location of the first token of this operand.
  SMLoc getStartLoc() const override {
    return StartLoc;
  }
  /// getEndLoc - Get the location of the last token of this operand.
  SMLoc getEndLoc() const override {
    return EndLoc;
  }

  void print(raw_ostream &OS) const override {
    switch (Kind) {
    case k_Token:     OS << "Token: " << getToken() << "\n"; break;
    case k_Register:  OS << "Reg: #" << getReg() << "\n"; break;
    case k_Immediate: OS << "Imm: " << getImm() << "\n"; break;
    case k_MemoryReg: OS << "Mem: " << getMemBase() << "+"
                         << getMemOffsetReg() << "\n"; break;
    case k_MemoryImm: assert(getMemOff() != nullptr);
      OS << "Mem: " << getMemBase()
         << "+" << *getMemOff()
         << "\n"; break;
    case k_ASITag:
      OS << "ASI tag: " << getASITag() << "\n";
      break;
    case k_PrefetchTag:
      OS << "Prefetch tag: " << getPrefetchTag() << "\n";
      break;
    case k_TailRelocSym:
      OS << "TailReloc: " << getTailRelocSym() << "\n";
      break;
    }
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    const MCExpr *Expr = getImm();
    addExpr(Inst, Expr);
  }

  void addShiftAmtImm5Operands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }
  void addShiftAmtImm6Operands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const{
    // Add as immediate when possible.  Null MCExpr = 0.
    if (!Expr)
      Inst.addOperand(MCOperand::createImm(0));
    else if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  void addMEMrrOperands(MCInst &Inst, unsigned N) const {
    assert(N == 2 && "Invalid number of operands!");

    Inst.addOperand(MCOperand::createReg(getMemBase()));

    assert(getMemOffsetReg() != 0 && "Invalid offset");
    Inst.addOperand(MCOperand::createReg(getMemOffsetReg()));
  }

  void addMEMriOperands(MCInst &Inst, unsigned N) const {
    assert(N == 2 && "Invalid number of operands!");

    Inst.addOperand(MCOperand::createReg(getMemBase()));

    const MCExpr *Expr = getMemOff();
    addExpr(Inst, Expr);
  }

  void addASITagOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createImm(getASITag()));
  }

  void addPrefetchTagOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createImm(getPrefetchTag()));
  }

  void addMembarTagOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    const MCExpr *Expr = getImm();
    addExpr(Inst, Expr);
  }

  void addCallTargetOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

  void addTailRelocSymOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getTailRelocSym());
  }

  static std::unique_ptr<RVTinyOperand> CreateToken(StringRef Str, SMLoc S) {
    auto Op = std::make_unique<RVTinyOperand>(k_Token);
    Op->Tok.Data = Str.data();
    Op->Tok.Length = Str.size();
    Op->StartLoc = S;
    Op->EndLoc = S;
    return Op;
  }

  static std::unique_ptr<RVTinyOperand> CreateReg(unsigned RegNum, unsigned Kind,
                                                 SMLoc S, SMLoc E) {
    auto Op = std::make_unique<RVTinyOperand>(k_Register);
    Op->Reg.RegNum = RegNum;
    Op->Reg.Kind   = (RVTinyOperand::RegisterKind)Kind;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<RVTinyOperand> CreateImm(const MCExpr *Val, SMLoc S,
                                                 SMLoc E) {
    auto Op = std::make_unique<RVTinyOperand>(k_Immediate);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<RVTinyOperand> CreateASITag(unsigned Val, SMLoc S,
                                                    SMLoc E) {
    auto Op = std::make_unique<RVTinyOperand>(k_ASITag);
    Op->ASI = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<RVTinyOperand> CreatePrefetchTag(unsigned Val, SMLoc S,
                                                         SMLoc E) {
    auto Op = std::make_unique<RVTinyOperand>(k_PrefetchTag);
    Op->Prefetch = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<RVTinyOperand> CreateTailRelocSym(const MCExpr *Val,
                                                          SMLoc S, SMLoc E) {
    auto Op = std::make_unique<RVTinyOperand>(k_TailRelocSym);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static bool MorphToIntPairReg(RVTinyOperand &Op) {
    unsigned Reg = Op.getReg();
    assert(Op.Reg.Kind == rk_IntReg);
    return true;
  }

  static bool MorphToDoubleReg(RVTinyOperand &Op) {
    unsigned Reg = Op.getReg();
    assert(Op.Reg.Kind == rk_FloatReg);
    return true;
  }

  static bool MorphToQuadReg(RVTinyOperand &Op) {
    unsigned Reg = Op.getReg();
    unsigned regIdx = 0;
    return true;
  }

  static bool MorphToCoprocPairReg(RVTinyOperand &Op) {
    return true;
  }

  static std::unique_ptr<RVTinyOperand>
  MorphToMEMrr(unsigned Base, std::unique_ptr<RVTinyOperand> Op) {
    unsigned offsetReg = Op->getReg();
    Op->Kind = k_MemoryReg;
    Op->Mem.Base = Base;
    Op->Mem.OffsetReg = offsetReg;
    Op->Mem.Off = nullptr;
    return Op;
  }

  static std::unique_ptr<RVTinyOperand>
  CreateMEMr(unsigned Base, SMLoc S, SMLoc E) {
    auto Op = std::make_unique<RVTinyOperand>(k_MemoryReg);
    Op->Mem.Base = Base;
    Op->Mem.OffsetReg = RVTiny::X0;  // always 0
    Op->Mem.Off = nullptr;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<RVTinyOperand>
  MorphToMEMri(unsigned Base, std::unique_ptr<RVTinyOperand> Op) {
    const MCExpr *Imm  = Op->getImm();
    Op->Kind = k_MemoryImm;
    Op->Mem.Base = Base;
    Op->Mem.OffsetReg = 0;
    Op->Mem.Off = Imm;
    return Op;
  }
};

} // end anonymous namespace

#define GET_MATCHER_IMPLEMENTATION
#define GET_REGISTER_MATCHER
#define GET_MNEMONIC_SPELL_CHECKER
#include "RVTinyGenAsmMatcher.inc"

// Use a custom function instead of the one from RVTinyGenAsmMatcher
// so we can differentiate between unavailable and unknown instructions.
RVTinyAsmParser::MatchResultTy
RVTinyAsmParser::mnemonicIsValid(StringRef Mnemonic, unsigned VariantID) {
  // Process all MnemonicAliases to remap the mnemonic.
  //applyMnemonicAliases(Mnemonic, getAvailableFeatures(), VariantID);

  // Find the appropriate table for this asm variant.
  const MatchEntry *Start, *End;
  switch (VariantID) {
  default:
    llvm_unreachable("invalid variant!");
  case 0:
    Start = std::begin(MatchTable0);
    End = std::end(MatchTable0);
    break;
  }

  // Search the table.
  auto MnemonicRange = std::equal_range(Start, End, Mnemonic, LessOpcode());

  if (MnemonicRange.first == MnemonicRange.second)
    return Match_MnemonicFail;

  for (const MatchEntry *it = MnemonicRange.first, *ie = MnemonicRange.second;
       it != ie; ++it) {
    const FeatureBitset &RequiredFeatures =
        FeatureBitsets[it->RequiredFeaturesIdx];
    if ((getAvailableFeatures() & RequiredFeatures) == RequiredFeatures)
      return Match_Success;
  }
  return Match_MissingFeature;
}

bool RVTinyAsmParser::expandSET(MCInst &Inst, SMLoc IDLoc,
                               SmallVectorImpl<MCInst> &Instructions) {
  MCOperand MCRegOp = Inst.getOperand(0);
  MCOperand MCValOp = Inst.getOperand(1);
  assert(MCRegOp.isReg());
  assert(MCValOp.isImm() || MCValOp.isExpr());

  // the imm operand can be either an expression or an immediate.
  bool IsImm = Inst.getOperand(1).isImm();
  int64_t RawImmValue = IsImm ? MCValOp.getImm() : 0;

  // Allow either a signed or unsigned 32-bit immediate.
  if (RawImmValue < -2147483648LL || RawImmValue > 4294967295LL) {
    return Error(IDLoc,
                 "set: argument must be between -2147483648 and 4294967295");
  }

  return false;
}

bool RVTinyAsmParser::expandSETSW(MCInst &Inst, SMLoc IDLoc,
                                 SmallVectorImpl<MCInst> &Instructions) {
  MCOperand MCRegOp = Inst.getOperand(0);
  MCOperand MCValOp = Inst.getOperand(1);
  assert(MCRegOp.isReg());
  assert(MCValOp.isImm() || MCValOp.isExpr());

  // The imm operand can be either an expression or an immediate.
  bool IsImm = Inst.getOperand(1).isImm();
  int64_t ImmValue = IsImm ? MCValOp.getImm() : 0;
  const MCExpr *ValExpr = IsImm ? MCConstantExpr::create(ImmValue, getContext())
                                : MCValOp.getExpr();

  bool IsSmallImm = IsImm && isInt<13>(ImmValue);
  bool NoLowBitsImm = IsImm && ((ImmValue & 0x3FF) == 0);

  return false;
}

bool RVTinyAsmParser::expandSETX(MCInst &Inst, SMLoc IDLoc,
                                SmallVectorImpl<MCInst> &Instructions) {
  MCOperand MCRegOp = Inst.getOperand(0);
  MCOperand MCValOp = Inst.getOperand(1);
  MCOperand MCTmpOp = Inst.getOperand(2);
  assert(MCRegOp.isReg() && MCTmpOp.isReg());
  assert(MCValOp.isImm() || MCValOp.isExpr());

  // the imm operand can be either an expression or an immediate.
  bool IsImm = MCValOp.isImm();
  int64_t ImmValue = IsImm ? MCValOp.getImm() : 0;

  const MCExpr *ValExpr = IsImm ? MCConstantExpr::create(ImmValue, getContext())
                                : MCValOp.getExpr();

  return false;
}

bool RVTinyAsmParser::matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                             OperandVector &Operands,
                                             MCStreamer &Out,
                                             uint64_t &ErrorInfo,
                                             bool MatchingInlineAsm) {
  MCInst Inst;
  SmallVector<MCInst, 8> Instructions;
  unsigned MatchResult = MatchInstructionImpl(Operands, Inst, ErrorInfo,
                                              MatchingInlineAsm);
  switch (MatchResult) {
  case Match_Success: {
    switch (Inst.getOpcode()) {
    default:
      Inst.setLoc(IDLoc);
      Instructions.push_back(Inst);
      break;
    }

    for (const MCInst &I : Instructions) {
      Out.emitInstruction(I, getSTI());
    }
    return false;
  }

  case Match_MissingFeature:
    return Error(IDLoc,
                 "instruction requires a CPU feature not currently enabled");

  case Match_InvalidOperand: {
    SMLoc ErrorLoc = IDLoc;
    if (ErrorInfo != ~0ULL) {
      if (ErrorInfo >= Operands.size())
        return Error(IDLoc, "too few operands for instruction");

      ErrorLoc = ((RVTinyOperand &)*Operands[ErrorInfo]).getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = IDLoc;
    }

    return Error(ErrorLoc, "invalid operand for instruction");
  }
  case Match_MnemonicFail:
    return Error(IDLoc, "invalid instruction mnemonic");
  }
  llvm_unreachable("Implement any new match types added!");
}

bool RVTinyAsmParser::parseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                   SMLoc &EndLoc) {
  if (!tryParseRegister(Reg, StartLoc, EndLoc).isSuccess())
    return Error(StartLoc, "invalid register name");
  return false;
}

ParseStatus RVTinyAsmParser::tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                             SMLoc &EndLoc) {
  const AsmToken &Tok = Parser.getTok();
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();
  Reg = RVTiny::NoRegister;
  if (getLexer().getKind() != AsmToken::Percent)
    return ParseStatus::NoMatch;
  Parser.Lex();
  unsigned RegKind = RVTinyOperand::rk_None;
  Reg = matchRegisterName(Tok, RegKind);
  if (Reg) {
    Parser.Lex();
    return ParseStatus::Success;
  }

  getLexer().UnLex(Tok);
  return ParseStatus::NoMatch;
}

bool RVTinyAsmParser::parseInstruction(ParseInstructionInfo &Info,
                                      StringRef Name, SMLoc NameLoc,
                                      OperandVector &Operands) {
  // Validate and reject unavailable mnemonics early before
  // running any operand parsing.
  // This is needed because some operands (mainly memory ones)
  // differ between V8 and V9 ISA and so any operand parsing errors
  // will cause IAS to bail out before it reaches matchAndEmitInstruction
  // (where the instruction as a whole, including the mnemonic, is validated
  // once again just before emission).
  // As a nice side effect this also allows us to reject unknown
  // instructions and suggest replacements.
  MatchResultTy MS = mnemonicIsValid(Name, 0);
  switch (MS) {
  case Match_Success:
    break;
  case Match_MissingFeature:
    return Error(NameLoc,
                 "instruction requires a CPU feature not currently enabled");
  case Match_MnemonicFail:
    return Error(NameLoc,
                 "invalid instruction mnemonic" +
                     RVTinyMnemonicSpellCheck(Name, getAvailableFeatures(), 0));
  default:
    llvm_unreachable("invalid return status!");
  }

  // First operand in MCInst is instruction mnemonic.
  Operands.push_back(RVTinyOperand::CreateToken(Name, NameLoc));

  // apply mnemonic aliases, if any, so that we can parse operands correctly.
  //applyMnemonicAliases(Name, getAvailableFeatures(), 0);

  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    // Read the first operand.
    if (getLexer().is(AsmToken::Comma)) {
      if (!parseBranchModifiers(Operands).isSuccess()) {
        SMLoc Loc = getLexer().getLoc();
        return Error(Loc, "unexpected token");
      }
    }
    if (!parseOperand(Operands, Name).isSuccess()) {
      SMLoc Loc = getLexer().getLoc();
      return Error(Loc, "unexpected token");
    }

    while (getLexer().is(AsmToken::Comma) || getLexer().is(AsmToken::Plus)) {
      if (getLexer().is(AsmToken::Plus)) {
      // Plus tokens are significant in software_traps (p83, RVTinyv8.pdf). We must capture them.
        Operands.push_back(RVTinyOperand::CreateToken("+", Parser.getTok().getLoc()));
      }
      Parser.Lex(); // Eat the comma or plus.
      // Parse and remember the operand.
      if (!parseOperand(Operands, Name).isSuccess()) {
        SMLoc Loc = getLexer().getLoc();
        return Error(Loc, "unexpected token");
      }
    }
  }
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    SMLoc Loc = getLexer().getLoc();
    return Error(Loc, "unexpected token");
  }
  Parser.Lex(); // Consume the EndOfStatement.
  return false;
}

ParseStatus RVTinyAsmParser::parseDirective(AsmToken DirectiveID) {
  StringRef IDVal = DirectiveID.getString();

  if (IDVal == ".register") {
    // For now, ignore .register directive.
    Parser.eatToEndOfStatement();
    return ParseStatus::Success;
  }
  if (IDVal == ".proc") {
    // For compatibility, ignore this directive.
    // (It's supposed to be an "optimization" in the Sun assembler)
    Parser.eatToEndOfStatement();
    return ParseStatus::Success;
  }

  // Let the MC layer to handle other directives.
  return ParseStatus::NoMatch;
}

ParseStatus RVTinyAsmParser::parseMEMOperand(OperandVector &Operands) {
  SMLoc S, E;

  std::unique_ptr<RVTinyOperand> LHS;
  if (!parseRVTinyAsmOperand(LHS).isSuccess())
    return ParseStatus::NoMatch;

  return ParseStatus::Success;
}

template <unsigned N>
ParseStatus RVTinyAsmParser::parseShiftAmtImm(OperandVector &Operands) {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);

  // This is a register, not an immediate
  if (getLexer().getKind() == AsmToken::Percent)
    return ParseStatus::NoMatch;

  const MCExpr *Expr;
  if (getParser().parseExpression(Expr))
    return ParseStatus::Failure;

  const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr);
  if (!CE)
    return Error(S, "constant expression expected");

  if (!isUInt<N>(CE->getValue()))
    return Error(S, "immediate shift value out of range");

  Operands.push_back(RVTinyOperand::CreateImm(Expr, S, E));
  return ParseStatus::Success;
}

template <RVTinyAsmParser::TailRelocKind Kind>
ParseStatus RVTinyAsmParser::parseTailRelocSym(OperandVector &Operands) {
  SMLoc S = getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);

  return ParseStatus::Success;
}

ParseStatus RVTinyAsmParser::parseMembarTag(OperandVector &Operands) {
  SMLoc S = Parser.getTok().getLoc();
  const MCExpr *EVal;
  int64_t ImmVal = 0;

  std::unique_ptr<RVTinyOperand> Mask;
  if (parseRVTinyAsmOperand(Mask).isSuccess()) {
    if (!Mask->isImm() || !Mask->getImm()->evaluateAsAbsolute(ImmVal) ||
        ImmVal < 0 || ImmVal > 127)
      return Error(S, "invalid membar mask number");
  }

  while (getLexer().getKind() == AsmToken::Hash) {
    SMLoc TagStart = getLexer().getLoc();
    Parser.Lex(); // Eat the '#'.
    unsigned MaskVal = StringSwitch<unsigned>(Parser.getTok().getString())
      .Case("LoadLoad", 0x1)
      .Case("StoreLoad", 0x2)
      .Case("LoadStore", 0x4)
      .Case("StoreStore", 0x8)
      .Case("Lookaside", 0x10)
      .Case("MemIssue", 0x20)
      .Case("Sync", 0x40)
      .Default(0);

    Parser.Lex(); // Eat the identifier token.

    if (!MaskVal)
      return Error(TagStart, "unknown membar tag");

    ImmVal |= MaskVal;

    if (getLexer().getKind() == AsmToken::Pipe)
      Parser.Lex(); // Eat the '|'.
  }

  EVal = MCConstantExpr::create(ImmVal, getContext());
  SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
  Operands.push_back(RVTinyOperand::CreateImm(EVal, S, E));
  return ParseStatus::Success;
}

ParseStatus RVTinyAsmParser::parseASITag(OperandVector &Operands) {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = Parser.getTok().getEndLoc();
  int64_t ASIVal = 0;

  if (getLexer().getKind() != AsmToken::Hash) {
    // If the ASI tag provided is not a named tag, then it
    // must be a constant expression.
    ParseStatus ParseExprStatus = parseExpression(ASIVal);
    if (!ParseExprStatus.isSuccess())
      return ParseExprStatus;

    if (!isUInt<8>(ASIVal))
      return Error(S, "invalid ASI number, must be between 0 and 255");

    Operands.push_back(RVTinyOperand::CreateASITag(ASIVal, S, E));
    return ParseStatus::Success;
  }

  return ParseStatus::Success;
}

ParseStatus RVTinyAsmParser::parsePrefetchTag(OperandVector &Operands) {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = Parser.getTok().getEndLoc();
  int64_t PrefetchVal = 0;

  if (getLexer().getKind() != AsmToken::Hash) {
    // If the prefetch tag provided is not a named tag, then it
    // must be a constant expression.
    ParseStatus ParseExprStatus = parseExpression(PrefetchVal);
    if (!ParseExprStatus.isSuccess())
      return ParseExprStatus;

    if (!isUInt<8>(PrefetchVal))
      return Error(S, "invalid prefetch number, must be between 0 and 31");

    Operands.push_back(RVTinyOperand::CreatePrefetchTag(PrefetchVal, S, E));
    return ParseStatus::Success;
  }

  SMLoc TagStart = getLexer().peekTok(false).getLoc();
  Parser.Lex(); // Eat the '#'.
  const StringRef PrefetchName = Parser.getTok().getString();
  return ParseStatus::Success;
}

ParseStatus RVTinyAsmParser::parseCallTarget(OperandVector &Operands) {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);

  switch (getLexer().getKind()) {
  default:
    return ParseStatus::NoMatch;
  case AsmToken::LParen:
  case AsmToken::Integer:
  case AsmToken::Identifier:
  case AsmToken::Dot:
    break;
  }

  const MCExpr *DestValue;
  if (getParser().parseExpression(DestValue))
    return ParseStatus::NoMatch;

  bool IsPic = getContext().getObjectFileInfo()->isPositionIndependent();
  return ParseStatus::Success;
}

ParseStatus RVTinyAsmParser::parseOperand(OperandVector &Operands,
                                         StringRef Mnemonic) {

  return ParseStatus::Success;
}

ParseStatus
RVTinyAsmParser::parseRVTinyAsmOperand(std::unique_ptr<RVTinyOperand> &Op,
                                     bool isCall) {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
  const MCExpr *EVal;

  Op = nullptr;
  return Op ? ParseStatus::Success : ParseStatus::Failure;
}

ParseStatus RVTinyAsmParser::parseBranchModifiers(OperandVector &Operands) {
  // parse (,a|,pn|,pt)+

  while (getLexer().is(AsmToken::Comma)) {
    Parser.Lex(); // Eat the comma

    if (!getLexer().is(AsmToken::Identifier))
      return ParseStatus::Failure;
    StringRef modName = Parser.getTok().getString();
    if (modName == "a" || modName == "pn" || modName == "pt") {
      Operands.push_back(RVTinyOperand::CreateToken(modName,
                                                   Parser.getTok().getLoc()));
      Parser.Lex(); // eat the identifier.
    }
  }
  return ParseStatus::Success;
}

ParseStatus RVTinyAsmParser::parseExpression(int64_t &Val) {
  AsmToken Tok = getLexer().getTok();

  if (!isPossibleExpression(Tok))
    return ParseStatus::NoMatch;

  return getParser().parseAbsoluteExpression(Val);
}

MCRegister RVTinyAsmParser::matchRegisterName(const AsmToken &Tok,
                                             unsigned &RegKind) {
  return false;
}

const RVTinyMCExpr *
RVTinyAsmParser::adjustPICRelocation(RVTinyMCExpr::VariantKind VK,
                                    const MCExpr *subExpr) {
  // When in PIC mode, "%lo(...)" and "%hi(...)" behave differently.
  // If the expression refers contains _GLOBAL_OFFSET_TABLE, it is
  // actually a %pc10 or %pc22 relocation. Otherwise, they are interpreted
  // as %got10 or %got22 relocation.

  return nullptr;
}

bool RVTinyAsmParser::matchRVTinyAsmModifiers(const MCExpr *&EVal,
                                            SMLoc &EndLoc) {
  AsmToken Tok = Parser.getTok();
  if (!Tok.is(AsmToken::Identifier))
    return false;

  StringRef name = Tok.getString();

  Parser.Lex(); // Eat the identifier.
  if (Parser.getTok().getKind() != AsmToken::LParen)
    return false;

  Parser.Lex(); // Eat the LParen token.
  const MCExpr *subExpr;
  if (Parser.parseParenExpression(subExpr, EndLoc))
    return false;

  //EVal = adjustPICRelocation(VK, subExpr);
  return true;
}

bool RVTinyAsmParser::isPossibleExpression(const AsmToken &Token) {
  switch (Token.getKind()) {
  case AsmToken::LParen:
  case AsmToken::Integer:
  case AsmToken::Identifier:
  case AsmToken::Plus:
  case AsmToken::Minus:
  case AsmToken::Tilde:
    return true;
  default:
    return false;
  }
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRVTinyAsmParser() {
  RegisterMCAsmParser<RVTinyAsmParser> A(getTheRVTinyTarget());
}

unsigned RVTinyAsmParser::validateTargetOperandClass(MCParsedAsmOperand &GOp,
                                                    unsigned Kind) {
  RVTinyOperand &Op = (RVTinyOperand &)GOp;
  if (Op.isFloatOrDoubleReg()) {
  }
  return Match_InvalidOperand;
}
