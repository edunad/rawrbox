#pragma once

#include <algorithm>
#include <functional>
#include <type_traits>
#include <vector>

namespace rawrbox {
	template <typename... CallbackArgs>
	class Event {
	public:
		using Func = std::function<void(CallbackArgs...)>;

		Event() = default;
		explicit Event(Func callback) { (*this) += callback; }
		explicit Event(Func& callback) { (*this) += callback; }

		Event& operator+=(Func callback) {
			calls.push_back(callback);
			return *this;
		}

		bool operator==(Event& right) { return calls == right.calls; }
		bool operator!=(Event& right) { return !(this == &right); }

		Func& operator[](int i) { return calls[i]; }

		size_t size() { return calls.size(); }
		bool empty() { return calls.empty(); }
		void clear() { calls.clear(); }

		template <typename F>
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

				this->calls.erase(std::remove_if(this->calls.begin(), this->calls.end(), [&](const Func& call) {
					return call.target_type() == func.target_type();
				}),
				    this->calls.end());
			}

			return *this;
		}

		void operator()(CallbackArgs... args) {
			// copy it so we can += and -= inside the callback
			auto callsCopy = calls;
			for (const auto& callback : callsCopy) {
				callback(args...);
			}
		}

	private:
		using FuncPtr = void (*)(CallbackArgs...);

		std::vector<Func> calls = {};

		template <typename Ptr>
		void removePointer(Ptr ptr) {
			this->calls.erase(std::remove_if(this->calls.begin(), this->calls.end(), [&](const Func& call) {
				const Ptr* target = call.template target<Ptr>();
				return target != nullptr && *target == ptr;
			}),
			    this->calls.end());
		}
	};
} // namespace rawrbox