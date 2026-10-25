#ifndef GEUI_POOL_HPP_
#define GEUI_POOL_HPP_

#include <cstdint>
#include <utility>
#include <vector>

namespace GEUI {
    namespace Tree {
        /** @brief Storage referenced by index
         */
        template <typename T>
        class Pool {
            // 0 番は「無し」を表すので、空けたまま使わない
            std::vector<T> items_ = std::vector<T>(1);
            std::vector<uint32_t> free_;

        public:
            /** @brief Add item
             * @param _item 値
             * @return index
             */
            uint32_t Add(T _item = {}) {
                if (!free_.empty()) {
                    const uint32_t index = free_.back();
                    free_.pop_back();
                    items_[index] = std::move(_item);
                    return index;
                }
                items_.push_back(std::move(_item));
                return static_cast<uint32_t>(items_.size() - 1);
            }

            /** @brief Remove item
             * @param _index 番号
             */
            void Remove(uint32_t& _index) {
                if (_index == 0 || _index >= items_.size()) {
                    return;
                }
                // 文字列などの中身を手放し、空きの番号として覚えておく
                items_[_index] = T{};
                free_.push_back(_index);
                _index = 0;
            }

            /** @brief Get item
             * @param _index 番号
             * @return item
             */
            T* Get(uint32_t _index) {
                return _index != 0 && _index < items_.size() ? &items_[_index] : nullptr;
            }

            /** @brief Get item
             * @param _index 番号
             * @return item
             */
            const T* Get(uint32_t _index) const {
                return _index != 0 && _index < items_.size() ? &items_[_index] : nullptr;
            }
        };
    }
}

#endif // GEUI_POOL_HPP_
