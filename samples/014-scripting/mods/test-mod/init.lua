include("./src/test.lua")
data = data or {}


function MOD:init()
    data.frame = nil
    data.ready = false

    if type(jit) == 'table' then print(jit.version) end
    print("\nTEST Mod initialized!\n")

    testRawrbox()

    -- ACCESS ENV ON MOD 2
    local mod2 = scripting:getMod("test-mod-2")
    if not mod2 then return end
    mod2:getENV().TEST()
    -------------------------------------------
end

function MOD:onLoad()
    if not resources then
        print("resources module not available, cannot continue")
        return
    end

    resources:preLoad("./content/gumball_wink.gif")
end

function drawUI()
    if not ui then
        print("UI module not available, cannot continue")
        return
    end
    if not data.ready then return end

    if data.frame and data.frame:isValid() then
        data.frame:remove()
        data.frame = nil
    end

    data.frame = ui:create("frame")
    if not data.frame or not data.frame:isValid() then return end

    data.frame:setTitle("test")
    data.frame:setSize(Vector2:new(240, 200))
    data.frame:setPos(Vector2:new(200, 100))
    data.frame:onClose(function()
        data.frame = nil
    end)

    local prog = ui:create("progress_bar", data.frame)
    if not prog or not prog:isValid() then return end
    prog:setSize(Vector2:new(200, 12))
    prog:setPos(Vector2:new(20, 22))
    prog:setValue(45)
    prog:setBarColor(Color:new(155, 0, 0, 255))


    local label = ui:create("label", data.frame)
    if not label or not label:isValid() then return end
    label:setText(i18n:get("", "test"))
    label:setFont("#/consola.ttf")
    label:setPos(Vector2:new(20, 3))
    label:sizeToContents()


    local button = ui:create("button", data.frame)
    if not button or not button:isValid() then return end
    button:setPos(Vector2:new(20, 40))
    button:setSize(Vector2:new(100, 24))
    button:setText("NYA PT")
    button:setEnabled(true)
    button:onClick(function()
        i18n:setLanguage("pt")
        label:setText(i18n:get("", "test"))
    end)

    local button2 = ui:create("button", data.frame)
    if not button2 or not button2:isValid() then return end
    button2:setPos(Vector2:new(110, 40))
    button2:setSize(Vector2:new(100, 24))
    button2:setText("NYA EN")
    button2:setEnabled(true)
    button2:onClick(function()
        i18n:setLanguage("en")
        label:setText(i18n:get("", "test"))
    end)

    local image = ui:create("image", data.frame)
    if not image or not image:isValid() then return end
    image:setPos(Vector2:new(10, 70))
    image:setSize(Vector2:new(64, 64))
    image:setTexture("./content/gumball_wink.gif")
end

function MOD:onReady()
    data.ready = true
    drawUI()
end

print(test:hello("hot reload"))
drawUI()
