#ifndef GEUI_LOG_HPP_
#define GEUI_LOG_HPP_

#include <string_view>

namespace GEUI {
    /** @brief Write message to debugger
     * @param _message UTF-8 の文
     */
    void Log(std::string_view _message);
}

#endif // GEUI_LOG_HPP_
