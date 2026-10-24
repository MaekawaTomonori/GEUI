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
        std::string name;
        Vector2 anchor{};
        Vector2 pivot{};
        Vector2 offset{};
        Vector2 size{};
        Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
        Vector4 selectedColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        std::string image;
        bool visible = true;
        bool selectable = false;
        bool selected = false;
        bool raisable = false;
        std::vector<std::unique_ptr<Widget>> children;
        Rect rect{};
    };

    /** @brief Get color to draw
     * @param _widget 対象の Widget
     * @return color
     */
    inline const Vector4& GetDrawColor(const Widget& _widget) {
        return _widget.selected ? _widget.selectedColor : _widget.color;
    }
}

#endif // GEUI_WIDGET_HPP_
