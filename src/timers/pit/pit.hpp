#pragma once
#include "static_assert.h"
#include <stddef.h>
#include <stdint.h>

namespace pit
{

constexpr float    PIT_FREQ_MHZ = 1.193182;
constexpr uint32_t PIT_FREQ_HZ  = PIT_FREQ_MHZ * 1'000'000;

constexpr uint16_t MAX_FREQ_DIVIDER = 65535; // maybe 0 is mapped to 65536

constexpr uint8_t PIT_IRQ_CHANEL0 = 0;

// Should i create an enum? It's not really an enumeration of possible value...
enum class IOPort : uint8_t
{
    channel_0_data           = 0x40, // read/write
    channel_1_data           = 0x41, // read/write
    channel_2_data           = 0x42, // read/write
    mode_or_command_register = 0x43  // (Write only, a read is ignored)
};

namespace ModeCommandRegister
{
enum class Bit_6_7 : uint8_t
{
    channel_0         = 0b00,
    channel_1         = 0b01,
    channel_2         = 0b10,
    read_back_command = 0b11, //(8254+ only, So supported by anything modern)
};

enum class Bit_4_5 : uint8_t
{
    latch_count_value_cmd = 0b00,
    low_byte_only         = 0b01,
    high_byte_only        = 0b10,
    low_or_high_byte      = 0b11,
};

enum class Bit_1_3 : uint8_t
{
    single_shot       = 0b000,
    channel_1         = 0b001,
    channel_2         = 0b010,
    read_back_command = 0b011,
};

enum class Bit_0 : bool
{
    binary16        = 0,
    four_digits_bcd = 1,
};

} // namespace ModeCommandRegister

struct command
{
    enum ModeCommandRegister::Bit_0   bcd_binary_mode : 1;
    enum ModeCommandRegister::Bit_1_3 operating_mode : 3;
    enum ModeCommandRegister::Bit_4_5 access_mode : 2;
    enum ModeCommandRegister::Bit_6_7 channel : 2;
};

STATIC_ASSERT(sizeof(command) == 1, "Must be 1 byte");

} // namespace pit
