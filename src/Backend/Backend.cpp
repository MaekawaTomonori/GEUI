#include "Backend/Backend.hpp"

#include <memory>
#include <utility>

#include <Windows.h>

#include "Render/Renderer.hpp"
#include "Render/ShaderLoader.hpp"
#include "Tree/CanvasStack.hpp"

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

        renderer_ = std::move(renderer);
        canvases_ = std::make_unique<Tree::CanvasStack>();
        textures_ = _textures;
        Resize(_width, _height);
        return true;
    }

    void Backend::Resize(uint32_t _width, uint32_t _height) {
        // 最小化中は 0 が来るので、前のサイズを保つ
        if (renderer_ == nullptr || _width == 0 || _height == 0) {
            return;
        }
        width_ = _width;
        height_ = _height;
        renderer_->Resize(_width, _height);
    }

    void Backend::Render(ID3D12GraphicsCommandList* _commandList) {
        if (renderer_ == nullptr || _commandList == nullptr) {
            return;
        }

        const Vector2 screenSize = { static_cast<float>(width_), static_cast<float>(height_) };
        renderer_->Draw(_commandList, textures_->GetHeap(), canvases_->Build(screenSize, *textures_));
    }

    bool Backend::HandleMessage(uint32_t _message, [[maybe_unused]] uintptr_t _wParam, intptr_t _lParam) {
        if (canvases_ == nullptr) {
            return false;
        }
        if (_message != WM_MOUSEMOVE && _message != WM_LBUTTONDOWN) {
            return false;
        }

        // 座標は符号付き 16bit で入っている（マルチモニタでは負になる）
        const float clientX = static_cast<float>(static_cast<int16_t>(_lParam & 0xFFFF));
        const float clientY = static_cast<float>(static_cast<int16_t>((_lParam >> 16) & 0xFFFF));

        // 表示先が設定されていなければ、描画先がそのままウィンドウに出ているとみなす
        const ScreenRect viewport = hasViewport_ ? viewport_ : ScreenRect{ 0.0f, 0.0f, static_cast<float>(width_), static_cast<float>(height_) };
        const float viewportWidth = viewport.right - viewport.left;
        const float viewportHeight = viewport.bottom - viewport.top;
        if (viewportWidth <= 0.0f || viewportHeight <= 0.0f) {
            pointerOver_ = false;
            return false;
        }

        const Vector2 ratio = { (clientX - viewport.left) / viewportWidth, (clientY - viewport.top) / viewportHeight };
        if (ratio.x < 0.0f || ratio.y < 0.0f || ratio.x >= 1.0f || ratio.y >= 1.0f) {
            pointerOver_ = false;
            return false;
        }

        const Vector2 point = { ratio.x * static_cast<float>(width_), ratio.y * static_cast<float>(height_) };

        if (_message == WM_LBUTTONDOWN) {
            pointerOver_ = canvases_->Click(point);
            return pointerOver_;
        }

        pointerOver_ = canvases_->HitTest(point);
        return false;
    }

    void Backend::SetViewport(const ScreenRect& _rect) {
        viewport_ = _rect;
        hasViewport_ = true;
    }

    void Backend::ResetViewport() {
        hasViewport_ = false;
    }

    bool Backend::IsPointerOver() const {
        return pointerOver_;
    }

    Tree::CanvasStack* Backend::GetCanvases() {
        return canvases_.get();
    }

    void Backend::Teardown() {
        canvases_.reset();
        renderer_.reset();
        textures_ = nullptr;
        hasViewport_ = false;
        pointerOver_ = false;
    }
}
