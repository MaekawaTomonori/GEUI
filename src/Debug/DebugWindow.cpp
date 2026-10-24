#include "Debug/DebugWindow.hpp"

#ifdef _DEBUG

#include <algorithm>
#include <cfloat>
#include <cstring>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "imgui/imgui.h"
#include "Utils.hpp"

#include "Backend/TextureProvider.hpp"
#include "Debug/DebugWidgets.hpp"
#include "Log.hpp"
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
        std::vector<std::string> loadedNames;
        std::vector<std::string> availableNames;

        std::string ToText(const std::filesystem::path& _path) {
            const std::u8string text = _path.generic_u8string();
            return std::string(text.begin(), text.end());
        }

        bool OpenImage(const std::filesystem::path& _directory, std::string& _name) {
            if (_directory.empty()) {
                Log("texture directory is not provided");
                return false;
            }

            // フィルタは '\0' 区切りなので、途中で切れないよう長さ付きで渡す
            using namespace std::string_literals;
            const std::filesystem::path file = Utils::OpenFileDialog("Image\0*.png;*.jpg;*.jpeg;*.dds;*.tga\0"s);
            if (file.empty()) {
                return false;
            }

            std::error_code error;
            const std::filesystem::path root = std::filesystem::absolute(_directory, error).lexically_normal();
            if (error) {
                return false;
            }

            // ホストが解決できるのはテクスチャのフォルダからの相対名だけ
            const std::filesystem::path relative = std::filesystem::relative(file, root, error);
            if (error || relative.empty() || *relative.begin() == "..") {
                Log(std::format("'{}' is outside '{}'", ToText(file), ToText(root)));
                return false;
            }
            _name = ToText(relative);
            return true;
        }

        void DrawImage(TextureProvider& _textures, Tree::Widget& _widget) {
            // 右に「...」ボタンを並べる分だけ、コンボボックスを縮める
            const ImGuiStyle& style = ImGui::GetStyle();
            const float button = ImGui::CalcTextSize("...").x + style.FramePadding.x * 2.0f + style.ItemSpacing.x;
            if (DebugWidgets::BeginCombo("Image", _widget.image.c_str(), button)) {
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
                    if (ImGui::Selectable(name.c_str(), name == _widget.image)) {
                        _widget.image = name;
                    }
                }
                ImGui::SeparatorText("Files");
                for (const std::string& name : availableNames) {
                    if (ImGui::Selectable(name.c_str(), name == _widget.image)) {
                        _widget.image = name;
                    }
                }
                DebugWidgets::EndCombo();
            }

            ImGui::SameLine();
            if (std::string name; ImGui::Button("...##image") && OpenImage(_textures.GetDirectory(), name)) {
                _widget.image = name;
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

        void Apply(Tree::CanvasStack& _canvases, Tree::Canvas& _canvas, const Request& _request) {
            switch (_request.action) {
            case Action::Add: {
                Tree::Widget& widget = _canvas.Create(*_request.target, _request.type);
                widget.size = { 0.2f, 0.2f };
                selectedWidget = _canvases.GetId(widget);
                break;
            }
            case Action::Remove:
                _canvases.Remove(*_request.destination, *_request.target);
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
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                selectedWidget = _canvases.GetId(_widget);
            }
            if (ImGui::BeginDragDropSource()) {
                const uint32_t id = _canvases.GetId(_widget);
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

        void DrawProperties(TextureProvider& _textures, Tree::Widget& _root, Tree::Widget& _widget) {
            if (bufferOwner != _widget.id) {
                Copy(nameBuffer, sizeof(nameBuffer), _widget.name);
                bufferOwner = _widget.id;
            }

            // 名前と種類は、どの Widget かを示す見出しとして1行にまとめる
            const float typeWidth = ImGui::GetFontSize() * 5.0f;
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - typeWidth - ImGui::GetStyle().ItemSpacing.x);
            if (ImGui::InputText("##Name", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                Tree::Widget* parent = FindParent(_root, _widget);
                if (parent == nullptr || !Tree::Rename(*parent, _widget, nameBuffer)) {
                    Copy(nameBuffer, sizeof(nameBuffer), _widget.name);
                }
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(typeWidth);
            int type = static_cast<int>(_widget.type);
            if (ImGui::Combo("##Type", &type, "Quad\0Image\0")) {
                _widget.type = static_cast<Tree::WidgetType>(type);
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
                if (_widget.type == Tree::WidgetType::Image) {
                    DrawImage(_textures, _widget);
                }
                DebugWidgets::EndGroup();
            }

            if (DebugWidgets::BeginGroup("Behavior")) {
                DebugWidgets::Label("Flags");
                ImGui::Checkbox("Visible", &_widget.visible);
                ImGui::SameLine();
                ImGui::Checkbox("Selectable", &_widget.selectable);
                ImGui::SameLine();
                ImGui::Checkbox("Raisable", &_widget.raisable);
                if (_widget.selectable) {
                    DebugWidgets::ColorEdit4("Selected", &_widget.selectedColor.x);
                }
                DebugWidgets::EndGroup();
            }
        }
    }

    void DrawDebugWindow(Tree::CanvasStack& _canvases, TextureProvider& _textures, bool* _open) {
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
                DrawProperties(_textures, root, *selected);
            }
            ImGui::EndChild();

            Apply(_canvases, *selectedCanvas, request);
        }

        ImGui::End();
    }
}

#else

namespace GEUI {
    void DrawDebugWindow(Tree::CanvasStack&, TextureProvider&, bool*) {}
}

#endif
