#ifndef GEUI_TEXTURE_PROVIDER_HPP_
#define GEUI_TEXTURE_PROVIDER_HPP_

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace GEUI {
    /** @brief GPU descriptor handle
     */
    struct DescriptorHandle {
        uint64_t ptr = 0;
    };

    /** @brief Texture resolver
     */
    class TextureProvider {
    public:
        virtual ~TextureProvider() = default;

        /** @brief Get descriptor heap
         * @return heapStart
         */
        virtual DescriptorHandle GetHeap() const = 0;

        /** @brief Resolve texture
         * @param _name テクスチャ名
         * @return index
         */
        virtual uint32_t Resolve(std::string_view _name) = 0;

        /** @brief Get loaded texture names
         * @param _names 出力先
         */
        virtual void GetLoadedNames([[maybe_unused]] std::vector<std::string>& _names) const {}

        /** @brief Get available texture names
         * @param _names 出力先
         */
        virtual void GetAvailableNames([[maybe_unused]] std::vector<std::string>& _names) const {}

        /** @brief Get texture directory
         * @return directory
         */
        virtual std::filesystem::path GetDirectory() const { return {}; }
    };
}

#endif // GEUI_TEXTURE_PROVIDER_HPP_
