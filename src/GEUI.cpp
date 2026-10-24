#include "GEUI.hpp"

#include "Current.hpp"
#include "Debug/DebugWindow.hpp"

namespace GEUI {
    namespace {
        // ホストの WndProc やハンドルから呼ばれるため、インスタンスを渡さずに辿れるようにする
        Backend* current = nullptr;
    }

    Tree::CanvasStack* GetCurrentCanvases() {
        return current != nullptr ? current->GetCanvases() : nullptr;
    }

    bool System::Setup(ID3D12Device* _device, DXGI_FORMAT _rtvFormat, uint32_t _width, uint32_t _height, TextureProvider* _textures) {
        if (!backend_.Setup(_device, _rtvFormat, _width, _height, _textures)) {
            return false;
        }
        current = &backend_;
        return true;
    }

    void System::Resize(uint32_t _width, uint32_t _height) {
        backend_.Resize(_width, _height);
    }

    void System::Render(ID3D12GraphicsCommandList* _commandList) {
        backend_.Render(_commandList);
    }

    void System::Teardown() {
        if (current == &backend_) {
            current = nullptr;
        }
        backend_.Teardown();
    }

    bool HandleMessage(uint32_t _message, uintptr_t _wParam, intptr_t _lParam) {
        return current != nullptr && current->HandleMessage(_message, _wParam, _lParam);
    }

    void SetViewport(const ScreenRect& _rect) {
        if (current != nullptr) {
            current->SetViewport(_rect);
        }
    }

    void ResetViewport() {
        if (current != nullptr) {
            current->ResetViewport();
        }
    }

    bool IsPointerOver() {
        return current != nullptr && current->IsPointerOver();
    }

    void Debug(bool* _open) {
        if (current != nullptr && current->GetCanvases() != nullptr && current->GetTextures() != nullptr && current->GetFonts() != nullptr) {
            DrawDebugWindow(*current->GetCanvases(), *current->GetTextures(), *current->GetFonts(), _open);
        }
    }
}
