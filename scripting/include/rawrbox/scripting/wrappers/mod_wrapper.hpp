
#pragma once
#include <rawrbox/scripting/mod.hpp>

namespace rawrbox {
	class ModWrapper {
	protected:
		rawrbox::Mod* _mod = nullptr;

	public:
		ModWrapper(rawrbox::Mod* mod);
		ModWrapper(const ModWrapper&) = default;
		ModWrapper(ModWrapper&&) = default;
		ModWrapper& operator=(const ModWrapper&) = default;
		ModWrapper& operator=(ModWrapper&&) = default;
		virtual ~ModWrapper() = default;

		[[nodiscard]] virtual sol::environment getENV() const;
		[[nodiscard]] virtual std::string getID() const;
		[[nodiscard]] virtual std::string getFolder() const;
		[[nodiscard]] virtual std::string getEntryFilePath() const;

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
