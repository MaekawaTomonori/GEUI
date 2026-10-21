#ifndef GEUI_BACKEND_HPP_
#define GEUI_BACKEND_HPP_

#include <dxgiformat.h>

struct ID3D12Device;
struct ID3D12GraphicsCommandList;

namespace GEUI {
    /** @brief ホストがGEUIを組み込むための窓口 */
    class Backend {
        ID3D12Device* device_ = nullptr;
        DXGI_FORMAT rtvFormat_ = DXGI_FORMAT_UNKNOWN;

    public:
        /** @brief GEUIを初期化する */
        bool Setup(ID3D12Device* _device, DXGI_FORMAT _rtvFormat);

        /** @brief UIを描画する */
        void Render(ID3D12GraphicsCommandList* _commandList);

        /** @brief GEUIを終了する */
        void Teardown();
    };
}

#endif // GEUI_BACKEND_HPP_
