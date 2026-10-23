#include "Tree/Layout.hpp"

#include <limits>

namespace GEUI {
    namespace {
        constexpr float ANCHOR_STEPS[] = { 0.0f, 0.5f, 1.0f };
        constexpr float PIVOT_STEPS[] = { 0.0f, 1.0f };

        float DistanceSquared(const Vector2& _a, const Vector2& _b) {
            const Vector2 d = _a - _b;
            return d.x * d.x + d.y * d.y;
        }
    }

    void Place(Widget& _widget, const Rect& _rect, const Rect& _parent) {
        float nearest = std::numeric_limits<float>::max();

        // 補助線で4分割した頂点（9点）と、Widget の頂点のうち最も近い組を選ぶ
        for (float ay : ANCHOR_STEPS) {
            for (float ax : ANCHOR_STEPS) {
                const Vector2 anchor = { ax, ay };
                const Vector2 anchorPoint = _parent.min + _parent.size * anchor;

                for (float py : PIVOT_STEPS) {
                    for (float px : PIVOT_STEPS) {
                        const Vector2 pivot = { px, py };
                        const Vector2 pivotPoint = _rect.min + _rect.size * pivot;

                        const float distance = DistanceSquared(anchorPoint, pivotPoint);
                        if (distance < nearest) {
                            nearest = distance;
                            _widget.anchor = anchor;
                            _widget.pivot = pivot;
                            _widget.offset = pivotPoint - anchorPoint;
                        }
                    }
                }
            }
        }

        _widget.size = _rect.size;
    }

    Rect Resolve(const Widget& _widget, const Rect& _parent, float _scale) {
        const Vector2 size = _widget.size * _scale;
        const Vector2 pivotPoint = _parent.min + _parent.size * _widget.anchor + _widget.offset * _scale;
        return { pivotPoint - size * _widget.pivot, size };
    }
}
