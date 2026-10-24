#include "Tree/Layout.hpp"

#include <limits>

namespace GEUI {
    namespace Tree {
        namespace {
            // 親も Widget も、補助線で4分割した頂点（9点）を候補にする
            constexpr float STEPS[] = { 0.0f, 0.5f, 1.0f };

            float DistanceSquared(const Vector2& _a, const Vector2& _b) {
                const Vector2 d = _a - _b;
                return d.x * d.x + d.y * d.y;
            }
        }

        void Place(Widget& _widget, const Rect& _rect, const Rect& _parent) {
            float nearest = std::numeric_limits<float>::max();

            for (float ay : STEPS) {
                for (float ax : STEPS) {
                    const Vector2 anchor = { ax, ay };
                    const Vector2 anchorPoint = _parent.min + _parent.size * anchor;

                    for (float py : STEPS) {
                        for (float px : STEPS) {
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
}
