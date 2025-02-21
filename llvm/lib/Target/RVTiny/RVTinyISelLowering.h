
#ifndef LLVM_LIB_TARGET_RVTINY_RVTINYISELLOWERING_H
#define LLVM_LIB_TARGET_RVTINY_RVTINYISELLOWERING_H

#include "RVTiny.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
  class RVTinySubtarget;

  class RVTinyTargetLowering : public TargetLowering {
    const RVTinySubtarget *Subtarget;
  public:
    RVTinyTargetLowering(const TargetMachine &TM, const RVTinySubtarget &STI);
    SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

    SDValue
    LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                         const SmallVectorImpl<ISD::InputArg> &Ins,
                         const SDLoc &dl, SelectionDAG &DAG,
                         SmallVectorImpl<SDValue> &InVals) const override;


    SDValue
      LowerCall(TargetLowering::CallLoweringInfo &CLI,
                SmallVectorImpl<SDValue> &InVals) const override;

    SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                        const SmallVectorImpl<ISD::OutputArg> &Outs,
                        const SmallVectorImpl<SDValue> &OutVals,
                        const SDLoc &dl, SelectionDAG &DAG) const override;


    SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;



  };

} // namespace llvm

#endif