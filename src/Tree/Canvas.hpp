#ifndef GEUI_CANVAS_HPP_
#define GEUI_CANVAS_HPP_

#include <filesystem>
#include <string_view>
#include <vector>

#include "Render/Instance.hpp"
#include "Tree/Pool.hpp"
#include "Tree/Widget.hpp"

namespace GEUI {
    class FontCache;
    class TextureProvider;

    namespace Tree {
        /** @brief Root of UI tree
         */
        class Canvas {
            std::filesystem::path path_;
            Vector2 referenceSize_{};
            Widget root_;
            Pool<std::string> images_;
            Pool<TextStyle> texts_;
            Pool<StateStyle> states_;
            Widget* pressed_ = nullptr;
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

            /** @brief Change type
             * @param _widget 対象
             * @param _type 種類
             */
            void SetType(Widget& _widget, WidgetType _type);

            /** @brief Release data of widget and descendants
             * @param _widget 対象
             */
            void Release(Widget& _widget);

            /** @brief Get image name
             * @param _widget 対象
             * @return image
             */
            std::string* GetImage(const Widget& _widget);

            /** @brief Get image name
             * @param _widget 対象
             * @return image
             */
            const std::string* GetImage(const Widget& _widget) const;

            /** @brief Get text parameters
             * @param _widget 対象
             * @return text
             */
            TextStyle* GetText(const Widget& _widget);

            /** @brief Get text parameters
             * @param _widget 対象
             * @return text
             */
            const TextStyle* GetText(const Widget& _widget) const;

            /** @brief Get selected appearance
             * @param _widget 対象
             * @return style
             */
            StateStyle* GetSelectedStyle(const Widget& _widget);

            /** @brief Get selected appearance
             * @param _widget 対象
             * @return style
             */
            const StateStyle* GetSelectedStyle(const Widget& _widget) const;

            /** @brief Add selected appearance
             * @param _widget 対象
             * @return style
             */
            StateStyle& AddSelectedStyle(Widget& _widget);

            /** @brief Remove selected appearance
             * @param _widget 対象
             */
            void RemoveSelectedStyle(Widget& _widget);

            /** @brief Select only this widget in canvas
             * @param _widget 対象
             */
            void Focus(Widget& _widget);

            /** @brief Get selected widget
             * @return focused
             */
            Widget* GetFocused();

            /** @brief Move selection to nearest widget in direction
             * @param _direction 画面上の向き（単位ベクトル）
             */
            void Navigate(const Vector2& _direction);

            /** @brief Press widget
             * @param _widget 対象
             */
            void Press(Widget& _widget);

            /** @brief Press selected widget
             */
            void Submit();

            /** @brief Get widget pressed in this frame
             * @return pressed
             */
            const Widget* GetPressed() const;

            /** @brief Forget pressed widget
             */
            void ClearPressed();

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
             * @param _fonts フォント
             * @param _instances 出力先
             */
            void Build(const Vector2& _screenSize, TextureProvider& _textures, FontCache& _fonts, std::vector<Instance>& _instances);

            /** @brief Hit test
             * @param _point 描画先の座標
             * @param _path 経路
             * @return hit
             */
            bool HitTest(const Vector2& _point, std::vector<Widget*>& _path);

        private:
            void Append(Widget& _widget, const Rect& _parent, float _scale, bool _parentSelected, TextureProvider& _textures, FontCache& _fonts, std::vector<Instance>& _instances);
            bool HitTest(Widget& _widget, const Vector2& _point, std::vector<Widget*>& _path);
            const Vector4& GetDrawColor(const Widget& _widget) const;
            void AddData(Widget& _widget);
            void RemoveData(Widget& _widget);
        };
    }
}

#endif // GEUI_CANVAS_HPP_
