#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace rawrbox {
	template <typename... CallbackArgs>
	class EventNamed {
		std::atomic<size_t> uniqueId = 0;

	public:
		using Func = std::function<void(CallbackArgs...)>;

		class EventScoped {
			EventNamed<CallbackArgs...>* map = nullptr;
			std::string name;

		public:
			void call(CallbackArgs... args) {
				if (this->map == nullptr) return;

				const auto& cb = this->map->find(this->name);
				if (cb == this->map->end()) return;

				cb->second(args...);
			}
		};

		EventNamed() = default;

		std::shared_ptr<EventScoped> add(Func callback) {
			std::string name = "__" + std::to_string(this->uniqueId++) + "__";
			this->calls[name] = callback;

			return std::make_shared<EventScoped>(this, name);
		}

		void add(const std::string& name, Func callback) {
			this->calls[name] = callback;
		}

		Func& operator[](const std::string& name) { return this->calls[name]; }
		auto find(const std::string& name) { return this->calls.find(name); }
		auto begin() { return this->calls.begin(); }
		auto end() { return this->calls.end(); }

		size_t size() { return this->calls.size(); }

		void clear() {
			this->calls.clear();
		}

		bool remove(const std::string& name) {
			const auto& found = this->calls.find(name);
			if (found == this->calls.end()) return false;

			this->calls.erase(found);
			return true;
		}

		void operator()(CallbackArgs... args) {
			// copy it so we can += and -= inside the callback
			auto callsCopy = this->calls;
			for (const auto& pair : callsCopy) {
				pair.second(args...);
			}
		}

	private:
		std::map<std::string, Func> calls;
	};
} // namespace rawrbox
