#ifndef GEUI_HPP_
#define GEUI_HPP_

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
         * @return 引数が不正なら false
         */
        bool Setup(ID3D12Device* _device, DXGI_FORMAT _rtvFormat);

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
