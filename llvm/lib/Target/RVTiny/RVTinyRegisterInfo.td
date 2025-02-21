
class RVTinyReg<string n> : Register<n> {
  let Namespace = "RVTiny";
}

// Ri - 32-bit integer registers
class Ri<bits<5> num, string n> :
RVTinyReg<n> {
  let HWEncoding{4-0} = num;
}

def X0 : Ri< 0, "x0">, DwarfRegNum<[0]> {
  let isConstant = true;
}
def X1 : Ri< 1, "x1">, DwarfRegNum<[1]>;
def X2 : Ri< 2, "x2">, DwarfRegNum<[2]>;
def X3 : Ri< 3, "x3">, DwarfRegNum<[3]>;
def X4 : Ri< 4, "x4">, DwarfRegNum<[4]>;
def X5 : Ri< 5, "x5">, DwarfRegNum<[5]>;
def X6 : Ri< 6, "x6">, DwarfRegNum<[6]>;
def X7 : Ri< 7, "x7">, DwarfRegNum<[7]>;
def X8 : Ri< 8, "x8">, DwarfRegNum<[8]>;
def X9 : Ri< 9, "x9">, DwarfRegNum<[9]>;
def X10 : Ri< 10, "x10">, DwarfRegNum<[10]>;
def X11 : Ri< 11, "x11">, DwarfRegNum<[11]>;
def X12 : Ri< 12, "x12">, DwarfRegNum<[12]>;
def X13 : Ri< 13, "x13">, DwarfRegNum<[13]>;
def X14 : Ri< 14, "x14">, DwarfRegNum<[14]>;
def X15 : Ri< 15, "x15">, DwarfRegNum<[15]>;
def X16 : Ri< 16, "x16">, DwarfRegNum<[16]>;
def X17 : Ri< 17, "x17">, DwarfRegNum<[17]>;
def X18 : Ri< 18, "x18">, DwarfRegNum<[18]>;
def X19 : Ri< 19, "x19">, DwarfRegNum<[19]>;
def X20 : Ri< 20, "x20">, DwarfRegNum<[20]>;
def X21 : Ri< 21, "x21">, DwarfRegNum<[21]>;
def X22 : Ri< 22, "x22">, DwarfRegNum<[22]>;
def X23 : Ri< 23, "x23">, DwarfRegNum<[23]>;
def X24 : Ri< 24, "x24">, DwarfRegNum<[24]>;
def X25 : Ri< 25, "x25">, DwarfRegNum<[25]>;
def X26 : Ri< 26, "x26">, DwarfRegNum<[26]>;
def X27 : Ri< 27, "x27">, DwarfRegNum<[27]>;
def X28 : Ri< 28, "x28">, DwarfRegNum<[28]>;
def X29 : Ri< 29, "x29">, DwarfRegNum<[29]>;
def X30 : Ri< 30, "x30">, DwarfRegNum<[30]>;
def X31 : Ri< 31, "x31">, DwarfRegNum<[31]>;

// Integer register class
def IntRegs : RegisterClass<"RVTiny", [i32], 32, (sequence "X%u", 0, 31)>;

