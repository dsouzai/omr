#ifndef OMR_COMPILATION_INLINE_INCL
#define OMR_COMPILATION_INLINE_INCL

#include "compile/OMRCompilation.hpp"

TR::Compilation *
OMR::Compilation::self()
   {
   return static_cast<TR::Compilation *>(this);
   }

#endif
