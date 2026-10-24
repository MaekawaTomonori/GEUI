#include "Text/TextLayout.hpp"

#include <algorithm>
#include <string_view>

#include "Text/Font.hpp"

namespace GEUI {
    namespace {
        float AlignFactor(Tree::TextAlign _align) {
            switch (_align) {
            case Tree::TextAlign::Center:
                return 0.5f;
            case Tree::TextAlign::End:
                return 1.0f;
            case Tree::TextAlign::Start:
                break;
            }
            return 0.0f;
        }

        // 揃えるには行の幅が先に要るので、置く前に一度だけ測る
        float MeasureLine(std::string_view _line, const Font& _font, float _glyphScale, float _spacing) {
            float width = 0.0f;
            for (const char code : _line) {
                width += _font.Find(code).advance * _glyphScale + _spacing;
            }
            return _line.empty() ? 0.0f : width - _spacing;
        }
    }

    void AppendText(const Tree::Widget& _widget, const Font& _font, float _scale, std::vector<Instance>& _instances) {
        const Tree::TextStyle& style = _widget.text;
        const float pixelSize = style.size * _scale;
        if (pixelSize <= 0.0f) {
            return;
        }
        const float glyphScale = pixelSize / _font.GetPixelSize();
        const float spacing = style.spacing * _scale;
        const float lineAdvance = pixelSize * style.lineHeight;
        const float lineBox = (_font.GetAscent() - _font.GetDescent()) * glyphScale;

        const std::string_view text = style.text;
        const auto lineCount = static_cast<float>(std::count(text.begin(), text.end(), '\n') + 1);
        const float height = lineAdvance * (lineCount - 1.0f) + lineBox;

        const Tree::Rect& rect = _widget.rect;
        float baseline = rect.min.y + (rect.size.y - height) * AlignFactor(style.alignY) + _font.GetAscent() * glyphScale;

        Instance instance{};
        instance.color = Tree::GetDrawColor(_widget);
        instance.texIndex = _font.GetTexIndex();
        instance.kind = KIND_GLYPH;
        // 太さは画面の px で指定するので、アトラス上の距離に直す
        instance.weight = style.weight / glyphScale * Font::GetWeightPerPixel();

        size_t begin = 0;
        while (begin <= text.size()) {
            const size_t end = std::min(text.find('\n', begin), text.size());
            const std::string_view line = text.substr(begin, end - begin);

            float pen = rect.min.x + (rect.size.x - MeasureLine(line, _font, glyphScale, spacing)) * AlignFactor(style.alignX);
            for (const char code : line) {
                const Glyph& glyph = _font.Find(code);
                if (glyph.width > 0.0f) {
                    const Vector2 halfSize = { glyph.width * glyphScale * 0.5f, glyph.height * glyphScale * 0.5f };
                    instance.center = { pen + glyph.x * glyphScale + halfSize.x, baseline + glyph.y * glyphScale + halfSize.y };
                    instance.halfSize = halfSize;
                    instance.uvRect = { glyph.u0, glyph.v0, glyph.u1, glyph.v1 };
                    _instances.push_back(instance);
                }
                pen += glyph.advance * glyphScale + spacing;
            }

            baseline += lineAdvance;
            begin = end + 1;
        }
    }
}
