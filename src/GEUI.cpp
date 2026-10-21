#include "GEUI.hpp"

namespace GEUI {
    bool System::Setup(ID3D12Device* _device, DXGI_FORMAT _rtvFormat) {
        return backend_.Setup(_device, _rtvFormat);
    }

    void System::Render(ID3D12GraphicsCommandList* _commandList) {
        backend_.Render(_commandList);
    }

    void System::Teardown() {
        backend_.Teardown();
    }
}
