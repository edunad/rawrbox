--[[function MOD:init()
    print("hello from init")
end

function MOD:update(meow)
    print("update", meow)
    print(curtime())
end

printTable({
    bla = 1,
    ok = true,
    meow = "aaa",
    okok = {
        hey = {
            bla = 2,
            ok = function()
                print("woooo")
                return 1
            end
        },
        sss = "sss",
        bla = nil
    }
})

print(string.bytesToReadable(2323))
print(json.encode({bla = 10}))

local vec = Vector2(2330)
print(vec.x)
vec.x = 23
print(vec.x)

meow = function(data)
    printTable(data)
    print("called meow")
end]]

--[[
    timer.create("meow", -1, 1000, function()
    camera.setPos(Vector3(0,math.random(0, 10),0))
end)
]]

--[[
timer.create("meow", -1, 1000, function()
    local window = window.get(0)
    window:setPos(Vector2(0,math.random(0, 10)))

    print("move window")
end)
]]
