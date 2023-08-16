include("./src/test.lua")

function MOD:init()
    if type(jit) == 'table' then print(jit.version) end
    print("\nTEST Mod initialized!\n")

    testRawrbox()

    -- ACCESS ENV ON MOD 2
    local mod2 = scripting:getMod("test-mod-2")
    if not mod2 then return end
    mod2:getENV().TEST()
end

print(test:hello("hot reload"))
