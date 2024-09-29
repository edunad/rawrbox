#pragma once

#include <algorithm>
#include <functional>


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

		Event& operator-=(Func callback) {
			calls.erase(std::remove_if(calls.begin(), calls.end(), [&](const Func& func) { return func.target_type() == callback.target_type(); }), calls.end());
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
		std::vector<Func> calls = {};
	};
} // namespace rawrbox
