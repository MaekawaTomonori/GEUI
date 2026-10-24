#include "Backend/Backend.hpp"

#include <filesystem>
#include <format>
#include <memory>
#include <string_view>
#include <system_error>
#include <utility>

#include <Windows.h>

#include "Log.hpp"
#include "Render/Renderer.hpp"
#include "Render/ShaderLoader.hpp"
#include "Tree/CanvasFile.hpp"
#include "Tree/CanvasStack.hpp"
#include "Tree/Layout.hpp"

namespace GEUI {
    namespace {
        constexpr Vector2 REFERENCE_SIZE = { 1280.0f, 720.0f };

        // 確認用の状態。公開ヘッダに出さないためここに置く
        Canvas* testPause = nullptr;

        Widget& AddWidget(Canvas& _canvas, Widget& _parent, const Rect& _parentRect, const Rect& _rect, const Vector4& _color, std::string_view _name = {}, const char* _image = "") {
            Widget& widget = _canvas.Create(_parent, _name);
            Place(widget, _rect, _parentRect);
            widget.color = _color;
            widget.image = _image;
            return widget;
        }

        void BuildTestHud(Canvas& _canvas) {
            Widget& root = _canvas.GetRoot();
            const Rect screen = _canvas.GetReferenceRect();

            // 左上: 背景 → 画像 → 半透明の重なりの順で描かれること
            const Rect panel = { { 40.0f, 40.0f }, { 320.0f, 160.0f } };
            Widget& panelWidget = AddWidget(_canvas, root, screen, panel, { 1.0f, 0.0f, 0.0f, 1.0f }, "Panel");
            AddWidget(_canvas, panelWidget, panel, { { 60.0f, 60.0f }, { 120.0f, 120.0f } }, { 1.0f, 1.0f, 1.0f, 1.0f }, "Image", "uvChecker.png");
            AddWidget(_canvas, panelWidget, panel, { { 140.0f, 100.0f }, { 200.0f, 80.0f } }, { 0.0f, 0.0f, 1.0f, 0.5f }, "Overlay");

            // 上の中央・中央・右下: それぞれの基準点に付いていくこと。名前は自動で付ける
            AddWidget(_canvas, root, screen, { { 540.0f, 20.0f }, { 200.0f, 40.0f } }, { 1.0f, 1.0f, 0.0f, 1.0f });
            AddWidget(_canvas, root, screen, { { 560.0f, 300.0f }, { 160.0f, 120.0f } }, { 1.0f, 1.0f, 1.0f, 0.8f });

            // 右下: Pause の表示中はクリックしても水色にならないこと
            Widget& corner = AddWidget(_canvas, root, screen, { { 1000.0f, 560.0f }, { 240.0f, 120.0f } }, { 0.0f, 1.0f, 0.0f, 1.0f }, "Corner");
            corner.selectable = true;
            corner.selectedColor = { 0.0f, 1.0f, 1.0f, 1.0f };
        }

        void BuildTestPause(Canvas& _canvas) {
            Widget& root = _canvas.GetRoot();
            const Rect screen = _canvas.GetReferenceRect();

            // 描画先が sRGB なので、0.5 では暗さがほとんど分からない
            AddWidget(_canvas, root, screen, screen, { 0.0f, 0.0f, 0.0f, 0.85f }, "Curtain");

            const Rect panel = { { 440.0f, 210.0f }, { 400.0f, 300.0f } };
            Widget& panelWidget = AddWidget(_canvas, root, screen, panel, { 0.3f, 0.3f, 0.3f, 1.0f }, "Panel");

            // 左: 押した項目だけが黄色になること
            constexpr std::string_view ITEMS[] = { "Resume", "Retry", "Quit" };
            for (int i = 0; i < 3; ++i) {
                const float top = 230.0f + 90.0f * static_cast<float>(i);
                Widget& item = AddWidget(_canvas, panelWidget, panel, { { 460.0f, top }, { 160.0f, 70.0f } }, { 0.6f, 0.6f, 0.6f, 1.0f }, ITEMS[i]);
                item.selectable = true;
                item.selectedColor = { 1.0f, 0.8f, 0.0f, 1.0f };
            }

            // 右: 押したパネルが手前に来ること。左の項目は位置が変わらないこと
            AddWidget(_canvas, panelWidget, panel, { { 640.0f, 240.0f }, { 150.0f, 120.0f } }, { 1.0f, 0.3f, 0.3f, 1.0f }, "Red").raisable = true;
            AddWidget(_canvas, panelWidget, panel, { { 670.0f, 330.0f }, { 150.0f, 120.0f } }, { 0.3f, 0.5f, 1.0f, 1.0f }, "Blue").raisable = true;
        }

        void BuildTestHidden(Canvas& _canvas) {
            AddWidget(_canvas, _canvas.GetRoot(), _canvas.GetReferenceRect(), _canvas.GetReferenceRect(), { 1.0f, 0.0f, 1.0f, 1.0f });
        }

        // ファイルが無ければコードで組み立てて書き出し、次の起動からはファイルを読む
        Canvas& LoadTestCanvas(CanvasStack& _canvases, const char* _path, int _order, bool _visible, bool _blockInput, void (*_build)(Canvas&)) {
            Canvas& canvas = _canvases.Add(REFERENCE_SIZE);
            // 読めなかったときに書き直すと、手で直した内容が消えるので書き出さない
            std::error_code error;
            if (std::filesystem::exists(_path, error)) {
                LoadCanvas(_path, canvas);
            } else {
                canvas.SetOrder(_order);
                canvas.SetVisible(_visible);
                canvas.SetBlockInput(_blockInput);
                _build(canvas);
                SaveCanvas(_path, canvas);
            }
            return canvas;
        }

        void CheckFind(Canvas& _canvas, std::string_view _path, bool _any) {
            const Widget* widget = _any ? _canvas.FindAny(_path) : _canvas.Find(_path);
            Log(std::format("{}(\"{}\") {}", _any ? "FindAny" : "Find", _path, widget != nullptr ? "found" : "not found"));
        }

        // 描画と入力の確認用。Editor ができたら削除する
        void BuildTestCanvases(CanvasStack& _canvases) {
            // 並べ替えを確かめるため、手前のものから追加する
            Canvas& pause = LoadTestCanvas(_canvases, "Assets/UI/TestPause.json", 100, true, true, BuildTestPause);
            testPause = &pause;
            LoadTestCanvas(_canvases, "Assets/UI/TestHidden.json", 50, false, false, BuildTestHidden);
            Canvas& hud = LoadTestCanvas(_canvases, "Assets/UI/TestHud.json", 0, true, false, BuildTestHud);

            CheckFind(pause, "Panel/Resume", false);
            CheckFind(pause, "Resume", false);
            CheckFind(pause, "Quit", true);
            CheckFind(hud, "Panel/Image", false);
            CheckFind(hud, "Widget_02", true);
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

    bool Backend::HandleMessage(uint32_t _message, [[maybe_unused]] uintptr_t _wParam, intptr_t _lParam) {
        if (canvases_ == nullptr) {
            return false;
        }
        if (_message != WM_MOUSEMOVE && _message != WM_LBUTTONDOWN && _message != WM_RBUTTONDOWN) {
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

        // 入力の確認用。Pause の表示を切り替える
        if (_message == WM_RBUTTONDOWN && testPause != nullptr) {
            testPause->SetVisible(!testPause->IsVisible());
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

    void Backend::Teardown() {
        testPause = nullptr;
        canvases_.reset();
        renderer_.reset();
        textures_ = nullptr;
        hasViewport_ = false;
        pointerOver_ = false;
    }
}
