#ifndef GEUI_BACKEND_HPP_
#define GEUI_BACKEND_HPP_

#include <dxgiformat.h>

struct ID3D12Device;
struct ID3D12GraphicsCommandList;

namespace GEUI {
    class Backend {
        ID3D12Device* device_ = nullptr;
        DXGI_FORMAT rtvFormat_ = DXGI_FORMAT_UNKNOWN;

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

#endif // GEUI_BACKEND_HPP_
