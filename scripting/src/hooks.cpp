#include <rawrbox/scripting/hooks.hpp>

namespace rawrbox {
	void Hooks::listen(const std::string& id, const std::string& name, sol::function func) {
		hooks[id].emplace_back(name, func);
	}

	void Hooks::remove(const std::string& id, const std::string& name) {
		auto nameMap = hooks.find(id);
		if (nameMap == hooks.end()) return;

		auto& arr = nameMap->second;
		auto hook = std::find_if(arr.begin(), arr.end(), [&](auto& elem) {
			return elem.name == name;
		});

		if (hook == arr.end()) return;
		arr.erase(hook);

		if (arr.empty()) hooks.erase(id);
	}

	// Utils ---
	size_t Hooks::count() const { return this->hooks.size(); }
	bool Hooks::empty() const { return this->hooks.empty(); }
	// ----

} // namespace rawrbox
