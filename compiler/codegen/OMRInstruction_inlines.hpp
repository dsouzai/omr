#ifndef OMR_INSTRUCTION_INLINE_INCL
#define OMR_INSTRUCTION_INLINE_INCL

#include "codegen/OMRInstruction.hpp"

TR::Instruction *
OMR::Instruction::self()
   {
   return static_cast<TR::Instruction *>(this);
   }

#endif

