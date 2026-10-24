#ifndef GEUI_NAME_HPP_
#define GEUI_NAME_HPP_

#include "Tree/Widget.hpp"

namespace GEUI {
    /** @brief Check widget name
     * @param _parent 親
     * @param _widget 親の子にある Widget
     */
    void CheckName(const Widget& _parent, Widget& _widget);

    /** @brief Fill missing names
     * @param _root 名前を付ける木
     */
    void FillNames(Widget& _root);
}

#endif // GEUI_NAME_HPP_
