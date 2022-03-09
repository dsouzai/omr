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

#include <gtest/gtest.h>
#include "../CodeGenTest.hpp"
#include "codegen/GenerateInstructions.hpp"

#define ARM64_INSTRUCTION_ALIGNMENT 32

class ARM64BinaryInstruction : public TRTest::BinaryInstruction {
public:

    ARM64BinaryInstruction() : TRTest::BinaryInstruction() {
    }
    ARM64BinaryInstruction(const char *instr) : TRTest::BinaryInstruction(instr) {
        // As the tests are encoded as big endian strings, we need to convert them to little endian
        for (int i = 0; i < _size / sizeof(uint32_t); i++) {
            std::swap(_buf[i * sizeof(uint32_t)], _buf[i * sizeof(uint32_t) + 3]);
            std::swap(_buf[i * sizeof(uint32_t) + 1], _buf[i * sizeof(uint32_t) + 2]);
        }
    }
};

std::ostream &operator<<(std::ostream &os, const ARM64BinaryInstruction &instr) {
    os << static_cast<TRTest::BinaryInstruction>(instr);
    return os;
}

class ARM64Trg1ImmEncodingTest : public TRTest::BinaryEncoderTest<ARM64_INSTRUCTION_ALIGNMENT>, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, uint32_t, ARM64BinaryInstruction>> {};

TEST_P(ARM64Trg1ImmEncodingTest, encode) {
    auto trgReg = cg()->machine()->getRealRegister(std::get<1>(GetParam()));
    auto imm = std::get<2>(GetParam());

    auto instr = generateTrg1ImmInstruction(cg(), std::get<0>(GetParam()), fakeNode, trgReg, imm);

    ASSERT_EQ(std::get<3>(GetParam()), encodeInstruction(instr));
}

class ARM64Trg1ImmShiftedEncodingTest : public TRTest::BinaryEncoderTest<ARM64_INSTRUCTION_ALIGNMENT>, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, uint32_t, uint32_t, ARM64BinaryInstruction>> {};

TEST_P(ARM64Trg1ImmShiftedEncodingTest, encode) {
    auto trgReg = cg()->machine()->getRealRegister(std::get<1>(GetParam()));
    auto imm = std::get<2>(GetParam());
    auto shiftAmount = std::get<3>(GetParam());

    auto instr = generateTrg1ImmShiftedInstruction(cg(), std::get<0>(GetParam()), fakeNode, trgReg, imm, shiftAmount);

    ASSERT_EQ(std::get<4>(GetParam()), encodeInstruction(instr));
}

class ARM64Trg1Src1EncodingTest : public TRTest::BinaryEncoderTest<ARM64_INSTRUCTION_ALIGNMENT>, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, ARM64BinaryInstruction>> {};

TEST_P(ARM64Trg1Src1EncodingTest, encode) {
    auto trgReg = cg()->machine()->getRealRegister(std::get<1>(GetParam()));
    auto src1Reg = cg()->machine()->getRealRegister(std::get<2>(GetParam()));

    auto instr = generateTrg1Src1Instruction(cg(), std::get<0>(GetParam()), fakeNode, trgReg, src1Reg);

    ASSERT_EQ(std::get<3>(GetParam()), encodeInstruction(instr));
}

class ARM64Trg1Src2EncodingTest : public TRTest::BinaryEncoderTest<ARM64_INSTRUCTION_ALIGNMENT>, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, TR::RealRegister::RegNum, ARM64BinaryInstruction>> {};

TEST_P(ARM64Trg1Src2EncodingTest, encode) {
    auto trgReg = cg()->machine()->getRealRegister(std::get<1>(GetParam()));
    auto src1Reg = cg()->machine()->getRealRegister(std::get<2>(GetParam()));
    auto src2Reg = cg()->machine()->getRealRegister(std::get<3>(GetParam()));

    auto instr = generateTrg1Src2Instruction(cg(), std::get<0>(GetParam()), fakeNode, trgReg, src1Reg, src2Reg);

    ASSERT_EQ(std::get<4>(GetParam()), encodeInstruction(instr));
}

INSTANTIATE_TEST_CASE_P(MOV, ARM64Trg1ImmEncodingTest, ::testing::Values(
    std::make_tuple(TR::InstOpCode::movzx,  TR::RealRegister::x3, 0, "d2800003"),
    std::make_tuple(TR::InstOpCode::movzx,  TR::RealRegister::x3, 0xffff, "d29fffe3"),
    std::make_tuple(TR::InstOpCode::movzx,  TR::RealRegister::x3, 0x1ffff, "d2bfffe3"),
    std::make_tuple(TR::InstOpCode::movzx,  TR::RealRegister::x3, 0x2ffff, "d2dfffe3"),
    std::make_tuple(TR::InstOpCode::movzx,  TR::RealRegister::x3, 0x3ffff, "d2ffffe3"),
    std::make_tuple(TR::InstOpCode::movzx,  TR::RealRegister::x28, 0, "d280001c"),
    std::make_tuple(TR::InstOpCode::movzx,  TR::RealRegister::x28, 0xffff, "d29ffffc"),
    std::make_tuple(TR::InstOpCode::movzx,  TR::RealRegister::x28, 0x1ffff, "d2bffffc"),
    std::make_tuple(TR::InstOpCode::movzx,  TR::RealRegister::x28, 0x2ffff, "d2dffffc"),
    std::make_tuple(TR::InstOpCode::movzx,  TR::RealRegister::x28, 0x3ffff, "d2fffffc"),

    std::make_tuple(TR::InstOpCode::movkx,  TR::RealRegister::x3, 0, "f2800003"),
    std::make_tuple(TR::InstOpCode::movkx,  TR::RealRegister::x3, 0xffff, "f29fffe3"),
    std::make_tuple(TR::InstOpCode::movkx,  TR::RealRegister::x3, 0x1ffff, "f2bfffe3"),
    std::make_tuple(TR::InstOpCode::movkx,  TR::RealRegister::x3, 0x2ffff, "f2dfffe3"),
    std::make_tuple(TR::InstOpCode::movkx,  TR::RealRegister::x3, 0x3ffff, "f2ffffe3"),
    std::make_tuple(TR::InstOpCode::movkx,  TR::RealRegister::x28, 0, "f280001c"),
    std::make_tuple(TR::InstOpCode::movkx,  TR::RealRegister::x28, 0xffff, "f29ffffc"),
    std::make_tuple(TR::InstOpCode::movkx,  TR::RealRegister::x28, 0x1ffff, "f2bffffc"),
    std::make_tuple(TR::InstOpCode::movkx,  TR::RealRegister::x28, 0x2ffff, "f2dffffc"),
    std::make_tuple(TR::InstOpCode::movkx,  TR::RealRegister::x28, 0x3ffff, "f2fffffc"),

    std::make_tuple(TR::InstOpCode::movnx,  TR::RealRegister::x3, 0, "92800003"),
    std::make_tuple(TR::InstOpCode::movnx,  TR::RealRegister::x3, 0xffff, "929fffe3"),
    std::make_tuple(TR::InstOpCode::movnx,  TR::RealRegister::x3, 0x1ffff, "92bfffe3"),
    std::make_tuple(TR::InstOpCode::movnx,  TR::RealRegister::x3, 0x2ffff, "92dfffe3"),
    std::make_tuple(TR::InstOpCode::movnx,  TR::RealRegister::x3, 0x3ffff, "92ffffe3"),
    std::make_tuple(TR::InstOpCode::movnx,  TR::RealRegister::x28, 0, "9280001c"),
    std::make_tuple(TR::InstOpCode::movnx,  TR::RealRegister::x28, 0xffff, "929ffffc"),
    std::make_tuple(TR::InstOpCode::movnx,  TR::RealRegister::x28, 0x1ffff, "92bffffc"),
    std::make_tuple(TR::InstOpCode::movnx,  TR::RealRegister::x28, 0x2ffff, "92dffffc"),
    std::make_tuple(TR::InstOpCode::movnx,  TR::RealRegister::x28, 0x3ffff, "92fffffc"),

    std::make_tuple(TR::InstOpCode::movzw,  TR::RealRegister::x3, 0, "52800003"),
    std::make_tuple(TR::InstOpCode::movzw,  TR::RealRegister::x3, 0xffff, "529fffe3"),
    std::make_tuple(TR::InstOpCode::movzw,  TR::RealRegister::x3, 0x1ffff, "52bfffe3"),
    std::make_tuple(TR::InstOpCode::movzw,  TR::RealRegister::x28, 0, "5280001c"),
    std::make_tuple(TR::InstOpCode::movzw,  TR::RealRegister::x28, 0xffff, "529ffffc"),
    std::make_tuple(TR::InstOpCode::movzw,  TR::RealRegister::x28, 0x1ffff, "52bffffc"),

    std::make_tuple(TR::InstOpCode::movkw,  TR::RealRegister::x3, 0, "72800003"),
    std::make_tuple(TR::InstOpCode::movkw,  TR::RealRegister::x3, 0xffff, "729fffe3"),
    std::make_tuple(TR::InstOpCode::movkw,  TR::RealRegister::x3, 0x1ffff, "72bfffe3"),
    std::make_tuple(TR::InstOpCode::movkw,  TR::RealRegister::x28, 0, "7280001c"),
    std::make_tuple(TR::InstOpCode::movkw,  TR::RealRegister::x28, 0xffff, "729ffffc"),
    std::make_tuple(TR::InstOpCode::movkw,  TR::RealRegister::x28, 0x1ffff, "72bffffc"),

    std::make_tuple(TR::InstOpCode::movnw,  TR::RealRegister::x3, 0, "12800003"),
    std::make_tuple(TR::InstOpCode::movnw,  TR::RealRegister::x3, 0xffff, "129fffe3"),
    std::make_tuple(TR::InstOpCode::movnw,  TR::RealRegister::x3, 0x1ffff, "12bfffe3"),
    std::make_tuple(TR::InstOpCode::movnw,  TR::RealRegister::x28, 0, "1280001c"),
    std::make_tuple(TR::InstOpCode::movnw,  TR::RealRegister::x28, 0xffff, "129ffffc"),
    std::make_tuple(TR::InstOpCode::movnw,  TR::RealRegister::x28, 0x1ffff, "12bffffc")
));

INSTANTIATE_TEST_CASE_P(FMOV, ARM64Trg1ImmEncodingTest, ::testing::Values(
    std::make_tuple(TR::InstOpCode::fmovimmd,  TR::RealRegister::v0, 0, "1e601000"),    // abcdefgh = 0, expanded to 2.0
    std::make_tuple(TR::InstOpCode::fmovimmd,  TR::RealRegister::v0, 0xff, "1e7ff000"), // abcdefgh = 0xff, expanded to -1.9375
    std::make_tuple(TR::InstOpCode::fmovimmd,  TR::RealRegister::v31, 0, "1e60101f"),
    std::make_tuple(TR::InstOpCode::fmovimmd,  TR::RealRegister::v31, 0xff, "1e7ff01f"),

    std::make_tuple(TR::InstOpCode::fmovimms,  TR::RealRegister::v0, 0, "1e201000"),
    std::make_tuple(TR::InstOpCode::fmovimms,  TR::RealRegister::v0, 0xff, "1e3ff000"),
    std::make_tuple(TR::InstOpCode::fmovimms,  TR::RealRegister::v31, 0, "1e20101f"),
    std::make_tuple(TR::InstOpCode::fmovimms,  TR::RealRegister::v31, 0xff, "1e3ff01f"),

    std::make_tuple(TR::InstOpCode::vfmov2d,  TR::RealRegister::v0, 0, "6f00f400"),
    std::make_tuple(TR::InstOpCode::vfmov2d,  TR::RealRegister::v0, 0xff, "6f07f7e0"),
    std::make_tuple(TR::InstOpCode::vfmov2d,  TR::RealRegister::v31, 0, "6f00f41f"),
    std::make_tuple(TR::InstOpCode::vfmov2d,  TR::RealRegister::v31, 0xff, "6f07f7ff"),

    std::make_tuple(TR::InstOpCode::vfmov4s,  TR::RealRegister::v0, 0, "4f00f400"),
    std::make_tuple(TR::InstOpCode::vfmov4s,  TR::RealRegister::v0, 0xff, "4f07f7e0"),
    std::make_tuple(TR::InstOpCode::vfmov4s,  TR::RealRegister::v31, 0, "4f00f41f"),
    std::make_tuple(TR::InstOpCode::vfmov4s,  TR::RealRegister::v31, 0xff, "4f07f7ff")
));

INSTANTIATE_TEST_CASE_P(MOVI, ARM64Trg1ImmEncodingTest, ::testing::Values(
    std::make_tuple(TR::InstOpCode::vmovi16b,  TR::RealRegister::v0, 0, "4f00e400"),
    std::make_tuple(TR::InstOpCode::vmovi16b,  TR::RealRegister::v0, 0x55, "4f02e6a0"),
    std::make_tuple(TR::InstOpCode::vmovi16b,  TR::RealRegister::v0, 0xff, "4f07e7e0"),
    std::make_tuple(TR::InstOpCode::vmovi16b,  TR::RealRegister::v31, 0, "4f00e41f"),
    std::make_tuple(TR::InstOpCode::vmovi16b,  TR::RealRegister::v31, 0x55, "4f02e6bf"),
    std::make_tuple(TR::InstOpCode::vmovi16b,  TR::RealRegister::v31, 0xff, "4f07e7ff"),

    std::make_tuple(TR::InstOpCode::vmovi8h,  TR::RealRegister::v0, 0, "4f008400"),
    std::make_tuple(TR::InstOpCode::vmovi8h,  TR::RealRegister::v0, 0x55, "4f0286a0"),
    std::make_tuple(TR::InstOpCode::vmovi8h,  TR::RealRegister::v0, 0xff, "4f0787e0"),
    std::make_tuple(TR::InstOpCode::vmovi8h,  TR::RealRegister::v31, 0, "4f00841f"),
    std::make_tuple(TR::InstOpCode::vmovi8h,  TR::RealRegister::v31, 0x55, "4f0286bf"),
    std::make_tuple(TR::InstOpCode::vmovi8h,  TR::RealRegister::v31, 0xff, "4f0787ff"),

    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v0, 0, "4f000400"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v0, 0x55, "4f0206a0"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v0, 0xff, "4f0707e0"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v31, 0, "4f00041f"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v31, 0x55, "4f0206bf"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v31, 0xff, "4f0707ff"),

    std::make_tuple(TR::InstOpCode::vmovi2d,  TR::RealRegister::v0, 0, "6f00e400"),
    std::make_tuple(TR::InstOpCode::vmovi2d,  TR::RealRegister::v0, 0xaa, "6f05e540"),
    std::make_tuple(TR::InstOpCode::vmovi2d,  TR::RealRegister::v0, 0x55, "6f02e6a0"),
    std::make_tuple(TR::InstOpCode::vmovi2d,  TR::RealRegister::v0, 0xff, "6f07e7e0"),
    std::make_tuple(TR::InstOpCode::vmovi2d,  TR::RealRegister::v31, 0, "6f00e41f"),
    std::make_tuple(TR::InstOpCode::vmovi2d,  TR::RealRegister::v31, 0xaa, "6f05e55f"),
    std::make_tuple(TR::InstOpCode::vmovi2d,  TR::RealRegister::v31, 0x55, "6f02e6bf"),
    std::make_tuple(TR::InstOpCode::vmovi2d,  TR::RealRegister::v31, 0xff, "6f07e7ff"),

    std::make_tuple(TR::InstOpCode::movid,  TR::RealRegister::v0, 0, "2f00e400"),
    std::make_tuple(TR::InstOpCode::movid,  TR::RealRegister::v0, 0x55, "2f02e6a0"),
    std::make_tuple(TR::InstOpCode::movid,  TR::RealRegister::v0, 0xff, "2f07e7e0"),
    std::make_tuple(TR::InstOpCode::movid,  TR::RealRegister::v31, 0, "2f00e41f"),
    std::make_tuple(TR::InstOpCode::movid,  TR::RealRegister::v31, 0x55, "2f02e6bf"),
    std::make_tuple(TR::InstOpCode::movid,  TR::RealRegister::v31, 0xff, "2f07e7ff"),

    std::make_tuple(TR::InstOpCode::vmovi2s,  TR::RealRegister::v0, 0, "0f000400"),
    std::make_tuple(TR::InstOpCode::vmovi2s,  TR::RealRegister::v0, 0x55, "0f0206a0"),
    std::make_tuple(TR::InstOpCode::vmovi2s,  TR::RealRegister::v0, 0xff, "0f0707e0"),
    std::make_tuple(TR::InstOpCode::vmovi2s,  TR::RealRegister::v31, 0, "0f00041f"),
    std::make_tuple(TR::InstOpCode::vmovi2s,  TR::RealRegister::v31, 0x55, "0f0206bf"),
    std::make_tuple(TR::InstOpCode::vmovi2s,  TR::RealRegister::v31, 0xff, "0f0707ff")
));

INSTANTIATE_TEST_CASE_P(MVNI, ARM64Trg1ImmEncodingTest, ::testing::Values(
    std::make_tuple(TR::InstOpCode::vmvni8h,  TR::RealRegister::v0, 0, "6f008400"),
    std::make_tuple(TR::InstOpCode::vmvni8h,  TR::RealRegister::v0, 0x55, "6f0286a0"),
    std::make_tuple(TR::InstOpCode::vmvni8h,  TR::RealRegister::v0, 0xff, "6f0787e0"),
    std::make_tuple(TR::InstOpCode::vmvni8h,  TR::RealRegister::v31, 0, "6f00841f"),
    std::make_tuple(TR::InstOpCode::vmvni8h,  TR::RealRegister::v31, 0x55, "6f0286bf"),
    std::make_tuple(TR::InstOpCode::vmvni8h,  TR::RealRegister::v31, 0xff, "6f0787ff"),

    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v0, 0, "6f000400"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v0, 0x55, "6f0206a0"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v0, 0xff, "6f0707e0"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v31, 0, "6f00041f"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v31, 0x55, "6f0206bf"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v31, 0xff, "6f0707ff")
));

INSTANTIATE_TEST_CASE_P(MOVI, ARM64Trg1ImmShiftedEncodingTest, ::testing::Values(
    std::make_tuple(TR::InstOpCode::vmovi8h,  TR::RealRegister::v0, 0x55, 8, "4f02a6a0"),
    std::make_tuple(TR::InstOpCode::vmovi8h,  TR::RealRegister::v0, 0xff, 8, "4f07a7e0"),
    std::make_tuple(TR::InstOpCode::vmovi8h,  TR::RealRegister::v31, 0x55, 8, "4f02a6bf"),
    std::make_tuple(TR::InstOpCode::vmovi8h,  TR::RealRegister::v31, 0xff, 8, "4f07a7ff"),

    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v0, 0x55, 8, "4f0226a0"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v0, 0xff, 8, "4f0727e0"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v0, 0x55, 16, "4f0246a0"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v0, 0xff, 16, "4f0747e0"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v0, 0x55, 24, "4f0266a0"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v0, 0xff, 24, "4f0767e0"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v31, 0x55, 8, "4f0226bf"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v31, 0xff, 8, "4f0727ff"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v31, 0x55, 16, "4f0246bf"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v31, 0xff, 16, "4f0747ff"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v31, 0x55, 24, "4f0266bf"),
    std::make_tuple(TR::InstOpCode::vmovi4s,  TR::RealRegister::v31, 0xff, 24, "4f0767ff"),

    std::make_tuple(TR::InstOpCode::vmovi4s_one,  TR::RealRegister::v0, 0x55, 8, "4f02c6a0"),
    std::make_tuple(TR::InstOpCode::vmovi4s_one,  TR::RealRegister::v0, 0xff, 8, "4f07c7e0"),
    std::make_tuple(TR::InstOpCode::vmovi4s_one,  TR::RealRegister::v0, 0x55, 16, "4f02d6a0"),
    std::make_tuple(TR::InstOpCode::vmovi4s_one,  TR::RealRegister::v0, 0xff, 16, "4f07d7e0"),
    std::make_tuple(TR::InstOpCode::vmovi4s_one,  TR::RealRegister::v31, 0x55, 8, "4f02c6bf"),
    std::make_tuple(TR::InstOpCode::vmovi4s_one,  TR::RealRegister::v31, 0xff, 8, "4f07c7ff"),
    std::make_tuple(TR::InstOpCode::vmovi4s_one,  TR::RealRegister::v31, 0x55, 16, "4f02d6bf"),
    std::make_tuple(TR::InstOpCode::vmovi4s_one,  TR::RealRegister::v31, 0xff, 16, "4f07d7ff")
));

INSTANTIATE_TEST_CASE_P(MVNI, ARM64Trg1ImmShiftedEncodingTest, ::testing::Values(
    std::make_tuple(TR::InstOpCode::vmvni8h,  TR::RealRegister::v0, 0x55, 8, "6f02a6a0"),
    std::make_tuple(TR::InstOpCode::vmvni8h,  TR::RealRegister::v0, 0xff, 8, "6f07a7e0"),
    std::make_tuple(TR::InstOpCode::vmvni8h,  TR::RealRegister::v31, 0x55, 8, "6f02a6bf"),
    std::make_tuple(TR::InstOpCode::vmvni8h,  TR::RealRegister::v31, 0xff, 8, "6f07a7ff"),

    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v0, 0x55, 8, "6f0226a0"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v0, 0xff, 8, "6f0727e0"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v0, 0x55, 16, "6f0246a0"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v0, 0xff, 16, "6f0747e0"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v0, 0x55, 24, "6f0266a0"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v0, 0xff, 24, "6f0767e0"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v31, 0x55, 8, "6f0226bf"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v31, 0xff, 8, "6f0727ff"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v31, 0x55, 16, "6f0246bf"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v31, 0xff, 16, "6f0747ff"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v31, 0x55, 24, "6f0266bf"),
    std::make_tuple(TR::InstOpCode::vmvni4s,  TR::RealRegister::v31, 0xff, 24, "6f0767ff"),

    std::make_tuple(TR::InstOpCode::vmvni4s_one,  TR::RealRegister::v0, 0x55, 8, "6f02c6a0"),
    std::make_tuple(TR::InstOpCode::vmvni4s_one,  TR::RealRegister::v0, 0xff, 8, "6f07c7e0"),
    std::make_tuple(TR::InstOpCode::vmvni4s_one,  TR::RealRegister::v0, 0x55, 16, "6f02d6a0"),
    std::make_tuple(TR::InstOpCode::vmvni4s_one,  TR::RealRegister::v0, 0xff, 16, "6f07d7e0"),
    std::make_tuple(TR::InstOpCode::vmvni4s_one,  TR::RealRegister::v31, 0x55, 8, "6f02c6bf"),
    std::make_tuple(TR::InstOpCode::vmvni4s_one,  TR::RealRegister::v31, 0xff, 8, "6f07c7ff"),
    std::make_tuple(TR::InstOpCode::vmvni4s_one,  TR::RealRegister::v31, 0x55, 16, "6f02d6bf"),
    std::make_tuple(TR::InstOpCode::vmvni4s_one,  TR::RealRegister::v31, 0xff, 16, "6f07d7ff")
));

INSTANTIATE_TEST_CASE_P(VectorSQRT, ARM64Trg1Src1EncodingTest, ::testing::Values(
    std::make_tuple(TR::InstOpCode::vfsqrt4s,  TR::RealRegister::v15, TR::RealRegister::v0, "6ea1f80f"),
    std::make_tuple(TR::InstOpCode::vfsqrt4s,  TR::RealRegister::v31, TR::RealRegister::v0, "6ea1f81f"),
    std::make_tuple(TR::InstOpCode::vfsqrt4s,  TR::RealRegister::v0, TR::RealRegister::v15, "6ea1f9e0"),
    std::make_tuple(TR::InstOpCode::vfsqrt4s,  TR::RealRegister::v0, TR::RealRegister::v31, "6ea1fbe0"),

    std::make_tuple(TR::InstOpCode::vfsqrt2d,  TR::RealRegister::v15, TR::RealRegister::v0, "6ee1f80f"),
    std::make_tuple(TR::InstOpCode::vfsqrt2d,  TR::RealRegister::v31, TR::RealRegister::v0, "6ee1f81f"),
    std::make_tuple(TR::InstOpCode::vfsqrt2d,  TR::RealRegister::v0, TR::RealRegister::v15, "6ee1f9e0"),
    std::make_tuple(TR::InstOpCode::vfsqrt2d,  TR::RealRegister::v0, TR::RealRegister::v31, "6ee1fbe0")
));

INSTANTIATE_TEST_CASE_P(VectorMLA, ARM64Trg1Src2EncodingTest, ::testing::Values(
    std::make_tuple(TR::InstOpCode::vfmla4s,  TR::RealRegister::v15, TR::RealRegister::v0, TR::RealRegister::v0, "4e20cc0f"),
    std::make_tuple(TR::InstOpCode::vfmla4s,  TR::RealRegister::v31, TR::RealRegister::v0, TR::RealRegister::v0, "4e20cc1f"),
    std::make_tuple(TR::InstOpCode::vfmla4s,  TR::RealRegister::v0, TR::RealRegister::v15, TR::RealRegister::v0, "4e20cde0"),
    std::make_tuple(TR::InstOpCode::vfmla4s,  TR::RealRegister::v0, TR::RealRegister::v31, TR::RealRegister::v0, "4e20cfe0"),
    std::make_tuple(TR::InstOpCode::vfmla4s,  TR::RealRegister::v0, TR::RealRegister::v0, TR::RealRegister::v15, "4e2fcc00"),
    std::make_tuple(TR::InstOpCode::vfmla4s,  TR::RealRegister::v0, TR::RealRegister::v0, TR::RealRegister::v31, "4e3fcc00"),

    std::make_tuple(TR::InstOpCode::vfmla2d,  TR::RealRegister::v15, TR::RealRegister::v0, TR::RealRegister::v0, "4e60cc0f"),
    std::make_tuple(TR::InstOpCode::vfmla2d,  TR::RealRegister::v31, TR::RealRegister::v0, TR::RealRegister::v0, "4e60cc1f"),
    std::make_tuple(TR::InstOpCode::vfmla2d,  TR::RealRegister::v0, TR::RealRegister::v15, TR::RealRegister::v0, "4e60cde0"),
    std::make_tuple(TR::InstOpCode::vfmla2d,  TR::RealRegister::v0, TR::RealRegister::v31, TR::RealRegister::v0, "4e60cfe0"),
    std::make_tuple(TR::InstOpCode::vfmla2d,  TR::RealRegister::v0, TR::RealRegister::v0, TR::RealRegister::v15, "4e6fcc00"),
    std::make_tuple(TR::InstOpCode::vfmla2d,  TR::RealRegister::v0, TR::RealRegister::v0, TR::RealRegister::v31, "4e7fcc00"),

    std::make_tuple(TR::InstOpCode::vmla16b,  TR::RealRegister::v15, TR::RealRegister::v0, TR::RealRegister::v0, "4e20940f"),
    std::make_tuple(TR::InstOpCode::vmla16b,  TR::RealRegister::v31, TR::RealRegister::v0, TR::RealRegister::v0, "4e20941f"),
    std::make_tuple(TR::InstOpCode::vmla16b,  TR::RealRegister::v0, TR::RealRegister::v15, TR::RealRegister::v0, "4e2095e0"),
    std::make_tuple(TR::InstOpCode::vmla16b,  TR::RealRegister::v0, TR::RealRegister::v31, TR::RealRegister::v0, "4e2097e0"),
    std::make_tuple(TR::InstOpCode::vmla16b,  TR::RealRegister::v0, TR::RealRegister::v0, TR::RealRegister::v15, "4e2f9400"),
    std::make_tuple(TR::InstOpCode::vmla16b,  TR::RealRegister::v0, TR::RealRegister::v0, TR::RealRegister::v31, "4e3f9400"),

    std::make_tuple(TR::InstOpCode::vmla8h,  TR::RealRegister::v15, TR::RealRegister::v0, TR::RealRegister::v0, "4e60940f"),
    std::make_tuple(TR::InstOpCode::vmla8h,  TR::RealRegister::v31, TR::RealRegister::v0, TR::RealRegister::v0, "4e60941f"),
    std::make_tuple(TR::InstOpCode::vmla8h,  TR::RealRegister::v0, TR::RealRegister::v15, TR::RealRegister::v0, "4e6095e0"),
    std::make_tuple(TR::InstOpCode::vmla8h,  TR::RealRegister::v0, TR::RealRegister::v31, TR::RealRegister::v0, "4e6097e0"),
    std::make_tuple(TR::InstOpCode::vmla8h,  TR::RealRegister::v0, TR::RealRegister::v0, TR::RealRegister::v15, "4e6f9400"),
    std::make_tuple(TR::InstOpCode::vmla8h,  TR::RealRegister::v0, TR::RealRegister::v0, TR::RealRegister::v31, "4e7f9400"),

    std::make_tuple(TR::InstOpCode::vmla4s,  TR::RealRegister::v15, TR::RealRegister::v0, TR::RealRegister::v0, "4ea0940f"),
    std::make_tuple(TR::InstOpCode::vmla4s,  TR::RealRegister::v31, TR::RealRegister::v0, TR::RealRegister::v0, "4ea0941f"),
    std::make_tuple(TR::InstOpCode::vmla4s,  TR::RealRegister::v0, TR::RealRegister::v15, TR::RealRegister::v0, "4ea095e0"),
    std::make_tuple(TR::InstOpCode::vmla4s,  TR::RealRegister::v0, TR::RealRegister::v31, TR::RealRegister::v0, "4ea097e0"),
    std::make_tuple(TR::InstOpCode::vmla4s,  TR::RealRegister::v0, TR::RealRegister::v0, TR::RealRegister::v15, "4eaf9400"),
    std::make_tuple(TR::InstOpCode::vmla4s,  TR::RealRegister::v0, TR::RealRegister::v0, TR::RealRegister::v31, "4ebf9400")
));
