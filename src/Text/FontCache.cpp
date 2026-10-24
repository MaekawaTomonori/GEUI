#include "Text/FontCache.hpp"

#include <algorithm>
#include <format>

#include "Log.hpp"

namespace GEUI {
    void FontCache::Setup(ID3D12Device* _device, TextureProvider* _textures) {
        device_ = _device;
        textures_ = _textures;
    }

    const Font* FontCache::Get(const std::string& _path) {
        if (_path.empty() || textures_ == nullptr) {
            return nullptr;
        }
        if (const auto it = fonts_.find(_path); it != fonts_.end()) {
            return it->second.get();
        }

        // 読めなかったものも覚えて、毎フレーム読み直さないようにする
        auto font = std::make_unique<Font>();
        if (!font->Load(_path, *textures_)) {
            Log(std::format("cannot load font '{}'", _path));
            font->Release(*textures_);
            font.reset();
        }
        return fonts_.emplace(_path, std::move(font)).first->second.get();
    }

    void FontCache::Upload(ID3D12GraphicsCommandList* _commandList) {
        for (const auto& [path, font] : fonts_) {
            if (font != nullptr) {
                font->Upload(device_, _commandList);
            }
        }
    }

    void FontCache::GetPaths(std::vector<std::string>& _paths) const {
        _paths.clear();
        for (const auto& [path, font] : fonts_) {
            if (font != nullptr) {
                _paths.push_back(path);
            }
        }
        std::sort(_paths.begin(), _paths.end());
    }

    void FontCache::Release() {
        for (const auto& [path, font] : fonts_) {
            if (font != nullptr && textures_ != nullptr) {
                font->Release(*textures_);
            }
        }
        fonts_.clear();
        device_ = nullptr;
        textures_ = nullptr;
    }
}
