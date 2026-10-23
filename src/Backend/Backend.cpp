#include "Backend/Backend.hpp"

#include <memory>
#include <utility>

#include "Render/Renderer.hpp"
#include "Render/ShaderLoader.hpp"
#include "Tree/CanvasStack.hpp"
#include "Tree/Layout.hpp"

namespace GEUI {
    namespace {
        constexpr Vector2 REFERENCE_SIZE = { 1280.0f, 720.0f };

        Widget& AddWidget(Widget& _parent, const Rect& _parentRect, const Rect& _rect, const Vector4& _color, const char* _image = "") {
            auto widget = std::make_unique<Widget>();
            Place(*widget, _rect, _parentRect);
            widget->color = _color;
            widget->image = _image;
            _parent.children.push_back(std::move(widget));
            return *_parent.children.back();
        }

        void BuildTestHud(Canvas& _canvas) {
            Widget& root = _canvas.GetRoot();
            const Rect screen = _canvas.GetReferenceRect();

            // 左上: 背景 → 画像 → 半透明の重なりの順で描かれること
            const Rect panel = { { 40.0f, 40.0f }, { 320.0f, 160.0f } };
            Widget& panelWidget = AddWidget(root, screen, panel, { 1.0f, 0.0f, 0.0f, 1.0f });
            AddWidget(panelWidget, panel, { { 60.0f, 60.0f }, { 120.0f, 120.0f } }, { 1.0f, 1.0f, 1.0f, 1.0f }, "uvChecker.png");
            AddWidget(panelWidget, panel, { { 140.0f, 100.0f }, { 200.0f, 80.0f } }, { 0.0f, 0.0f, 1.0f, 0.5f });

            // 上の中央・中央・右下: それぞれの基準点に付いていくこと
            AddWidget(root, screen, { { 540.0f, 20.0f }, { 200.0f, 40.0f } }, { 1.0f, 1.0f, 0.0f, 1.0f });
            AddWidget(root, screen, { { 560.0f, 300.0f }, { 160.0f, 120.0f } }, { 1.0f, 1.0f, 1.0f, 0.8f });
            AddWidget(root, screen, { { 1000.0f, 560.0f }, { 240.0f, 120.0f } }, { 0.0f, 1.0f, 0.0f, 1.0f });
        }

        void BuildTestPause(Canvas& _canvas) {
            Widget& root = _canvas.GetRoot();
            const Rect screen = _canvas.GetReferenceRect();

            // 描画先が sRGB なので、0.5 では暗さがほとんど分からない
            AddWidget(root, screen, screen, { 0.0f, 0.0f, 0.0f, 0.85f });
            AddWidget(root, screen, { { 440.0f, 210.0f }, { 400.0f, 300.0f } }, { 0.3f, 0.3f, 0.3f, 1.0f });
        }

        // 描画の確認用。ファイルから読めるようになったら削除する
        void BuildTestCanvases(CanvasStack& _canvases) {
            // 並べ替えを確かめるため、手前のものから追加する
            Canvas& pause = _canvases.Add(REFERENCE_SIZE);
            pause.SetOrder(100);
            BuildTestPause(pause);

            Canvas& hidden = _canvases.Add(REFERENCE_SIZE);
            hidden.SetOrder(50);
            hidden.SetVisible(false);
            AddWidget(hidden.GetRoot(), hidden.GetReferenceRect(), hidden.GetReferenceRect(), { 1.0f, 0.0f, 1.0f, 1.0f });

            Canvas& hud = _canvases.Add(REFERENCE_SIZE);
            hud.SetOrder(0);
            BuildTestHud(hud);
        }
    }

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

        auto canvases = std::make_unique<CanvasStack>();
        BuildTestCanvases(*canvases);

        renderer_ = std::move(renderer);
        canvases_ = std::move(canvases);
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

    void Backend::Teardown() {
        canvases_.reset();
        renderer_.reset();
        textures_ = nullptr;
    }
}
