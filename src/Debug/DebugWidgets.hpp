#ifndef GEUI_DEBUG_WIDGETS_HPP_
#define GEUI_DEBUG_WIDGETS_HPP_

#include <cstddef>

namespace GEUI {
    namespace DebugWidgets {
        /** @brief Begin collapsible group
         * @param _label 見出し
         * @return opened
         */
        bool BeginGroup(const char* _label);

        /** @brief End group opened by BeginGroup
         */
        void EndGroup();

        /** @brief Drag two floats in one row
         * @param _label ラベル
         * @param _v 値
         * @param _speed 速さ
         * @param _min 最小値
         * @param _max 最大値
         * @param _axes 軸の名前（2文字）
         * @return changed
         */
        bool DragFloat2(const char* _label, float* _v, float _speed, float _min = 0.0f, float _max = 0.0f, const char* _axes = "XY");

        /** @brief Edit color in one row
         * @param _label ラベル
         * @param _v 色
         * @return changed
         */
        bool ColorEdit4(const char* _label, float* _v);

        /** @brief Begin combo in one row
         * @param _label ラベル
         * @param _preview 表示する値
         * @param _reserve 右に空ける幅
         * @return opened
         */
        bool BeginCombo(const char* _label, const char* _preview, float _reserve = 0.0f);

        /** @brief End combo opened by BeginCombo
         */
        void EndCombo();

        /** @brief Begin row with label
         * @param _label ラベル
         */
        void Label(const char* _label);
    }
}

#endif // GEUI_DEBUG_WIDGETS_HPP_
