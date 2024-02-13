--[[local sun = nil

function MOD:draw(pass)
    if pass ~= DRAW_PASS.OVERLAY then return end

    local frame = renderer.frame()
    stencil.drawBox(Vector2(15 + math.cos(frame * 0.1) * 10,15 + math.sin(frame * 0.1) * 10), Vector2(50, 50), Color(255, 0, 0, 255))
end


function MOD:onReady()
    print("test-mod onReady")
    sun = lights.get(1)
end


function MOD:update()
    if sun == nil then return end
    local frame = renderer.frame()
    sun:setColor(Color(math.cos(frame * 0.1) * 10, 0, 0, 255))
end


local ma = Vector2i(20, 20)
print(tostring(ma))

local be = Vector2(40, 40)
print(tostring(be))
print(be:floor())]]
