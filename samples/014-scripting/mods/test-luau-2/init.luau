
--__mod_folder = "aaaa"
__mod_folder = "potato"
print(__mod_folder)
print(_G.__mod_folder)
print(_G.__mod_id)


--_G.__mod_id = "a"
include("./src/mew.luau")

function MOD:test()
    print("called mod test on test-luau-2!")
end

