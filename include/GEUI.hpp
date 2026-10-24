#ifndef GEUI_HPP_
#define GEUI_HPP_

#include <cstdint>

#include "Backend/Backend.hpp"
#include "Canvas.hpp"
#include "Widget.hpp"

/**
 * @brief GEUI: DirectX12 向けのリテインド型UIライブラリ
 */
namespace GEUI {
    class System {
        Backend backend_;

    public:
        /** @brief Initialize
         * @param _device デバイス
         * @param _rtvFormat レンダーターゲットのフォーマット
         * @param _width 描画先の幅
         * @param _height 描画先の高さ
         * @param _textures テクスチャ
         * @return succeeded
         */
        bool Setup(ID3D12Device* _device, DXGI_FORMAT _rtvFormat, uint32_t _width, uint32_t _height, TextureProvider* _textures);

        /** @brief Resize
         * @param _width 描画先の幅
         * @param _height 描画先の高さ
         */
        void Resize(uint32_t _width, uint32_t _height);

        /** @brief Render UI
         * @param _commandList コマンドリスト
         */
        void Render(ID3D12GraphicsCommandList* _commandList);

        /** @brief Release
         */
        void Teardown();
    };

    /** @brief Handle window message
     * @param _message メッセージ
     * @param _wParam WPARAM
     * @param _lParam LPARAM
     * @return consumed
     */
    bool HandleMessage(uint32_t _message, uintptr_t _wParam, intptr_t _lParam);

    /** @brief Set viewport
     * @param _rect 表示先の矩形
     */
    void SetViewport(const ScreenRect& _rect);

    /** @brief Reset viewport
     */
    void ResetViewport();

    /** @brief Check pointer over UI
     * @return over
     */
    bool IsPointerOver();

    /** @brief Draw debug window
     * @param _open 開閉の状態
     */
    void Debug(bool* _open = nullptr);
}

#endif // GEUI_HPP_
