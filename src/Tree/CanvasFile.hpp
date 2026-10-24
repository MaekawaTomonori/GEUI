#ifndef GEUI_CANVAS_FILE_HPP_
#define GEUI_CANVAS_FILE_HPP_

#include <filesystem>

namespace GEUI {
    class Canvas;

    /** @brief Load canvas from file
     * @param _path 読むファイル
     * @param _canvas 中身を置き換える Canvas
     * @return succeeded
     */
    bool LoadCanvas(const std::filesystem::path& _path, Canvas& _canvas);

    /** @brief Save canvas to file
     * @param _path 書くファイル
     * @param _canvas 書き出す Canvas
     * @return succeeded
     */
    bool SaveCanvas(const std::filesystem::path& _path, const Canvas& _canvas);
}

#endif // GEUI_CANVAS_FILE_HPP_
