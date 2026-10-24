#ifndef GEUI_INSTANCE_HPP_
#define GEUI_INSTANCE_HPP_

#include <cstdint>

#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"

namespace GEUI {
    constexpr uint32_t NO_TEXTURE = UINT32_MAX;
    constexpr uint32_t KIND_QUAD = 0;
    constexpr uint32_t KIND_GLYPH = 1;

    /** @brief Quad data per instance
     */
    struct Instance {
        Vector2 center;
        Vector2 halfSize;
        Vector4 color;
        Vector4 uvRect{ 0.0f, 0.0f, 1.0f, 1.0f };
        uint32_t texIndex = NO_TEXTURE;
        uint32_t kind = KIND_QUAD;
        float weight = 0.0f;
    };
}

#endif // GEUI_INSTANCE_HPP_
