#ifndef GEUI_CANVAS_STACK_HPP_
#define GEUI_CANVAS_STACK_HPP_

#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <unordered_map>
#include <vector>

#include "Render/Instance.hpp"
#include "Tree/Canvas.hpp"

namespace GEUI {
    namespace Tree {
        /** @brief Ordered canvases
         */
        class CanvasStack {
            std::vector<std::unique_ptr<Canvas>> canvases_;
            std::unordered_map<uint32_t, Canvas*> canvasIds_;
            std::unordered_map<uint32_t, Widget*> widgetIds_;
            uint32_t nextId_ = 1;
            std::vector<Instance> instances_;
            std::vector<Widget*> path_;

        public:
            /** @brief Load canvas
             * @param _path 読むファイル
             * @return id
             */
            uint32_t Load(const std::filesystem::path& _path);

            /** @brief Unload canvas
             * @param _id Canvas の番号
             */
            void Unload(uint32_t _id);

            /** @brief Get canvas
             * @param _id Canvas の番号
             * @return canvas
             */
            Canvas* GetCanvas(uint32_t _id);

            /** @brief Get widget
             * @param _id Widget の番号
             * @return widget
             */
            Widget* GetWidget(uint32_t _id);

            /** @brief Get widget id
             * @param _widget 番号を知りたい Widget
             * @return id
             */
            uint32_t GetId(Widget& _widget);

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

        private:
            void Forget(const Widget& _widget);
        };
    }
}

#endif // GEUI_CANVAS_STACK_HPP_
