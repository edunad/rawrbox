_G.TEST = function()
    print("You called test! On test-mod-2")
end

function MOD:drawOverlay()
    stencil:drawBox(Vector2:new(0, 0), Vector2:new(10, 10), Color:new(255, 0, 0, 255))
end
