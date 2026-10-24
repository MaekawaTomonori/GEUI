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

        Widget ToHandle(Tree::Widget* _widget, std::string_view _name) {
            if (_widget == nullptr) {
                Log(std::format("'{}' not found", _name));
                return Widget();
            }
            return Widget(Key{}, GetCurrentCanvases()->GetId(*_widget));
        }
    }

    Widget::Widget(const Key&, uint32_t _id) : id_(_id) {}

    bool Widget::IsValid() const {
        Tree::CanvasStack* canvases = GetCurrentCanvases();
        return canvases != nullptr && canvases->GetWidget(id_) != nullptr;
    }

    Widget Widget::Find(std::string_view _path) const {
        Tree::Widget* widget = GetWidget(id_, "Find");
        return widget != nullptr ? ToHandle(Tree::Find(*widget, _path), _path) : Widget();
    }

    Widget Widget::FindAny(std::string_view _name) const {
        Tree::Widget* widget = GetWidget(id_, "FindAny");
        return widget != nullptr ? ToHandle(Tree::FindAny(*widget, _name), _name) : Widget();
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

    void Widget::SetImage(std::string_view _image) {
        if (Tree::Widget* widget = GetWidget(id_, "SetImage")) {
            widget->image = _image;
        }
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

    Widget Canvas::Find(std::string_view _path) const {
        Tree::Canvas* canvas = GetCanvas(id_, "Find");
        return canvas != nullptr ? ToHandle(Tree::Find(canvas->GetRoot(), _path), _path) : Widget();
    }

    Widget Canvas::FindAny(std::string_view _name) const {
        Tree::Canvas* canvas = GetCanvas(id_, "FindAny");
        return canvas != nullptr ? ToHandle(Tree::FindAny(canvas->GetRoot(), _name), _name) : Widget();
    }
}
