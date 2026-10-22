#ifndef GEUI_INSTANCE_HPP_
#define GEUI_INSTANCE_HPP_

#include <cstdint>

#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"

namespace GEUI {
    constexpr uint32_t NO_TEXTURE = UINT32_MAX;

    /** @brief Quad data per instance
     */
    struct Instance {
        Vector2 center;
        Vector2 halfSize;
        Vector4 color;
        uint32_t texIndex = NO_TEXTURE;
    };
}

#endif // GEUI_INSTANCE_HPP_
