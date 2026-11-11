#include <rawrbox/scripting/manager.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/scripting/wrappers/i18n.hpp>
#include <rawrbox/scripting/wrappers/math/color.hpp>
#include <rawrbox/scripting/wrappers/math/vector2.hpp>
#include <rawrbox/utils/i18n.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <string>

namespace rawrbox {
	enum class LuaUtilsTestEnum : uint8_t {
		FIRST = 1,
		SECOND = 4
	};

	struct LuaUtilsTestCounter {
		int value = 0;
		void bump() { ++this->value; }
	};

	class LuaUtilsTestScripting : public rawrbox::SCRIPTING {
	public:
		static void add(std::unique_ptr<rawrbox::Mod> mod) { SCRIPTING::_mods.push_back(std::move(mod)); }
		static void clear() { SCRIPTING::_mods.clear(); }
	};

	static lua_State* newUtilsState() {
		lua_State* L = luaL_newstate();
		luaL_openlibs(L);

		rawrbox::Vector2Wrapper::registerLua(L);
		rawrbox::ColorWrapper::registerLua(L);
		return L;
	}

	static void runUtilsLua(lua_State* L, const std::string& code) {
		REQUIRE_NOTHROW(rawrbox::LuaUtils::compileAndLoadScript(L, "test", code));
	}

	static bool failsUtilsLua(lua_State* L, const std::string& code) {
		const int top = lua_gettop(L);
		const std::string bytecode = rawrbox::LuaUtils::compile(code);

		lua_State* thread = rawrbox::LuaUtils::loadThread(L, "test", bytecode);
		const bool failed = lua_resume(thread, L, 0) != LUA_OK;

		lua_settop(L, top);
		return failed;
	}

	static glz::generic parseUtilsJSON(const std::string& text) {
		glz::generic value = {};
		REQUIRE(glz::read_json(value, text) == glz::error_code::none);
		return value;
	}

	static std::string writeUtilsJSON(const glz::generic& value) {
		std::string output;
		REQUIRE_FALSE(glz::write_json(value, output));
		return output;
	}

	static luabridge::LuaRef evalUtilsLua(lua_State* L, const std::string& expression) {
		rawrbox::runUtilsLua(L, "RESULT = " + expression);
		return luabridge::getGlobal(L, "RESULT");
	}
} // namespace rawrbox

template <>
struct luabridge::Stack<rawrbox::LuaUtilsTestEnum> : rawrbox::LuaEnum<rawrbox::LuaUtilsTestEnum> {};

TEST_CASE("Lua Utils should behave as expected", "[rawrbox::LuaUtils]") {
	SECTION("rawrbox::LuaUtils::getContent") {
		auto test1 = rawrbox::LuaUtils::getContent("./assets/bla.png", "my_mod/");
		REQUIRE(test1.first.empty());
		REQUIRE(test1.second == "my_mod/assets/bla.png");

		auto test2 = rawrbox::LuaUtils::getContent("#/bla.png", "local_content/");
		REQUIRE(test2.first.empty());
		REQUIRE(test2.second == "assets/bla.png");

		auto test3 = rawrbox::LuaUtils::getContent("@mod_2/assets/bla.png", "my_mod/");
		REQUIRE(test3.first == "mod_2");
		REQUIRE(test3.second == "assets/bla.png");

		REQUIRE_THROWS(rawrbox::LuaUtils::getContent("", ""));
		REQUIRE_THROWS(rawrbox::LuaUtils::getContent("@/assets/bla.png", ""));

		REQUIRE(rawrbox::LuaUtils::getContent("assets\\models\\bla.png", "my_mod").second == "my_mod/assets/models/bla.png");
		REQUIRE(rawrbox::LuaUtils::getContent("./assets/./x/../bla.png", "my_mod").second == "my_mod/assets/bla.png");
		REQUIRE(rawrbox::LuaUtils::getContent("assets/bla.png", "").second == "assets/bla.png");
		REQUIRE(rawrbox::LuaUtils::getContent("@mod_2", "my_mod").second.empty());

		REQUIRE_THROWS(rawrbox::LuaUtils::getContent("../x.png", "my_mod"));
		REQUIRE_THROWS(rawrbox::LuaUtils::getContent("assets/../../x.png", "my_mod"));
		REQUIRE_THROWS(rawrbox::LuaUtils::getContent("#/../x.png", "my_mod"));
		REQUIRE_THROWS(rawrbox::LuaUtils::getContent("/etc/x.png", "my_mod"));
	}

	SECTION("rawrbox::LuaUtils::resolveContent") {
		auto own = std::make_unique<rawrbox::Mod>("my_mod", "mods/my_mod", rawrbox::ModMetadata{});
		auto other = std::make_unique<rawrbox::Mod>("other", "mods/other", rawrbox::ModMetadata{});

		auto& mod = *own;
		rawrbox::LuaUtilsTestScripting::add(std::move(own));
		rawrbox::LuaUtilsTestScripting::add(std::move(other));

		REQUIRE(rawrbox::LuaUtils::resolveContent("./assets/bla.png", mod).value() == "mods/my_mod/assets/bla.png");
		REQUIRE(rawrbox::LuaUtils::resolveContent("#/bla.png", mod).value() == "assets/bla.png");
		REQUIRE(rawrbox::LuaUtils::resolveContent("@other/lib/util.luau", mod).value() == "mods/other/lib/util.luau");

		REQUIRE_FALSE(rawrbox::LuaUtils::resolveContent("@missing/x.png", mod).has_value());
		REQUIRE_FALSE(rawrbox::LuaUtils::resolveContent("@other/../my_mod/x.png", mod).has_value());
		REQUIRE_FALSE(rawrbox::LuaUtils::resolveContent("../../secrets.txt", mod).has_value());

		lua_State* L = luaL_newstate();
		REQUIRE_FALSE(rawrbox::LuaUtils::resolveContent("x.png", L).has_value());
		lua_close(L);

		REQUIRE(rawrbox::LuaUtils::resolveContent("x.png", mod.getEnvironment()).value() == "mods/my_mod/x.png");
		rawrbox::LuaUtilsTestScripting::clear();
	}

	SECTION("rawrbox::Mod::fromState") {
		rawrbox::Mod mod = {"my_mod", "./my_mod", {}};
		lua_State* L = mod.getEnvironment();

		REQUIRE(rawrbox::Mod::fromState(L) == &mod);
		REQUIRE(rawrbox::SCRIPTING::getMod(L) == &mod);

		lua_State* thread = lua_newthread(L);
		REQUIRE(rawrbox::Mod::fromState(thread) == &mod);
		lua_pop(L, 1);

		lua_State* plain = luaL_newstate();
		REQUIRE(rawrbox::Mod::fromState(plain) == nullptr);
		lua_close(plain);
	}

	SECTION("rawrbox::LuaUtils::loadThread") {
		lua_State* L = luaL_newstate();
		luaL_openlibs(L);

		const int top = lua_gettop(L);

		lua_State* thread = rawrbox::LuaUtils::loadThread(L, "thread", rawrbox::LuaUtils::compile("LOADED = 5"));
		REQUIRE(lua_gettop(L) == top + 1);
		REQUIRE(lua_type(thread, -1) == LUA_TFUNCTION);

		lua_getglobal(L, "LOADED");
		REQUIRE(lua_isnil(L, -1));
		lua_pop(L, 1);

		REQUIRE(lua_resume(thread, L, 0) == LUA_OK);
		lua_pop(L, 1);
		REQUIRE(lua_gettop(L) == top);

		lua_getglobal(L, "LOADED");
		REQUIRE(lua_tonumber(L, -1) == 5);
		lua_pop(L, 1);

		const std::string broken("\0broken bytecode", 16);
		try {
			static_cast<void>(rawrbox::LuaUtils::loadThread(L, "broken", broken));
			FAIL("loadThread should throw");
		} catch (const std::runtime_error& err) {
			REQUIRE(std::string(err.what()).find("broken bytecode") != std::string::npos);
		}

		REQUIRE(lua_gettop(L) == top);

		lua_close(L);
	}

	SECTION("rawrbox::LuaUtils::loadBytecode keeps the stack") {
		lua_State* L = luaL_newstate();
		luaL_openlibs(L);

		const int top = lua_gettop(L);

		REQUIRE_NOTHROW(rawrbox::LuaUtils::compileAndLoadScript(L, "ok", "OK = true"));
		REQUIRE(lua_gettop(L) == top);

		try {
			rawrbox::LuaUtils::compileAndLoadScript(L, "boom", "error('boom')");
			FAIL("The script should fail");
		} catch (const std::runtime_error& err) {
			REQUIRE(std::string(err.what()).find("boom") != std::string::npos);
		}

		REQUIRE(lua_gettop(L) == top);

		try {
			rawrbox::LuaUtils::loadBytecode(L, "broken", std::string("\0broken bytecode", 16));
			FAIL("The bytecode should fail");
		} catch (const std::runtime_error& err) {
			REQUIRE(std::string(err.what()).find("broken bytecode") != std::string::npos);
		}

		REQUIRE(lua_gettop(L) == top);
		lua_close(L);
	}

	SECTION("rawrbox::LuaUtils::toJSON") {
		lua_State* L = rawrbox::newUtilsState();

		REQUIRE(rawrbox::writeUtilsJSON(rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "{1, 2, 3}")).value()) == "[1,2,3]");
		REQUIRE(rawrbox::writeUtilsJSON(rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "{b = 'x', a = {true, false}}")).value()) == R"({"a":[true,false],"b":"x"})");
		REQUIRE(rawrbox::writeUtilsJSON(rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "{}")).value()) == "{}");
		REQUIRE(rawrbox::writeUtilsJSON(rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "nil")).value()) == "null");
		REQUIRE(rawrbox::writeUtilsJSON(rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "{pos = Vector2i(4, -2), tint = Color(1, 0.5, 0, 1)}")).value()) == R"({"pos":{"Vector2i":[4,-2]},"tint":{"Color":[1,0.5,0,1]}})");

		rawrbox::runUtilsLua(L, "LIVE = {}; for i = 1, 40 do LIVE['k' .. i] = {n = i, [i % 2 == 0 and 'even' or 'odd'] = true} end; for i = 1, 40, 3 do LIVE['k' .. i] = nil end; LIVE.k1 = {n = 1, odd = true}");
		rawrbox::runUtilsLua(L, "REBUILT = {}; for i = 40, 1, -1 do if i == 1 or i % 3 ~= 1 then REBUILT['k' .. i] = {[i % 2 == 0 and 'even' or 'odd'] = true, n = i} end end");

		const auto live = rawrbox::LuaUtils::toJSON(luabridge::getGlobal(L, "LIVE"));
		const auto rebuilt = rawrbox::LuaUtils::toJSON(luabridge::getGlobal(L, "REBUILT"));
		REQUIRE(rawrbox::writeUtilsJSON(live.value()) == rawrbox::writeUtilsJSON(rebuilt.value()));
		REQUIRE(std::ranges::is_sorted(live.value().get_object(), {}, [](const auto& field) { return field.first; }));
		REQUIRE(rawrbox::writeUtilsJSON(rawrbox::LuaUtils::toJSON(rawrbox::LuaUtils::fromJSON(L, live.value()).value()).value()) == rawrbox::writeUtilsJSON(live.value()));

		const auto gaps = rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "{list = {[1] = 1, [3] = 3}}"));
		REQUIRE_FALSE(gaps.has_value());
		REQUIRE(gaps.error() == "value.list invalid list");

		REQUIRE(rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "{1, 2, name = 'x'}")).error() == "value invalid json keys");
		REQUIRE(rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "{a = {{fn = print}}}")).error() == "value.a[1].fn is a function! Not plain data");
		REQUIRE(rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "{Vector2i = 5}")).error().find("rename the field") != std::string::npos);
		REQUIRE(rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "0/0")).error() == "value not a finite number");
		REQUIRE(rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "{{{1}}}"), {.maxDepth = 2}).error() == "value[1][1] is nested deeper than 2 tables");

		rawrbox::runUtilsLua(L, "CYCLE = {}; CYCLE.self = CYCLE");
		REQUIRE_FALSE(rawrbox::LuaUtils::toJSON(luabridge::getGlobal(L, "CYCLE")).has_value());

		auto loose = rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "{1, 2, name = 'x', fn = print}"), {.strict = false});
		REQUIRE(loose.has_value());
		REQUIRE(loose.value()["1"].get<double>() == 1.0);
		REQUIRE(loose.value()["name"].get<std::string>() == "x");
		REQUIRE(loose.value()["fn"].get<std::string>().starts_with("function("));

		lua_close(L);
	}

	SECTION("rawrbox::LuaUtils::fromJSON") {
		lua_State* L = rawrbox::newUtilsState();
		const int top = lua_gettop(L);

		{
			const auto value = rawrbox::LuaUtils::fromJSON(L, rawrbox::parseUtilsJSON(R"({"list": [1, 2, 3], "pos": {"Vector2i": [4, -2]}, "tint": {"Color": [1, 0.5, 0, 1]}, "empty": null})"));
			REQUIRE(value.has_value());
			REQUIRE(lua_gettop(L) == top);

			luabridge::setGlobal(L, value.value(), "DATA");
			rawrbox::runUtilsLua(L, "OK = #DATA.list == 3 and DATA.list[3] == 3 and DATA.pos.x == 4 and DATA.pos.y == -2 and DATA.tint.g == 0.5 and DATA.empty == nil");
			REQUIRE(luabridge::getGlobal(L, "OK").unsafe_cast<bool>());

			const auto back = rawrbox::LuaUtils::toJSON(value.value());
			REQUIRE(rawrbox::writeUtilsJSON(back.value()) == R"({"list":[1,2,3],"pos":{"Vector2i":[4,-2]},"tint":{"Color":[1,0.5,0,1]}})");
		}

		{
			const auto limit = rawrbox::LuaUtils::toJSON(rawrbox::evalUtilsLua(L, "{{1, 2}}"), {.maxDepth = 2});
			REQUIRE(limit.has_value());

			const auto value = rawrbox::LuaUtils::fromJSON(L, limit.value(), {.maxDepth = 2});
			REQUIRE(value.has_value());
			REQUIRE(value.value()[1][2].unsafe_cast<int>() == 2);
		}

		REQUIRE(rawrbox::LuaUtils::fromJSON(L, rawrbox::parseUtilsJSON("[[[1]]]"), {.maxDepth = 2}).error() == "value[1][1] is nested deeper than 2 tables");
		REQUIRE(rawrbox::LuaUtils::fromJSON(L, rawrbox::parseUtilsJSON(R"({"a": {"Vector2i": [1.5, 2]}})")).error() == "value.a is not a valid Vector2i");
		REQUIRE(rawrbox::LuaUtils::fromJSON(L, rawrbox::parseUtilsJSON(R"({"a": {"Vector2i": [1, 2, 3]}})")).error() == "value.a is not a valid Vector2i");
		REQUIRE(lua_gettop(L) == top);

		lua_close(L);
	}

	SECTION("rawrbox::LuaUtils::copy") {
		lua_State* a = rawrbox::newUtilsState();
		lua_State* b = rawrbox::newUtilsState();

		const int top = lua_gettop(b);

		rawrbox::runUtilsLua(a, "DATA = {1, 2, {nested = true}, name = 'x', pos = Vector2i(3, 4), [true] = 'yes'}");
		lua_getglobal(a, "DATA");
		REQUIRE(rawrbox::LuaUtils::copy(a, -1, b).has_value());
		lua_pop(a, 1);

		REQUIRE(lua_gettop(b) == top + 1);
		lua_setglobal(b, "COPY");

		rawrbox::runUtilsLua(b, "OK = COPY[1] == 1 and COPY[3].nested and COPY.name == 'x' and COPY.pos.x == 3 and COPY.pos.y == 4 and COPY[true] == 'yes'");
		REQUIRE(luabridge::getGlobal(b, "OK").unsafe_cast<bool>());

		rawrbox::runUtilsLua(b, "COPY[3].nested = false; COPY.name = 'y'");
		rawrbox::runUtilsLua(a, "OK = DATA[3].nested and DATA.name == 'x'");
		REQUIRE(luabridge::getGlobal(a, "OK").unsafe_cast<bool>());

		rawrbox::runUtilsLua(a, "BAD = {1, {fn = print}}");
		lua_getglobal(a, "BAD");

		const auto failed = rawrbox::LuaUtils::copy(a, -1, b);
		REQUIRE_FALSE(failed.has_value());
		REQUIRE(failed.error() == "value[2].fn is a function. Cannot be copied");
		REQUIRE(lua_gettop(b) == top);

		lua_pop(a, 1);

		const int same = lua_gettop(a);

		rawrbox::runUtilsLua(a, "SAME = {1, {2, 3}, name = 'x', pos = Vector2i(5, 6)}");
		lua_getglobal(a, "SAME");
		REQUIRE(rawrbox::LuaUtils::copy(a, -1, a).has_value());
		REQUIRE(lua_gettop(a) == same + 2);
		REQUIRE(lua_rawequal(a, -1, -2) == 0);

		lua_setglobal(a, "SAME_COPY");
		lua_pop(a, 1);

		rawrbox::runUtilsLua(a, "OK = SAME_COPY[1] == 1 and SAME_COPY[2][2] == 3 and SAME_COPY.name == 'x' and SAME_COPY.pos.y == 6 and SAME_COPY[2] ~= SAME[2]");
		REQUIRE(luabridge::getGlobal(a, "OK").unsafe_cast<bool>());

		lua_getglobal(a, "BAD");
		REQUIRE_FALSE(rawrbox::LuaUtils::copy(a, -1, a).has_value());
		REQUIRE(lua_gettop(a) == same + 1);
		lua_pop(a, 1);

		lua_close(a);
		lua_close(b);
	}

	SECTION("rawrbox::LuaUtils::freeze") {
		lua_State* L = rawrbox::newUtilsState();

		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::LuaUtilsTestCounter>("Counter")
		    .addConstructor<void()>()
		    .addPropertyReadWrite("value", &rawrbox::LuaUtilsTestCounter::value)
		    .addFunction("bump", &rawrbox::LuaUtilsTestCounter::bump)
		    .endClass();

		rawrbox::runUtilsLua(L, "DATA = {list = {1, 2}, counter = Counter(), [{}] = Counter()}; DATA.counter.value = 1; DATA.counter:bump()");
		lua_getglobal(L, "DATA");
		rawrbox::LuaUtils::freeze(L, -1);
		lua_pop(L, 1);

		REQUIRE(rawrbox::failsUtilsLua(L, "DATA.extra = 1"));
		REQUIRE(rawrbox::failsUtilsLua(L, "DATA.list[1] = 5"));
		REQUIRE(rawrbox::failsUtilsLua(L, "DATA.counter.value = 5"));
		REQUIRE(rawrbox::failsUtilsLua(L, "DATA.counter:bump()"));
		REQUIRE(rawrbox::failsUtilsLua(L, "for key in DATA do if type(key) == 'table' then key.x = 1 end end"));
		REQUIRE_FALSE(rawrbox::failsUtilsLua(L, "assert(DATA.counter.value == 2 and DATA.list[2] == 2)"));

		lua_close(L);
	}

	SECTION("rawrbox::LuaUtils::cached") {
		lua_State* L = rawrbox::newUtilsState();

		int builds = 0;

		{
			const auto build = [&builds](lua_State* main) {
				++builds;
				
				auto table = luabridge::newTable(main);
				table["builds"] = builds;

				return table;
			};

			auto first = rawrbox::LuaUtils::cached(L, "test", uint64_t{1}, build);
			auto again = rawrbox::LuaUtils::cached(L, "test", uint64_t{1}, build);
			REQUIRE(builds == 1);
			REQUIRE(first.rawequal(again));

			auto next = rawrbox::LuaUtils::cached(L, "test", uint64_t{2}, build);
			REQUIRE(builds == 2);
			REQUIRE(next["builds"].unsafe_cast<int>() == 2);

			luabridge::setGlobal(L, next, "CACHED");
		}

		REQUIRE(rawrbox::failsUtilsLua(L, "CACHED.builds = 5"));
		lua_close(L);
	}

	SECTION("rawrbox::LuaUtils::registerEnum") {
		lua_State* L = rawrbox::newUtilsState();
		rawrbox::LuaUtils::registerEnum<rawrbox::LuaUtilsTestEnum>(L, "TestEnum");

		REQUIRE_FALSE(rawrbox::failsUtilsLua(L, "assert(TestEnum.FIRST == 1 and TestEnum.SECOND == 4)"));
		REQUIRE(rawrbox::failsUtilsLua(L, "TestEnum.THIRD = 5"));

		REQUIRE(static_cast<bool>(luabridge::Stack<rawrbox::LuaUtilsTestEnum>::push(L, rawrbox::LuaUtilsTestEnum::SECOND)));
		REQUIRE(luabridge::Stack<rawrbox::LuaUtilsTestEnum>::get(L, -1).value() == rawrbox::LuaUtilsTestEnum::SECOND);
		lua_pop(L, 1);

		lua_pushnumber(L, 2);
		REQUIRE_FALSE(static_cast<bool>(luabridge::Stack<rawrbox::LuaUtilsTestEnum>::get(L, -1)));
		lua_pop(L, 1);

		lua_close(L);
	}

	SECTION("i18n.find") {
		lua_State* L = rawrbox::newUtilsState();
		rawrbox::I18NWrapper::registerLua(L);

		rawrbox::I18N::setLanguage("en");
		rawrbox::I18N::addToLanguagePack("lua_find", "en", {{"hello", "hello {}"}});

		REQUIRE_FALSE(rawrbox::failsUtilsLua(L, "assert(i18n.find('lua_find', 'hello', 'cat') == 'hello cat')"));
		REQUIRE_FALSE(rawrbox::failsUtilsLua(L, "assert(i18n.find('lua_find', 'missing') == nil)"));
		REQUIRE_FALSE(rawrbox::failsUtilsLua(L, "assert(i18n.find('nope', 'hello') == nil)"));

		lua_close(L);
	}

	SECTION("rawrbox::LuaUtils::precompile") {
		const std::string folder = "./rawrbox-tests-lua";
		std::filesystem::create_directories(folder);

		const auto bytecode = rawrbox::LuaUtils::precompile("PRECOMPILED = 42");
		REQUIRE(bytecode.starts_with("\x1bLuau"));
		REQUIRE_THROWS(rawrbox::LuaUtils::precompile("this is not lua"));

		std::ofstream(folder + "/precompiled.lua", std::ios::binary) << bytecode;
		std::ofstream(folder + "/source.lua", std::ios::binary) << "SOURCE = 7";

		lua_State* L = luaL_newstate();
		luaL_openlibs(L);

		REQUIRE_NOTHROW(rawrbox::LuaUtils::compileAndLoadFile(L, "precompiled", folder + "/precompiled.lua"));
		REQUIRE_NOTHROW(rawrbox::LuaUtils::compileAndLoadFile(L, "source", folder + "/source.lua"));

		lua_getglobal(L, "PRECOMPILED");
		REQUIRE(lua_tonumber(L, -1) == 42);
		lua_getglobal(L, "SOURCE");
		REQUIRE(lua_tonumber(L, -1) == 7);

		lua_close(L);
		std::filesystem::remove_all(folder);
	}

	SECTION("rawrbox::LuaUtils::loadBytecode") {
		lua_State* L = luaL_newstate();
		luaL_openlibs(L);
		Luau::CodeGen::create(L);

		REQUIRE_NOTHROW(rawrbox::LuaUtils::compileAndLoadScript(L, "native", "function SUM(n) local s = 0 for i = 1, n do s += i end return s end"));

		lua_getglobal(L, "SUM");
		REQUIRE(Luau::CodeGen::compile(L, -1, 0).result == Luau::CodeGen::CodeGenCompilationResult::NothingToCompile);

		lua_pushnumber(L, 10);
		lua_call(L, 1, 1);
		REQUIRE(lua_tonumber(L, -1) == 55);

		lua_close(L);
	}
}