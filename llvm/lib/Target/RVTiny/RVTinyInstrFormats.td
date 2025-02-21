
class RVInst<dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin = NoItinerary> : Instruction {
  field bits<32> Inst;
  let Namespace = "RVTiny";
  let Size = 4;

  bits<7> opcode;
  let Inst{6-0} = opcode;
  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;

  let DecoderNamespace = "RVTiny";

  let Itinerary = itin;
}

class R_Type<dag outs, dag ins, string asmstr, list<dag> pattern>
    : RVInst<outs, ins, asmstr, pattern> {
  bits<5> rd;
  bits<3> funct3;
  bits<5> rs1;
  bits<5> rs2;
  bits<7> funct7;
  let Inst{11-7} = rd;
  let Inst{14-12} = funct3;
  let Inst{19-15} = rs1;
  let Inst{24-20} = rs2;
  let Inst{31-25} = funct7;
}

class R_Inst<bits<3> f3, bits<7> f7, dag outs, dag ins, string asmstr, list<dag> pattern>
    : R_Type<outs, ins, asmstr, pattern> {
  let opcode = 0b0110011;
  let funct3 = f3;
  let funct7 = f7;
}