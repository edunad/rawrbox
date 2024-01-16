function TEST()
    print("You called test! On test-mod-2")
end

function MOD:drawOverlay()
    stencil:drawBox(Vector2:new(0, 0), Vector2:new(10, 10), Color:new(255, 0, 0, 255))
end

function MOD:update()
    local mdl = test_model()
    if not mdl or mdl:isValid() then
        local mesh1 = mdl:getMesh(0)
        if not mesh1 then return end

        local p = mesh1:getPos()
        p.z = math.sin(FRAME() * 0.01) * 2.
        p.x = -5.

        mesh1:setPos(p)
    end
end
