#ifndef GEUI_NAME_HPP_
#define GEUI_NAME_HPP_

#include <string_view>

#include "Tree/Widget.hpp"

namespace GEUI {
    namespace Tree {
        /** @brief Check widget name
         * @param _parent 親
         * @param _widget 親の子にある Widget
         */
        void CheckName(const Widget& _parent, Widget& _widget);

        /** @brief Fill missing names
         * @param _root 名前を付ける木
         */
        void FillNames(Widget& _root);

        /** @brief Find widget by path
         * @param _from 探し始める Widget
         * @param _path 名前を '/' で区切ったもの
         * @return widget
         */
        Widget* Find(Widget& _from, std::string_view _path);

        /** @brief Find descendant by name
         * @param _from 探し始める Widget
         * @param _name 名前
         * @return widget
         */
        Widget* FindAny(Widget& _from, std::string_view _name);
    }
}

#endif // GEUI_NAME_HPP_
