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
