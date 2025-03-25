

include "llvm/Target/Target.td"


//def FeatureM
//  : SubtargetFeature<"M", "hasMul", "true",
//                     "Enable Mul instructions">;

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "RVTinyRegisterInfo.td"
include "RVTinyCallingConv.td"
//include "RVTinySchedule.td"
include "RVTinyInstrInfo.td"

def RVTinyInstrInfo : InstrInfo;

def RVTinyAsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = true;
  let AllowDuplicateRegisterNames = true;
}

def RVTinyAsmParserVariant : AsmParserVariant {
  let RegisterPrefix = "%";
}

//===

class Proc<string Name, list<SubtargetFeature> Features,
           list<SubtargetFeature> TuneFeatures = []>
 : Processor<Name, NoItineraries, Features, TuneFeatures>;

def : Proc<"rv32i", []>;
//def : Proc<"rv32im", [FeatureM]>;

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def RVTinyAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  int PassSubtarget = 1;
  int Variant = 0;
}

def RVTiny : Target {
  // Pull in Instruction Info:
  let InstructionSet = RVTinyInstrInfo;
  let AssemblyParsers  = [RVTinyAsmParser];
  let AssemblyParserVariants = [RVTinyAsmParserVariant];
  let AssemblyWriters = [RVTinyAsmWriter];
  let AllowRegisterRenaming = 1;
}
