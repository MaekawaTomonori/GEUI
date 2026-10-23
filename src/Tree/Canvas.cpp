#include "Tree/Canvas.hpp"

#include <algorithm>

#include "Backend/TextureProvider.hpp"
#include "Tree/Layout.hpp"

namespace GEUI {
    void Canvas::Setup(const Vector2& _referenceSize) {
        referenceSize_ = _referenceSize;
    }

    Widget& Canvas::GetRoot() {
        return root_;
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

    void Canvas::Build(const Vector2& _screenSize, TextureProvider& _textures, std::vector<Instance>& _instances) const {
        if (referenceSize_.x <= 0.0f || referenceSize_.y <= 0.0f) {
            return;
        }

        // 縦横で同じ倍率を使い、アスペクト比が変わっても歪ませない
        const float scale = std::min(_screenSize.x / referenceSize_.x, _screenSize.y / referenceSize_.y);
        const Rect screen = { {}, _screenSize };
        for (const auto& child : root_.children) {
            Append(*child, screen, scale, _textures, _instances);
        }
    }

    void Canvas::Append(const Widget& _widget, const Rect& _parent, float _scale, TextureProvider& _textures, std::vector<Instance>& _instances) const {
        if (!_widget.visible) {
            return;
        }

        const Rect rect = Resolve(_widget, _parent, _scale);

        // 透明な Widget は子をまとめる入れ物として使う
        if (_widget.color.w > 0.0f) {
            Instance instance{ rect.min + rect.size * 0.5f, rect.size * 0.5f, _widget.color };
            if (!_widget.image.empty()) {
                instance.texIndex = _textures.Resolve(_widget.image);
            }
            _instances.push_back(instance);
        }

        for (const auto& child : _widget.children) {
            Append(*child, rect, _scale, _textures, _instances);
        }
    }
}
