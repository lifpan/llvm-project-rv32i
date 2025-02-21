
def CC_RVTiny : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[X1, X2, X3, X4, X5, X6, ]>>,
  CCAssignToStack<4, 4>
]>;


def RetCC_RVTiny : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[X1]>>
]>;



