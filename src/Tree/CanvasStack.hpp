#ifndef GEUI_CANVAS_STACK_HPP_
#define GEUI_CANVAS_STACK_HPP_

#include <memory>
#include <span>
#include <vector>

#include "Render/Instance.hpp"
#include "Tree/Canvas.hpp"

namespace GEUI {
    /** @brief Ordered canvases
     */
    class CanvasStack {
        std::vector<std::unique_ptr<Canvas>> canvases_;
        std::vector<Instance> instances_;
        std::vector<Widget*> path_;

    public:
        /** @brief Add canvas
         * @param _referenceSize 基準解像度
         * @return canvas
         */
        Canvas& Add(const Vector2& _referenceSize);

        /** @brief Build instances
         * @param _screenSize 描画先のサイズ
         * @param _textures テクスチャの解決先
         * @return instances
         */
        std::span<const Instance> Build(const Vector2& _screenSize, TextureProvider& _textures);

        /** @brief Hit test
         * @param _point 描画先の座標
         * @return over
         */
        bool HitTest(const Vector2& _point);

        /** @brief Click
         * @param _point 描画先の座標
         * @return over
         */
        bool Click(const Vector2& _point);
    };
}

#endif // GEUI_CANVAS_STACK_HPP_
