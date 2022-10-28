/*******************************************************************************
 * Copyright (c) 2022, 2022 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
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
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#include <stdio.h>
#include "compile/Compilation.hpp"
#include "infra/UnitTester.hpp"

TR_UnitTester::UnitTestFP TR_UnitTester::_unitTestFPs[] =
   {
   test_trarraycopy_checkArrayStore,
   };

TR_UnitTester::TR_UnitTester(TR::Compilation *comp)
   : _comp(comp),
     _totalTests(sizeof(_unitTestFPs) / sizeof(_unitTestFPs[0])),
     _testsPassed(0),
     _testsFailed(0)
   {}

void
TR_UnitTester::run()
   {
   for (auto i = 0; i < _totalTests; i++)
      {
      if (_unitTestFPs[i](*this))
         {
         _testsPassed++;
         }
      else
         {
         _testsFailed++;
         }
      }
   }

void
TR_UnitTester::summarize()
   {
   fprintf(stdout,
           "%u Total Tests, "
           "%u Tests passed, "
           "%u Tests failed\n",
           _totalTests, _testsPassed, _testsFailed);
   }