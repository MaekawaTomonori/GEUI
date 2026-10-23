#ifndef GEUI_CANVAS_HPP_
#define GEUI_CANVAS_HPP_

#include <span>
#include <vector>

#include "Render/Instance.hpp"
#include "Tree/Widget.hpp"

namespace GEUI {
    class TextureProvider;

    /** @brief Root of UI tree
     */
    class Canvas {
        Vector2 referenceSize_{};
        Widget root_;
        std::vector<Instance> instances_;

    public:
        /** @brief Initialize
         * @param _referenceSize 基準解像度
         */
        void Setup(const Vector2& _referenceSize);

        /** @brief Get root widget
         * @return root
         */
        Widget& GetRoot();

        /** @brief Get reference rectangle
         * @return referenceRect
         */
        Rect GetReferenceRect() const;

        /** @brief Build instances
         * @param _screenSize 描画先のサイズ
         * @param _textures テクスチャの解決先
         * @return instances
         */
        std::span<const Instance> Build(const Vector2& _screenSize, TextureProvider& _textures);

    private:
        void Append(const Widget& _widget, const Rect& _parent, float _scale, TextureProvider& _textures);
    };
}

#endif // GEUI_CANVAS_HPP_
