#include "Debug/DebugWidgets.hpp"

#ifdef _DEBUG

#include <cfloat>

#include "imgui/imgui.h"

namespace GEUI {
    namespace DebugWidgets {
        namespace {
            // ラベルの列は全グループで同じ幅にして、値の列の左端を揃える
            constexpr float LABEL_WIDTH = 5.5f;

            bool DragAxis(const char* _id, char _axis, float* _v, float _speed, float _min, float _max, float _width) {
                char format[] = "X %.3f";
                format[0] = _axis;
                ImGui::SetNextItemWidth(_width);
                return ImGui::DragFloat(_id, _v, _speed, _min, _max, format);
            }
        }

        bool BeginGroup(const char* _label) {
            if (!ImGui::CollapsingHeader(_label, ImGuiTreeNodeFlags_DefaultOpen)) {
                return false;
            }
            if (!ImGui::BeginTable(_label, 2, ImGuiTableFlags_SizingFixedFit)) {
                return false;
            }
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * LABEL_WIDTH);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            return true;
        }

        void EndGroup() {
            ImGui::EndTable();
        }

        void Label(const char* _label) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(_label);
            ImGui::TableNextColumn();
        }

        // ラベルを "##" と連結すると毎回文字列を確保するので、ID は PushID で分ける
        bool DragFloat(const char* _label, float* _v, float _speed, float _min, float _max) {
            Label(_label);
            ImGui::PushID(_label);
            ImGui::SetNextItemWidth(-FLT_MIN);
            const bool changed = ImGui::DragFloat("##", _v, _speed, _min, _max);
            ImGui::PopID();
            return changed;
        }

        bool Combo(const char* _label, int* _current, const char* _items) {
            Label(_label);
            ImGui::PushID(_label);
            ImGui::SetNextItemWidth(-FLT_MIN);
            const bool changed = ImGui::Combo("##", _current, _items);
            ImGui::PopID();
            return changed;
        }

        bool InputTextMultiline(const char* _label, char* _buffer, size_t _size) {
            Label(_label);
            ImGui::PushID(_label);
            const bool changed = ImGui::InputTextMultiline("##", _buffer, _size, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3.0f));
            ImGui::PopID();
            return changed;
        }

        bool DragFloat2(const char* _label, float* _v, float _speed, float _min, float _max, const char* _axes) {
            Label(_label);
            ImGui::PushID(_label);
            const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            const float width = (ImGui::GetContentRegionAvail().x - spacing) * 0.5f;
            bool changed = DragAxis("##x", _axes[0], &_v[0], _speed, _min, _max, width);
            ImGui::SameLine(0.0f, spacing);
            changed |= DragAxis("##y", _axes[1], &_v[1], _speed, _min, _max, width);
            ImGui::PopID();
            return changed;
        }

        bool ColorEdit4(const char* _label, float* _v) {
            Label(_label);
            ImGui::PushID(_label);
            ImGui::SetNextItemWidth(-FLT_MIN);
            const bool changed = ImGui::ColorEdit4("##", _v);
            ImGui::PopID();
            return changed;
        }

        // 開いている間は現在のウィンドウがポップアップになるので、ID は EndCombo の後で戻す
        bool BeginCombo(const char* _label, const char* _preview, float _reserve) {
            Label(_label);
            ImGui::PushID(_label);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - _reserve);
            const bool opened = ImGui::BeginCombo("##", _preview);
            if (!opened) {
                ImGui::PopID();
            }
            return opened;
        }

        void EndCombo() {
            ImGui::EndCombo();
            ImGui::PopID();
        }
    }
}

#endif
