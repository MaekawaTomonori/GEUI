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

            // 大きさ 0 の親に対しては割合が決まらないので、0 として扱う
            Vector2 Ratio(const Vector2& _value, const Vector2& _base) {
                return { _base.x != 0.0f ? _value.x / _base.x : 0.0f, _base.y != 0.0f ? _value.y / _base.y : 0.0f };
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
                                _widget.offset = Ratio(pivotPoint - anchorPoint, _parent.size);
                            }
                        }
                    }
                }
            }

            _widget.size = Ratio(_rect.size, _parent.size);
        }

        // 親が伸びれば、アンカーを基準点にして子も同じ割合で伸びる
        Rect Resolve(const Widget& _widget, const Rect& _parent) {
            const Vector2 size = _parent.size * _widget.size;
            const Vector2 pivotPoint = _parent.min + _parent.size * _widget.anchor + _parent.size * _widget.offset;
            return { pivotPoint - size * _widget.pivot, size };
        }
    }
}
