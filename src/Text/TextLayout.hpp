#ifndef GEUI_TEXT_LAYOUT_HPP_
#define GEUI_TEXT_LAYOUT_HPP_

#include <vector>

#include "Render/Instance.hpp"
#include "Tree/Widget.hpp"

namespace GEUI {
    class Font;

    /** @brief Append glyph instances
     * @param _widget 対象
     * @param _font フォント
     * @param _scale 基準解像度からの倍率
     * @param _instances 出力先
     */
    void AppendText(const Tree::Widget& _widget, const Font& _font, float _scale, std::vector<Instance>& _instances);
}

#endif // GEUI_TEXT_LAYOUT_HPP_
