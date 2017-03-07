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

#include <stdint.h>                                 // for int32_t
#include "codegen/CodeGenerator.hpp"
#include "codegen/CodeGenerator_inlines.hpp"                // for CodeGenerator
#include "compile/Compilation.hpp"                  // for Compilation
#include "control/Options.hpp"
#include "control/Options_inlines.hpp"
#include "env/TRMemory.hpp"
#include "il/Block.hpp"                             // for Block
#include "il/Node.hpp"                              // for Node, vcount_t
#include "infra/BitVector.hpp"                      // for TR_BitVector
#include "optimizer/Structure.hpp"
#include "optimizer/DataFlowAnalysis.hpp"

namespace TR { class Optimizer; }

#define MAX_REGISTERS 32

// Methods to implement the forward data flow analysis required for
// computing the register availability. The results of the analysis
// will be used to compute optimal save points for callee-preserved registers
// together with the register anticipatability results
//
TR_DataFlowAnalysis::Kind TR_RegisterAvailability::getKind()
   {
   return RegisterAvailability;
   }

TR_RegisterAvailability *TR_RegisterAvailability::asRegisterAvailability()
   {
   return this;
   }

int32_t TR_RegisterAvailability::getNumberOfBits()
   {
   return TR::RealRegister::NumRegisters;
   }


void TR_RegisterAvailability::analyzeNode(TR::Node *, vcount_t, TR_BlockStructure *, TR_BitVector *)
   {
   }

void TR_RegisterAvailability::initializeRegisterUsageInfo()
   {
   // initialize the register usage info and the inSets bitvector
   // to hold the results of the data-flow
   //
   TR_BitVector **originalRegUsageInfo = _registerUsageInfo;
   _registerUsageInfo = (TR_BitVector **) trMemory()->allocateStackMemory(_numberOfNodes * sizeof(TR_BitVector *));
   _inSetInfo = (TR_BitVector **) trMemory()->allocateStackMemory(_numberOfNodes * sizeof(TR_BitVector *));

   for (int32_t i = 0; i < _numberOfNodes; i++)
      {
      _registerUsageInfo[i] = new (trStackMemory()) TR_BitVector(_numberOfBits, trMemory(), stackAlloc);
      copyFromInto(originalRegUsageInfo[i], _registerUsageInfo[i]);
      _inSetInfo[i] = new (trStackMemory()) TR_BitVector(_numberOfBits, trMemory(), stackAlloc);
      _inSetInfo[i]->empty();
      }

   }

TR_RegisterAvailability::TR_RegisterAvailability(TR::Compilation *comp,
                                                 TR::Optimizer *optimizer,
                                                 TR_Structure *rootStructure,
                                                 TR_BitVector **regUsageInfo,
                                                 bool trace)
   : TR_IntersectionBitVectorAnalysis(comp, comp->getFlowGraph(), optimizer, trace)
   {
   if (comp->getOption(TR_TraceShrinkWrapping))
      traceMsg(comp, "Starting RegisterAvailability\n");

   _registerUsageInfo = regUsageInfo;   // Will be copied in initializeRegisterUsageInfo

   performAnalysis(rootStructure, false);

   if (comp->getOption(TR_TraceShrinkWrapping))
      {
      for (int32_t i = 0; i < _numberOfNodes; i++)
         {
         traceMsg(comp, "Block number : %d has solution : ", i);
         _blockAnalysisInfo[i]->print(comp);
         traceMsg(comp, "\n");
         }

      for (int32_t i = 0; i < _numberOfNodes; i++)
         {
         traceMsg(comp, "Block number : %d has inSet : ", i);
         _inSetInfo[i]->print(comp);
         traceMsg(comp, "\n");
         }

      traceMsg(comp, "Ending RegisterAvailability\n");
      }
   }

bool TR_RegisterAvailability::postInitializationProcessing()
   {
   // initialize the RUSE vectors for each block
   //
   initializeRegisterUsageInfo();
   return true;
   }

void TR_RegisterAvailability::analyzeBlockZeroStructure(TR_BlockStructure *blockStructure)
   {
   // initialize the analysis info by zeroing out the initial inSet for availability
   // since no register is available on method entry
   //
   copyFromInto(_originalInSetInfo, _regularInfo);
   copyFromInto(_originalInSetInfo, _exceptionInfo);
   }

void TR_RegisterAvailability::analyzeTreeTopsInBlockStructure(TR_BlockStructure *blockStructure)
   {
   // compute the _outSet = _inSet + RUSE
   //
   int32_t blockNum = blockStructure->getBlock()->getNumber();

   /*
   traceMsg(comp(), "_currentInSet for block_%d: ", blockNum);
   _currentInSetInfo->print(comp());
   traceMsg(comp(), "\n");
   */

   // store the results of the _inSet for later use
   //
   copyFromInto(_currentInSetInfo, _inSetInfo[blockNum]);

   /*
   traceMsg(comp(), "reguseinfo for block_%d: ", blockNum);
   _registerUsageInfo[blockNum]->print(comp());
   traceMsg(comp(), "\n");

   traceMsg(comp(), "_regularInfo block_%d: ", blockNum);
   _regularInfo->print(comp());
   traceMsg(comp(), "\n");
   */

   *_regularInfo |= *_registerUsageInfo[blockNum];
   *_exceptionInfo |= *_registerUsageInfo[blockNum];

   if (comp()->getOption(TR_TraceShrinkWrapping))
      {
      traceMsg(comp(), "Normal info of block_%d : ", blockNum);
      _regularInfo->print(comp());
      traceMsg(comp(), "\n");
      }
   copyFromInto(_regularInfo, _blockAnalysisInfo[blockNum]);
   }
