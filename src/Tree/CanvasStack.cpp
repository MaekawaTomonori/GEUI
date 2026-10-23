#include "Tree/CanvasStack.hpp"

#include <algorithm>
#include <utility>

namespace GEUI {
    namespace {
        // _path は親から子の順。当たった Widget に最も近い条件を満たす祖先を探す
        template <typename Predicate>
        size_t FindNearest(const std::vector<Widget*>& _path, Predicate _predicate) {
            for (size_t i = _path.size(); i-- > 1;) {
                if (_predicate(*_path[i])) {
                    return i;
                }
            }
            return 0;
        }

        void Select(Widget& _parent, const Widget& _target) {
            for (const auto& child : _parent.children) {
                if (child->selectable) {
                    child->selected = child.get() == &_target;
                }
            }
        }

        // 入れ替えできない兄弟の位置は変えず、入れ替えできる兄弟の間でだけ末尾へ送る
        void Raise(Widget& _parent, const Widget& _target) {
            auto& children = _parent.children;
            size_t current = children.size();
            for (size_t i = 0; i < children.size(); ++i) {
                if (children[i].get() == &_target) {
                    current = i;
                } else if (current < children.size() && children[i]->raisable) {
                    std::swap(children[current], children[i]);
                    current = i;
                }
            }
        }
    }

    Canvas& CanvasStack::Add(const Vector2& _referenceSize) {
        auto canvas = std::make_unique<Canvas>();
        canvas->Setup(_referenceSize);
        canvases_.push_back(std::move(canvas));
        return *canvases_.back();
    }

    std::span<const Instance> CanvasStack::Build(const Vector2& _screenSize, TextureProvider& _textures) {
        // 同じ順番どうしは追加した順に描く
        std::stable_sort(canvases_.begin(), canvases_.end(), [](const auto& _a, const auto& _b) {
            return _a->GetOrder() < _b->GetOrder();
        });

        instances_.clear();
        for (const auto& canvas : canvases_) {
            if (canvas->IsVisible()) {
                canvas->Build(_screenSize, _textures, instances_);
            }
        }
        return instances_;
    }

    bool CanvasStack::HitTest(const Vector2& _point) {
        path_.clear();
        for (auto it = canvases_.rbegin(); it != canvases_.rend(); ++it) {
            Canvas& canvas = **it;
            if (!canvas.IsVisible()) {
                continue;
            }
            if (canvas.HitTest(_point, path_) || canvas.IsBlockingInput()) {
                return true;
            }
        }
        return false;
    }

    bool CanvasStack::Click(const Vector2& _point) {
        if (!HitTest(_point)) {
            return false;
        }

        if (const size_t index = FindNearest(path_, [](const Widget& _widget) { return _widget.selectable; }); index > 0) {
            Select(*path_[index - 1], *path_[index]);
        }
        if (const size_t index = FindNearest(path_, [](const Widget& _widget) { return _widget.raisable; }); index > 0) {
            Raise(*path_[index - 1], *path_[index]);
        }
        return true;
    }
}
