#include "Tree/Canvas.hpp"

#include <algorithm>
#include <cstdint>
#include <format>
#include <memory>

#include "Backend/TextureProvider.hpp"
#include "Log.hpp"
#include "Tree/Layout.hpp"
#include "Tree/Name.hpp"

namespace GEUI {
    namespace {
        bool Contains(const Rect& _rect, const Vector2& _point) {
            return _point.x >= _rect.min.x && _point.x < _rect.min.x + _rect.size.x
                && _point.y >= _rect.min.y && _point.y < _rect.min.y + _rect.size.y;
        }

        Widget* FindChild(Widget& _parent, std::string_view _name) {
            for (const auto& child : _parent.children) {
                if (child->name == _name) {
                    return child.get();
                }
            }
            return nullptr;
        }

        void FindAll(Widget& _widget, std::string_view _name, Widget*& _first, uint32_t& _count) {
            for (const auto& child : _widget.children) {
                if (child->name == _name) {
                    if (_first == nullptr) {
                        _first = child.get();
                    }
                    ++_count;
                }
                FindAll(*child, _name, _first, _count);
            }
        }
    }

    void Canvas::Setup(const Vector2& _referenceSize) {
        referenceSize_ = _referenceSize;
    }

    Widget& Canvas::GetRoot() {
        return root_;
    }

    const Widget& Canvas::GetRoot() const {
        return root_;
    }

    Widget& Canvas::Create(Widget& _parent, std::string_view _name) {
        auto widget = std::make_unique<Widget>();
        widget->name = _name;
        _parent.children.push_back(std::move(widget));

        Widget& created = *_parent.children.back();
        CheckName(_parent, created);
        FillNames(root_);
        return created;
    }

    Widget* Canvas::Find(std::string_view _path) {
        Widget* widget = &root_;
        while (widget != nullptr && !_path.empty()) {
            const size_t separator = _path.find('/');
            widget = FindChild(*widget, _path.substr(0, separator));
            _path = separator == std::string_view::npos ? std::string_view() : _path.substr(separator + 1);
        }
        return widget == &root_ ? nullptr : widget;
    }

    Widget* Canvas::FindAny(std::string_view _name) {
        Widget* first = nullptr;
        uint32_t count = 0;
        FindAll(root_, _name, first, count);

        // 兄弟の外では重複を許すので、どれが返るかは並び順で決まる
        if (count > 1) {
            Log(std::format("'{}' matches {} widgets", _name, count));
        }
        return first;
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

    void Canvas::Build(const Vector2& _screenSize, TextureProvider& _textures, std::vector<Instance>& _instances) {
        if (referenceSize_.x <= 0.0f || referenceSize_.y <= 0.0f) {
            return;
        }

        // 縦横で同じ倍率を使い、アスペクト比が変わっても歪ませない
        const float scale = std::min(_screenSize.x / referenceSize_.x, _screenSize.y / referenceSize_.y);
        root_.rect = { {}, _screenSize };
        for (const auto& child : root_.children) {
            Append(*child, root_.rect, scale, _textures, _instances);
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

    void Canvas::Append(Widget& _widget, const Rect& _parent, float _scale, TextureProvider& _textures, std::vector<Instance>& _instances) {
        if (!_widget.visible) {
            return;
        }

        // ヒット判定は次のフレームでこの矩形を使う
        _widget.rect = Resolve(_widget, _parent, _scale);

        // 透明な Widget は子をまとめる入れ物として使う
        const Vector4& color = GetDrawColor(_widget);
        if (color.w > 0.0f) {
            Instance instance{ _widget.rect.min + _widget.rect.size * 0.5f, _widget.rect.size * 0.5f, color };
            if (!_widget.image.empty()) {
                instance.texIndex = _textures.Resolve(_widget.image);
            }
            _instances.push_back(instance);
        }

        for (const auto& child : _widget.children) {
            Append(*child, _widget.rect, _scale, _textures, _instances);
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

        // 描かれていない入れ物には当たらない
        if (GetDrawColor(_widget).w > 0.0f && Contains(_widget.rect, _point)) {
            return true;
        }

        _path.pop_back();
        return false;
    }
}
