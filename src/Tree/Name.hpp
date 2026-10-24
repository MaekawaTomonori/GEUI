#ifndef GEUI_NAME_HPP_
#define GEUI_NAME_HPP_

#include <string_view>

#include "Tree/Widget.hpp"

namespace GEUI {
    namespace Tree {
        /** @brief Check widget name
         * @param _parent 親
         * @param _widget 対象
         */
        void CheckName(const Widget& _parent, Widget& _widget);

        /** @brief Rename widget
         * @param _parent 親
         * @param _widget 対象
         * @param _name 名前
         * @return renamed
         */
        bool Rename(const Widget& _parent, Widget& _widget, std::string_view _name);

        /** @brief Fill missing names
         * @param _root 対象の木
         */
        void FillNames(Widget& _root);

        /** @brief Find widget by path
         * @param _from 起点
         * @param _path パス
         * @return widget
         */
        Widget* Find(Widget& _from, std::string_view _path);

        /** @brief Find descendant by name
         * @param _from 起点
         * @param _name 名前
         * @return widget
         */
        Widget* FindAny(Widget& _from, std::string_view _name);
    }
}

#endif // GEUI_NAME_HPP_
