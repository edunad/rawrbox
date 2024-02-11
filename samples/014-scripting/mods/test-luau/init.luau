
function MOD:draw(pass)
    if pass ~= DRAW_PASS.OVERLAY then return end

    local frame = renderer.frame()
    stencil.drawBox(Vector2(15 + math.cos(frame * 0.1) * 10,15 + math.sin(frame * 0.1) * 10), Vector2(50, 50), Color(255, 0, 0, 255))
end
