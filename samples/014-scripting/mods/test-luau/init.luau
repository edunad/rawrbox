
function MOD:init()
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

print("owwwoo")
