#ifndef GEUI_CANVAS_STACK_HPP_
#define GEUI_CANVAS_STACK_HPP_

#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
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
             * @param _path パス
             * @return id
             */
            uint32_t Load(const std::filesystem::path& _path);

            /** @brief Unload canvas
             * @param _id 番号
             */
            void Unload(uint32_t _id);

            /** @brief Save canvas to its file
             * @param _canvas 対象
             * @return succeeded
             */
            bool Save(const Canvas& _canvas);

            /** @brief Reload canvas from its file
             * @param _canvas 対象
             * @return succeeded
             */
            bool Reload(Canvas& _canvas);

            /** @brief Remove widget
             * @param _parent 親
             * @param _widget 対象
             */
            void Remove(Widget& _parent, const Widget& _widget);

            /** @brief Get all canvases
             * @return canvases
             */
            const std::vector<std::unique_ptr<Canvas>>& GetAll() const;

            /** @brief Get canvas
             * @param _id 番号
             * @return canvas
             */
            Canvas* GetCanvas(uint32_t _id);

            /** @brief Get widget
             * @param _id 番号
             * @return widget
             */
            Widget* GetWidget(uint32_t _id);

            /** @brief Get widget id
             * @param _widget 対象
             * @return id
             */
            uint32_t GetId(Widget& _widget);

            /** @brief Build instances
             * @param _screenSize 描画先のサイズ
             * @param _textures テクスチャ
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
            void Restore(Widget& _widget, const std::string& _prefix, const std::unordered_map<std::string, uint32_t>& _ids);
        };
    }
}

#endif // GEUI_CANVAS_STACK_HPP_
