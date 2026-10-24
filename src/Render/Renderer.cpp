#include "Render/Renderer.hpp"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <vector>

#include "Render/ShaderLoader.hpp"

namespace GEUI {
    namespace {
        // 文字は1文字ごとに1つ使うので、Quad だけのときより多めに取る
        constexpr uint32_t MAX_INSTANCE_COUNT = 4096;
        constexpr UINT ROOT_SCREEN_SIZE = 0;
        constexpr UINT ROOT_INSTANCES = 1;
        constexpr UINT ROOT_TEXTURES = 2;
        constexpr UINT VERTEX_COUNT_PER_QUAD = 6;
    }

    bool Renderer::Setup(ID3D12Device* _device, DXGI_FORMAT _rtvFormat, const ShaderLoader& _shaders) {
        const std::vector<uint8_t> vertexShader = _shaders.Load(L"Quad.VS.hlsl", L"vs_6_0");
        const std::vector<uint8_t> pixelShader = _shaders.Load(L"Quad.PS.hlsl", L"ps_6_0");
        if (vertexShader.empty() || pixelShader.empty()) {
            return false;
        }

        return CreateRootSignature(_device) && CreatePipelineState(_device, _rtvFormat, vertexShader, pixelShader) && CreateInstanceBuffer(_device);
    }

    void Renderer::Resize(uint32_t _width, uint32_t _height) {
        screenSize_[0] = static_cast<float>(_width);
        screenSize_[1] = static_cast<float>(_height);
    }

    void Renderer::Draw(ID3D12GraphicsCommandList* _commandList, DescriptorHandle _textureHeap, std::span<const Instance> _instances) {
        if (_instances.empty()) {
            return;
        }

        const size_t count = std::min<size_t>(_instances.size(), MAX_INSTANCE_COUNT);

        // バッファは1本なので、前フレームの GPU 処理が終わっている前提で上書きする
        std::memcpy(mappedInstances_, _instances.data(), count * sizeof(Instance));

        _commandList->SetGraphicsRootSignature(rootSignature_.Get());
        _commandList->SetPipelineState(pipelineState_.Get());
        _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        _commandList->SetGraphicsRoot32BitConstants(ROOT_SCREEN_SIZE, 2, screenSize_, 0);
        _commandList->SetGraphicsRootShaderResourceView(ROOT_INSTANCES, instanceBuffer_->GetGPUVirtualAddress());
        _commandList->SetGraphicsRootDescriptorTable(ROOT_TEXTURES, D3D12_GPU_DESCRIPTOR_HANDLE{ _textureHeap.ptr });
        _commandList->DrawInstanced(VERTEX_COUNT_PER_QUAD, static_cast<UINT>(count), 0, 0);
    }

    bool Renderer::CreateRootSignature(ID3D12Device* _device) {
        D3D12_ROOT_PARAMETER parameters[3] = {};

        parameters[ROOT_SCREEN_SIZE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        parameters[ROOT_SCREEN_SIZE].Constants.ShaderRegister = 0;
        parameters[ROOT_SCREEN_SIZE].Constants.Num32BitValues = 2;
        parameters[ROOT_SCREEN_SIZE].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        parameters[ROOT_INSTANCES].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        parameters[ROOT_INSTANCES].Descriptor.ShaderRegister = 0;
        parameters[ROOT_INSTANCES].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        // ヒープ全体を束ね、インスタンスごとの番号で引く
        D3D12_DESCRIPTOR_RANGE textureRange = {};
        textureRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        textureRange.NumDescriptors = UINT_MAX;
        textureRange.BaseShaderRegister = 0;
        textureRange.RegisterSpace = 1;
        textureRange.OffsetInDescriptorsFromTableStart = 0;

        parameters[ROOT_TEXTURES].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        parameters[ROOT_TEXTURES].DescriptorTable.NumDescriptorRanges = 1;
        parameters[ROOT_TEXTURES].DescriptorTable.pDescriptorRanges = &textureRange;
        parameters[ROOT_TEXTURES].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        // UI は等倍前後で描くので、ミップで滲ませない
        sampler.MaxLOD = 0.0f;
        sampler.ShaderRegister = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters = static_cast<UINT>(std::size(parameters));
        desc.pParameters = parameters;
        desc.NumStaticSamplers = 1;
        desc.pStaticSamplers = &sampler;

        Microsoft::WRL::ComPtr<ID3DBlob> signature;
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        if (FAILED(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error))) {
            return false;
        }

        return SUCCEEDED(_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature_)));
    }

    bool Renderer::CreatePipelineState(ID3D12Device* _device, DXGI_FORMAT _rtvFormat, std::span<const uint8_t> _vertexShader, std::span<const uint8_t> _pixelShader) {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = rootSignature_.Get();
        desc.VS = { _vertexShader.data(), _vertexShader.size() };
        desc.PS = { _pixelShader.data(), _pixelShader.size() };

        D3D12_RENDER_TARGET_BLEND_DESC& blend = desc.BlendState.RenderTarget[0];
        blend.BlendEnable = TRUE;
        blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blend.BlendOp = D3D12_BLEND_OP_ADD;
        blend.SrcBlendAlpha = D3D12_BLEND_ONE;
        blend.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blend.LogicOp = D3D12_LOGIC_OP_NOOP;
        blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        desc.RasterizerState.DepthClipEnable = TRUE;
        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets = 1;
        desc.RTVFormats[0] = _rtvFormat;
        desc.SampleDesc.Count = 1;

        return SUCCEEDED(_device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState_)));
    }

    bool Renderer::CreateInstanceBuffer(ID3D12Device* _device) {
        D3D12_HEAP_PROPERTIES heap = {};
        heap.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width = sizeof(Instance) * MAX_INSTANCE_COUNT;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        if (FAILED(_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&instanceBuffer_)))) {
            return false;
        }

        // CPU からは読まないので範囲は空
        const D3D12_RANGE readRange = {};
        return SUCCEEDED(instanceBuffer_->Map(0, &readRange, reinterpret_cast<void**>(&mappedInstances_)));
    }
}
