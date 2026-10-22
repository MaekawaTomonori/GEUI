#ifndef GEUI_HPP_
#define GEUI_HPP_

#include <cstdint>

#include "Backend/Backend.hpp"

/**
 * @brief GEUI: DirectX12 向けのリテインド型UIライブラリ
 */
namespace GEUI {
    class System {
        Backend backend_;

    public:
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

        /** @brief Release
         */
        void Teardown();
    };
}

#endif // GEUI_HPP_
