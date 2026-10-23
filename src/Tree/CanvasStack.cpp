#include "Tree/CanvasStack.hpp"

#include <algorithm>

namespace GEUI {
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
}
