include("./src/test.luau")
data = data or {}


local function drawUI()
    if not ui then
        error("UI module not available, cannot continue")
        return
    end
    if not data.ready then return end

    if data.frame then
        data.frame:remove()
        data.frame = nil
    end

    data.frame = ui.createFrame()
    if not data.frame then return end

    data.frame:setTitle("test")
    data.frame:setSize(Vector2(240, 200))
    data.frame:setPos(Vector2(200, 100))
    data.frame:onClose(function()
        data.frame = nil
    end)

    local prog = ui.createProgressBar(data.frame)
    if not prog then return end
    prog:setSize(Vector2(200, 12))
    prog:setPos(Vector2(20, 22))
    prog:setValue(45)
    prog:setBarColor(Color(0.5, 0, 0, 1))


    local label = ui.createLabel(data.frame)
    if not label then return end
    label:setText(i18n.get("", "test"))
    label:setPos(Vector2(20, 3))
    label:sizeToContents()


    local button = ui.createButton(data.frame)
    if not button then return end
    button:setPos(Vector2(20, 40))
    button:setSize(Vector2(100, 24))
    button:setText("NYA PT")
    button:setEnabled(true)
    button:onClick(function()
        i18n.setLanguage("pt")
        label:setText(i18n.get("", "test"))
    end)

    local button2 = ui.createButton(data.frame)
    if not button2 then return end
    button2:setPos(Vector2(110, 40))
    button2:setSize(Vector2(100, 24))
    button2:setText("NYA EN")
    button2:setEnabled(true)
    button2:onClick(function()
        i18n.setLanguage("en")
        label:setText(i18n.get("", "test"))
    end)

    local image = ui.createImage(data.frame)
    if not image then return end
    image:setPos(Vector2(10, 70))
    image:setSize(Vector2(64, 64))
    image:setTexturePath("./assets/gumball_wink.gif")
end

function MOD:onInit()
    data.frame = nil
    data.ready = false

    print("TEST Mod initialized!")
    testRawrbox()
end

function MOD:onLoad()
    if not resources then
        error("resources module not available, cannot continue")
        return
    end

    resources.preLoad("./assets/gumball_wink.gif")
end

function MOD:onReady()
    -- ACCESS ENV ON MOD 2
    local mod2 = mod.get("test-mod-2")
    if not mod2 then return end
    mod2:call("TEST")
    ---------------------------------------

    data.ready = true
    drawUI()
end
