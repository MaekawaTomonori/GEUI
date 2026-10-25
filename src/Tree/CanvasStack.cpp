#include "Tree/CanvasStack.hpp"

#include <algorithm>
#include <utility>

#include "Tree/CanvasFile.hpp"

namespace GEUI {
    namespace Tree {
        namespace {
            // _path は親から子の順。当たった Widget に最も近い条件を満たす祖先を探す
            template <typename Predicate>
            size_t FindNearest(const std::vector<Widget*>& _path, Predicate _predicate) {
                for (size_t i = _path.size(); i-- > 1;) {
                    if (_predicate(*_path[i])) {
                        return i;
                    }
                }
                return 0;
            }

            // 入れ替えできない兄弟の位置は変えず、入れ替えできる兄弟の間でだけ末尾へ送る
            void Raise(Widget& _parent, const Widget& _target) {
                auto& children = _parent.children;
                size_t current = children.size();
                for (size_t i = 0; i < children.size(); ++i) {
                    if (children[i].get() == &_target) {
                        current = i;
                    } else if (current < children.size() && children[i]->raisable) {
                        std::swap(children[current], children[i]);
                        current = i;
                    }
                }
            }

            std::string JoinPath(const std::string& _prefix, const std::string& _name) {
                return _prefix.empty() ? _name : _prefix + "/" + _name;
            }

            void CollectIds(const Widget& _widget, const std::string& _prefix, std::unordered_map<std::string, uint32_t>& _ids) {
                for (const auto& child : _widget.children) {
                    const std::string path = JoinPath(_prefix, child->name);
                    if (child->id != 0) {
                        _ids.emplace(path, child->id);
                    }
                    CollectIds(*child, path, _ids);
                }
            }
        }

        uint32_t CanvasStack::Load(const std::filesystem::path& _path) {
            auto canvas = std::make_unique<Canvas>();
            if (!LoadCanvas(_path, *canvas)) {
                return 0;
            }
            canvas->SetPath(_path);

            // 番号は使い回さない。古いハンドルが別の Canvas を指さないようにする
            const uint32_t id = nextId_++;
            canvasIds_[id] = canvas.get();
            canvases_.push_back(std::move(canvas));
            return id;
        }

        void CanvasStack::Unload(uint32_t _id) {
            const auto it = canvasIds_.find(_id);
            if (it == canvasIds_.end()) {
                return;
            }

            Canvas* canvas = it->second;
            Forget(canvas->GetRoot());
            canvasIds_.erase(it);
            path_.clear();
            std::erase_if(canvases_, [canvas](const auto& _canvas) { return _canvas.get() == canvas; });
        }

        bool CanvasStack::Save(const Canvas& _canvas) {
            return SaveCanvas(_canvas.GetPath(), _canvas);
        }

        bool CanvasStack::Reload(Canvas& _canvas) {
            Canvas loaded;
            if (!LoadCanvas(_canvas.GetPath(), loaded)) {
                return false;
            }

            // 同じパスにある Widget へ前の番号を付け直し、ゲームが持つハンドルを読み直し後も使えるようにする
            std::unordered_map<std::string, uint32_t> ids;
            CollectIds(_canvas.GetRoot(), {}, ids);
            Forget(_canvas.GetRoot());
            path_.clear();

            loaded.SetPath(_canvas.GetPath());
            _canvas = std::move(loaded);
            Restore(_canvas, _canvas.GetRoot(), {}, ids);
            return true;
        }

        void CanvasStack::Remove(Canvas& _canvas, Widget& _parent, Widget& _widget) {
            Forget(_widget);
            path_.clear();
            _canvas.Release(_widget);
            std::erase_if(_parent.children, [&_widget](const auto& _child) { return _child.get() == &_widget; });
        }

        const std::vector<std::unique_ptr<Canvas>>& CanvasStack::GetAll() const {
            return canvases_;
        }

        Canvas* CanvasStack::GetCanvas(uint32_t _id) {
            const auto it = canvasIds_.find(_id);
            return it != canvasIds_.end() ? it->second : nullptr;
        }

        Widget* CanvasStack::GetWidget(uint32_t _id) {
            const auto it = widgetIds_.find(_id);
            return it != widgetIds_.end() ? it->second.widget : nullptr;
        }

        Canvas* CanvasStack::GetOwner(uint32_t _id) {
            const auto it = widgetIds_.find(_id);
            return it != widgetIds_.end() ? it->second.canvas : nullptr;
        }

        // ゲームが触る Widget は一部なので、ハンドルを渡すときに初めて番号を振る
        uint32_t CanvasStack::GetId(Canvas& _canvas, Widget& _widget) {
            if (_widget.id == 0) {
                _widget.id = nextId_++;
                widgetIds_[_widget.id] = { &_widget, &_canvas };
            }
            return _widget.id;
        }

        std::span<const Instance> CanvasStack::Build(const Vector2& _screenSize, TextureProvider& _textures, FontCache& _fonts) {
            // 同じ順番どうしは追加した順に描く
            std::stable_sort(canvases_.begin(), canvases_.end(), [](const auto& _a, const auto& _b) {
                return _a->GetOrder() < _b->GetOrder();
            });

            instances_.clear();
            for (const auto& canvas : canvases_) {
                if (canvas->IsVisible()) {
                    canvas->Build(_screenSize, _textures, _fonts, instances_);
                }
                // ゲームは描く前の更新で押されたかを読むので、描き終えたら次のフレームのために忘れる
                canvas->ClearPressed();
            }
            return instances_;
        }

        bool CanvasStack::HitTest(const Vector2& _point) {
            path_.clear();
            hit_ = nullptr;
            for (auto it = canvases_.rbegin(); it != canvases_.rend(); ++it) {
                Canvas& canvas = **it;
                if (!canvas.IsVisible()) {
                    continue;
                }
                if (canvas.HitTest(_point, path_) || canvas.IsBlockingInput()) {
                    hit_ = &canvas;
                    return true;
                }
            }
            return false;
        }

        bool CanvasStack::Hover(const Vector2& _point) {
            if (!HitTest(_point)) {
                return false;
            }

            // 毎回選び直すと選択の見た目が無駄に揺れないよう、外れているときだけ移す
            if (Widget* target = FindSelectable(); target != nullptr && !target->selected) {
                hit_->Focus(*target);
            }
            return true;
        }

        bool CanvasStack::Click(const Vector2& _point) {
            if (!HitTest(_point)) {
                return false;
            }

            if (Widget* target = FindSelectable()) {
                hit_->Focus(*target);
                hit_->Press(*target);
            }
            if (const size_t index = FindNearest(path_, [](const Widget& _widget) { return _widget.raisable; }); index > 0) {
                Raise(*path_[index - 1], *path_[index]);
            }
            return true;
        }

        Widget* CanvasStack::FindSelectable() const {
            const size_t index = FindNearest(path_, [](const Widget& _widget) { return _widget.selectable; });
            return index > 0 ? path_[index] : nullptr;
        }

        void CanvasStack::Forget(const Widget& _widget) {
            if (_widget.id != 0) {
                widgetIds_.erase(_widget.id);
            }
            for (const auto& child : _widget.children) {
                Forget(*child);
            }
        }

        void CanvasStack::Restore(Canvas& _canvas, Widget& _widget, const std::string& _prefix, const std::unordered_map<std::string, uint32_t>& _ids) {
            for (const auto& child : _widget.children) {
                const std::string path = JoinPath(_prefix, child->name);
                if (const auto it = _ids.find(path); it != _ids.end()) {
                    child->id = it->second;
                    widgetIds_[child->id] = { child.get(), &_canvas };
                }
                Restore(_canvas, *child, path, _ids);
            }
        }
    }
}
