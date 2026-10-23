#ifndef GEUI_WIDGET_HPP_
#define GEUI_WIDGET_HPP_

#include <memory>
#include <string>
#include <vector>

#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"

namespace GEUI {
    /** @brief Rectangle
     */
    struct Rect {
        Vector2 min{};
        Vector2 size{};
    };

    /** @brief Node of UI tree
     */
    struct Widget {
        Vector2 anchor{};
        Vector2 pivot{};
        Vector2 offset{};
        Vector2 size{};
        Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
        std::string image;
        bool visible = true;
        std::vector<std::unique_ptr<Widget>> children;
    };
}

#endif // GEUI_WIDGET_HPP_
