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

#include <stddef.h>                                 // for NULL
#include <stdint.h>                                 // for int32_t
#include "compile/Compilation.hpp"
#include "compile/Compilation_inlines.hpp"                  // for Compilation
#include "env/TRMemory.hpp"                         // for TR_Memory
#include "il/Block.hpp"                             // for Block
#include "il/Node.hpp"                              // for Node, vcount_t
#include "il/TreeTop.hpp"                           // for TreeTop
#include "il/TreeTop_inlines.hpp"
#include "infra/BitVector.hpp"                      // for TR_BitVector
#include "optimizer/DataFlowAnalysis.hpp"           // for TR_Delayedness, etc
#include "optimizer/LocalAnalysis.hpp"
#include "optimizer/Structure.hpp"

namespace TR { class Optimizer; }

// #define MAX_BLOCKS_FOR_STACK_ALLOCATION 16

// This file contains an implementation of Delayedness which
// is the third global bit vector analyses used by PRE. Delayedness
// attempts to discover for each expression, all the points
// that might possibly be optimal. The final choice of the optimal
// computation point(s) for an expression is made using the set of
// prospective points returned by Delayedness. Basically any point
// beginning at the earliest optimal point and sweeping downwards
// over the CFG that also precedes any computation
// of the expression in the original program is a possible optimal
// point returned by Delayedness.
//
//
TR_DataFlowAnalysis::Kind TR_Delayedness::getKind()
   {
   return Delayedness;
   }

TR_Delayedness *TR_Delayedness::asDelayedness()
   {
   return this;
   }


int32_t TR_Delayedness::getNumberOfBits()
   {
   return _earliestness->_numberOfBits;
   }


void TR_Delayedness::analyzeNode(TR::Node *, vcount_t, TR_BlockStructure *, ContainerType *)
   {
   }


TR_Delayedness::TR_Delayedness(TR::Compilation *comp, TR::Optimizer *optimizer, TR_Structure *rootStructure, bool trace)
   : TR_IntersectionBitVectorAnalysis(comp, comp->getFlowGraph(), optimizer, trace)
   {

   _earliestness = new (comp->allocator()) TR_Earliestness(comp, optimizer, rootStructure, trace);

   if (trace)
      traceMsg(comp, "Starting Delayedness\n");

   _supportedNodesAsArray = _earliestness->_supportedNodesAsArray;
   _temp = NULL;

   performAnalysis(rootStructure, false);

   if (trace)
      {
      int32_t i;
      for (i = 0; i < _numberOfNodes; i++)
         {
         traceMsg(comp, "Block number : %d has solution : ", i);
         _inSetInfo[i]->print(comp);
         traceMsg(comp, "\n");
         }

      traceMsg(comp, "\nEnding Delayedness\n");
      }

   // Null out info that will not be used by callers
   _earliestness->_globalAnticipatability->_blockAnalysisInfo = NULL;
   _earliestness->_inSetInfo = NULL;
   _blockAnalysisInfo = NULL;
   }

bool TR_Delayedness::postInitializationProcessing()
   {
   _inSetInfo = (ContainerType **)trMemory()->allocateStackMemory(_numberOfNodes*sizeof(ContainerType *));

   int32_t i;
   for (i = 0; i < _numberOfNodes; i++)
      allocateContainer(_inSetInfo+i);
   return true;
   }




// Overrides the implementation in the superclass as this analysis
// is slightly different from conventional bit vector analyses.
// It uses the results from local analyses instead of examining
// each tree top for effects on the input bit vector at that tree top.
// This analysis has a trivial analyzeNode(...) method as a result.
//
//
void TR_Delayedness::analyzeTreeTopsInBlockStructure(TR_BlockStructure *blockStructure)
   {
   if (trace())
       {
       /////traceMsg(comp(), "\ncurrentInSetInfo when entering Block : %d\n", blockStructure->getNumber());
       /////_currentInSetInfo->print(_compilation);
       /////traceMsg(comp(), "\nOut Set of Block : %d\n", blockStructure->getNumber());
       /////_blockAnalysisInfo[blockStructure->getNumber()]->print(_compilation->getOutFile());
       }

   // Block info is local to this analysis, so allocate from there
   if (_temp == NULL)
      allocateBlockInfoContainer(&_temp);
   copyFromInto(_earliestness->_globalAnticipatability->_blockAnalysisInfo[blockStructure->getNumber()], _temp);
   *_temp &= *(_earliestness->_inSetInfo[blockStructure->getNumber()]);
   *_currentInSetInfo |= *_temp;

   copyFromInto(_currentInSetInfo, _inSetInfo[blockStructure->getNumber()]);
   copyFromInto(_currentInSetInfo, _blockAnalysisInfo[blockStructure->getNumber()]);

   _temp->setAll(_numberOfBits);
   *_temp -= *(_earliestness->_globalAnticipatability->_localAnticipatability.getDownwardExposedAnalysisInfo(blockStructure->getBlock()->getNumber()));
   *(_blockAnalysisInfo[blockStructure->getNumber()]) &= *_temp;
   copyFromInto(_blockAnalysisInfo[blockStructure->getNumber()], _regularInfo);

   if (trace())
       {
       /////traceMsg(comp(), "\nIn Set of Block : %d\n", blockStructure->getNumber());
       /////_inSetInfo[blockStructure->getNumber()]->print(_compilation->getOutFile());
       /////traceMsg(comp(), "\nOut Set of Block : %d\n", blockStructure->getNumber());
       /////_blockAnalysisInfo[blockStructure->getNumber()]->print(_compilation->getOutFile());
       }

   TR::Block *block = blockStructure->getBlock();
   TR::TreeTop *currentTree = block->getEntry();
   TR::TreeTop *exitTree = block->getExit();
   bool notSeenTreeWithChecks = true;

   _containsExceptionTreeTop = false;
   while (!(currentTree == exitTree))
      {
      if (notSeenTreeWithChecks)
         {
         bool currentTreeHasChecks = treeHasChecks(currentTree);
         if (currentTreeHasChecks)
            {
            notSeenTreeWithChecks = false;
            _containsExceptionTreeTop = true;
            copyFromInto(_blockAnalysisInfo[blockStructure->getNumber()], _exceptionInfo);
            }
         }
      else
         break;

      if (!(currentTree == exitTree))
         currentTree = currentTree->getNextTreeTop();
      }

   getAnalysisInfo(blockStructure)->_containsExceptionTreeTop = _containsExceptionTreeTop;
   }
