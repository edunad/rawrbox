#pragma once
#include <functional>

namespace rawrBox {
	template <typename... CallbackArgs>
	class Event {
	public:
		using Func = std::function<void(CallbackArgs...)>;

		Event() = default;
		~Event() = default;
		Event(Func callback) { (*this) += callback; }
		Event(Func& callback) { (*this) += callback; }

		Event& operator+=(Func callback) {
			calls.push_back(callback);
			return *this;
		}

		bool operator==(Event& right) { return calls == right.calls; }
		bool operator!=(Event& right) { return !(this == &right); }

		Func& operator[](int i) { return calls[i]; }

		size_t size() { return calls.size(); }

		void clear() {
			calls.clear();
		}

		Event& operator-=(Func callback) {
			auto found = std::find(calls.begin(), calls.end(), callback);
			if (found == calls.end()) return *this;

			calls.erase(found);

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
		std::vector<Func> calls;
	};
} // namespace rawrBox
