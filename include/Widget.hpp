#ifndef GEUI_WIDGET_HANDLE_HPP_
#define GEUI_WIDGET_HANDLE_HPP_

#include <cstdint>
#include <string_view>

#include "Math/Vector4.hpp"

namespace GEUI {
    struct Key;

    /** @brief Handle to widget
     */
    class Widget {
        uint32_t id_ = 0;

    public:
        Widget() = default;

        /** @brief Create from id
         * @param _key 鍵
         * @param _id 番号
         */
        Widget(const Key& _key, uint32_t _id);

        /** @brief Check target exists
         * @return valid
         */
        bool IsValid() const;

        /** @brief Compare target
         * @param _other 比較先
         * @return same
         */
        bool operator==(const Widget& _other) const = default;

        /** @brief Find widget by path
         * @param _path パス
         * @return widget
         */
        Widget Find(std::string_view _path) const;

        /** @brief Find descendant by name
         * @param _name 名前
         * @return widget
         */
        Widget FindAny(std::string_view _name) const;

        /** @brief Set visibility
         * @param _visible 表示するか
         */
        void SetVisible(bool _visible);

        /** @brief Set color
         * @param _color 色
         */
        void SetColor(const Vector4& _color);

        /** @brief Set image
         * @param _image 画像名
         */
        void SetImage(std::string_view _image);

        /** @brief Set text
         * @param _text 文字列
         */
        void SetText(std::string_view _text);

        /** @brief Set selection state
         * @param _selected 選択中か
         */
        void SetSelected(bool _selected);

        /** @brief Set pressable
         * @param _enabled 押せるか
         */
        void SetEnabled(bool _enabled);

        /** @brief Check pressed in this frame
         * @return pressed
         */
        bool WasPressed() const;
    };
}

#endif // GEUI_WIDGET_HANDLE_HPP_
