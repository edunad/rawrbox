
function MOD:init()
    print("hello from init")
end

function MOD:update(meow)
    print("update", meow)
    print(curtime())
end

function MOD:test()
    print("called mod test on test-luau!")
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
