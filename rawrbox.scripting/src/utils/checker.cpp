#include <rawrbox/scripting/utils/checker.hpp>
#include <rawrbox/utils/vfs.hpp>

#include <Luau/BuiltinDefinitions.h>
#include <Luau/Config.h>
#include <Luau/ConfigResolver.h>
#include <Luau/Frontend.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <iterator>
#include <optional>
#include <unordered_map>
#include <utility>

LUAU_FASTFLAG(LuauSubtypingMissingPropertiesAsNil)

namespace {
	std::string readText(const std::filesystem::path& path) {
		const auto bytes = rawrbox::VFS::read(path);
		return {bytes.begin(), bytes.end()};
	}

	std::string problem(const std::string& file, const Luau::Location& location, std::string_view kind, std::string_view message) {
		return fmt::format("{}:{}:{}: {}: {}", file, location.begin.line + 1, location.begin.column + 1, kind, message);
	}

	struct VFSFiles : Luau::FileResolver {
		std::optional<Luau::SourceCode> readSource(const Luau::ModuleName& name) override {
			if (!rawrbox::VFS::contains(name)) return std::nullopt;
			return Luau::SourceCode{.source = readText(name), .type = Luau::SourceCode::Module};
		}
	};

	struct FolderConfig : Luau::ConfigResolver {
		mutable std::unordered_map<std::string, Luau::Config> cache = {};
		mutable std::vector<std::string> errors = {};

		const Luau::Config& getConfig(const Luau::ModuleName& name, const Luau::TypeCheckLimits& /*limits*/) const override {
			return this->folder(std::filesystem::path(name).parent_path());
		}

		const Luau::Config& folder(const std::filesystem::path& path) const {
			const auto key = path.generic_string();
			if (auto fnd = this->cache.find(key); fnd != this->cache.end()) return fnd->second;

			const bool root = path.empty() || path == path.parent_path();
			Luau::Config config = root ? Luau::Config{} : this->folder(path.parent_path());

			const auto luaurc = fmt::format("{}/.luaurc", key);
			if (!root && rawrbox::VFS::contains(luaurc)) {
				if (auto error = Luau::parseConfig(readText(luaurc), config); error.has_value()) this->errors.push_back(fmt::format("{}: {}", luaurc, *error));
			}

			return this->cache.emplace(key, std::move(config)).first->second;
		}
	};
} // namespace

namespace rawrbox {
	LuaChecker::LuaChecker(const std::vector<std::filesystem::path>& definitions) : _files(std::make_unique<VFSFiles>()), _config(std::make_unique<FolderConfig>()) {
		FFlag::LuauSubtypingMissingPropertiesAsNil.value = true;

		Luau::FrontendOptions options = {};
		options.runLintChecks = true;

		this->_frontend = std::make_unique<Luau::Frontend>(Luau::SolverMode::New, this->_files.get(), this->_config.get(), options);

		auto& globals = this->_frontend->globals;
		Luau::registerBuiltinGlobals(*this->_frontend, globals);

		for (const auto& path : definitions) {
			const auto file = path.generic_string();
			if (!rawrbox::VFS::contains(path)) throw std::runtime_error(fmt::format("Definition file '{}' not found", file));

			const auto loaded = this->_frontend->loadDefinitionFile(globals, globals.globalScope, readText(path), fmt::format("@{}", path.filename().generic_string()), false);
			if (loaded.success) continue;

			std::vector<std::string> errors = {};
			for (const auto& error : loaded.parseResult.errors) {
				errors.push_back(problem(file, error.getLocation(), "SyntaxError", error.getMessage()));
			}

			if (loaded.module != nullptr) {
				for (const auto& error : loaded.module->errors) {
					errors.push_back(problem(file, error.location, "TypeError", Luau::toString(error)));
				}
			}

			throw std::runtime_error(fmt::format("Failed to load definition file '{}'\n{}", file, fmt::join(errors, "\n")));
		}

		Luau::freeze(globals.globalTypes);
	}

	LuaChecker::~LuaChecker() = default;

	std::vector<std::string> LuaChecker::check(const std::filesystem::path& file) const {
		const auto name = file.lexically_normal().generic_string();
		if (!rawrbox::VFS::contains(name)) {
			auto error = fmt::format("{}: file not found", name);
			this->_logger->error("{}", error);
			return {error};
		}

		this->_frontend->markDirty(name);
		const auto checked = this->_frontend->check(name);

		auto errors = std::exchange(static_cast<FolderConfig*>(this->_config.get())->errors, {});
		for (const auto& error : checked.errors) {
			const auto* kind = Luau::get_if<Luau::SyntaxError>(&error.data) != nullptr ? "SyntaxError" : "TypeError";
			errors.push_back(problem(name, error.location, kind, Luau::toString(error, {.fileResolver = this->_files.get()})));
		}

		for (const auto& lint : checked.lintResult.errors) {
			errors.push_back(problem(name, lint.location, Luau::LintWarning::getName(lint.code), lint.text));
		}

		for (const auto& lint : checked.lintResult.warnings) {
			this->_logger->warn("{}", problem(name, lint.location, Luau::LintWarning::getName(lint.code), lint.text));
		}

		for (const auto& error : errors) {
			this->_logger->error("{}", error);
		}

		return errors;
	}

	std::vector<std::string> LuaChecker::checkFolder(const std::filesystem::path& folder) const {
		std::vector<std::filesystem::path> files = rawrbox::VFS::list(folder, true);
		std::erase_if(files, [](const std::filesystem::path& path) {
			const auto name = path.generic_string();
			return !(name.ends_with(".lua") || name.ends_with(".luau")) || name.ends_with(".d.luau");
		});

		std::ranges::sort(files);

		std::vector<std::string> errors = {};
		for (const auto& file : files) {
			std::ranges::move(this->check(file), std::back_inserter(errors));
		}

		return errors;
	}
} // namespace rawrbox