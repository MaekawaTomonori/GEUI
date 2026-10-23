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
        bool blockInput_ = false;

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

        /** @brief Set input blocking
         * @param _block 当たらなくても後ろへ入力を通さないか
         */
        void SetBlockInput(bool _block);

        /** @brief Get input blocking
         * @return blocking
         */
        bool IsBlockingInput() const;

        /** @brief Build instances
         * @param _screenSize 描画先のサイズ
         * @param _textures テクスチャの解決先
         * @param _instances 書き足す先
         */
        void Build(const Vector2& _screenSize, TextureProvider& _textures, std::vector<Instance>& _instances);

        /** @brief Hit test
         * @param _point 描画先の座標
         * @param _path ルートから当たった Widget までの経路
         * @return hit
         */
        bool HitTest(const Vector2& _point, std::vector<Widget*>& _path);

    private:
        void Append(Widget& _widget, const Rect& _parent, float _scale, TextureProvider& _textures, std::vector<Instance>& _instances);
        bool HitTest(Widget& _widget, const Vector2& _point, std::vector<Widget*>& _path);
    };
}

#endif // GEUI_CANVAS_HPP_
