#include <rawrbox/scripting/manager.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/utils/string.hpp>
#include <rawrbox/utils/vfs.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <cmath>
#include <mutex>
#include <shared_mutex>

namespace rawrbox {
	// PROTECTED ---
	std::shared_mutex LuaUtils::_jsonTypesLock;
	std::vector<rawrbox::LuaJSONType> LuaUtils::_jsonTypes = {};
	char LuaUtils::_cacheKey = 0;
	// -------------

	// JSON TYPES ---
	std::optional<rawrbox::LuaJSONType> LuaUtils::findJSONType(lua_State* L, int index) {
		const std::shared_lock lock(LuaUtils::_jsonTypesLock);

		for (const auto& type : LuaUtils::_jsonTypes) {
			if (type.is(L, index)) return type;
		}

		return std::nullopt;
	}

	std::optional<rawrbox::LuaJSONType> LuaUtils::findJSONType(std::string_view tag) {
		const std::shared_lock lock(LuaUtils::_jsonTypesLock);

		const auto& fnd = std::ranges::find(LuaUtils::_jsonTypes, tag, &rawrbox::LuaJSONType::tag);
		if (fnd == LuaUtils::_jsonTypes.end()) return std::nullopt;

		return *fnd;
	}
	// ---------

	// ERRORS ---
	std::unexpected<LuaUtils::JSONError> LuaUtils::valueError(std::string reason) { return std::unexpected(JSONError{.path = "", .reason = std::move(reason)}); }

	LuaUtils::JSONError LuaUtils::atKey(JSONError error, const std::string& key) {
		error.path = fmt::format(".{}{}", key, error.path);
		return error;
	}

	LuaUtils::JSONError LuaUtils::atIndex(JSONError error, size_t index) {
		error.path = fmt::format("[{}]{}", index, error.path);
		return error;
	}

	std::string LuaUtils::describe(lua_State* L, int index) { return fmt::format("{}({})", luaL_typename(L, index), lua_topointer(L, index)); }
	std::string LuaUtils::formatError(const JSONError& error, std::string_view root) { return fmt::format("{}{} {}", root, error.path, error.reason); }
	// ---------

	// TABLES ---
	LuaUtils::TableKeys LuaUtils::countKeys(lua_State* L, int index) {
		TableKeys keys = {};

		lua_pushnil(L);
		while (lua_next(L, index) != 0) {
			++keys.count;

			if (lua_type(L, -2) == LUA_TSTRING) {
				++keys.strings;
			} else if (lua_type(L, -2) == LUA_TNUMBER) {
				const double key = lua_tonumber(L, -2);
				if (key >= 1.0 && key == std::floor(key) && key <= 9007199254740992.0) {
					++keys.numbers;
					keys.highest = std::max(keys.highest, key);
				}
			}

			lua_pop(L, 1);
		}

		return keys;
	}

	std::optional<std::string> LuaUtils::keyName(lua_State* L, int index, bool strict) {
		switch (lua_type(L, index)) {
			case LUA_TSTRING:
				{
					size_t length = 0;
					const char* text = lua_tolstring(L, index, &length);
					return std::string(text, length);
				}
			case LUA_TNUMBER:
				{
					if (strict) return std::nullopt;

					const double number = lua_tonumber(L, index);
					if (number == std::floor(number) && std::abs(number) <= 9007199254740992.0) return fmt::format("{}", static_cast<int64_t>(number));
					return fmt::format("{}", number);
				}
			case LUA_TBOOLEAN:
				if (strict) return std::nullopt;
				return lua_toboolean(L, index) != 0 ? "true" : "false";
			default:
				if (strict) return std::nullopt;
				return LuaUtils::describe(L, index);
		}
	}

	std::expected<glz::generic, LuaUtils::JSONError> LuaUtils::encode(lua_State* L, int index, const rawrbox::LuaJSONOptions& options, uint32_t depth) {
		index = lua_absindex(L, index);

		switch (lua_type(L, index)) {
			case LUA_TNIL:
			case LUA_TNONE:
				return glz::generic(nullptr);
			case LUA_TBOOLEAN:
				return glz::generic(lua_toboolean(L, index) != 0);
			case LUA_TNUMBER:
				{
					const double number = lua_tonumber(L, index);
					if (!std::isfinite(number)) {
						if (options.strict) return LuaUtils::valueError("not a finite number");
						return glz::generic(nullptr);
					}

					return glz::generic(number);
				}
			case LUA_TSTRING:
				{
					size_t length = 0;
					const char* text = lua_tolstring(L, index, &length);
					return glz::generic(std::string(text, length));
				}
			case LUA_TUSERDATA:
				{
					const auto type = LuaUtils::findJSONType(L, index);
					if (type.has_value()) {
						auto json = type->encode(L, index);
						if (!json.has_value()) return LuaUtils::valueError(fmt::format("failed to encode '{}'", type->tag));

						return glz::generic(glz::generic::object_t{{type->tag, std::move(json.value())}});
					}

					if (options.strict) return LuaUtils::valueError("userdata without a registered JSON type");
					return glz::generic(LuaUtils::describe(L, index));
				}
			case LUA_TTABLE:
				{
					if (depth >= options.maxDepth) return LuaUtils::valueError(fmt::format("is nested deeper than {} tables", options.maxDepth));
					if (lua_checkstack(L, 4) == 0) return LuaUtils::valueError("lua stack overflow");

					const auto keys = LuaUtils::countKeys(L, index);
					if (keys.count == 0) return glz::generic(glz::generic::object_t{});

					// ARRAY ---
					if (keys.numbers == keys.count && static_cast<size_t>(keys.highest) == keys.count) {
						glz::generic::array_t items = {};
						items.reserve(keys.count);

						for (size_t i = 1; i <= keys.count; i++) {
							lua_rawgeti(L, index, static_cast<int>(i));
							auto item = LuaUtils::encode(L, -1, options, depth + 1);
							lua_pop(L, 1);

							if (!item.has_value()) return std::unexpected(LuaUtils::atIndex(std::move(item.error()), i));
							items.push_back(std::move(item.value()));
						}

						return glz::generic(std::move(items));
					}
					// --------

					if (options.strict) {
						if (keys.strings != keys.count && keys.strings > 0) return LuaUtils::valueError("invalid json keys");
						if (keys.numbers == keys.count) return LuaUtils::valueError("invalid list");
						if (keys.strings != keys.count) return LuaUtils::valueError("has keys that are not names / list indexes");
					}

					// OBJECT ---
					std::vector<std::pair<std::string, glz::generic>> fields = {};
					fields.reserve(keys.count);

					lua_pushnil(L);
					while (lua_next(L, index) != 0) {
						auto key = LuaUtils::keyName(L, -2, options.strict);
						if (!key.has_value()) {
							lua_pop(L, 2);
							return LuaUtils::valueError("non-string invalid keys");
						}

						auto value = LuaUtils::encode(L, -1, options, depth + 1);
						lua_pop(L, 1);

						if (!value.has_value()) {
							lua_pop(L, 1);
							return std::unexpected(LuaUtils::atKey(std::move(value.error()), key.value()));
						}

						fields.emplace_back(std::move(key.value()), std::move(value.value()));
					}

					std::ranges::stable_sort(fields, {}, &std::pair<std::string, glz::generic>::first);

					glz::generic::object_t object = {};
					object.reserve(fields.size());

					for (auto& [name, field] : fields) {
						object.insert_or_assign(std::move(name), std::move(field));
					}

					if (options.strict && object.size() == 1U && LuaUtils::findJSONType(object.begin()->first).has_value()) {
						return LuaUtils::valueError(fmt::format("has '{}' as its only field, which is how a {} is stored; rename the field", object.begin()->first, object.begin()->first));
					}

					return glz::generic(std::move(object));
					// --------
				}
			default:
				if (options.strict) return LuaUtils::valueError(fmt::format("is a {}! Not plain data", luaL_typename(L, index)));
				return glz::generic(LuaUtils::describe(L, index));
		}
	}

	std::expected<void, LuaUtils::JSONError> LuaUtils::decode(lua_State* L, const glz::generic& json, const rawrbox::LuaJSONOptions& options, uint32_t depth) {
		if (lua_checkstack(L, 3) == 0) return LuaUtils::valueError("lua stack overflow");

		if (json.is_null()) {
			lua_pushnil(L);
			return {};
		}

		if (const auto* boolean = json.get_if<bool>(); boolean != nullptr) {
			lua_pushboolean(L, *boolean ? 1 : 0);
			return {};
		}

		if (const auto* number = json.get_if<double>(); number != nullptr) {
			lua_pushnumber(L, *number);
			return {};
		}

		if (const auto* text = json.get_if<std::string>(); text != nullptr) {
			lua_pushlstring(L, text->data(), text->size());
			return {};
		}

		if (const auto* items = json.get_if<glz::generic::array_t>(); items != nullptr) {
			if (depth >= options.maxDepth) return LuaUtils::valueError(fmt::format("is nested deeper than {} tables", options.maxDepth));
			lua_createtable(L, static_cast<int>(items->size()), 0);

			for (size_t i = 0; i < items->size(); i++) {
				auto item = LuaUtils::decode(L, (*items)[i], options, depth + 1);
				if (!item.has_value()) {
					lua_pop(L, 1);
					return std::unexpected(LuaUtils::atIndex(std::move(item.error()), i + 1));
				}

				lua_rawseti(L, -2, static_cast<int>(i + 1));
			}

			return {};
		}

		if (const auto* fields = json.get_if<glz::generic::object_t>(); fields != nullptr) {
			if (fields->size() == 1U) {
				const auto type = LuaUtils::findJSONType(fields->begin()->first);
				if (type.has_value()) {
					if (!type->decode(L, fields->begin()->second)) return LuaUtils::valueError(fmt::format("is not a valid {}", type->tag));
					return {};
				}
			}

			if (depth >= options.maxDepth) return LuaUtils::valueError(fmt::format("is nested deeper than {} tables", options.maxDepth));
			lua_createtable(L, 0, static_cast<int>(fields->size()));

			for (const auto& [key, value] : *fields) {
				lua_pushlstring(L, key.data(), key.size());

				auto field = LuaUtils::decode(L, value, options, depth + 1);
				if (!field.has_value()) {
					lua_pop(L, 2);
					return std::unexpected(LuaUtils::atKey(std::move(field.error()), key));
				}

				lua_rawset(L, -3);
			}

			return {};
		}

		return LuaUtils::valueError("unknown json type");
	}

	std::expected<void, LuaUtils::JSONError> LuaUtils::copyValue(lua_State* from, int index, lua_State* to, const rawrbox::LuaJSONOptions& options, uint32_t depth) {
		index = lua_absindex(from, index);
		if (lua_checkstack(to, 3) == 0) return LuaUtils::valueError("lua stack overflow");

		switch (lua_type(from, index)) {
			case LUA_TNIL:
			case LUA_TNONE:
				lua_pushnil(to);
				return {};
			case LUA_TBOOLEAN:
				lua_pushboolean(to, lua_toboolean(from, index));
				return {};
			case LUA_TNUMBER:
				lua_pushnumber(to, lua_tonumber(from, index));
				return {};
			case LUA_TSTRING:
				{
					size_t length = 0;
					const char* text = lua_tolstring(from, index, &length);
					lua_pushlstring(to, text, length);
					return {};
				}
			case LUA_TVECTOR:
				{
					const auto* vec = lua_tovector(from, index);
#if LUA_VECTOR_SIZE == 4
					lua_pushvector(to, vec[0], vec[1], vec[2], vec[3]);
#else
					lua_pushvector(to, vec[0], vec[1], vec[2]);
#endif
					return {};
				}
			case LUA_TUSERDATA:
				{
					const auto type = LuaUtils::findJSONType(from, index);
					if (type.has_value()) {
						if (!type->copy(from, index, to)) return LuaUtils::valueError("failed to copy different types");
						return {};
					}

					if (options.strict) return LuaUtils::valueError("is a userdata without a registered JSON type");
					const auto text = LuaUtils::describe(from, index);
					lua_pushlstring(to, text.data(), text.size());
					return {};
				}
			case LUA_TTABLE:
				{
					if (depth >= options.maxDepth) return LuaUtils::valueError(fmt::format("is nested deeper than {} tables", options.maxDepth));
					if (lua_checkstack(from, 3) == 0) return LuaUtils::valueError("lua stack overflow");

					const int top = lua_gettop(to);
					lua_createtable(to, lua_objlen(from, index), 0);

					const int table = lua_gettop(to);
					const int base = lua_gettop(from);

					const auto unwind = [from, to, top, base]() {
						if (from != to) lua_settop(from, base);
						lua_settop(to, top);
					};
					// ----

					lua_pushnil(from);
					while (lua_next(from, index) != 0) {
						const int key = lua_absindex(from, -2);
						const int value = lua_absindex(from, -1);

						const int keyType = lua_type(from, key);
						if (keyType != LUA_TSTRING && keyType != LUA_TNUMBER && keyType != LUA_TBOOLEAN) {
							if (!options.strict) {
								lua_pop(from, 1);
								continue;
							}

							const std::string reason = fmt::format("has a {} key", lua_typename(from, keyType));
							unwind();
							return LuaUtils::valueError(reason);
						}

						auto copied = LuaUtils::copyValue(from, key, to, options, depth + 1);
						if (copied.has_value()) copied = LuaUtils::copyValue(from, value, to, options, depth + 1);

						if (!copied.has_value()) {
							const double position = keyType == LUA_TNUMBER ? lua_tonumber(from, key) : 0.0;
							auto error = position >= 1.0 && position == std::floor(position) ? LuaUtils::atIndex(std::move(copied.error()), static_cast<size_t>(position)) : LuaUtils::atKey(std::move(copied.error()), LuaUtils::keyName(from, key, false).value_or("?"));

							unwind();
							return std::unexpected(std::move(error));
						}

						lua_rawset(to, table);
						lua_pop(from, 1);
					}

					return {};
				}
			default:
				{
					if (options.strict) return LuaUtils::valueError(fmt::format("is a {}. Cannot be copied", luaL_typename(from, index)));

					const auto text = LuaUtils::describe(from, index);
					lua_pushlstring(to, text.data(), text.size());
					return {};
				}
		}
	}
	// ---------

	// CACHE ---
	bool LuaUtils::pushCacheTable(lua_State* L, bool create) {
		if (lua_rawgetp(L, LUA_REGISTRYINDEX, &LuaUtils::_cacheKey) == LUA_TTABLE) return true;
		lua_pop(L, 1);

		if (!create) return false;

		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_rawsetp(L, LUA_REGISTRYINDEX, &LuaUtils::_cacheKey);
		return true;
	}
	// ---------

	// FREEZE ---
	void LuaUtils::freezeUserdata(lua_State* L, int index) {
		if (lua_getmetatable(L, index) == 0) return;

		luabridge::lua_rawgetp_x(L, -1, luabridge::detail::getConstKey());
		if (lua_istable(L, -1)) {
			lua_setmetatable(L, index);
			lua_pop(L, 1);
			return;
		}

		lua_pop(L, 2);
	}
	// ---------

	void LuaUtils::compileAndLoadFile(lua_State* L, const std::string& chunkID, const std::filesystem::path& path) {
		if (L == nullptr) throw std::runtime_error("Invalid lua state");
		if (!rawrbox::VFS::contains(path)) throw std::runtime_error(fmt::format("File '{}' not found", path.generic_string()));

		// Load script ---
		const auto bytes = rawrbox::VFS::read(path);
		if (bytes.empty()) throw std::runtime_error("File empty / failed to load");
		// --------------

		const std::string data(bytes.begin(), bytes.end());
		if (data.starts_with("\x1bLuau")) {
			LuaUtils::loadBytecode(L, chunkID, data.substr(5));
			return;
		}

		LuaUtils::compileAndLoadScript(L, chunkID, data);
	}

	void LuaUtils::compileAndLoadScript(lua_State* L, const std::string& chunkID, const std::string& script) {
		if (L == nullptr) throw std::runtime_error("Invalid lua state");
		LuaUtils::loadBytecode(L, chunkID, LuaUtils::compile(script));
	}

	std::string LuaUtils::compile(const std::string& script) {
		Luau::CompileOptions options = {};
#ifndef _DEBUG
		options.optimizationLevel = 2; // Remove debug info & inline lua
		options.debugLevel = 0;
#endif

		Luau::ParseOptions parser = {};
		parser.allowDeclarationSyntax = false;
		parser.captureComments = false;

		std::string bytecode = Luau::compile(script, options, parser);
		if (bytecode.empty() || bytecode[0] == '\0') {
			size_t pos = bytecode.find(':'); // extract the error message
			std::string errorMessage = pos != std::string::npos ? bytecode.substr(pos + 1) : "Unknown lua error";
			throw std::runtime_error(errorMessage);
		}

		return bytecode;
	}

	std::string LuaUtils::precompile(const std::string& script) {
		return "\x1bLuau" + LuaUtils::compile(script);
	}

	void LuaUtils::loadBytecode(lua_State* L, const std::string& chunkID, const std::string& bytecode) {
		if (L == nullptr) throw std::runtime_error("Invalid lua state");

		const int top = lua_gettop(L);
		auto* thread = LuaUtils::loadThread(L, chunkID, bytecode);

		luabridge::detail::ScopeGuard restore([L, top]() { lua_settop(L, top); });
		rawrbox::LuaUtils::resume(thread, L);
	}

	lua_State* LuaUtils::loadThread(lua_State* L, const std::string& chunkID, const std::string& bytecode) {
		if (L == nullptr) throw std::runtime_error("Invalid lua state");

		const int top = lua_gettop(L);
		auto* thread = lua_newthread(L);

		const std::string chunk = fmt::format("={}", chunkID);
		if (luau_load(thread, chunk.c_str(), bytecode.data(), bytecode.size(), 0) != 0) {
			const std::string error = rawrbox::LuaUtils::getError(thread);
			lua_settop(L, top);

			throw std::runtime_error(error);
		}

		if (Luau::CodeGen::isNativeExecutionEnabled(thread)) Luau::CodeGen::compile(thread, -1, 0);
		return thread;
	}

	void LuaUtils::resume(lua_State* L, lua_State* from) {
		if (L == nullptr) throw std::runtime_error("Invalid lua state");
		if (lua_resume(L, from, 0) != 0) {
			throw std::runtime_error(rawrbox::LuaUtils::getError(L));
		}
	}

	void LuaUtils::run(lua_State* L) {
		if (L == nullptr) throw std::runtime_error("Invalid lua state");
		if (lua_pcall(L, 0, 0, 0) != 0) {
			throw std::runtime_error(rawrbox::LuaUtils::getError(L));
		}
	}
	void LuaUtils::collect_garbage(lua_State* L) {
		if (L == nullptr) throw std::runtime_error("Invalid lua state");
		lua_gc(L, LUA_GCCOLLECT, 0);
	}

	// RESULT ---
	LuaResult::LuaResult(std::vector<luabridge::LuaRef> values, std::string error) : _values(std::move(values)), _error(std::move(error)) {}

	bool LuaResult::wasOk() const { return this->_error.empty(); }
	bool LuaResult::hasFailed() const { return !this->_error.empty(); }
	const std::string& LuaResult::errorMessage() const { return this->_error; }

	size_t LuaResult::size() const { return this->_values.size(); }
	const luabridge::LuaRef& LuaResult::operator[](size_t index) const {
		if (index >= this->_values.size()) throw std::out_of_range(fmt::format("Invalid lua result index {}", index));
		return this->_values[index];
	}
	// ---------

	std::string LuaUtils::getError(lua_State* L) {
		if (L == nullptr) throw std::runtime_error("Invalid lua state");

		const char* error = lua_tostring(L, -1);
		return error == nullptr ? "Unknown lua error" : error;
	}

	std::vector<std::string> LuaUtils::argsToString(lua_State* L, bool filterNonStr) {
		std::vector<std::string> args = {};

		int nargs = lua_gettop(L);
		if (nargs == 0) return args;

		for (int i = 1; i <= nargs; i++) {
			if (filterNonStr) {
				if (lua_isstring(L, i) == 0) continue;
				args.emplace_back(lua_tostring(L, i));
			} else {
				int type = lua_type(L, i);

				switch (type) {
					case LUA_TFUNCTION:
						args.emplace_back(fmt::format("function({})", lua_topointer(L, i)));
						break;
					case LUA_TTABLE:
						args.emplace_back(fmt::format("table({})", lua_topointer(L, i)));
						break;
					case LUA_TVECTOR:
						args.emplace_back(fmt::format("vector({})", lua_topointer(L, i)));
						break;
					case LUA_TUSERDATA:
						args.emplace_back(fmt::format("userdata({})", lua_topointer(L, i)));
						break;
					case LUA_TNIL:
					case LUA_TNONE:
						args.emplace_back("nil");
						break;
					default:
						if (lua_isstring(L, i) == 0) continue;
						args.emplace_back(lua_tostring(L, i));
						break;
				}
			}
		}

		return args;
	}

	void LuaUtils::getVariadicArgs(const luabridge::LuaRef& in, luabridge::LuaRef& out) {
		auto* L = in.state();

		int nargs = lua_gettop(L);
		if (nargs == 0) return;

		int validArgs = 0;
		for (int i = 1; i <= nargs; i++) {
			int type = lua_type(L, i);

			switch (type) {
				case LUA_TNUMBER:
					out[validArgs++] = lua_tonumber(L, i);
					break;
				case LUA_TSTRING:
					out[validArgs++] = lua_tostring(L, i);
					break;
				case LUA_TBOOLEAN:
					out[validArgs++] = lua_toboolean(L, i) != 0;
					break;
				default:
					break;
			}
		}
	}

	// JSON ---
	void LuaUtils::registerJSONType(rawrbox::LuaJSONType type) {
		if (type.tag.empty()) RAWRBOX_CRITICAL("JSON type tag cannot be empty");
		if (type.is == nullptr || type.encode == nullptr || type.decode == nullptr || type.copy == nullptr) RAWRBOX_CRITICAL("Invalid JSON type '{}'", type.tag);

		const std::unique_lock lock(LuaUtils::_jsonTypesLock);

		const auto& fnd = std::ranges::find(LuaUtils::_jsonTypes, type.tag, &rawrbox::LuaJSONType::tag);
		if (fnd != LuaUtils::_jsonTypes.end()) {
			*fnd = std::move(type);
			return;
		}

		LuaUtils::_jsonTypes.push_back(std::move(type));
	}

	std::vector<std::string> LuaUtils::getJSONTypes() {
		const std::shared_lock lock(LuaUtils::_jsonTypesLock);

		std::vector<std::string> tags = {};
		tags.reserve(LuaUtils::_jsonTypes.size());

		for (const auto& type : LuaUtils::_jsonTypes) {
			tags.push_back(type.tag);
		}

		return tags;
	}

	std::expected<glz::generic, std::string> LuaUtils::toJSON(const luabridge::LuaRef& value, const rawrbox::LuaJSONOptions& options) {
		lua_State* L = value.state();
		if (L == nullptr) return std::unexpected("Invalid lua state");

		value.push(L);
		auto json = LuaUtils::toJSON(L, -1, options);
		lua_pop(L, 1);

		return json;
	}

	std::expected<glz::generic, std::string> LuaUtils::toJSON(lua_State* L, int index, const rawrbox::LuaJSONOptions& options) {
		if (L == nullptr) return std::unexpected("Invalid lua state");

		auto json = LuaUtils::encode(L, index, options, 0);
		if (!json.has_value()) return std::unexpected(LuaUtils::formatError(json.error(), options.root));
		return std::move(json.value());
	}

	std::expected<luabridge::LuaRef, std::string> LuaUtils::fromJSON(lua_State* L, const glz::generic& json, const rawrbox::LuaJSONOptions& options) {
		if (L == nullptr) return std::unexpected("Invalid lua state");

		const int top = lua_gettop(L);

		auto pushed = LuaUtils::decode(L, json, options, 0);
		if (!pushed.has_value()) {
			lua_settop(L, top);
			return std::unexpected(LuaUtils::formatError(pushed.error(), options.root));
		}

		auto value = luabridge::LuaRef::fromStack(L, -1);
		lua_settop(L, top);

		return value;
	}

	std::expected<void, std::string> LuaUtils::copy(lua_State* from, int index, lua_State* to, const rawrbox::LuaJSONOptions& options) {
		if (from == nullptr || to == nullptr) return std::unexpected("Invalid lua state");

		auto copied = LuaUtils::copyValue(from, index, to, options, 0);
		if (!copied.has_value()) return std::unexpected(LuaUtils::formatError(copied.error(), options.root));

		return {};
	}
	// ---------

	// STATE ---
	void LuaUtils::freeze(lua_State* L, int index) {
		if (L == nullptr) throw std::runtime_error("Invalid lua state");
		index = lua_absindex(L, index);

		if (lua_type(L, index) == LUA_TUSERDATA) {
			LuaUtils::freezeUserdata(L, index);
			return;
		}

		if (lua_type(L, index) != LUA_TTABLE || lua_getreadonly(L, index) != 0) return;
		lua_setreadonly(L, index, 1);

		if (lua_checkstack(L, 3) == 0) throw std::runtime_error("Lua stack overflow while freezing");

		lua_pushnil(L);
		while (lua_next(L, index) != 0) {
			LuaUtils::freeze(L, -1);
			LuaUtils::freeze(L, -2);

			lua_pop(L, 1);
		}
	}

	bool LuaUtils::pushCached(lua_State* L, std::string_view key, std::string_view version) {
		if (!LuaUtils::pushCacheTable(L, false)) return false;

		lua_pushlstring(L, key.data(), key.size());
		if (lua_rawget(L, -2) != LUA_TTABLE) {
			lua_pop(L, 2);
			return false;
		}

		lua_rawgeti(L, -1, 1);

		size_t length = 0;
		const char* stamp = lua_tolstring(L, -1, &length);
		const bool same = stamp != nullptr && std::string_view(stamp, length) == version;
		lua_pop(L, 1);

		if (!same) {
			lua_pop(L, 2);
			return false;
		}

		lua_rawgeti(L, -1, 2);
		lua_remove(L, -2);
		lua_remove(L, -2);
		return true;
	}

	void LuaUtils::storeCached(lua_State* L, std::string_view key, std::string_view version, const luabridge::LuaRef& value) {
		LuaUtils::pushCacheTable(L, true);

		lua_pushlstring(L, key.data(), key.size());
		lua_createtable(L, 2, 0);

		lua_pushlstring(L, version.data(), version.size());
		lua_rawseti(L, -2, 1);

		value.push(L);
		lua_rawseti(L, -2, 2);

		lua_rawset(L, -3);
		lua_pop(L, 1);
	}
	// ---------

	// #/ == Root assets
	// normal_path == current mod
	// @cats/ == `cats` mod
	std::pair<std::string, std::filesystem::path> LuaUtils::getContent(const std::filesystem::path& path, const std::filesystem::path& modPath) {
		if (path.empty()) throw std::runtime_error("Invalid path"); // Invalid path

		const auto pth = rawrbox::StrUtils::replaceAll(path.generic_string(), "\\", "/");

		std::string modID = {};
		std::filesystem::path root = modPath;
		std::string relative = pth;

		if (pth.front() == '#') { // #/ == Root assets
			const auto slash = pth.find('/');
			if (slash == std::string::npos) throw std::runtime_error("Invalid path");

			root = "assets";
			relative = pth.substr(slash + 1);
		} else if (pth.front() == '@') { // @cats/ == `cats` mod
			const auto slash = pth.find('/');

			modID = pth.substr(1, slash == std::string::npos ? std::string::npos : slash - 1);
			if (modID.empty()) throw std::runtime_error("Invalid mod ID");

			root.clear();
			relative = slash == std::string::npos ? "" : pth.substr(slash + 1);
		}

		auto normal = std::filesystem::path(relative).lexically_normal();
		if (normal.has_root_path() || (!normal.empty() && *normal.begin() == "..")) throw std::runtime_error(fmt::format("Path '{}' escapes its folder", path.generic_string()));
		if (normal == ".") normal.clear();

		if (root.empty()) return {modID, normal};
		return {modID, (root / normal).lexically_normal()};
	}

	std::expected<std::filesystem::path, std::string> LuaUtils::resolveContent(const std::filesystem::path& path, const rawrbox::Mod& mod) {
		try {
			auto content = LuaUtils::getContent(path, mod.getFolder());
			if (content.first.empty()) return std::move(content.second);

			const auto* owner = rawrbox::SCRIPTING::getMod(content.first);
			if (owner == nullptr) return std::unexpected(fmt::format("Mod '{}' is not loaded", content.first));

			return (owner->getFolder() / content.second).lexically_normal();
		} catch (const std::exception& err) {
			return std::unexpected(err.what());
		}
	}

	std::expected<std::filesystem::path, std::string> LuaUtils::resolveContent(const std::filesystem::path& path, lua_State* L) {
		const auto* mod = rawrbox::SCRIPTING::getMod(L);
		if (mod == nullptr) return std::unexpected("Not called from a mod");

		return LuaUtils::resolveContent(path, *mod);
	}

	std::expected<std::filesystem::path, std::string> LuaUtils::resolveContent(const std::filesystem::path& path, std::string_view modId) {
		const auto* mod = rawrbox::SCRIPTING::getMod(std::string(modId));
		if (mod == nullptr) return std::unexpected(fmt::format("Mod '{}' is not loaded", modId));

		return LuaUtils::resolveContent(path, *mod);
	}
} // namespace rawrbox