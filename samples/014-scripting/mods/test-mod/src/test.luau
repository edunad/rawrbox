function testRawrbox()
    local v2 = Vector2(23, 43)
    print(string.vformat("test vector2 | x: '{}' y: '{}'", v2.x, v2.y))

    local v3 = Vector3(23, 43, 53)
    print(string.vformat("test vector3 | x: '{}' y: '{}' z: '{}'", v3.x, v3.y, v3.z))

    local v4 = Vector4(23, 43, 53, 12)
    print(string.vformat("test vector4 | x: '{}' y: '{}' z: '{}' w: '{}'", v4.x, v4.y, v4.z, v4.w))

    local c = Color(255, 0, 0, 255)
    print(string.vformat("test color | r: '{}' g: '{}' b: '{}' a: '{}'", c.r, c.g, c.b, c.a))

    local m = Matrix()
    print("test matrix")

    for i = 0, 15 do
        print(string.vformat("[{}] = {}", i, m[i]))
    end
end
