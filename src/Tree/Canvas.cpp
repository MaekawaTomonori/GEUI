#include "Tree/Canvas.hpp"

#include <memory>

#include "Backend/TextureProvider.hpp"
#include "Text/FontCache.hpp"
#include "Text/TextLayout.hpp"
#include "Tree/Layout.hpp"
#include "Tree/Name.hpp"

namespace GEUI {
    namespace Tree {
        namespace {
            bool Contains(const Rect& _rect, const Vector2& _point) {
                return _point.x >= _rect.min.x && _point.x < _rect.min.x + _rect.size.x
                    && _point.y >= _rect.min.y && _point.y < _rect.min.y + _rect.size.y;
            }

            Rect ApplyStyle(const Rect& _rect, const Vector2& _pivot, const StateStyle& _style, const Rect& _parent) {
                const Vector2 pivotPoint = _rect.min + _rect.size * _pivot + _parent.size * _style.shift;
                const Vector2 size = _rect.size * _style.scale;
                return { pivotPoint - size * _pivot, size };
            }
        }

        void Canvas::Setup(const Vector2& _referenceSize) {
            referenceSize_ = _referenceSize;
        }

        void Canvas::SetPath(const std::filesystem::path& _path) {
            path_ = _path;
        }

        const std::filesystem::path& Canvas::GetPath() const {
            return path_;
        }

        Widget& Canvas::GetRoot() {
            return root_;
        }

        const Widget& Canvas::GetRoot() const {
            return root_;
        }

        Widget& Canvas::Create(Widget& _parent, WidgetType _type, std::string_view _name) {
            auto widget = std::make_unique<Widget>();
            widget->name = _name;
            _parent.children.push_back(std::move(widget));

            Widget& created = *_parent.children.back();
            SetType(created, _type);
            CheckName(_parent, created);
            FillNames(root_);
            return created;
        }

        void Canvas::SetType(Widget& _widget, WidgetType _type) {
            RemoveData(_widget);
            _widget.type = _type;
            AddData(_widget);
        }

        void Canvas::Release(Widget& _widget) {
            RemoveData(_widget);
            states_.Remove(_widget.selectedStyle);
            for (const auto& child : _widget.children) {
                Release(*child);
            }
        }

        std::string* Canvas::GetImage(const Widget& _widget) {
            return _widget.type == WidgetType::Image ? images_.Get(_widget.data) : nullptr;
        }

        const std::string* Canvas::GetImage(const Widget& _widget) const {
            return _widget.type == WidgetType::Image ? images_.Get(_widget.data) : nullptr;
        }

        TextStyle* Canvas::GetText(const Widget& _widget) {
            return _widget.type == WidgetType::Text ? texts_.Get(_widget.data) : nullptr;
        }

        const TextStyle* Canvas::GetText(const Widget& _widget) const {
            return _widget.type == WidgetType::Text ? texts_.Get(_widget.data) : nullptr;
        }

        StateStyle* Canvas::GetSelectedStyle(const Widget& _widget) {
            return states_.Get(_widget.selectedStyle);
        }

        const StateStyle* Canvas::GetSelectedStyle(const Widget& _widget) const {
            return states_.Get(_widget.selectedStyle);
        }

        StateStyle& Canvas::AddSelectedStyle(Widget& _widget) {
            if (_widget.selectedStyle == 0) {
                // 選択中の色の初期値は通常の色にして、付けた直後に見た目が変わらないようにする
                _widget.selectedStyle = states_.Add({ _widget.color });
            }
            return *states_.Get(_widget.selectedStyle);
        }

        void Canvas::RemoveSelectedStyle(Widget& _widget) {
            states_.Remove(_widget.selectedStyle);
        }

        Rect Canvas::GetReferenceRect() const {
            return { {}, referenceSize_ };
        }

        void Canvas::SetOrder(int _order) {
            order_ = _order;
        }

        int Canvas::GetOrder() const {
            return order_;
        }

        void Canvas::SetVisible(bool _visible) {
            visible_ = _visible;
        }

        bool Canvas::IsVisible() const {
            return visible_;
        }

        void Canvas::SetBlockInput(bool _block) {
            blockInput_ = _block;
        }

        bool Canvas::IsBlockingInput() const {
            return blockInput_;
        }

        void Canvas::Build(const Vector2& _screenSize, TextureProvider& _textures, FontCache& _fonts, std::vector<Instance>& _instances) {
            // 文字の大きさは割合ではなく基準解像度の px で持つので、画面の高さに合わせて拡縮する
            const float scale = referenceSize_.y > 0.0f ? _screenSize.y / referenceSize_.y : 1.0f;
            root_.rect = { {}, _screenSize };
            for (const auto& child : root_.children) {
                Append(*child, root_.rect, scale, false, _textures, _fonts, _instances);
            }
        }

        bool Canvas::HitTest(const Vector2& _point, std::vector<Widget*>& _path) {
            _path.clear();
            _path.push_back(&root_);

            // 後に描いたものほど手前なので、逆順に調べる
            for (auto it = root_.children.rbegin(); it != root_.children.rend(); ++it) {
                if (HitTest(**it, _point, _path)) {
                    return true;
                }
            }

            _path.clear();
            return false;
        }

        void Canvas::Append(Widget& _widget, const Rect& _parent, float _scale, bool _parentSelected, TextureProvider& _textures, FontCache& _fonts, std::vector<Instance>& _instances) {
            if (!_widget.visible) {
                return;
            }

            _widget.drawnSelected = (_widget.selectable && _widget.selected) || (_widget.followState && _parentSelected);

            // ヒット判定は次のフレームでこの矩形を使う
            _widget.rect = Resolve(_widget, _parent);
            // 拡縮は子の文字にも掛け、中身ごと大きくなるようにする
            float scale = _scale;
            if (const StateStyle* style = GetSelectedStyle(_widget); style != nullptr && _widget.drawnSelected) {
                _widget.rect = ApplyStyle(_widget.rect, _widget.pivot, *style, _parent);
                scale *= style->scale;
            }

            // 透明な Widget は子をまとめる入れ物として使う
            const Vector4& color = GetDrawColor(_widget);
            if (_widget.type != WidgetType::Button && color.w > 0.0f) {
                if (const TextStyle* text = GetText(_widget)) {
                    if (const Font* font = _fonts.Get(text->font)) {
                        AppendText(*text, _widget.rect, color, *font, scale, _instances);
                    }
                } else if (_widget.type != WidgetType::Text) {
                    Instance instance{ _widget.rect.min + _widget.rect.size * 0.5f, _widget.rect.size * 0.5f, color };
                    if (const std::string* image = GetImage(_widget); image != nullptr && !image->empty()) {
                        instance.texIndex = _textures.Resolve(*image);
                    }
                    _instances.push_back(instance);
                }
            }

            for (const auto& child : _widget.children) {
                Append(*child, _widget.rect, scale, _widget.drawnSelected, _textures, _fonts, _instances);
            }
        }

        bool Canvas::HitTest(Widget& _widget, const Vector2& _point, std::vector<Widget*>& _path) {
            if (!_widget.visible) {
                return false;
            }

            _path.push_back(&_widget);

            for (auto it = _widget.children.rbegin(); it != _widget.children.rend(); ++it) {
                if (HitTest(**it, _point, _path)) {
                    return true;
                }
            }

            // 描かれていない入れ物には当たらない。Button は何も描かないが、押す対象なので当たる
            const bool hittable = _widget.type == WidgetType::Button || GetDrawColor(_widget).w > 0.0f;
            if (hittable && Contains(_widget.rect, _point)) {
                return true;
            }

            _path.pop_back();
            return false;
        }

        const Vector4& Canvas::GetDrawColor(const Widget& _widget) const {
            const StateStyle* style = GetSelectedStyle(_widget);
            return style != nullptr && _widget.drawnSelected ? style->color : _widget.color;
        }

        void Canvas::AddData(Widget& _widget) {
            switch (_widget.type) {
            case WidgetType::Image:
                _widget.data = images_.Add();
                break;
            case WidgetType::Text:
                _widget.data = texts_.Add();
                break;
            case WidgetType::Quad:
            case WidgetType::Button:
                break;
            }
        }

        void Canvas::RemoveData(Widget& _widget) {
            switch (_widget.type) {
            case WidgetType::Image:
                images_.Remove(_widget.data);
                break;
            case WidgetType::Text:
                texts_.Remove(_widget.data);
                break;
            case WidgetType::Quad:
            case WidgetType::Button:
                break;
            }
        }
    }
}
