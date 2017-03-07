#ifndef OMR_CODEGENERATOR_INLINE_INCL
#define OMR_CODEGENERATOR_INLINE_INCL

#include "codegen/OMRCodeGenerator.hpp"

TR::CodeGenerator*
OMR::CodeGenerator::self()
   {
   return static_cast<TR::CodeGenerator*>(this);
   }

#endif
