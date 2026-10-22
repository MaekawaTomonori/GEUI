#ifndef GEUI_SHADERLOADER_HPP_
#define GEUI_SHADERLOADER_HPP_

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace GEUI {
    /** @brief Shader loader with cache
     */
    class ShaderLoader {
        std::filesystem::file_time_type sourceTime_ = std::filesystem::file_time_type::min();

    public:
        /** @brief Initialize
         */
        void Setup();

        /** @brief Load bytecode
         * @param _fileName ファイル名
         * @param _profile プロファイル
         * @return bytecode
         */
        std::vector<uint8_t> Load(const std::wstring& _fileName, const std::wstring& _profile) const;
    };
}

#endif // GEUI_SHADERLOADER_HPP_
