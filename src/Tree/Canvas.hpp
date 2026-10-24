#ifndef GEUI_CANVAS_HPP_
#define GEUI_CANVAS_HPP_

#include <filesystem>
#include <string_view>
#include <vector>

#include "Render/Instance.hpp"
#include "Tree/Widget.hpp"

namespace GEUI {
    class TextureProvider;

    namespace Tree {
        /** @brief Root of UI tree
         */
        class Canvas {
            std::filesystem::path path_;
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

            /** @brief Set file path
             * @param _path ファイルのパス
             */
            void SetPath(const std::filesystem::path& _path);

            /** @brief Get file path
             * @return path
             */
            const std::filesystem::path& GetPath() const;

            /** @brief Get root widget
             * @return root
             */
            Widget& GetRoot();

            /** @brief Get root widget
             * @return root
             */
            const Widget& GetRoot() const;

            /** @brief Create widget
             * @param _parent 親
             * @param _type 種類
             * @param _name 名前
             * @return widget
             */
            Widget& Create(Widget& _parent, WidgetType _type, std::string_view _name = {});

            /** @brief Get reference rectangle
             * @return referenceRect
             */
            Rect GetReferenceRect() const;

            /** @brief Set order
             * @param _order 順番
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
             * @param _block 入力を遮るか
             */
            void SetBlockInput(bool _block);

            /** @brief Get input blocking
             * @return blocking
             */
            bool IsBlockingInput() const;

            /** @brief Build instances
             * @param _screenSize 描画先のサイズ
             * @param _textures テクスチャ
             * @param _instances 出力先
             */
            void Build(const Vector2& _screenSize, TextureProvider& _textures, std::vector<Instance>& _instances);

            /** @brief Hit test
             * @param _point 描画先の座標
             * @param _path 経路
             * @return hit
             */
            bool HitTest(const Vector2& _point, std::vector<Widget*>& _path);

        private:
            void Append(Widget& _widget, const Rect& _parent, TextureProvider& _textures, std::vector<Instance>& _instances);
            bool HitTest(Widget& _widget, const Vector2& _point, std::vector<Widget*>& _path);
        };
    }
}

#endif // GEUI_CANVAS_HPP_
