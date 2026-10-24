#include "Log.hpp"

#include <string>

#include <Windows.h>

namespace GEUI {
    void Log(std::string_view _message) {
        const std::string text = "[GEUI] : " + std::string(_message) + "\n";

        // パスに日本語が入っても化けないよう、ワイド文字で出す
        const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
        if (length <= 0) {
            return;
        }
        std::wstring wide(static_cast<size_t>(length), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), wide.data(), length);
        OutputDebugStringW(wide.c_str());
    }
}
