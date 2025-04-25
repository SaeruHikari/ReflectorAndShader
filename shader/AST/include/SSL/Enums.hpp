#pragma once
#include <stdint.h>

namespace skr::SSL {

enum class BinaryOp : uint32_t {

    // arithmetic
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    BIT_AND,
    BIT_OR,
    BIT_XOR,
    SHL,
    SHR,
    AND,
    OR,

    // relational
    LESS,
    GREATER,
    LESS_EQUAL,
    GREATER_EQUAL,
    EQUAL,
    NOT_EQUAL,

    //
    ASSIGN = 0x1000,
    ADD_ASSIGN = 0x1001,
    SUB_ASSIGN = 0x1002,
    MUL_ASSIGN = 0x1003,
    DIV_ASSIGN = 0x1004,
    MOD_ASSIGN = 0x1005,
};

}