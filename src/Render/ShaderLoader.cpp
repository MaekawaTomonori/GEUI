#include "Render/ShaderLoader.hpp"

#include <fstream>
#include <iterator>
#include <system_error>

#include <Windows.h>
#include <wrl/client.h>
#include <dxcapi.h>

#pragma comment(lib, "dxcompiler.lib")

namespace GEUI {
    namespace {
        constexpr const wchar_t* SOURCE_DIRECTORY = GEUI_SHADER_SOURCE_DIRECTORY;
        constexpr const wchar_t* CACHE_DIRECTORY = L"resources/shader";

        // 作業フォルダは起動の仕方で変わるので、キャッシュは exe の隣に置く
        std::filesystem::path ExecutableDirectory() {
            std::wstring path(MAX_PATH, L'\0');
            const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
            path.resize(length);
            return std::filesystem::path(path).parent_path();
        }

        std::vector<uint8_t> ReadFile(const std::filesystem::path& _path) {
            std::ifstream file(_path, std::ios::binary);
            if (!file) {
                return {};
            }
            return std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        }

        // キャッシュは無くても動くので、書けなくても続ける
        void WriteFile(const std::filesystem::path& _path, const std::vector<uint8_t>& _data) {
            std::error_code error;
            std::filesystem::create_directories(_path.parent_path(), error);

            std::ofstream file(_path, std::ios::binary);
            file.write(reinterpret_cast<const char*>(_data.data()), static_cast<std::streamsize>(_data.size()));
        }

        std::vector<uint8_t> Compile(const std::wstring& _fileName, const std::wstring& _profile) {
            Microsoft::WRL::ComPtr<IDxcUtils> utils;
            Microsoft::WRL::ComPtr<IDxcCompiler3> compiler;
            Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
            if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils))) || FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))) || FAILED(utils->CreateDefaultIncludeHandler(&includeHandler))) {
                return {};
            }

            const std::filesystem::path sourcePath = std::filesystem::path(SOURCE_DIRECTORY) / _fileName;
            Microsoft::WRL::ComPtr<IDxcBlobEncoding> source;
            if (FAILED(utils->LoadFile(sourcePath.c_str(), nullptr, &source))) {
                return {};
            }

            const DxcBuffer buffer = { source->GetBufferPointer(), source->GetBufferSize(), DXC_CP_UTF8 };
            LPCWSTR arguments[] = {
                _fileName.c_str(),
                L"-E", L"main",
                L"-T", _profile.c_str(),
                L"-I", SOURCE_DIRECTORY,
#ifdef _DEBUG
                L"-Zi", L"-Qembed_debug", L"-Od",
#else
                L"-O3",
#endif
            };

            Microsoft::WRL::ComPtr<IDxcResult> result;
            if (FAILED(compiler->Compile(&buffer, arguments, static_cast<UINT32>(std::size(arguments)), includeHandler.Get(), IID_PPV_ARGS(&result)))) {
                return {};
            }

            Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
            result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
            if (errors != nullptr && errors->GetStringLength() != 0) {
                OutputDebugStringA(errors->GetStringPointer());
            }

            HRESULT status = S_OK;
            if (FAILED(result->GetStatus(&status)) || FAILED(status)) {
                return {};
            }

            Microsoft::WRL::ComPtr<IDxcBlob> object;
            if (FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&object), nullptr)) || object == nullptr) {
                return {};
            }

            const auto* bytes = static_cast<const uint8_t*>(object->GetBufferPointer());
            return std::vector<uint8_t>(bytes, bytes + object->GetBufferSize());
        }
    }

    void ShaderLoader::Setup() {
        // ソースが無いときは min のままにして、どのキャッシュも新しいとみなす
        sourceTime_ = std::filesystem::file_time_type::min();
        cacheDirectory_ = ExecutableDirectory() / CACHE_DIRECTORY;

        std::error_code error;
        for (std::filesystem::directory_iterator it(SOURCE_DIRECTORY, error), end; !error && it != end; it.increment(error)) {
            const std::filesystem::path extension = it->path().extension();
            if (extension != L".hlsl" && extension != L".hlsli") {
                continue;
            }

            const std::filesystem::file_time_type time = it->last_write_time(error);
            if (!error && sourceTime_ < time) {
                sourceTime_ = time;
            }
        }
    }

    std::vector<uint8_t> ShaderLoader::Load(const std::wstring& _fileName, const std::wstring& _profile) const {
        const std::filesystem::path cachePath = cacheDirectory_ / std::filesystem::path(_fileName).replace_extension(L".cso");

        std::error_code error;
        const std::filesystem::file_time_type cacheTime = std::filesystem::last_write_time(cachePath, error);
        if (!error && sourceTime_ <= cacheTime) {
            std::vector<uint8_t> bytecode = ReadFile(cachePath);
            if (!bytecode.empty()) {
                return bytecode;
            }
        }

        std::vector<uint8_t> bytecode = Compile(_fileName, _profile);
        if (!bytecode.empty()) {
            WriteFile(cachePath, bytecode);
        }
        return bytecode;
    }
}
