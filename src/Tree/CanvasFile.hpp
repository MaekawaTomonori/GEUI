#ifndef GEUI_CANVAS_FILE_HPP_
#define GEUI_CANVAS_FILE_HPP_

#include <filesystem>

namespace GEUI {
    namespace Tree {
        class Canvas;

        /** @brief Load canvas from file
         * @param _path パス
         * @param _canvas 出力先
         * @return succeeded
         */
        bool LoadCanvas(const std::filesystem::path& _path, Canvas& _canvas);

        /** @brief Save canvas to file
         * @param _path パス
         * @param _canvas 対象
         * @return succeeded
         */
        bool SaveCanvas(const std::filesystem::path& _path, const Canvas& _canvas);
    }
}

#endif // GEUI_CANVAS_FILE_HPP_
