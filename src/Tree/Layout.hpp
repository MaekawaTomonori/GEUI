#ifndef GEUI_LAYOUT_HPP_
#define GEUI_LAYOUT_HPP_

#include "Tree/Widget.hpp"

namespace GEUI {
    namespace Tree {
        /** @brief Place widget
         * @param _widget 対象
         * @param _rect 矩形
         * @param _parent 親の矩形
         */
        void Place(Widget& _widget, const Rect& _rect, const Rect& _parent);

        /** @brief Resolve rectangle
         * @param _widget 対象
         * @param _parent 親の矩形
         * @return rect
         */
        Rect Resolve(const Widget& _widget, const Rect& _parent);
    }
}

#endif // GEUI_LAYOUT_HPP_
