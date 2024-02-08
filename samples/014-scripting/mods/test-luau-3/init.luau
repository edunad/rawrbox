
--[[i18n.setLanguage("en")
print(i18n.get("", "hello", "world"))
i18n.setLanguage("repti")
print(i18n.get("", "hello", "world"))


if not hasMOD("aaaaa") then
    print("mod not found")
end

local mod = getMOD("test-luau")
print(mod:getID())
print(mod:getFolder())
mod:call("meow", 20, true, {potato = 20})

print(curtime())]]--

--[[console.removeCommand("test")
console.registerMethod("test", function(args)
    print("called test command")
    printTable(args)
    return true, "ok"
end, "meow")

console.execute("test", "arg1")]]--

--[[local f = string.vformat("hello {}", "cookies")
print(f)
print("hello {}", "cookies")
]]--


--[[
local sound = {}
function MOD:init()
    sound = bass.loadHTTPSound("https://ams3.digitaloceanspaces.com/failcake/public/random_sounds/singusasong.ogg", BASS.FLAGS.BEAT_DETECTION)
    sound:setVolume(0.3)
    sound:play()

    sound:onBEAT(function(beat)
        print("beat", beat)
    end)
end
]]--

--[[
local headers = {
    ["Content-Type"] = "application/json"
}

http.request("https://rawr.dev", HTTP_METHOD.GET, headers, function(success, data)
    print(success)
    printTable(data)
end)
]]--
