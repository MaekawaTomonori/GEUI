#include "Tree/Name.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>

#include "Log.hpp"

namespace GEUI {
    namespace {
        // 種類ができるまでは、すべて同じ種類として数える
        constexpr std::string_view TYPE = "Widget";

        bool IsValidName(std::string_view _name) {
            if (_name.empty()) {
                return false;
            }
            // ゲームのデータの id と同じ規則にする。'/' はパスの区切りに使う
            return std::all_of(_name.begin(), _name.end(), [](char _c) {
                return (_c >= 'a' && _c <= 'z') || (_c >= 'A' && _c <= 'Z') || (_c >= '0' && _c <= '9') || _c == '_';
            });
        }

        void FindMaxNumber(const Widget& _widget, std::string_view _prefix, uint32_t& _max) {
            for (const auto& child : _widget.children) {
                const std::string_view name = child->name;
                if (name.size() > _prefix.size() && name.starts_with(_prefix)) {
                    const std::string_view digits = name.substr(_prefix.size());
                    uint32_t number = 0;
                    const auto [end, error] = std::from_chars(digits.data(), digits.data() + digits.size(), number);
                    if (error == std::errc() && end == digits.data() + digits.size()) {
                        _max = std::max(_max, number);
                    }
                }
                FindMaxNumber(*child, _prefix, _max);
            }
        }

        // 消した番号は詰めない。名前が変わるとゲーム側の文字列が別の Widget を指すため
        std::string MakeName(const Widget& _root) {
            const std::string prefix = std::string(TYPE) + "_";
            uint32_t max = 0;
            FindMaxNumber(_root, prefix, max);
            return std::format("{}{:02}", prefix, max + 1);
        }

        void Assign(const Widget& _root, Widget& _widget) {
            for (const auto& child : _widget.children) {
                if (child->name.empty()) {
                    child->name = MakeName(_root);
                }
                Assign(_root, *child);
            }
        }
    }

    void CheckName(const Widget& _parent, Widget& _widget) {
        if (_widget.name.empty()) {
            return;
        }
        if (!IsValidName(_widget.name)) {
            Log(std::format("invalid name '{}' is replaced", _widget.name));
            _widget.name.clear();
            return;
        }

        for (const auto& sibling : _parent.children) {
            if (sibling.get() != &_widget && sibling->name == _widget.name) {
                Log(std::format("duplicate name '{}'", _widget.name));
                return;
            }
        }
    }

    void FillNames(Widget& _root) {
        Assign(_root, _root);
    }
}
