
#include "RVTinyISelLowering.h"
//#include "MCTargetDesc/RVTinyMCExpr.h"
//#include "MCTargetDesc/RVTinyMCTargetDesc.h"
//#include "RVTinyMachineFunctionInfo.h"
#include "RVTinyRegisterInfo.h"
#include "RVTinyTargetMachine.h"
//#include "RVTinyTargetObjectFile.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/KnownBits.h"
using namespace llvm;


RVTinyTargetLowering::RVTinyTargetLowering(const TargetMachine &TM,
                                           const RVTinySubtarget &STI)
    : TargetLowering(TM), Subtarget(&STI) {
  MVT PtrVT = MVT::getIntegerVT(TM.getPointerSizeInBits(0));

  addRegisterClass(MVT::i32, &RVTiny::IntRegsRegClass);

}

SDValue
RVTinyTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {

  return Op;
}

SDValue RVTinyTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  return Chain;
}


SDValue
RVTinyTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                               SmallVectorImpl<SDValue> &InVals) const {
  return CLI.Chain;
}


SDValue
RVTinyTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                  bool IsVarArg,
                                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                                  const SmallVectorImpl<SDValue> &OutVals,
                                  const SDLoc &DL, SelectionDAG &DAG) const {
  return Chain;
}


SDValue RVTinyTargetLowering::PerformDAGCombine(SDNode *N,
                                                DAGCombinerInfo &DCI) const {
  switch (N->getOpcode()) {
  default:
    break;
  }
  return SDValue();
}