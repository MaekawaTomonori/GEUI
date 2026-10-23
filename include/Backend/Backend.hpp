#ifndef GEUI_BACKEND_HPP_
#define GEUI_BACKEND_HPP_

#include <cstdint>
#include <memory>

#include <dxgiformat.h>

#include "Backend/TextureProvider.hpp"

struct ID3D12Device;
struct ID3D12GraphicsCommandList;

namespace GEUI {
    class CanvasStack;
    class Renderer;

    /** @brief Rectangle in client coordinates
     */
    struct ScreenRect {
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;
    };

    class Backend {
        std::unique_ptr<Renderer> renderer_;
        std::unique_ptr<CanvasStack> canvases_;
        TextureProvider* textures_ = nullptr;
        uint32_t width_ = 0;
        uint32_t height_ = 0;
        ScreenRect viewport_{};
        bool hasViewport_ = false;
        bool pointerOver_ = false;

    public:
        Backend();
        ~Backend();

        /** @brief Initialize
         * @param _device 描画に使うデバイス
         * @param _rtvFormat レンダーターゲットのフォーマット
         * @param _width 描画先の幅
         * @param _height 描画先の高さ
         * @param _textures テクスチャの解決先
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

        /** @brief Handle window message
         * @param _message メッセージ
         * @param _wParam WPARAM
         * @param _lParam LPARAM
         * @return consumed
         */
        bool HandleMessage(uint32_t _message, uintptr_t _wParam, intptr_t _lParam);

        /** @brief Set viewport
         * @param _rect 描画先を表示している矩形
         */
        void SetViewport(const ScreenRect& _rect);

        /** @brief Reset viewport
         */
        void ResetViewport();

        /** @brief Check pointer over UI
         * @return over
         */
        bool IsPointerOver() const;

        /** @brief Release
         */
        void Teardown();
    };
}

#endif // GEUI_BACKEND_HPP_
