#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace rawrbox {
	template <typename... CallbackArgs>
	class Event {
	public:
		using Func = std::function<void(CallbackArgs...)>;

		Event() = default;
		explicit Event(Func callback) { (*this) += std::move(callback); }
		explicit Event(Func& callback) { (*this) += callback; }

		size_t operator+=(Func callback) {
			const size_t id = this->_nextId++;

			if (this->_firing > 0) {
				this->_pending.push_back({id, std::move(callback)});
			} else {
				this->_calls.push_back({id, std::move(callback)});
			}

			++this->_live;
			return id;
		}

		Event& operator-=(size_t id) {
			if (id != 0) this->removeIf([id](const Entry& entry) { return entry.id == id; });
			return *this;
		}

		bool operator==(const Event& right) const { return this == &right; }
		bool operator!=(const Event& right) const { return !(*this == right); }

		Func& operator[](int i) { return this->_calls[i].call; }

		[[nodiscard]] size_t size() const { return this->_live; }
		[[nodiscard]] bool empty() const { return this->_live == 0; }
		void clear() {
			this->removeIf([](const Entry& /*entry*/) { return true; });
		}

		template <typename F>
			requires(!std::is_integral_v<std::remove_cvref_t<F>>)
		Event& operator-=(F&& callback) {
			using Decayed = std::decay_t<F>;

			if constexpr (std::is_pointer_v<Decayed> && std::is_function_v<std::remove_pointer_t<Decayed>>) {
				this->removePointer(callback);
			} else {
				Func func = std::forward<F>(callback);

				const FuncPtr* ptr = func.template target<FuncPtr>();
				if (ptr != nullptr) {
					this->removePointer(*ptr);
					return *this;
				}

				this->removeIf([&func](const Entry& entry) {
					return entry.call.target_type() == func.target_type();
				});
			}

			return *this;
		}

		void operator()(CallbackArgs... args) {
			const Firing firing(*this);

			const size_t total = this->_calls.size();
			for (size_t i = 0; i < total; i++) {
				if (this->_calls[i].id == 0) continue;
				this->_calls[i].call(args...);
			}
		}

	private:
		using FuncPtr = void (*)(CallbackArgs...);

		struct Entry {
			size_t id = 0;
			Func call = nullptr;
		};

		struct Firing {
			Event* event = nullptr;

			explicit Firing(Event& target) : event(&target) { ++this->event->_firing; }
			Firing(const Firing&) = delete;
			Firing(Firing&&) = delete;
			Firing& operator=(const Firing&) = delete;
			Firing& operator=(Firing&&) = delete;
			~Firing() {
				if (--this->event->_firing == 0) this->event->flush();
			}
		};

		std::vector<Entry> _calls = {};
		std::vector<Entry> _pending = {};

		size_t _nextId = 1;
		size_t _live = 0;
		size_t _firing = 0;
		bool _dirty = false;

		template <typename Pred>
		size_t removeIf(const Pred& pred) {
			size_t removed = 0;

			for (auto& entry : this->_calls) {
				if (entry.id == 0 || !pred(entry)) continue;
				++removed;

				if (this->_firing > 0) {
					entry.id = 0;
					this->_dirty = true;
				}
			}

			if (this->_firing == 0) std::erase_if(this->_calls, [&pred](const Entry& entry) { return entry.id != 0 && pred(entry); });
			removed += std::erase_if(this->_pending, [&pred](const Entry& entry) { return pred(entry); });

			this->_live -= removed;
			return removed;
		}

		void flush() {
			if (this->_dirty) {
				std::erase_if(this->_calls, [](const Entry& entry) { return entry.id == 0; });
				this->_dirty = false;
			}

			if (this->_pending.empty()) return;
			this->_calls.insert(this->_calls.end(), std::make_move_iterator(this->_pending.begin()), std::make_move_iterator(this->_pending.end()));
			this->_pending.clear();
		}

		template <typename Ptr>
		void removePointer(Ptr ptr) {
			this->removeIf([&ptr](const Entry& entry) {
				const Ptr* target = entry.call.template target<Ptr>();
				return target != nullptr && *target == ptr;
			});
		}
	};
} // namespace rawrbox