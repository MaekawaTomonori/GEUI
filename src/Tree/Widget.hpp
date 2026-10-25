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
        // 大量に作るので、全ての Widget が使う項目だけを持つ。種類ごとの中身とオプションは Canvas が持ち、番号で引く
        struct Widget {
            std::string name;
            std::vector<std::unique_ptr<Widget>> children;
            Vector2 anchor{};
            Vector2 pivot{};
            Vector2 offset{};
            Vector2 size{};
            Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
            Rect rect{};
            uint32_t id = 0;
            uint32_t data = 0;
            uint32_t selectedStyle = 0;
            WidgetType type = WidgetType::Quad;
            bool visible : 1 = true;
            bool selectable : 1 = false;
            bool selected : 1 = false;
            bool followState : 1 = false;
            bool raisable : 1 = false;
            bool drawnSelected : 1 = false;
        };
    }
}

#endif // GEUI_WIDGET_HPP_
