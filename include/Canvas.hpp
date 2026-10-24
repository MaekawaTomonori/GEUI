#ifndef GEUI_CANVAS_HANDLE_HPP_
#define GEUI_CANVAS_HANDLE_HPP_

#include <cstdint>
#include <filesystem>
#include <string_view>

#include "Widget.hpp"

namespace GEUI {
    /** @brief Handle to canvas
     */
    class Canvas {
        uint32_t id_ = 0;

        explicit Canvas(uint32_t _id);

    public:
        Canvas() = default;

        /** @brief Load canvas
         * @param _path パス
         * @return canvas
         */
        static Canvas Load(const std::filesystem::path& _path);

        /** @brief Unload canvas
         */
        void Unload();

        /** @brief Check target exists
         * @return valid
         */
        bool IsValid() const;

        /** @brief Set visibility
         * @param _visible 表示するか
         */
        void SetVisible(bool _visible);

        /** @brief Get visibility
         * @return visible
         */
        bool IsVisible() const;

        /** @brief Set order
         * @param _order 順番
         */
        void SetOrder(int _order);

        /** @brief Set input blocking
         * @param _block 入力を遮るか
         */
        void SetBlockInput(bool _block);

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
    };
}

#endif // GEUI_CANVAS_HANDLE_HPP_
