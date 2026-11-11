#pragma once

#pragma warning(push)
#pragma warning(disable : 4702)
#include <lualib.h>
// --- LuaBridge.h needs to be included after lualib.h!
#include <LuaBridge/LuaBridge.h>
#pragma warning(pop)

#include <rawrbox/utils/logger.hpp>

#include <glaze/glaze.hpp>
#include <magic_enum/magic_enum.hpp>

#include <Luau/CodeGen.h>
#include <Luau/Compiler.h>

#include <array>
#include <bit>
#include <expected>
#include <filesystem>
#include <functional>
#include <limits>
#include <optional>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace rawrbox {
	class Mod;

	struct LuaJSONOptions {
		uint32_t maxDepth = 32;
		bool strict = true;
	};

	struct LuaJSONType {
		std::string tag;

		std::function<bool(lua_State*, int)> is = nullptr;
		std::function<std::optional<glz::generic>(lua_State*, int)> encode = nullptr;
		std::function<bool(lua_State*, const glz::generic&)> decode = nullptr;
		std::function<bool(lua_State*, int, lua_State*)> copy = nullptr;
	};

	template <typename E, size_t... I>
	auto luaEnumValues(std::index_sequence<I...> /*values*/) -> luabridge::Enum<E, magic_enum::enum_value<E, I>()...>;

	template <typename E>
		requires(std::is_enum_v<E>)
	using LuaEnum = decltype(rawrbox::luaEnumValues<E>(std::make_index_sequence<magic_enum::enum_count<E>()>{}));

	class LuaResult {
	protected:
		std::vector<luabridge::LuaRef> _values = {};
		std::string _error;

	public:
		LuaResult(std::vector<luabridge::LuaRef> values, std::string error);

		[[nodiscard]] bool wasOk() const;
		[[nodiscard]] bool hasFailed() const;
		[[nodiscard]] const std::string& errorMessage() const;

		[[nodiscard]] size_t size() const;
		[[nodiscard]] const luabridge::LuaRef& operator[](size_t index) const;
	};

	class LuaUtils {
	protected:
		struct JSONError {
			std::string path;
			std::string reason;
		};

		struct TableKeys {
			size_t count = 0;
			size_t numbers = 0;
			size_t strings = 0;
			double highest = 0;
		};

		static std::shared_mutex _jsonTypesLock;
		static std::vector<rawrbox::LuaJSONType> _jsonTypes;
		static char _cacheKey;

		// JSON ---
		static void registerJSONType(rawrbox::LuaJSONType type);
		static std::optional<rawrbox::LuaJSONType> findJSONType(lua_State* L, int index);
		static std::optional<rawrbox::LuaJSONType> findJSONType(std::string_view tag);

		static std::unexpected<JSONError> valueError(std::string reason);

		static JSONError atKey(JSONError error, const std::string& key);
		static JSONError atIndex(JSONError error, size_t index);

		static std::string describe(lua_State* L, int index);
		static std::string formatError(const JSONError& error);

		static TableKeys countKeys(lua_State* L, int index);
		static std::optional<std::string> keyName(lua_State* L, int index, bool strict);

		static std::expected<glz::generic, JSONError> encode(lua_State* L, int index, const rawrbox::LuaJSONOptions& options, uint32_t depth);
		static std::expected<void, JSONError> decode(lua_State* L, const glz::generic& json, const rawrbox::LuaJSONOptions& options, uint32_t depth);
		static std::expected<void, JSONError> copyValue(lua_State* from, int index, lua_State* to, const rawrbox::LuaJSONOptions& options, uint32_t depth);
		// ---

		// STATE ---
		static bool pushCacheTable(lua_State* L, bool create);
		static bool pushCached(lua_State* L, std::string_view key, std::string_view version);
		static void storeCached(lua_State* L, std::string_view key, std::string_view version, const luabridge::LuaRef& value);
		static void freezeUserdata(lua_State* L, int index);
		// ---

	public:
		template <typename... CallbackArgs>
		static rawrbox::LuaResult call(const luabridge::LuaRef& func, CallbackArgs&&... args) {
			lua_State* L = func.state();
			if (L == nullptr) return {{}, "Invalid lua state"};

			const int top = lua_gettop(L);
			func.push(L);

			// ARGS ---
			const bool pushed = (true && ... && luabridge::push(L, std::forward<CallbackArgs>(args)));

			if (!pushed) {
				lua_settop(L, top);
				return {{}, "Failed to push lua function arguments"};
			}
			// ----

			if (lua_pcall(L, static_cast<int>(sizeof...(CallbackArgs)), LUA_MULTRET, 0) != LUA_OK) {
				std::string error = LuaUtils::getError(L);
				lua_settop(L, top);

				return {{}, error};
			}

			// Results ---
			std::vector<luabridge::LuaRef> values = {};
			for (int i = top + 1; i <= lua_gettop(L); i++) {
				values.push_back(luabridge::LuaRef::fromStack(L, i));
			}

			lua_settop(L, top);
			return {std::move(values), ""};
			// ----
		}

		static void compileAndLoadFile(lua_State* L, const std::string& chunkID, const std::filesystem::path& path);
		static void compileAndLoadScript(lua_State* L, const std::string& chunkID, const std::string& script);

		static std::string compile(const std::string& script);
		static std::string precompile(const std::string& script); // Bytecode with the '\x1bLuau' header
		static void loadBytecode(lua_State* L, const std::string& chunkID, const std::string& bytecode);

		static lua_State* loadThread(lua_State* L, const std::string& chunkID, const std::string& bytecode);

		static void resume(lua_State* L, lua_State* from);
		static void run(lua_State* L);
		static void collect_garbage(lua_State* L);

		static std::string getError(lua_State* L);

		static std::vector<std::string> argsToString(lua_State* L, bool filterNonStr = false);
		static void getVariadicArgs(const luabridge::LuaRef& in, luabridge::LuaRef& out);

		// JSON ---
		[[nodiscard]] static std::expected<glz::generic, std::string> toJSON(const luabridge::LuaRef& value, const rawrbox::LuaJSONOptions& options = {});
		[[nodiscard]] static std::expected<glz::generic, std::string> toJSON(lua_State* L, int index, const rawrbox::LuaJSONOptions& options = {});
		[[nodiscard]] static std::expected<luabridge::LuaRef, std::string> fromJSON(lua_State* L, const glz::generic& json, const rawrbox::LuaJSONOptions& options = {});

		[[nodiscard]] static std::expected<void, std::string> copy(lua_State* from, int index, lua_State* to, const rawrbox::LuaJSONOptions& options = {});

		template <typename T>
		static void registerJSONType(const std::string& tag, std::function<glz::generic(const T&)> encode, std::function<std::optional<T>(const glz::generic&)> decode) {
			if (encode == nullptr || decode == nullptr) RAWRBOX_CRITICAL("Invalid JSON type '{}' callbacks", tag);

			rawrbox::LuaJSONType type = {};
			type.tag = tag;
			type.is = [](lua_State* L, int index) { return luabridge::isInstance<T>(L, index); };
			type.encode = [encode = std::move(encode)](lua_State* L, int index) -> std::optional<glz::generic> {
				auto value = luabridge::Stack<T>::get(L, index);
				if (!value) return std::nullopt;

				const T& source = *value;
				return encode(source);
			};
			type.decode = [decode = std::move(decode)](lua_State* L, const glz::generic& json) {
				auto value = decode(json);
				return value.has_value() && static_cast<bool>(luabridge::Stack<T>::push(L, std::move(value.value())));
			};
			type.copy = [](lua_State* from, int index, lua_State* to) {
				auto value = luabridge::Stack<T>::get(from, index);
				if (!value) return false;

				const T& source = *value;
				return static_cast<bool>(luabridge::Stack<T>::push(to, T(source)));
			};

			LuaUtils::registerJSONType(std::move(type));
		}

		template <typename T>
			requires(requires(const T& value) { value.data(); })
		static void registerJSONType(const std::string& tag) {
			using Components = decltype(std::declval<const T&>().data());
			using Number = typename Components::value_type;
			constexpr size_t count = std::tuple_size_v<Components>;

			registerJSONType<T>(
			    tag, [](const T& value) {
				    glz::generic::array_t parts = {};
				    parts.reserve(count);

				    for (const auto part : value.data()) {
					    parts.emplace_back(static_cast<double>(part));
				    }

				    return glz::generic(std::move(parts)); },
			    [](const glz::generic& json) -> std::optional<T> {
				    const auto* parts = json.get_if<glz::generic::array_t>();
				    if (parts == nullptr || parts->size() != count) return std::nullopt;

				    Components values = {};
				    for (size_t i = 0; i < count; i++) {
					    const auto* number = (*parts)[i].template get_if<double>();
					    if (number == nullptr) return std::nullopt;

					    if constexpr (std::is_integral_v<Number>) {
						    if (*number != std::floor(*number) || *number < static_cast<double>(std::numeric_limits<Number>::lowest()) || *number > static_cast<double>(std::numeric_limits<Number>::max())) return std::nullopt;
					    }

					    values[i] = static_cast<Number>(*number);
				    }

				    return T(values);
			    });
		}

		[[nodiscard]] static std::vector<std::string> getJSONTypes();
		// ---

		// STATE ---
		static void freeze(lua_State* L, int index);

		template <typename V, typename Build>
			requires(std::has_unique_object_representations_v<V> && std::is_invocable_r_v<luabridge::LuaRef, const Build&, lua_State*>)
		[[nodiscard]] static luabridge::LuaRef cached(lua_State* L, std::string_view key, const V& version, const Build& build, bool frozen = true) {
			if (L == nullptr) throw std::runtime_error("Invalid lua state");

			lua_State* main = lua_mainthread(L);
			const auto bytes = std::bit_cast<std::array<char, sizeof(V)>>(version);
			const std::string_view stamp(bytes.data(), bytes.size());

			if (LuaUtils::pushCached(main, key, stamp)) {
				auto value = luabridge::LuaRef::fromStack(main, -1);
				lua_pop(main, 1);

				return value;
			}

			luabridge::LuaRef value = build(main);
			if (frozen) {
				value.push(main);
				LuaUtils::freeze(main, -1);
				lua_pop(main, 1);
			}

			LuaUtils::storeCached(main, key, stamp, value);
			return value;
		}

		template <typename E>
			requires(std::is_enum_v<E>)
		static void registerEnum(lua_State* L, std::string_view name = magic_enum::enum_type_name<E>()) {
			if (L == nullptr) throw std::runtime_error("Invalid lua state");

			lua_createtable(L, 0, static_cast<int>(magic_enum::enum_count<E>()));
			for (const auto& [value, key] : magic_enum::enum_entries<E>()) {
				lua_pushlstring(L, key.data(), key.size());
				lua_pushnumber(L, static_cast<double>(std::to_underlying(value)));
				lua_rawset(L, -3);
			}

			lua_setreadonly(L, -1, 1);
			lua_setglobal(L, std::string(name).c_str());
		}
		// ---

		template <typename T>
			requires(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
		static T getLuaENVVar(lua_State* L, const std::string& varId) {
			if (L == nullptr) throw std::runtime_error("Invalid lua state");

			lua_getfield(L, LUA_ENVIRONINDEX, varId.c_str());

			const auto* conv = lua_tostring(L, -1);
			if (conv == nullptr) {
				lua_pop(L, 1);
				throw std::runtime_error(fmt::format("Invalid lua env variable '{}'", varId));
			}

			if constexpr (std::is_same_v<T, std::string>) {
				std::string value = conv;
				lua_pop(L, 1);
				return value;
			} else {
				const auto value = static_cast<T>(lua_tonumber(L, -1));
				lua_pop(L, 1);
				return value;
			}
		}

		template <typename T>
			requires(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
		static luabridge::LuaRef vectorToTable(lua_State* L, const std::vector<T>& in) {
			auto tbl = luabridge::newTable(L);
			for (size_t i = 0; i < in.size(); i++) {
				tbl[i + 1] = in[i];
			}

			return tbl;
		}

		// #/ == Root content
		// normal_path == current mod
		// @cats/ == `cats` mod
		static std::pair<std::string, std::filesystem::path> getContent(const std::filesystem::path& path, const std::filesystem::path& modPath);

		[[nodiscard]] static std::expected<std::filesystem::path, std::string> resolveContent(const std::filesystem::path& path, const rawrbox::Mod& mod);
		[[nodiscard]] static std::expected<std::filesystem::path, std::string> resolveContent(const std::filesystem::path& path, lua_State* L);
	};
} // namespace rawrbox