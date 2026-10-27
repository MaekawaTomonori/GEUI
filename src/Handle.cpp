#include "Canvas.hpp"
#include "Widget.hpp"

#include <format>

#include "Current.hpp"
#include "Key.hpp"
#include "Log.hpp"
#include "Tree/CanvasStack.hpp"
#include "Tree/Name.hpp"

namespace GEUI {
    namespace {
        Tree::Canvas* GetCanvas(uint32_t _id, std::string_view _operation) {
            Tree::CanvasStack* canvases = GetCurrentCanvases();
            Tree::Canvas* canvas = canvases != nullptr ? canvases->GetCanvas(_id) : nullptr;
            if (canvas == nullptr) {
                Log(std::format("{} on invalid canvas", _operation));
            }
            return canvas;
        }

        Tree::Widget* GetWidget(uint32_t _id, std::string_view _operation) {
            Tree::CanvasStack* canvases = GetCurrentCanvases();
            Tree::Widget* widget = canvases != nullptr ? canvases->GetWidget(_id) : nullptr;
            if (widget == nullptr) {
                Log(std::format("{} on invalid widget", _operation));
            }
            return widget;
        }

        Widget ToHandle(Tree::Canvas& _canvas, Tree::Widget* _widget, std::string_view _name) {
            if (_widget == nullptr) {
                Log(std::format("'{}' not found", _name));
                return Widget();
            }
            return Widget(Key{}, GetCurrentCanvases()->GetId(_canvas, *_widget));
        }

        // Widget の中身を触る操作は、持ち主の Canvas も要る
        Tree::Canvas* GetOwner(uint32_t _id) {
            Tree::CanvasStack* canvases = GetCurrentCanvases();
            return canvases != nullptr ? canvases->GetOwner(_id) : nullptr;
        }
    }

    Widget::Widget(const Key&, uint32_t _id) : id_(_id) {}

    bool Widget::IsValid() const {
        Tree::CanvasStack* canvases = GetCurrentCanvases();
        return canvases != nullptr && canvases->GetWidget(id_) != nullptr;
    }

    Widget Widget::Find(std::string_view _path) const {
        Tree::Widget* widget = GetWidget(id_, "Find");
        return widget != nullptr ? ToHandle(*GetOwner(id_), Tree::Find(*widget, _path), _path) : Widget();
    }

    Widget Widget::FindAny(std::string_view _name) const {
        Tree::Widget* widget = GetWidget(id_, "FindAny");
        return widget != nullptr ? ToHandle(*GetOwner(id_), Tree::FindAny(*widget, _name), _name) : Widget();
    }

    void Widget::SetVisible(bool _visible) {
        if (Tree::Widget* widget = GetWidget(id_, "SetVisible")) {
            widget->visible = _visible;
        }
    }

    void Widget::SetColor(const Vector4& _color) {
        if (Tree::Widget* widget = GetWidget(id_, "SetColor")) {
            widget->color = _color;
        }
    }

    Vector4 Widget::GetColor() const {
        const Tree::Widget* widget = GetWidget(id_, "GetColor");
        return widget != nullptr ? widget->color : Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
    }

    void Widget::SetImage(std::string_view _image) {
        Tree::Widget* widget = GetWidget(id_, "SetImage");
        if (widget == nullptr) {
            return;
        }
        Tree::ImageStyle* image = GetOwner(id_)->GetImage(*widget);
        if (image == nullptr) {
            Log(std::format("SetImage on '{}' which is not an image", widget->name));
            return;
        }
        image->name = _image;
    }

    void Widget::SetImageRegion(const Vector2& _min, const Vector2& _size) {
        Tree::Widget* widget = GetWidget(id_, "SetImageRegion");
        if (widget == nullptr) {
            return;
        }
        Tree::ImageStyle* image = GetOwner(id_)->GetImage(*widget);
        if (image == nullptr) {
            Log(std::format("SetImageRegion on '{}' which is not an image", widget->name));
            return;
        }
        image->region = { _min.x, _min.y, _size.x, _size.y };
    }

    void Widget::SetOffset(const Vector2& _offset) {
        if (Tree::Widget* widget = GetWidget(id_, "SetOffset")) {
            widget->offset = _offset;
        }
    }

    Vector2 Widget::GetOffset() const {
        const Tree::Widget* widget = GetWidget(id_, "GetOffset");
        return widget != nullptr ? widget->offset : Vector2{};
    }

    void Widget::SetSize(const Vector2& _size) {
        if (Tree::Widget* widget = GetWidget(id_, "SetSize")) {
            widget->size = _size;
        }
    }

    void Widget::SetScale(float _scale) {
        if (Tree::Widget* widget = GetWidget(id_, "SetScale")) {
            widget->scale = _scale;
        }
    }

    float Widget::GetScale() const {
        const Tree::Widget* widget = GetWidget(id_, "GetScale");
        return widget != nullptr ? widget->scale : 1.0f;
    }

    void Widget::SetText(std::string_view _text) {
        Tree::Widget* widget = GetWidget(id_, "SetText");
        if (widget == nullptr) {
            return;
        }
        Tree::TextStyle* text = GetOwner(id_)->GetText(*widget);
        if (text == nullptr) {
            Log(std::format("SetText on '{}' which is not a text", widget->name));
            return;
        }
        text->text = _text;
    }

    void Widget::SetSelected(bool _selected) {
        Tree::Widget* widget = GetWidget(id_, "SetSelected");
        if (widget == nullptr) {
            return;
        }
        if (!widget->selectable) {
            Log(std::format("SetSelected on '{}' which is not selectable", widget->name));
            return;
        }
        if (_selected) {
            GetOwner(id_)->Focus(*widget);
        } else {
            widget->selected = false;
        }
    }

    void Widget::SetEnabled(bool _enabled) {
        if (Tree::Widget* widget = GetWidget(id_, "SetEnabled")) {
            widget->enabled = _enabled;
        }
    }

    bool Widget::WasPressed() const {
        const Tree::Widget* widget = GetWidget(id_, "WasPressed");
        return widget != nullptr && GetOwner(id_)->GetPressed() == widget;
    }

    Canvas::Canvas(uint32_t _id) : id_(_id) {}

    Canvas Canvas::Load(const std::filesystem::path& _path) {
        Tree::CanvasStack* canvases = GetCurrentCanvases();
        if (canvases == nullptr) {
            Log("Load before setup");
            return Canvas();
        }
        return Canvas(canvases->Load(_path));
    }

    void Canvas::Unload() {
        if (Tree::CanvasStack* canvases = GetCurrentCanvases()) {
            canvases->Unload(id_);
        }
        id_ = 0;
    }

    bool Canvas::IsValid() const {
        Tree::CanvasStack* canvases = GetCurrentCanvases();
        return canvases != nullptr && canvases->GetCanvas(id_) != nullptr;
    }

    void Canvas::SetVisible(bool _visible) {
        if (Tree::Canvas* canvas = GetCanvas(id_, "SetVisible")) {
            canvas->SetVisible(_visible);
        }
    }

    bool Canvas::IsVisible() const {
        const Tree::Canvas* canvas = GetCanvas(id_, "IsVisible");
        return canvas != nullptr && canvas->IsVisible();
    }

    void Canvas::SetOrder(int _order) {
        if (Tree::Canvas* canvas = GetCanvas(id_, "SetOrder")) {
            canvas->SetOrder(_order);
        }
    }

    void Canvas::SetBlockInput(bool _block) {
        if (Tree::Canvas* canvas = GetCanvas(id_, "SetBlockInput")) {
            canvas->SetBlockInput(_block);
        }
    }

    void Canvas::SetAlpha(float _alpha) {
        if (Tree::Canvas* canvas = GetCanvas(id_, "SetAlpha")) {
            canvas->SetAlpha(_alpha);
        }
    }

    Widget Canvas::Find(std::string_view _path) const {
        Tree::Canvas* canvas = GetCanvas(id_, "Find");
        return canvas != nullptr ? ToHandle(*canvas, Tree::Find(canvas->GetRoot(), _path), _path) : Widget();
    }

    Widget Canvas::FindAny(std::string_view _name) const {
        Tree::Canvas* canvas = GetCanvas(id_, "FindAny");
        return canvas != nullptr ? ToHandle(*canvas, Tree::FindAny(canvas->GetRoot(), _name), _name) : Widget();
    }

    void Canvas::Navigate(Direction _direction) {
        // 描画先は y が下向き
        constexpr Vector2 DIRECTIONS[] = { { 0.0f, -1.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f }, { 1.0f, 0.0f } };
        if (Tree::Canvas* canvas = GetCanvas(id_, "Navigate")) {
            canvas->Navigate(DIRECTIONS[static_cast<size_t>(_direction)]);
        }
    }

    void Canvas::Submit() {
        if (Tree::Canvas* canvas = GetCanvas(id_, "Submit")) {
            canvas->Submit();
        }
    }

    Widget Canvas::GetFocused() const {
        Tree::Canvas* canvas = GetCanvas(id_, "GetFocused");
        Tree::Widget* focused = canvas != nullptr ? canvas->GetFocused() : nullptr;
        // 何も選ばれていないのは普通のことなので、見つからなくても警告しない
        return focused != nullptr ? Widget(Key{}, GetCurrentCanvases()->GetId(*canvas, *focused)) : Widget();
    }
}
