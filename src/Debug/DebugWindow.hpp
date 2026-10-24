#ifndef GEUI_DEBUG_WINDOW_HPP_
#define GEUI_DEBUG_WINDOW_HPP_

namespace GEUI {
    class TextureProvider;

    namespace Tree {
        class CanvasStack;
    }

    /** @brief Draw debug window
     * @param _canvases 対象
     * @param _textures テクスチャ
     * @param _open 開閉の状態
     */
    void DrawDebugWindow(Tree::CanvasStack& _canvases, TextureProvider& _textures, bool* _open);
}

#endif // GEUI_DEBUG_WINDOW_HPP_
