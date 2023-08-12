_G.TEST = function()
    print("You called test! On test-mod-2")
end

function MOD:drawOverlay()
    stencil:drawBox(Vector2:new(0, 0), Vector2:new(10, 10), Color:new(255, 0, 0, 255))
end

function MOD:update()
    local mdl = test_model()
    if mdl:isValid() then
        local mesh1 = mdl:getMesh(0)
        if not mesh1 then return end

        local p = mesh1:getPos()
        p.x = math.cos(curtime() * 0.001) * 2.
        p.z = math.sin(curtime() * 0.001) * 2.

        mesh1:setPos(p)
    end
end
