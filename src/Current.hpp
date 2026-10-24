#ifndef GEUI_CURRENT_HPP_
#define GEUI_CURRENT_HPP_

namespace GEUI {
    namespace Tree {
        class CanvasStack;
    }

    /** @brief Get canvases of current system
     * @return canvases
     */
    Tree::CanvasStack* GetCurrentCanvases();
}

#endif // GEUI_CURRENT_HPP_
