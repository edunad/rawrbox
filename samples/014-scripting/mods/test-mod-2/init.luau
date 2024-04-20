local testMDL = nil
local testMDLMesh = nil

function TEST()
    print("You called test! On test-mod-2")
end

function MOD:draw(pass)
    if pass ~= _G.DRAW_PASS.OVERLAY then return end

    local frame = renderer.frame()
    stencil.drawBox(Vector2(10 + math.cos(frame * 0.1) * 5, 10 + math.sin(frame * 0.1) * 5), Vector2(10, 10), Color(1, 0, 0, 1))
end

function MOD:update()
    if not testMDL then
        local mdl = test_model()
        if not mdl then return end

        testMDL = mdl
    end

    if testMDL then
        if not testMDLMesh then
            local mesh1 = testMDL:getMesh(0)
            if not mesh1 then return end
            testMDLMesh = mesh1
        end

        if testMDLMesh then
            local frame = renderer.frame()
            testMDLMesh:setPos(Vector3(-5., 0, math.sin(frame * 0.01) * 2))
        end
    end
end
