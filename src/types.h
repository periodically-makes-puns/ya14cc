#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef struct {
    u16 cp; // 0-9
    u8 iq; // 10-13
    u8 dur; // 14-17
    u8 manip; // 18-21
    u8 wn; // 22-25
    u8 inno; // 26-28
    u8 gs; // 29-30
    bool has; // 31
    u8 t; // 32-39
} qual_state;