
#ifndef LLVM_LIB_TARGET_RVTINY_MCTARGETDESC_RVTINYMCTARGETDESC_H
#define LLVM_LIB_TARGET_RVTINY_MCTARGETDESC_RVTINYMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"
#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;

MCCodeEmitter *createRVTinyMCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx);

MCAsmBackend *createRVTinyAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                     const MCRegisterInfo &MRI,
                                     const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter> createRVTinyObjectTargetWriter();
} // namespace llvm

// Defines symbolic names for RVTiny registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "RVTinyGenRegisterInfo.inc"

// Defines symbolic names for the RVTiny instructions.
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "RVTinyGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "RVTinyGenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_RVTINY_MCTARGETDESC_RVTINYMCTARGETDESC_H
