#ifndef GEUI_FONT_HPP_
#define GEUI_FONT_HPP_

#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

#include <d3d12.h>
#include <wrl/client.h>

#include "Backend/TextureProvider.hpp"

namespace GEUI {
    /** @brief Glyph in atlas
     */
    struct Glyph {
        float u0 = 0.0f;
        float v0 = 0.0f;
        float u1 = 0.0f;
        float v1 = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        float advance = 0.0f;
    };

    /** @brief SDF font atlas
     */
    class Font {
        std::array<Glyph, 128> glyphs_{};
        float ascent_ = 0.0f;
        float descent_ = 0.0f;
        std::vector<uint8_t> pixels_;
        Microsoft::WRL::ComPtr<ID3D12Resource> texture_;
        Microsoft::WRL::ComPtr<ID3D12Resource> upload_;
        DescriptorSlot slot_{};
        bool uploaded_ = false;

    public:
        /** @brief Bake atlas
         * @param _path パス
         * @param _textures テクスチャ
         * @return succeeded
         */
        bool Load(const std::filesystem::path& _path, TextureProvider& _textures);

        /** @brief Upload atlas
         * @param _device デバイス
         * @param _commandList コマンドリスト
         */
        void Upload(ID3D12Device* _device, ID3D12GraphicsCommandList* _commandList);

        /** @brief Release descriptor
         * @param _textures テクスチャ
         */
        void Release(TextureProvider& _textures);

        /** @brief Find glyph
         * @param _code 文字
         * @return glyph
         */
        const Glyph& Find(char _code) const;

        /** @brief Get pixel size of atlas
         * @return size
         */
        float GetPixelSize() const;

        /** @brief Get ascent
         * @return ascent
         */
        float GetAscent() const;

        /** @brief Get descent
         * @return descent
         */
        float GetDescent() const;

        /** @brief Get weight per pixel
         * @return weight
         */
        static float GetWeightPerPixel();

        /** @brief Get texture index
         * @return index
         */
        uint32_t GetTexIndex() const;
    };
}

#endif // GEUI_FONT_HPP_
