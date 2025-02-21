

include "llvm/Target/Target.td"


//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "RVTinyRegisterInfo.td"
include "RVTinyCallingConv.td"
//include "RVTinySchedule.td"
include "RVTinyInstrInfo.td"

def RVTinyInstrInfo : InstrInfo;


class Proc<string Name, list<SubtargetFeature> Features,
           list<SubtargetFeature> TuneFeatures = []>
 : Processor<Name, NoItineraries, Features, TuneFeatures>;

def : Proc<"rv32i", []>;

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def RVTiny : Target {
  // Pull in Instruction Info:
  let InstructionSet = RVTinyInstrInfo;
  //let AssemblyParsers  = [RVTinyAsmParser];
  //let AssemblyParserVariants = [RVTinyAsmParserVariant];
  //let AssemblyWriters = [RVTinyAsmWriter];
  let AllowRegisterRenaming = 1;
}
