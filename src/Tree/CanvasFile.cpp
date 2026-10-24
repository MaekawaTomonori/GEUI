#include "Tree/CanvasFile.hpp"

#include <format>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <system_error>
#include <utility>

#include <nlohmann/json.hpp>

#include "Log.hpp"
#include "Tree/Canvas.hpp"
#include "Tree/Name.hpp"

namespace GEUI {
    namespace {
        // 書き出す順を保ち、人が読んだときに children が最後に来るようにする
        using Json = nlohmann::ordered_json;

        std::string ToText(const std::filesystem::path& _path) {
            const std::u8string text = _path.u8string();
            return std::string(text.begin(), text.end());
        }

        // 例外を使わないため、型を確かめてから取り出す。合わなければ既定値のまま
        void Read(const Json& _json, const char* _key, float& _value) {
            const auto it = _json.find(_key);
            if (it != _json.end() && it->is_number()) {
                _value = it->get<float>();
            }
        }

        void Read(const Json& _json, const char* _key, int& _value) {
            const auto it = _json.find(_key);
            if (it != _json.end() && it->is_number_integer()) {
                _value = it->get<int>();
            }
        }

        void Read(const Json& _json, const char* _key, bool& _value) {
            const auto it = _json.find(_key);
            if (it != _json.end() && it->is_boolean()) {
                _value = it->get<bool>();
            }
        }

        void Read(const Json& _json, const char* _key, std::string& _value) {
            const auto it = _json.find(_key);
            if (it != _json.end() && it->is_string()) {
                _value = it->get<std::string>();
            }
        }

        bool ReadFloats(const Json& _json, const char* _key, float* _values, size_t _count) {
            const auto it = _json.find(_key);
            if (it == _json.end() || !it->is_array() || it->size() != _count) {
                return false;
            }
            for (size_t i = 0; i < _count; ++i) {
                if (!(*it)[i].is_number()) {
                    return false;
                }
            }
            for (size_t i = 0; i < _count; ++i) {
                _values[i] = (*it)[i].get<float>();
            }
            return true;
        }

        void Read(const Json& _json, const char* _key, Vector2& _value) {
            float values[2] = {};
            if (ReadFloats(_json, _key, values, 2)) {
                _value = { values[0], values[1] };
            }
        }

        void Read(const Json& _json, const char* _key, Vector4& _value) {
            float values[4] = {};
            if (ReadFloats(_json, _key, values, 4)) {
                _value = { values[0], values[1], values[2], values[3] };
            }
        }

        void ReadChildren(const Json& _json, Widget& _parent) {
            const auto it = _json.find("children");
            if (it == _json.end() || !it->is_array()) {
                return;
            }

            for (const Json& item : *it) {
                if (!item.is_object()) {
                    continue;
                }

                auto widget = std::make_unique<Widget>();
                Read(item, "name", widget->name);
                Read(item, "anchor", widget->anchor);
                Read(item, "pivot", widget->pivot);
                Read(item, "offset", widget->offset);
                Read(item, "size", widget->size);
                Read(item, "color", widget->color);
                Read(item, "selectedColor", widget->selectedColor);
                Read(item, "image", widget->image);
                Read(item, "visible", widget->visible);
                Read(item, "selectable", widget->selectable);
                Read(item, "raisable", widget->raisable);
                ReadChildren(item, *widget);

                _parent.children.push_back(std::move(widget));
                CheckName(_parent, *_parent.children.back());
            }
        }

        Json Write(const Vector2& _value) {
            return Json::array({ _value.x, _value.y });
        }

        Json Write(const Vector4& _value) {
            return Json::array({ _value.x, _value.y, _value.z, _value.w });
        }

        Json WriteChildren(const Widget& _parent) {
            Json children = Json::array();
            for (const auto& child : _parent.children) {
                Json item;
                item["name"] = child->name;
                item["anchor"] = Write(child->anchor);
                item["pivot"] = Write(child->pivot);
                item["offset"] = Write(child->offset);
                item["size"] = Write(child->size);
                item["color"] = Write(child->color);
                item["selectedColor"] = Write(child->selectedColor);
                item["image"] = child->image;
                item["visible"] = child->visible;
                item["selectable"] = child->selectable;
                item["raisable"] = child->raisable;
                item["children"] = WriteChildren(*child);
                children.push_back(std::move(item));
            }
            return children;
        }
    }

    bool LoadCanvas(const std::filesystem::path& _path, Canvas& _canvas) {
        std::ifstream file(_path, std::ios::binary);
        if (!file) {
            Log(std::format("cannot open '{}'", ToText(_path)));
            return false;
        }
        const std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        const Json json = Json::parse(text, nullptr, false);
        if (json.is_discarded() || !json.is_object()) {
            Log(std::format("cannot parse '{}'", ToText(_path)));
            return false;
        }

        Vector2 referenceSize{};
        int order = 0;
        bool visible = true;
        bool blockInput = false;
        Read(json, "referenceSize", referenceSize);
        Read(json, "order", order);
        Read(json, "visible", visible);
        Read(json, "blockInput", blockInput);
        if (referenceSize.x <= 0.0f || referenceSize.y <= 0.0f) {
            Log(std::format("'{}' has no referenceSize", ToText(_path)));
        }

        Widget root;
        ReadChildren(json, root);

        _canvas.Setup(referenceSize);
        _canvas.SetOrder(order);
        _canvas.SetVisible(visible);
        _canvas.SetBlockInput(blockInput);
        _canvas.GetRoot().children = std::move(root.children);

        // 書かれた名前の番号をすべて見てから、空いている番号を付ける
        FillNames(_canvas.GetRoot());
        return true;
    }

    bool SaveCanvas(const std::filesystem::path& _path, const Canvas& _canvas) {
        Json json;
        json["referenceSize"] = Write(_canvas.GetReferenceRect().size);
        json["order"] = _canvas.GetOrder();
        json["visible"] = _canvas.IsVisible();
        json["blockInput"] = _canvas.IsBlockingInput();
        json["children"] = WriteChildren(_canvas.GetRoot());

        // 不正な UTF-8 で例外を投げないよう、置き換えて書く
        const std::string text = json.dump(4, ' ', false, Json::error_handler_t::replace);

        std::error_code error;
        if (_path.has_parent_path()) {
            std::filesystem::create_directories(_path.parent_path(), error);
        }

        std::ofstream file(_path, std::ios::binary);
        if (!file) {
            Log(std::format("cannot write '{}'", ToText(_path)));
            return false;
        }
        file.write(text.data(), static_cast<std::streamsize>(text.size()));
        return static_cast<bool>(file);
    }
}
