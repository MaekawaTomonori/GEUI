#include "GEUI.hpp"

namespace GEUI {
    bool System::Setup(ID3D12Device* _device, DXGI_FORMAT _rtvFormat, uint32_t _width, uint32_t _height, TextureProvider* _textures) {
        return backend_.Setup(_device, _rtvFormat, _width, _height, _textures);
    }

    void System::Resize(uint32_t _width, uint32_t _height) {
        backend_.Resize(_width, _height);
    }

    void System::Render(ID3D12GraphicsCommandList* _commandList) {
        backend_.Render(_commandList);
    }

    void System::Teardown() {
        backend_.Teardown();
    }
}
