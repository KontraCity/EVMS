#pragma once

// Custom modules
#include "display/master.hpp"

namespace kc {

namespace Display
{
    namespace Font
    {
        class TextCharacter : public StaticBitMap<8, 16>
        {
        private:
            /// @brief Convert character to pixel map
            /// @param character The character to convert
            /// @return Converted character
            static StaticBitMap<8, 16> CharacterToMap(char character);

        public:
            /// @brief Create text character
            /// @param character The character to create
            TextCharacter(char character);
        };

        class ValueCharacter : public StaticBitMap<25, 35>
        {
        private:
            /// @brief Convert character to pixel map
            /// @param character The character to convert
            /// @return Converted character
            static StaticBitMap<25, 35> CharacterToMap(char character);

        public:
            /// @brief Create value character
            /// @param character The character to create
            ValueCharacter(char character);
        };
    }
}

} // namespace kc
