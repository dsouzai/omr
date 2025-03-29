/*******************************************************************************
 * Copyright IBM Corp. and others 2018
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] https://openjdk.org/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0-only WITH Classpath-exception-2.0 OR GPL-2.0-only WITH OpenJDK-assembly-exception-1.0
 *******************************************************************************/

#ifndef ARM64OUTOFLINECODESECTION_INCL
#define ARM64OUTOFLINECODESECTION_INCL

#include "codegen/InstOpCode.hpp"
#include "codegen/OutOfLineCodeSection.hpp"

namespace TR {
class CodeGenerator;
class LabelSymbol;
} // namespace TR

class TR_ARM64OutOfLineCodeSection : public TR_OutOfLineCodeSection {
public:
    /**
     * @brief Constructor
     * @param[in] entryLabel : entry label
     * @param[in] cg : code generator
     */
    TR_ARM64OutOfLineCodeSection(TR::LabelSymbol *entryLabel, TR::CodeGenerator *cg)
        : TR_OutOfLineCodeSection(entryLabel, cg)
    {}

    /**
     * @brief Constructor
     * @param[in] entryLabel : entry label
     * @param[in] restartLabel : restart label
     * @param[in] cg : code generator
     */
    TR_ARM64OutOfLineCodeSection(TR::LabelSymbol *entryLabel, TR::LabelSymbol *restartLabel, TR::CodeGenerator *cg)
        : TR_OutOfLineCodeSection(entryLabel, restartLabel, cg)
    {}

    // For calls

    /**
     * @brief Constructor
     * @param[in] callNode : call node
     * @param[in] callOp : call opcode
     * @param[in] targetReg : target register
     * @param[in] entryLabel : entry label
     * @param[in] restartLabel : restart label
     * @param[in] cg : code generator
     */
    TR_ARM64OutOfLineCodeSection(TR::Node *callNode, TR::ILOpCodes callOp, TR::Register *targetReg,
        TR::LabelSymbol *entryLabel, TR::LabelSymbol *restartLabel, TR::CodeGenerator *cg);

    /**
     * @brief Generates out-of-line code section dispatch
     */
    void generateARM64OutOfLineCodeSectionDispatch();
};
#endif
