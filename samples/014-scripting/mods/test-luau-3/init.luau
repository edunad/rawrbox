
i18n.setLanguage("en")
print(i18n.get("", "hello", "world"))
i18n.setLanguage("repti")
print(i18n.get("", "hello", "world"))


if not hasMOD("aaaaa") then
    print("mod not found")
end

local mod = getMOD("test-luau")
mod:call("meow", 20, true, {potato = 20})
