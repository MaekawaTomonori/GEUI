#include "Backend.hpp"

namespace GEUI {
    bool Backend::Setup(ID3D12Device* _device, DXGI_FORMAT _rtvFormat) {
        if (_device == nullptr || _rtvFormat == DXGI_FORMAT_UNKNOWN) {
            return false;
        }

        device_ = _device;
        rtvFormat_ = _rtvFormat;
        return true;
    }

    void Backend::Render([[maybe_unused]] ID3D12GraphicsCommandList* _commandList) {
    }

    void Backend::Teardown() {
        device_ = nullptr;
        rtvFormat_ = DXGI_FORMAT_UNKNOWN;
    }
}
