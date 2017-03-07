/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2000, 2016
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 *******************************************************************************/

#ifndef BACKINGSTORE_INCL
#define BACKINGSTORE_INCL

#include <stdint.h>                    // for int32_t, uint8_t, int8_t
#include "compile/Compilation.hpp"
#include "compile/Compilation_inlines.hpp"     // for Compilation, comp
#include "env/TRMemory.hpp"            // for TR_Memory, etc
#include "il/SymbolReference.hpp"      // for SymbolReference
#include "infra/Flags.hpp"             // for flags8_t

namespace TR { class Symbol; }
namespace TR { class SymbolReferenceTable; }

class TR_BackingStore
   {

public:

   TR_ALLOC(TR_Memory::BackingStore)

   TR_BackingStore(TR::SymbolReferenceTable *symRefTab) :
      _flags(0),
      _maxSpillDepth(-1)
       {
       _symRef = new (TR::comp()->trHeapMemory()) TR::SymbolReference(symRefTab);
       }
   TR_BackingStore(TR::SymbolReferenceTable *symRefTab, TR::Symbol *s, int32_t o) :
      _flags(0),
      _maxSpillDepth(-1)
       {
       _symRef = new (TR::comp()->trHeapMemory()) TR::SymbolReference(symRefTab,s,o);
       }
   TR_BackingStore(TR::SymbolReference *symRef) :
      _symRef(symRef),
      _flags(0),
      _maxSpillDepth(-1) {}

   TR::SymbolReference *getSymbolReference() { return _symRef; }

   bool isEmpty()       { return _flags.testValue(OccupiedMask, 0); }
   void setIsEmpty()    { _flags.setValue(OccupiedMask, 0); }
   bool isOccupied()    { return _flags.testAny(OccupiedMask); }
   void setIsOccupied() { _flags.set(OccupiedMask); }

   bool firstHalfIsOccupied()     { return _flags.testAny(FirstHalfIsOccupied); }
   void setFirstHalfIsOccupied()  { _flags.set(FirstHalfIsOccupied); }
   void setFirstHalfIsEmpty()     { _flags.reset(FirstHalfIsOccupied); }
   bool secondHalfIsOccupied()    { return _flags.testAny(SecondHalfIsOccupied); }
   void setSecondHalfIsOccupied() { _flags.set(SecondHalfIsOccupied); }
   void setSecondHalfIsEmpty()    { _flags.reset(SecondHalfIsOccupied); }

   bool containsCollectedReference()          { return _flags.testAny(ContainsCollectedReference); }
   void setContainsCollectedReference(bool b) { _flags.set(ContainsCollectedReference, b); }

   int32_t setMaxSpillDepth(int32_t d)    { return _maxSpillDepth = d; }
   int32_t getMaxSpillDepth()             { return _maxSpillDepth; }

private:

   enum
      {
      OccupiedMask               = 0x03,
      FirstHalfIsOccupied        = 0x01,
      SecondHalfIsOccupied       = 0x02,

      ContainsCollectedReference = 0x04,
      };

   TR::SymbolReference *_symRef;

   int32_t  _maxSpillDepth;
   flags8_t _flags;

   };

#endif
