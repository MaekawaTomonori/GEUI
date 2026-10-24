#include "Text/Font.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>

// ホストも stb_truetype を持つことがあるので、関数をこのファイルの中だけにする
// 使わない static 関数の警告はファイルの最後で出るため、push の外で止める
#pragma warning(disable : 4505)
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#pragma warning(push, 0)
#include "stb/stb_truetype.h"
#pragma warning(pop)

namespace GEUI {
    namespace {
        constexpr int ATLAS_SIZE = 512;
        constexpr float PIXEL_SIZE = 40.0f;
        constexpr int PADDING = 4;
        // Quad.PS.hlsl の GLYPH_EDGE と同じ値にする
        constexpr unsigned char ON_EDGE = 180;
        constexpr float DISTANCE_SCALE = 16.0f;
        constexpr int FIRST_CODE = 32;
        constexpr int LAST_CODE = 126;
    }

    bool Font::Load(const std::filesystem::path& _path, TextureProvider& _textures) {
        std::ifstream file(_path, std::ios::binary);
        if (!file) {
            return false;
        }
        const std::vector<unsigned char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        stbtt_fontinfo info{};
        if (!stbtt_InitFont(&info, data.data(), stbtt_GetFontOffsetForIndex(data.data(), 0))) {
            return false;
        }

        const float scale = stbtt_ScaleForPixelHeight(&info, PIXEL_SIZE);
        int ascent = 0;
        int descent = 0;
        int lineGap = 0;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
        ascent_ = static_cast<float>(ascent) * scale;
        descent_ = static_cast<float>(descent) * scale;

        pixels_.assign(static_cast<size_t>(ATLAS_SIZE) * ATLAS_SIZE, 0);
        int cursorX = 0;
        int cursorY = 0;
        int rowHeight = 0;

        for (int code = FIRST_CODE; code <= LAST_CODE; ++code) {
            const int index = stbtt_FindGlyphIndex(&info, code);
            int advance = 0;
            stbtt_GetGlyphHMetrics(&info, index, &advance, nullptr);
            Glyph& glyph = glyphs_[static_cast<size_t>(code)];
            glyph.advance = static_cast<float>(advance) * scale;

            int width = 0;
            int height = 0;
            int x = 0;
            int y = 0;
            unsigned char* sdf = stbtt_GetGlyphSDF(&info, scale, index, PADDING, ON_EDGE, DISTANCE_SCALE, &width, &height, &x, &y);
            // 空白などは形が無いので、送り幅だけを持つ
            if (sdf == nullptr) {
                continue;
            }

            if (cursorX + width > ATLAS_SIZE) {
                cursorX = 0;
                cursorY += rowHeight;
                rowHeight = 0;
            }
            if (cursorY + height > ATLAS_SIZE) {
                stbtt_FreeSDF(sdf, nullptr);
                break;
            }

            for (int row = 0; row < height; ++row) {
                std::memcpy(&pixels_[static_cast<size_t>(cursorY + row) * ATLAS_SIZE + cursorX], sdf + static_cast<size_t>(row) * width, static_cast<size_t>(width));
            }
            stbtt_FreeSDF(sdf, nullptr);

            constexpr float INVERSE = 1.0f / static_cast<float>(ATLAS_SIZE);
            glyph.u0 = static_cast<float>(cursorX) * INVERSE;
            glyph.v0 = static_cast<float>(cursorY) * INVERSE;
            glyph.u1 = static_cast<float>(cursorX + width) * INVERSE;
            glyph.v1 = static_cast<float>(cursorY + height) * INVERSE;
            glyph.x = static_cast<float>(x);
            glyph.y = static_cast<float>(y);
            glyph.width = static_cast<float>(width);
            glyph.height = static_cast<float>(height);

            cursorX += width;
            rowHeight = std::max(rowHeight, height);
        }

        // 読み込みの時点で番号を決め、同じフレームのうちに描く文字へ渡せるようにする
        return _textures.AllocateDescriptor(slot_);
    }

    void Font::Upload(ID3D12Device* _device, ID3D12GraphicsCommandList* _commandList) {
        if (uploaded_ || pixels_.empty()) {
            return;
        }

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width = ATLAS_SIZE;
        desc.Height = ATLAS_SIZE;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_R8_UNORM;
        desc.SampleDesc.Count = 1;

        D3D12_HEAP_PROPERTIES heap = {};
        heap.Type = D3D12_HEAP_TYPE_DEFAULT;
        if (FAILED(_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture_)))) {
            return;
        }

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
        UINT64 total = 0;
        _device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, nullptr, nullptr, &total);

        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = total;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        heap.Type = D3D12_HEAP_TYPE_UPLOAD;
        if (FAILED(_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload_)))) {
            return;
        }

        uint8_t* mapped = nullptr;
        const D3D12_RANGE readRange = {};
        if (FAILED(upload_->Map(0, &readRange, reinterpret_cast<void**>(&mapped)))) {
            return;
        }
        // 行の幅はアラインメントで広がるので、1行ずつ詰めて書く
        for (int row = 0; row < ATLAS_SIZE; ++row) {
            std::memcpy(mapped + footprint.Offset + static_cast<size_t>(row) * footprint.Footprint.RowPitch, &pixels_[static_cast<size_t>(row) * ATLAS_SIZE], ATLAS_SIZE);
        }
        upload_->Unmap(0, nullptr);

        D3D12_TEXTURE_COPY_LOCATION destination = {};
        destination.pResource = texture_.Get();
        destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        D3D12_TEXTURE_COPY_LOCATION source = {};
        source.pResource = upload_.Get();
        source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        source.PlacedFootprint = footprint;
        _commandList->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = texture_.Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        _commandList->ResourceBarrier(1, &barrier);

        D3D12_SHADER_RESOURCE_VIEW_DESC view = {};
        view.Format = DXGI_FORMAT_R8_UNORM;
        view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        view.Texture2D.MipLevels = 1;
        _device->CreateShaderResourceView(texture_.Get(), &view, D3D12_CPU_DESCRIPTOR_HANDLE{ static_cast<SIZE_T>(slot_.cpu) });

        // 写し終えた画素は GPU 側にあるので、CPU 側では持たない
        pixels_.clear();
        pixels_.shrink_to_fit();
        uploaded_ = true;
    }

    void Font::Release(TextureProvider& _textures) {
        if (slot_.index != UINT32_MAX) {
            _textures.FreeDescriptor(slot_.index);
            slot_ = {};
        }
        texture_.Reset();
        upload_.Reset();
        uploaded_ = false;
    }

    const Glyph& Font::Find(char _code) const {
        const auto code = static_cast<unsigned char>(_code);
        return glyphs_[code < glyphs_.size() ? code : static_cast<size_t>('?')];
    }

    float Font::GetPixelSize() const {
        return PIXEL_SIZE;
    }

    float Font::GetAscent() const {
        return ascent_;
    }

    float Font::GetDescent() const {
        return descent_;
    }

    float Font::GetWeightPerPixel() {
        return DISTANCE_SCALE / 255.0f;
    }

    uint32_t Font::GetTexIndex() const {
        return slot_.index;
    }
}
