#pragma once
#include "keycodes.hpp"

// Size 256 so that any Key value (uint8_t) can index it
namespace keycodes
{

extern const char *key_strings[256];
// Lookup function
static inline const char *key_to_string(Key key)
{
    const char *s = key_strings[static_cast<uint8_t>(key)];
    return s;
}

// This needs to happen, because my g++ doesn't allow array designator. And larges switches breaks neovim
void initialize_keycodes_name_converter();

} // namespace keycodes
