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
         * @param _key GEUI の中でだけ作れる鍵
         * @param _id 番号
         */
        Widget(const Key& _key, uint32_t _id);

        /** @brief Check target exists
         * @return valid
         */
        bool IsValid() const;

        /** @brief Find widget by path
         * @param _path この Widget からの名前を '/' で区切ったもの
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
         * @param _image 画像の名前
         */
        void SetImage(std::string_view _image);
    };
}

#endif // GEUI_WIDGET_HANDLE_HPP_
