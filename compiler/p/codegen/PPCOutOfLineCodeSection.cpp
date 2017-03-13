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

#include "p/codegen/PPCOutOfLineCodeSection.hpp"

#include <stddef.h>                            // for NULL
#include "codegen/CodeGenerator.hpp"
#include "codegen/CodeGenerator_inlines.hpp"           // for CodeGenerator
#include "codegen/Instruction.hpp"
#include "codegen/Instruction_inlines.hpp"             // for Instruction, etc
#include "codegen/Machine.hpp"
#include "codegen/Machine_inlines.hpp"                 // for Machine
#include "codegen/TreeEvaluator.hpp"           // for TreeEvaluator
#include "compile/Compilation.hpp"
#include "compile/Compilation_inlines.hpp"             // for Compilation
#include "env/TRMemory.hpp"
#include "il/ILOps.hpp"                        // for ILOpCode
#include "il/Node.hpp"                         // for Node
#include "il/symbol/LabelSymbol.hpp"           // for LabelSymbol
#include "infra/Assert.hpp"                    // for TR_ASSERT
#include "p/codegen/GenerateInstructions.hpp"
#include "p/codegen/PPCInstruction.hpp"

namespace TR { class RegisterDependencyConditions; }

TR_PPCOutOfLineCodeSection::TR_PPCOutOfLineCodeSection(TR::Node  *callNode,
                            TR::ILOpCodes      callOp,
                            TR::Register      *targetReg,
                            TR::LabelSymbol    *entryLabel,
                            TR::LabelSymbol    *restartLabel,
                            TR::CodeGenerator *cg) :
                            TR_OutOfLineCodeSection(callNode,callOp,targetReg,entryLabel,restartLabel,cg)
   {
   generatePPCOutOfLineCodeSectionDispatch();
   }

void TR_PPCOutOfLineCodeSection::generatePPCOutOfLineCodeSectionDispatch()
   {
   // Switch to cold helper instruction stream.
   //
   swapInstructionListsWithCompilation();

   new (_cg->trHeapMemory()) TR::PPCLabelInstruction(TR::InstOpCode::label, _callNode, _entryLabel, _cg);

   TR::Register *resultReg = NULL;
   if (_callNode->getOpCode().isCallIndirect())
      resultReg = TR::TreeEvaluator::performCall(_callNode, true, _cg);
   else
      resultReg = TR::TreeEvaluator::performCall(_callNode, false, _cg);

   if (_targetReg)
      {
      TR_ASSERT(resultReg, "assertion failure");
      generateTrg1Src1Instruction(_cg, TR::InstOpCode::mr, _callNode, _targetReg, resultReg);
      }
   _cg->decReferenceCount(_callNode);

   if (_restartLabel)
      generateLabelInstruction(_cg, TR::InstOpCode::b, _callNode, _restartLabel);

   generateLabelInstruction(_cg, TR::InstOpCode::label, _callNode, TR::LabelSymbol::create(_cg->trHeapMemory(),_cg));

   // Switch from cold helper instruction stream.
   //
   swapInstructionListsWithCompilation();
   }

void TR_PPCOutOfLineCodeSection::assignRegisters(TR_RegisterKinds kindsToBeAssigned)
   {
   TR::Compilation* comp = _cg->comp();
   if (hasBeenRegisterAssigned())
     return;

   // nested internal control flow assert:
   _cg->setInternalControlFlowSafeNestingDepth(_cg->internalControlFlowNestingDepth());

   // Create a dependency list on the first instruction in this stream that captures all current real register associations.
   // This is necessary to get the register assigner back into its original state before the helper stream was processed.
   _cg->incOutOfLineColdPathNestedDepth();

   // This prevents the OOL entry label from resetting all register's startOfranges during RA
   _cg->toggleIsInOOLSection();
   TR::RegisterDependencyConditions  *liveRealRegDeps = _cg->machine()->createCondForLiveAndSpilledGPRs(true, _cg->getSpilledRegisterList());

   if (liveRealRegDeps)
      _firstInstruction->setDependencyConditions(liveRealRegDeps);
   _cg->toggleIsInOOLSection(); // toggle it back because swapInstructionListsWithCompilation() also calls toggle...

   // Register assign the helper dispatch instructions.
   swapInstructionListsWithCompilation();
   _cg->doRegisterAssignment(kindsToBeAssigned);
   swapInstructionListsWithCompilation();

   _cg->decOutOfLineColdPathNestedDepth();

   // Returning to mainline, reset this counter
   _cg->setInternalControlFlowSafeNestingDepth(0);

   // Link in the helper stream into the mainline code.
   // We will end up with the OOL items attached at the bottom of the instruction stream
   TR::Instruction *appendInstruction = comp->getAppendInstruction();
   appendInstruction->setNext(_firstInstruction);
   _firstInstruction->setPrev(appendInstruction);
   comp->setAppendInstruction(_appendInstruction);

   setHasBeenRegisterAssigned(true);
   }

