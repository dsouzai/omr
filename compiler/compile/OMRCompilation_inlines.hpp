#ifndef OMR_COMPILATION_INLINE_INCL
#define OMR_COMPILATION_INLINE_INCL

#include "compile/OMRCompilation.hpp"

TR::Compilation *
OMR::Compilation::self()
   {
   return static_cast<TR::Compilation *>(this);
   }

vcount_t
OMR::Compilation::getVisitCount()
   {
   return _visitCount;
   }

vcount_t
OMR::Compilation:: setVisitCount(vcount_t vc)
   {
   return (_visitCount = vc);
   }

#endif
