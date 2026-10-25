#include "Debug/DebugWindow.hpp"

#ifdef _DEBUG

#include <algorithm>
#include <cfloat>
#include <cstring>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "imgui/imgui.h"
#include "Utils.hpp"

#include "Backend/TextureProvider.hpp"
#include "Debug/DebugWidgets.hpp"
#include "Log.hpp"
#include "Text/FontCache.hpp"
#include "Tree/CanvasStack.hpp"
#include "Tree/Name.hpp"

namespace GEUI {
    namespace {
        constexpr const char* PAYLOAD = "GEUI_WIDGET";

        enum class Action { None, Add, Remove, Up, Down, Move };

        // 描画中に木を変えると走査が壊れるので、操作は描き終えてからまとめて行う
        struct Request {
            Action action = Action::None;
            Tree::Widget* target = nullptr;
            Tree::Widget* destination = nullptr;
            Tree::WidgetType type = Tree::WidgetType::Quad;
        };

        // 選択は番号で持ち、消えた Widget を指したままにならないようにする
        Tree::Canvas* selectedCanvas = nullptr;
        uint32_t selectedWidget = 0;
        uint32_t bufferOwner = 0;
        char nameBuffer[64] = {};
        char textBuffer[1024] = {};
        std::vector<std::string> loadedNames;
        std::vector<std::string> availableNames;
        std::vector<std::string> fontPaths;

        std::string ToText(const std::filesystem::path& _path) {
            const std::u8string text = _path.generic_u8string();
            return std::string(text.begin(), text.end());
        }

        // 選んだファイルを _directory からの相対名に直す。外にあるファイルは、読む側が解決できないので受け付けない
        bool OpenFile(const std::string& _filter, const std::filesystem::path& _directory, std::string& _name) {
            const std::filesystem::path file = Utils::OpenFileDialog(_filter);
            if (file.empty()) {
                return false;
            }

            std::error_code error;
            const std::filesystem::path root = std::filesystem::absolute(_directory, error).lexically_normal();
            if (error) {
                return false;
            }

            const std::filesystem::path relative = std::filesystem::relative(file, root, error);
            if (error || relative.empty() || *relative.begin() == "..") {
                Log(std::format("'{}' is outside '{}'", ToText(file), ToText(root)));
                return false;
            }
            _name = ToText(relative);
            return true;
        }

        bool OpenImage(const std::filesystem::path& _directory, std::string& _name) {
            if (_directory.empty()) {
                Log("texture directory is not provided");
                return false;
            }
            // フィルタは '\0' 区切りなので、途中で切れないよう長さ付きで渡す
            using namespace std::string_literals;
            return OpenFile("Image\0*.png;*.jpg;*.jpeg;*.dds;*.tga\0"s, _directory, _name);
        }

        // フォントのパスは、Canvas のファイルと同じくカレントディレクトリからの相対で持つ
        bool OpenFont(std::string& _name) {
            using namespace std::string_literals;
            return OpenFile("Font\0*.ttf;*.otf\0"s, std::filesystem::current_path(), _name);
        }

        float ButtonWidth(const char* _label) {
            const ImGuiStyle& style = ImGui::GetStyle();
            return ImGui::CalcTextSize(_label).x + style.FramePadding.x * 2.0f + style.ItemSpacing.x;
        }

        void DrawFont(FontCache& _fonts, Tree::TextStyle& _style) {
            if (DebugWidgets::BeginCombo("Font", _style.font.c_str(), ButtonWidth("..."))) {
                if (ImGui::IsWindowAppearing()) {
                    _fonts.GetPaths(fontPaths);
                }
                for (const std::string& path : fontPaths) {
                    if (ImGui::Selectable(path.c_str(), path == _style.font)) {
                        _style.font = path;
                    }
                }
                DebugWidgets::EndCombo();
            }

            ImGui::SameLine();
            if (std::string name; ImGui::Button("...##font") && OpenFont(name)) {
                _style.font = name;
            }
        }

        void DrawTextStyle(FontCache& _fonts, Tree::TextStyle& _style) {
            if (DebugWidgets::InputTextMultiline("Text", textBuffer, sizeof(textBuffer))) {
                _style.text = textBuffer;
            }
            DrawFont(_fonts, _style);
            DebugWidgets::DragFloat("Size", &_style.size, 0.5f, 1.0f, 512.0f);
            DebugWidgets::DragFloat("Weight", &_style.weight, 0.05f, -4.0f, 4.0f);
            DebugWidgets::DragFloat("Spacing", &_style.spacing, 0.1f);
            DebugWidgets::DragFloat("Line", &_style.lineHeight, 0.01f, 0.1f, 8.0f);

            int alignX = static_cast<int>(_style.alignX);
            if (DebugWidgets::Combo("Align X", &alignX, "Left\0Center\0Right\0")) {
                _style.alignX = static_cast<Tree::TextAlign>(alignX);
            }
            int alignY = static_cast<int>(_style.alignY);
            if (DebugWidgets::Combo("Align Y", &alignY, "Top\0Middle\0Bottom\0")) {
                _style.alignY = static_cast<Tree::TextAlign>(alignY);
            }
        }

        void DrawImage(TextureProvider& _textures, std::string& _image) {
            // 右に「...」ボタンを並べる分だけ、コンボボックスを縮める
            if (DebugWidgets::BeginCombo("Image", _image.c_str(), ButtonWidth("..."))) {
                // ディスクを読む一覧もあるので、開いたときだけ取り直す
                if (ImGui::IsWindowAppearing()) {
                    loadedNames.clear();
                    availableNames.clear();
                    _textures.GetLoadedNames(loadedNames);
                    _textures.GetAvailableNames(availableNames);
                    std::erase_if(availableNames, [](const std::string& _name) {
                        return std::find(loadedNames.begin(), loadedNames.end(), _name) != loadedNames.end();
                    });
                }

                ImGui::SeparatorText("Loaded");
                for (const std::string& name : loadedNames) {
                    if (ImGui::Selectable(name.c_str(), name == _image)) {
                        _image = name;
                    }
                }
                ImGui::SeparatorText("Files");
                for (const std::string& name : availableNames) {
                    if (ImGui::Selectable(name.c_str(), name == _image)) {
                        _image = name;
                    }
                }
                DebugWidgets::EndCombo();
            }

            ImGui::SameLine();
            if (std::string name; ImGui::Button("...##image") && OpenImage(_textures.GetDirectory(), name)) {
                _image = name;
            }
        }

        void Copy(char* _buffer, size_t _size, const std::string& _text) {
            const size_t length = _text.copy(_buffer, _size - 1);
            _buffer[length] = '\0';
        }

        Tree::Widget* FindParent(Tree::Widget& _from, const Tree::Widget& _target) {
            for (const auto& child : _from.children) {
                if (child.get() == &_target) {
                    return &_from;
                }
                if (Tree::Widget* parent = FindParent(*child, _target)) {
                    return parent;
                }
            }
            return nullptr;
        }

        bool Contains(const Tree::Widget& _widget, const Tree::Widget& _target) {
            if (&_widget == &_target) {
                return true;
            }
            return std::any_of(_widget.children.begin(), _widget.children.end(), [&_target](const auto& _child) {
                return Contains(*_child, _target);
            });
        }

        void Move(Tree::Widget& _root, Tree::Widget& _target, Tree::Widget& _destination) {
            // 自分の子孫の下へ移すと、木から切り離されて消えてしまう
            if (Contains(_target, _destination)) {
                return;
            }
            Tree::Widget* parent = FindParent(_root, _target);
            if (parent == nullptr || parent == &_destination) {
                return;
            }

            auto& children = parent->children;
            const auto it = std::find_if(children.begin(), children.end(), [&_target](const auto& _child) { return _child.get() == &_target; });
            std::unique_ptr<Tree::Widget> moved = std::move(*it);
            children.erase(it);
            _destination.children.push_back(std::move(moved));
            Tree::CheckName(_destination, *_destination.children.back());
        }

        void Shift(Tree::Widget& _parent, const Tree::Widget& _target, bool _forward) {
            auto& children = _parent.children;
            const auto it = std::find_if(children.begin(), children.end(), [&_target](const auto& _child) { return _child.get() == &_target; });
            if (it == children.end()) {
                return;
            }
            const size_t index = static_cast<size_t>(it - children.begin());
            if (_forward && index + 1 < children.size()) {
                std::swap(children[index], children[index + 1]);
            } else if (!_forward && index > 0) {
                std::swap(children[index], children[index - 1]);
            }
        }

        Tree::Widget& AddPart(Tree::Canvas& _canvas, Tree::Widget& _button, std::string_view _name, const Vector2& _size, const Vector4& _color, const Vector4& _selectedColor) {
            Tree::Widget& part = _canvas.Create(_button, Tree::WidgetType::Quad, _name);
            part.anchor = { 0.5f, 0.5f };
            part.pivot = { 0.5f, 0.5f };
            part.size = _size;
            part.color = _color;
            part.followState = true;
            _canvas.AddSelectedStyle(part).color = _selectedColor;
            return part;
        }

        // 固定部と可動部を最初から置き、アイコンや文字は可動部の子として足してもらう
        void AddParts(Tree::Canvas& _canvas, Tree::Widget& _button) {
            _button.selectable = true;
            AddPart(_canvas, _button, "Background", { 1.0f, 1.0f }, { 0.15f, 0.15f, 0.15f, 1.0f }, { 0.25f, 0.25f, 0.25f, 1.0f });
            Tree::Widget& foreground = AddPart(_canvas, _button, "Foreground", { 0.9f, 0.8f }, { 0.4f, 0.4f, 0.4f, 1.0f }, { 0.9f, 0.6f, 0.2f, 1.0f });
            _canvas.AddSelectedStyle(foreground).scale = 1.05f;
        }

        void Apply(Tree::CanvasStack& _canvases, Tree::Canvas& _canvas, const Request& _request) {
            switch (_request.action) {
            case Action::Add: {
                Tree::Widget& widget = _canvas.Create(*_request.target, _request.type);
                widget.size = { 0.2f, 0.2f };
                if (Tree::TextStyle* text = _canvas.GetText(widget)) {
                    text->text = "Text";
                }
                if (widget.type == Tree::WidgetType::Button) {
                    AddParts(_canvas, widget);
                }
                selectedWidget = _canvases.GetId(_canvas, widget);
                break;
            }
            case Action::Remove:
                _canvases.Remove(_canvas, *_request.destination, *_request.target);
                selectedWidget = 0;
                break;
            case Action::Up:
                Shift(*_request.destination, *_request.target, false);
                break;
            case Action::Down:
                Shift(*_request.destination, *_request.target, true);
                break;
            case Action::Move:
                Move(_canvas.GetRoot(), *_request.target, *_request.destination);
                break;
            case Action::None:
                break;
            }
        }

        void AcceptDrop(Tree::CanvasStack& _canvases, Tree::Widget& _destination, Request& _request) {
            if (!ImGui::BeginDragDropTarget()) {
                return;
            }
            // 運ぶのは番号にして、ドラッグ中に消えた Widget を触らないようにする
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD)) {
                uint32_t id = 0;
                std::memcpy(&id, payload->Data, sizeof(id));
                if (Tree::Widget* target = _canvases.GetWidget(id)) {
                    _request = { Action::Move, target, &_destination };
                }
            }
            ImGui::EndDragDropTarget();
        }

        void DrawCanvases(Tree::CanvasStack& _canvases) {
            if (!ImGui::BeginTable("Canvases", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                return;
            }
            ImGui::TableSetupColumn("Canvas");
            ImGui::TableSetupColumn("Order");
            ImGui::TableSetupColumn("Visible");
            ImGui::TableSetupColumn("Block");
            ImGui::TableSetupColumn("File");
            ImGui::TableHeadersRow();

            for (const auto& canvas : _canvases.GetAll()) {
                ImGui::PushID(canvas.get());
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                const std::string path = ToText(canvas->GetPath());
                if (ImGui::Selectable(path.c_str(), canvas.get() == selectedCanvas)) {
                    selectedCanvas = canvas.get();
                    selectedWidget = 0;
                }

                ImGui::TableNextColumn();
                int order = canvas->GetOrder();
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::DragInt("##Order", &order)) {
                    canvas->SetOrder(order);
                }

                ImGui::TableNextColumn();
                bool visible = canvas->IsVisible();
                if (ImGui::Checkbox("##Visible", &visible)) {
                    canvas->SetVisible(visible);
                }

                ImGui::TableNextColumn();
                bool block = canvas->IsBlockingInput();
                if (ImGui::Checkbox("##Block", &block)) {
                    canvas->SetBlockInput(block);
                }

                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Save") && _canvases.Save(*canvas)) {
                    Log(std::format("saved '{}'", path));
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Reload") && _canvases.Reload(*canvas)) {
                    bufferOwner = 0;
                    Log(std::format("reloaded '{}'", path));
                }

                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        void DrawNode(Tree::CanvasStack& _canvases, Tree::Widget& _widget, Request& _request) {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
            if (_widget.children.empty()) {
                flags |= ImGuiTreeNodeFlags_Leaf;
            }
            if (_widget.id != 0 && _widget.id == selectedWidget) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            ImGui::PushID(&_widget);
            const bool open = ImGui::TreeNodeEx(_widget.name.c_str(), flags);
            // 描いているのは選ばれている Canvas の木だけなので、持ち主はそれになる
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                selectedWidget = _canvases.GetId(*selectedCanvas, _widget);
            }
            if (ImGui::BeginDragDropSource()) {
                const uint32_t id = _canvases.GetId(*selectedCanvas, _widget);
                ImGui::SetDragDropPayload(PAYLOAD, &id, sizeof(id));
                ImGui::TextUnformatted(_widget.name.c_str());
                ImGui::EndDragDropSource();
            }
            AcceptDrop(_canvases, _widget, _request);

            if (open) {
                for (const auto& child : _widget.children) {
                    DrawNode(_canvases, *child, _request);
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        void DrawTree(Tree::CanvasStack& _canvases, Tree::Widget& _root, Tree::Widget* _selected, Request& _request) {
            Tree::Widget* parent = _selected != nullptr ? FindParent(_root, *_selected) : nullptr;

            Tree::Widget* parentOfNew = _selected != nullptr ? _selected : &_root;
            if (ImGui::Button("Add Quad")) {
                _request = { Action::Add, parentOfNew, nullptr, Tree::WidgetType::Quad };
            }
            ImGui::SameLine();
            if (ImGui::Button("Add Image")) {
                _request = { Action::Add, parentOfNew, nullptr, Tree::WidgetType::Image };
            }
            ImGui::SameLine();
            if (ImGui::Button("Add Text")) {
                _request = { Action::Add, parentOfNew, nullptr, Tree::WidgetType::Text };
            }
            ImGui::SameLine();
            if (ImGui::Button("Add Button")) {
                _request = { Action::Add, parentOfNew, nullptr, Tree::WidgetType::Button };
            }
            ImGui::BeginDisabled(_selected == nullptr);
            ImGui::SameLine();
            if (ImGui::Button("Remove")) {
                _request = { Action::Remove, _selected, parent };
            }
            ImGui::SameLine();
            if (ImGui::Button("Up")) {
                _request = { Action::Up, _selected, parent };
            }
            ImGui::SameLine();
            if (ImGui::Button("Down")) {
                _request = { Action::Down, _selected, parent };
            }
            ImGui::EndDisabled();
            ImGui::Separator();

            ImGui::Selectable("(root)");
            AcceptDrop(_canvases, _root, _request);
            for (const auto& child : _root.children) {
                DrawNode(_canvases, *child, _request);
            }
        }

        // ビットフィールドは ImGui にポインタで渡せないので、一度 bool で受けて書き戻す
        bool Checkbox(const char* _label, bool _value) {
            ImGui::Checkbox(_label, &_value);
            return _value;
        }

        void DrawProperties(TextureProvider& _textures, FontCache& _fonts, Tree::Canvas& _canvas, Tree::Widget& _widget) {
            Tree::Widget& root = _canvas.GetRoot();
            if (bufferOwner != _widget.id) {
                Copy(nameBuffer, sizeof(nameBuffer), _widget.name);
                const Tree::TextStyle* text = _canvas.GetText(_widget);
                Copy(textBuffer, sizeof(textBuffer), text != nullptr ? text->text : std::string());
                bufferOwner = _widget.id;
            }

            // 名前と種類は、どの Widget かを示す見出しとして1行にまとめる
            const float typeWidth = ImGui::GetFontSize() * 5.0f;
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - typeWidth - ImGui::GetStyle().ItemSpacing.x);
            if (ImGui::InputText("##Name", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                Tree::Widget* parent = FindParent(root, _widget);
                if (parent == nullptr || !Tree::Rename(*parent, _widget, nameBuffer)) {
                    Copy(nameBuffer, sizeof(nameBuffer), _widget.name);
                }
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(typeWidth);
            int type = static_cast<int>(_widget.type);
            if (ImGui::Combo("##Type", &type, "Quad\0Image\0Text\0Button\0")) {
                // 種類ごとの中身は作り直しになるので、文字の入力欄も取り直す
                _canvas.SetType(_widget, static_cast<Tree::WidgetType>(type));
                bufferOwner = 0;
            }

            if (DebugWidgets::BeginGroup("Layout")) {
                DebugWidgets::DragFloat2("Anchor", &_widget.anchor.x, 0.01f, 0.0f, 1.0f);
                DebugWidgets::DragFloat2("Pivot", &_widget.pivot.x, 0.01f, 0.0f, 1.0f);
                DebugWidgets::DragFloat2("Offset", &_widget.offset.x, 0.001f);
                DebugWidgets::DragFloat2("Size", &_widget.size.x, 0.001f, 0.0f, 0.0f, "WH");
                const Tree::Rect& rect = _widget.rect;
                DebugWidgets::Label("Rect");
                ImGui::TextDisabled("%.1f, %.1f   %.1f x %.1f", rect.min.x, rect.min.y, rect.size.x, rect.size.y);
                DebugWidgets::EndGroup();
            }

            if (DebugWidgets::BeginGroup("Appearance")) {
                DebugWidgets::ColorEdit4("Color", &_widget.color.x);
                if (std::string* image = _canvas.GetImage(_widget)) {
                    DrawImage(_textures, *image);
                }
                DebugWidgets::EndGroup();
            }

            if (Tree::TextStyle* text = _canvas.GetText(_widget); text != nullptr && DebugWidgets::BeginGroup("Text")) {
                DrawTextStyle(_fonts, *text);
                DebugWidgets::EndGroup();
            }

            if (DebugWidgets::BeginGroup("Behavior")) {
                DebugWidgets::Label("Flags");
                _widget.visible = Checkbox("Visible", _widget.visible);
                ImGui::SameLine();
                _widget.selectable = Checkbox("Selectable", _widget.selectable);
                ImGui::SameLine();
                _widget.followState = Checkbox("Follow", _widget.followState);
                ImGui::SameLine();
                _widget.raisable = Checkbox("Raisable", _widget.raisable);
                DebugWidgets::EndGroup();
            }

            // フラグを切っても調整した値を失わないよう、選択中の見た目は手で足して手で消す
            if ((_widget.selectable || _widget.followState) && DebugWidgets::BeginGroup("Selected")) {
                if (_widget.selectable) {
                    DebugWidgets::Label("State");
                    _widget.selected = Checkbox("Selected", _widget.selected);
                }
                if (Tree::StateStyle* style = _canvas.GetSelectedStyle(_widget)) {
                    DebugWidgets::ColorEdit4("Color", &style->color.x);
                    DebugWidgets::DragFloat2("Shift", &style->shift.x, 0.001f);
                    DebugWidgets::DragFloat("Scale", &style->scale, 0.005f, 0.0f, 4.0f);
                    if (ImGui::SmallButton("x##selectedStyle")) {
                        _canvas.RemoveSelectedStyle(_widget);
                    }
                } else if (ImGui::SmallButton("Add Style")) {
                    _canvas.AddSelectedStyle(_widget);
                }
                DebugWidgets::EndGroup();
            }
        }
    }

    void DrawDebugWindow(Tree::CanvasStack& _canvases, TextureProvider& _textures, FontCache& _fonts, bool* _open) {
        // 同梱のヘッダとホストの ImGui の版が違うと、構造体の並びがずれて壊れる
        IMGUI_CHECKVERSION();

        if (!ImGui::Begin("GEUI", _open)) {
            ImGui::End();
            return;
        }

        DrawCanvases(_canvases);

        const auto& canvases = _canvases.GetAll();
        if (std::none_of(canvases.begin(), canvases.end(), [](const auto& _canvas) { return _canvas.get() == selectedCanvas; })) {
            selectedCanvas = nullptr;
            selectedWidget = 0;
        }

        if (selectedCanvas != nullptr) {
            Tree::Widget& root = selectedCanvas->GetRoot();
            Tree::Widget* selected = _canvases.GetWidget(selectedWidget);
            if (selected != nullptr && FindParent(root, *selected) == nullptr) {
                selected = nullptr;
            }

            Request request;
            ImGui::BeginChild("Tree", ImVec2(ImGui::GetContentRegionAvail().x * 0.4f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
            DrawTree(_canvases, root, selected, request);
            ImGui::EndChild();

            ImGui::SameLine();
            ImGui::BeginChild("Properties", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
            if (selected != nullptr) {
                DrawProperties(_textures, _fonts, *selectedCanvas, *selected);
            }
            ImGui::EndChild();

            Apply(_canvases, *selectedCanvas, request);
        }

        ImGui::End();
    }
}

#else

namespace GEUI {
    void DrawDebugWindow(Tree::CanvasStack&, TextureProvider&, FontCache&, bool*) {}
}

#endif
