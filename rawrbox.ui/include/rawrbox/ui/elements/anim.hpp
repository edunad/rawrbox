
#pragma once
#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/color.hpp>
#include <rawrbox/ui/container.hpp>
#include <rawrbox/utils/logger.hpp>

#include <glaze/glaze.hpp>

#include <functional>
#include <optional>
#include <vector>

namespace rawrbox {
	template <typename T>
	concept isResizable = requires(T t, const rawrbox::Vector2f& p) {
		{ t.setSize(p) };
	};

	template <typename T>
	concept isMovable = requires(T t, const rawrbox::Vector2f& p) {
		{ t.setPos(p) };
	};

	template <typename T>
	concept isColorable = requires(T t, const rawrbox::Color& p) {
		{ t.setColor(p) };
	};

	struct UIAnimKeyframe {
	public:
		float time = 1.F;

		std::optional<rawrbox::Vector2f> size = std::nullopt;
		std::optional<rawrbox::Vector2f> pos = std::nullopt;
		std::optional<rawrbox::Colorf> color = std::nullopt;
	};

	template <class T>
	class UIAnim {
	protected:
		int _index = 0;
		float _timeElapsed = 0;

		bool _loop = false;
		bool _reverse = false;
		bool _active = false;

		rawrbox::Vector2f _posOffset = {};
		std::vector<rawrbox::UIAnimKeyframe> _keyframes = {};
		T* _element = nullptr;

	public:
		std::function<void()> onFinish = nullptr;

		UIAnim() = default;
		UIAnim(const UIAnim&) = delete;
		UIAnim(UIAnim&&) = delete;
		UIAnim& operator=(const UIAnim&) = delete;
		UIAnim& operator=(UIAnim&&) = delete;
		virtual ~UIAnim() = default;

		virtual void setLoop(bool loop) { this->_loop = loop; };
		virtual void setReverse(bool reverse) { this->_reverse = reverse; };
		virtual void setPos(const rawrbox::Vector2f& pos) { this->_posOffset = pos; };
		virtual void setElement(T* element) { this->_element = element; };
		virtual void setAnimation(const glz::json_t& json) {
			if (!json.contains("anim")) throw rawrbox::Logger::err("RawrBox-UI", "Missing anim data");
			if (!json["anim"].holds<glz::json_t::array_t>()) throw rawrbox::Logger::err("RawrBox-UI", "Invalid anim data");

			auto jsonData = json["anim"].get<glz::json_t::array_t>();
			for (auto& data : jsonData) {
				rawrbox::UIAnimKeyframe key;

				if (data.contains("time")) {
					key.time = data["time"].as<float>();
				}

				if (data.contains("pos")) {
					auto x = data["pos"]["x"].as<float>();
					auto y = data["pos"]["y"].as<float>();

					key.pos = {x, y};
				}

				if (data.contains("size")) {
					auto x = data["size"]["x"].as<float>();
					auto y = data["size"]["y"].as<float>();

					key.size = {x, y};
				}

				if (data.contains("color")) {
					auto r = data["color"]["r"].as<int>();
					auto g = data["color"]["g"].as<int>();
					auto b = data["color"]["b"].as<int>();
					auto a = data["color"]["a"].as<int>();

					key.color = rawrbox::Colori(r, g, b, a).cast<float>();
				}

				this->_keyframes.push_back(key);
			}
		};

		virtual void play() {
			if (this->_active) return;

			this->_timeElapsed = 0;
			this->_active = true;
			this->reset();
		};

		virtual void stop() {
			if (!this->_active) return;

			this->_timeElapsed = 0;
			this->_active = false;
			if (this->onFinish != nullptr) this->onFinish();
		}

		virtual void reset() {
			if (this->_keyframes.empty()) return;

			if (this->_reverse)
				this->_index = static_cast<int>(this->_keyframes.size()) - 1;
			else
				this->_index = 0;

			// setup first frame
			this->setKeyframe(this->_keyframes[this->_index]);
		};

		virtual void setKeyframe(UIAnimKeyframe& frame) {
			if (_element == nullptr) throw std::runtime_error("[RawrBox-UI] UI element not set!");
			if (frame.pos.has_value()) {
				if constexpr (isMovable<T>) {
					_element->setPos(this->_posOffset + frame.pos.value());
				}
			}

			if (frame.size.has_value()) {
				if constexpr (isResizable<T>) {
					_element->setSize(frame.size.value());
				}
			}

			if (frame.color.has_value()) {
				if constexpr (isColorable<T>) {
					_element->setColor(frame.color.value());
				}
			}
		}

		virtual void lerpKeyframe(UIAnimKeyframe& prevKey, UIAnimKeyframe& newKey, float time) {
			if (_element == nullptr) throw std::runtime_error("[RawrBox-UI] UI element not set!");
			if (prevKey.pos.has_value() && newKey.pos.has_value()) {
				if constexpr (isMovable<T>) {
					_element->setPos(this->_posOffset + prevKey.pos.value().lerp(newKey.pos.value(), time));
				}
			}

			if (prevKey.size.has_value() && newKey.size.has_value()) {
				if constexpr (isResizable<T>) {
					_element->setSize(prevKey.size.value().lerp(newKey.size.value(), time));
				}
			}

			if (prevKey.color.has_value() && newKey.color.has_value()) {
				if constexpr (isColorable<T>) {
					auto cl = prevKey.color.value().lerp(newKey.color.value(), time);
					_element->setColor(cl);
				}
			}
		}

		virtual void update() {
			if (!this->_active) return;

			int keySize = static_cast<int>(this->_keyframes.size());
			int nextIndx = this->_index + (this->_reverse ? -1 : 1);
			if (nextIndx < 0 || nextIndx >= keySize) {
				this->stop();
				return;
			}

			auto currentFrame = this->_keyframes[this->_index];
			auto nextFrame = this->_keyframes[nextIndx];

			float lerpVal = this->_timeElapsed / currentFrame.time;

			this->lerpKeyframe(currentFrame, nextFrame, lerpVal);
			this->_timeElapsed += rawrbox::DELTA_TIME;

			if (lerpVal >= 1.F) {
				this->_index += this->_reverse ? -1 : 1;

				if (this->_reverse && this->_index <= 0) {
					if (!this->_loop) return;
					this->_index = keySize - 1;
				} else if (this->_index >= keySize - 1) {
					if (!this->_loop) return;
					this->_index = 0;
				}

				this->_timeElapsed = 0.F;
			}
		};
	};
} // namespace rawrbox
