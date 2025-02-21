
#include "RVTiny.h"
#include "RVTinyTargetMachine.h"
#include "RVTinyMachineFunctionInfo.h"
#include "RVTinyTargetObjectFile.h"
//#include "TargetInfo/RVTinyTargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include <optional>
using namespace llvm;

Target &llvm::getTheRVTinyTarget() {
  static Target TheRVTinyTarget;
  return TheRVTinyTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRVTinyTargetInfo() {
  RegisterTarget<Triple::rvtiny, /*HasJIT=*/false> X(getTheRVTinyTarget(),
                                                    "rvtiny", "RVTiny", "RVTiny");
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRVTinyTarget() {
  // Register the target.
  RegisterTargetMachine<RVTiny32ITargetMachine> X(getTheRVTinyTarget());

  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeRVTinyDAGToDAGISelLegacyPass(PR);
}

static std::string computeDataLayout(const Triple &T, bool is64Bit) {
  // RVTiny is typically big endian, but some are little.
  std::string Ret = "e";
  Ret += "-m:e";

  // Some ABIs have 32bit pointers.
  if (!is64Bit)
    Ret += "-p:32:32";

  // Alignments for 64 bit integers.
  Ret += "-i64:64";

  if (is64Bit)
    Ret += "-S128";
  else
    Ret += "-S64";

  return Ret;
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

// Code models. Some only make sense for 64-bit code.
//
// SunCC  Reloc   CodeModel  Constraints
// abs32  Static  Small      text+data+bss linked below 2^32 bytes
// abs44  Static  Medium     text+data+bss linked below 2^44 bytes
// abs64  Static  Large      text smaller than 2^31 bytes
// pic13  PIC_    Small      GOT < 2^13 bytes
// pic32  PIC_    Medium     GOT < 2^32 bytes
//
// All code models require that the text segment is smaller than 2GB.
static CodeModel::Model
getEffectiveSparcCodeModel(std::optional<CodeModel::Model> CM, Reloc::Model RM,
                           bool Is64Bit, bool JIT) {
  if (CM) {
    if (*CM == CodeModel::Tiny)
      report_fatal_error("Target does not support the tiny CodeModel", false);
    if (*CM == CodeModel::Kernel)
      report_fatal_error("Target does not support the kernel CodeModel", false);
    return *CM;
  }
  if (Is64Bit) {
    if (JIT)
      return CodeModel::Large;
    return RM == Reloc::PIC_ ? CodeModel::Small : CodeModel::Medium;
  }
  return CodeModel::Small;
}

RVTinyTargetMachine::RVTinyTargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT,
                                         bool is64bit)
    : CodeGenTargetMachineImpl(
          T, computeDataLayout(TT, is64bit), TT, CPU, FS, Options,
          getEffectiveRelocModel(RM),
          getEffectiveSparcCodeModel(CM, getEffectiveRelocModel(RM), is64bit,JIT),
          OL),
      TLOF(std::make_unique<RVTinyELFTargetObjectFile>()), is64Bit(is64bit) {
  initAsmInfo();
}

RVTinyTargetMachine::~RVTinyTargetMachine() = default;

void RVTinyTargetMachine::anchor() { }

const RVTinySubtarget *
RVTinyTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute TuneAttr = F.getFnAttribute("tune-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU =
      CPUAttr.isValid() ? CPUAttr.getValueAsString().str() : TargetCPU;
  std::string TuneCPU =
      TuneAttr.isValid() ? TuneAttr.getValueAsString().str() : CPU;
  std::string FS =
      FSAttr.isValid() ? FSAttr.getValueAsString().str() : TargetFS;

  //if (softFloat)
  //  FS += FS.empty() ? "+soft-float" : ",+soft-float";

  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<RVTinySubtarget>(CPU, TuneCPU, FS, *this,
                                         this->is64Bit);
  }
  return I.get();
}

MachineFunctionInfo *RVTinyTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return RVTinyMachineFunctionInfo::create<RVTinyMachineFunctionInfo>(Allocator,
                                                                      F, STI);
}

namespace {
    /// RVTiny Code Generator Pass Configuration Options.
    class RVTinyPassConfig : public TargetPassConfig {
    public:
      RVTinyPassConfig(RVTinyTargetMachine &TM, PassManagerBase &PM)
        : TargetPassConfig(TM, PM) {}
    
      RVTinyTargetMachine &getRVTinyTargetMachine() const {
        return getTM<RVTinyTargetMachine>();
      }
    
      void addIRPasses() override;
      bool addInstSelector() override;
      void addPreEmitPass() override;
    };
} // namespace


TargetPassConfig *RVTinyTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new RVTinyPassConfig(*this, PM);
}

void RVTinyPassConfig::addIRPasses() {
  addPass(createAtomicExpandLegacyPass());

  TargetPassConfig::addIRPasses();
}

bool RVTinyPassConfig::addInstSelector() {
  addPass(createRVTinyISelDag(getRVTinyTargetMachine()));
  return false;
}

void RVTinyPassConfig::addPreEmitPass(){
}


void RVTiny32ITargetMachine::anchor() { }

RVTiny32ITargetMachine::RVTiny32ITargetMachine(const Target &T, const Triple &TT,
                                              StringRef CPU, StringRef FS,
                                              const TargetOptions &Options,
                                              std::optional<Reloc::Model> RM,
                                              std::optional<CodeModel::Model> CM,
                                              CodeGenOptLevel OL, bool JIT)
    : RVTinyTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT, false) {}