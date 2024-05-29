#pragma once

// Custom modules
#include "display/master.hpp"

namespace kc {

namespace Display
{
    namespace Font
    {
        // Static character pixel map
        using CharacterMap = StaticBitMap<8, 16>;

        /// @brief Get character's pixel map
        /// @param character Character to get the map for
        /// @return Character's pixel map
        CharacterMap GetCharacterMap(char character);
    }
}

} // namespace kc
