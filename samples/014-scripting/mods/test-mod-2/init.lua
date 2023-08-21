local snd = {}

function TEST()
    print("You called test! On test-mod-2")
end

function MOD:drawOverlay()
    stencil:drawBox(Vector2:new(0, 0), Vector2:new(10, 10), Color:new(255, 0, 0, 255))
end

function MOD:onLoad()
    print("---- MOD LOADING")
    resources:preLoad("./content/honk.ogg")
end

function MOD:onReady()
    --[[http:request("https://ams3.digitaloceanspaces.com/failcake/public/badapple/caw.txt", HTTP.GET, {},
        function(err, data)
            if err then
                print("ERROR: ", data)
                return
            end
        end)]]

    snd = BASS:loadSound("./content/honk.ogg")
    if snd:isValid() then
        timer:create("meow", -1, 2, function()
            snd:setVolume(0.5)
            snd:play()
        end, function() end)
    end
end

function MOD:update()
    local mdl = test_model()
    if mdl:isValid() then
        local mesh1 = mdl:getMesh(0)
        if not mesh1 then return end

        local p = mesh1:getPos()
        p.x = math.cos(BGFX_FRAME() * 0.01) * 2.
        p.z = math.sin(BGFX_FRAME() * 0.01) * 2.

        mesh1:setPos(p)
    end
end
