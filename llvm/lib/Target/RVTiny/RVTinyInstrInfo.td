
include "RVTinyInstrFormats.td"

def ADDrr : R_Inst<0, 0,
                   (outs IntRegs:$rd), (ins IntRegs:$rs1, IntRegs:$rs2),
                   "add $rd, $rs1, $rs2",
                   [(set i32:$rd, (add i32:$rs1, i32:$rs2))]>;


def : Pat<(add i32:$lhs, i32:$rhs), (ADDrr $lhs, $rhs)>;