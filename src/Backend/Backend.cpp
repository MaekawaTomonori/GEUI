#include "Backend/Backend.hpp"

#include <memory>
#include <utility>

#include "Render/Instance.hpp"
#include "Render/Renderer.hpp"
#include "Render/ShaderLoader.hpp"

namespace GEUI {
    Backend::Backend() = default;
    Backend::~Backend() = default;

    bool Backend::Setup(ID3D12Device* _device, DXGI_FORMAT _rtvFormat, uint32_t _width, uint32_t _height, TextureProvider* _textures) {
        if (_device == nullptr || _rtvFormat == DXGI_FORMAT_UNKNOWN || _width == 0 || _height == 0 || _textures == nullptr) {
            return false;
        }

        ShaderLoader shaders;
        shaders.Setup();

        auto renderer = std::make_unique<Renderer>();
        if (!renderer->Setup(_device, _rtvFormat, shaders)) {
            return false;
        }
        renderer->Resize(_width, _height);

        renderer_ = std::move(renderer);
        textures_ = _textures;
        return true;
    }

    void Backend::Resize(uint32_t _width, uint32_t _height) {
        // 最小化中は 0 が来るので、前のサイズを保つ
        if (renderer_ == nullptr || _width == 0 || _height == 0) {
            return;
        }
        renderer_->Resize(_width, _height);
    }

    void Backend::Render(ID3D12GraphicsCommandList* _commandList) {
        if (renderer_ == nullptr || _commandList == nullptr) {
            return;
        }

        // 描画の確認用。Widget ツリーができたら削除する
        const Instance instances[] = {
            { { 200.0f, 200.0f }, { 100.0f, 100.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 260.0f, 260.0f }, { 100.0f, 100.0f }, { 0.0f, 0.0f, 1.0f, 0.5f } },
            { { 420.0f, 200.0f }, { 100.0f, 100.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, textures_->Resolve("uvChecker.png") },
        };
        renderer_->Draw(_commandList, textures_->GetHeap(), instances);
    }

    void Backend::Teardown() {
        renderer_.reset();
        textures_ = nullptr;
    }
}
