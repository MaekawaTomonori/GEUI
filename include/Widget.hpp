#ifndef GEUI_WIDGET_HANDLE_HPP_
#define GEUI_WIDGET_HANDLE_HPP_

#include <cstdint>
#include <string_view>

#include "Math/Vector2.hpp"
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

        /** @brief Get color
         * @return color
         */
        Vector4 GetColor() const;

        /** @brief Set image
         * @param _image 画像名
         */
        void SetImage(std::string_view _image);

        /** @brief Set drawn part of image
         * @param _min 左上（画像に対する割合）
         * @param _size 大きさ（画像に対する割合）
         */
        void SetImageRegion(const Vector2& _min, const Vector2& _size);

        /** @brief Set offset
         * @param _offset 位置（親に対する割合）
         */
        void SetOffset(const Vector2& _offset);

        /** @brief Get offset
         * @return offset
         */
        Vector2 GetOffset() const;

        /** @brief Set size
         * @param _size 大きさ（親に対する割合）
         */
        void SetSize(const Vector2& _size);

        /** @brief Set scale around pivot (children and text are scaled too)
         * @param _scale 倍率
         */
        void SetScale(float _scale);

        /** @brief Get scale
         * @return scale
         */
        float GetScale() const;

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
