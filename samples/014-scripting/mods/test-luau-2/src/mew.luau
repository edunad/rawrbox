--[[print("meow script!")

print(engine.deltaTime)

-- HOOK TESTS ----
hooks.remove("test", "my-hook")
hooks.add("test", "my-hook", function(aeee)
    print("hook test")
    print(aeee)
end)
hooks.call("test", 20)
-- -----------------

-- IO TESTS ----
local saveErr = io.save("meow.txt", engine.deltaTime)
if saveErr ~= "" then print(saveErr) end

local loadData = io.load("meow.txt")
if loadData[1] ~= "" then
    print(loadData[1])
else
    print("loaded data")
    print(loadData[2])
end
-- -----------------

---- TIMER TEST -----
timer.simple("meow", 4000, function()
    print("simple timer")
end, function() print("simple timer end") end)

timer.create("meow2", 2, 1000, function()
    print("timer tick")
end)
----------------------
]]
