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
        public:
            static constexpr int Width = value_type().size();
            static constexpr int Height = std::tuple_size<array>::value;

        private:
            /// @brief Convert character to pixel map
            /// @param character The character to convert
            /// @return Converted character
            static constexpr array CharacterToMap(char character);

        public:
            /// @brief Create text character
            /// @param character The character to create
            TextCharacter(char character);
        };

        class ValueCharacter : public StaticBitMap<25, 35>
        {
        public:
            static constexpr int Width = value_type().size();
            static constexpr int Height = std::tuple_size<array>::value;

        private:
            /// @brief Convert character to pixel map
            /// @param character The character to convert
            /// @return Converted character
            static constexpr array CharacterToMap(char character);

        public:
            /// @brief Create value character
            /// @param character The character to create
            ValueCharacter(char character);
        };
    }
}

} // namespace kc
