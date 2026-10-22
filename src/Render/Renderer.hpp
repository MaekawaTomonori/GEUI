#ifndef GEUI_RENDERER_HPP_
#define GEUI_RENDERER_HPP_

#include <cstdint>
#include <span>

#include <d3d12.h>
#include <wrl/client.h>

#include "Backend/TextureProvider.hpp"
#include "Render/Instance.hpp"

namespace GEUI {
    class ShaderLoader;

    /** @brief Quad renderer
     */
    class Renderer {
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
        Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer_;
        Instance* mappedInstances_ = nullptr;
        float screenSize_[2] = {};

    public:
        /** @brief Initialize
         * @param _device 描画に使うデバイス
         * @param _rtvFormat レンダーターゲットのフォーマット
         * @param _shaders シェーダの読み込み元
         * @return succeeded
         */
        bool Setup(ID3D12Device* _device, DXGI_FORMAT _rtvFormat, const ShaderLoader& _shaders);

        /** @brief Resize
         * @param _width 描画先の幅
         * @param _height 描画先の高さ
         */
        void Resize(uint32_t _width, uint32_t _height);

        /** @brief Draw instances
         * @param _commandList コマンドリスト
         * @param _textureHeap テクスチャのヒープの先頭
         * @param _instances インスタンス
         */
        void Draw(ID3D12GraphicsCommandList* _commandList, DescriptorHandle _textureHeap, std::span<const Instance> _instances);

    private:
        bool CreateRootSignature(ID3D12Device* _device);
        bool CreatePipelineState(ID3D12Device* _device, DXGI_FORMAT _rtvFormat, std::span<const uint8_t> _vertexShader, std::span<const uint8_t> _pixelShader);
        bool CreateInstanceBuffer(ID3D12Device* _device);
    };
}

#endif // GEUI_RENDERER_HPP_
