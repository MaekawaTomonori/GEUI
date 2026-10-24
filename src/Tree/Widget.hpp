#ifndef GEUI_WIDGET_HPP_
#define GEUI_WIDGET_HPP_

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"

namespace GEUI {
    namespace Tree {
        /** @brief Rectangle
         */
        struct Rect {
            Vector2 min{};
            Vector2 size{};
        };

        /** @brief Kind of widget
         */
        enum class WidgetType : uint8_t { Quad, Image, Text, Button };

        constexpr std::string_view TYPE_NAMES[] = { "Quad", "Image", "Text", "Button" };

        /** @brief Appearance in state
         */
        struct StateStyle {
            Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
            Vector2 shift{};
            float scale = 1.0f;
        };

        /** @brief Alignment in rectangle
         */
        enum class TextAlign : uint8_t { Start, Center, End };

        /** @brief Text parameters
         */
        struct TextStyle {
            std::string text;
            std::string font;
            float size = 24.0f;
            float weight = 0.0f;
            float spacing = 0.0f;
            float lineHeight = 1.2f;
            TextAlign alignX = TextAlign::Start;
            TextAlign alignY = TextAlign::Start;
        };

        /** @brief Get type name
         * @param _type 種類
         * @return name
         */
        constexpr std::string_view GetTypeName(WidgetType _type) {
            return TYPE_NAMES[static_cast<size_t>(_type)];
        }

        /** @brief Node of UI tree
         */
        struct Widget {
            std::string name;
            WidgetType type = WidgetType::Quad;
            Vector2 anchor{};
            Vector2 pivot{};
            Vector2 offset{};
            Vector2 size{};
            Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
            StateStyle selectedStyle;
            std::string image;
            TextStyle text;
            bool visible = true;
            bool selectable = false;
            bool selected = false;
            bool followState = false;
            bool raisable = false;
            std::vector<std::unique_ptr<Widget>> children;
            Rect rect{};
            bool drawnSelected = false;
            uint32_t id = 0;
        };

        /** @brief Get color to draw
         * @param _widget 対象
         * @return color
         */
        inline const Vector4& GetDrawColor(const Widget& _widget) {
            return _widget.drawnSelected ? _widget.selectedStyle.color : _widget.color;
        }
    }
}

#endif // GEUI_WIDGET_HPP_
