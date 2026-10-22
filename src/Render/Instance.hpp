#ifndef GEUI_INSTANCE_HPP_
#define GEUI_INSTANCE_HPP_

#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"

namespace GEUI {
    /** @brief Quad data per instance
     */
    struct Instance {
        Vector2 center;
        Vector2 halfSize;
        Vector4 color;
    };
}

#endif // GEUI_INSTANCE_HPP_
