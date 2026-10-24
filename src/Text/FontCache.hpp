#ifndef GEUI_FONT_CACHE_HPP_
#define GEUI_FONT_CACHE_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Text/Font.hpp"

namespace GEUI {
    /** @brief Fonts by path
     */
    class FontCache {
        ID3D12Device* device_ = nullptr;
        TextureProvider* textures_ = nullptr;
        std::unordered_map<std::string, std::unique_ptr<Font>> fonts_;

    public:
        /** @brief Initialize
         * @param _device デバイス
         * @param _textures テクスチャ
         */
        void Setup(ID3D12Device* _device, TextureProvider* _textures);

        /** @brief Get font
         * @param _path パス
         * @return font
         */
        const Font* Get(const std::string& _path);

        /** @brief Upload new atlases
         * @param _commandList コマンドリスト
         */
        void Upload(ID3D12GraphicsCommandList* _commandList);

        /** @brief Get loaded font paths
         * @param _paths 出力先
         */
        void GetPaths(std::vector<std::string>& _paths) const;

        /** @brief Release
         */
        void Release();
    };
}

#endif // GEUI_FONT_CACHE_HPP_
