#ifndef GEUI_CANVAS_HPP_
#define GEUI_CANVAS_HPP_

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
        int order_ = 0;
        bool visible_ = true;

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

        /** @brief Set order
         * @param _order 前後の順番（大きいほど手前）
         */
        void SetOrder(int _order);

        /** @brief Get order
         * @return order
         */
        int GetOrder() const;

        /** @brief Set visibility
         * @param _visible 表示するか
         */
        void SetVisible(bool _visible);

        /** @brief Get visibility
         * @return visible
         */
        bool IsVisible() const;

        /** @brief Build instances
         * @param _screenSize 描画先のサイズ
         * @param _textures テクスチャの解決先
         * @param _instances 書き足す先
         */
        void Build(const Vector2& _screenSize, TextureProvider& _textures, std::vector<Instance>& _instances) const;

    private:
        void Append(const Widget& _widget, const Rect& _parent, float _scale, TextureProvider& _textures, std::vector<Instance>& _instances) const;
    };
}

#endif // GEUI_CANVAS_HPP_
