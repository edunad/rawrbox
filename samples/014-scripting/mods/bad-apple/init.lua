local apple = apple or {}
apple.frames = {}
apple.buffer = {}
apple.width = 60  -- 128 -- 60
apple.height = 40 -- 96 -- 40
apple.frameRate = 15
apple.snd = {}

local function parse(data)
    apple.data = data

    -- Calculate all frames
    apple.frames = {}
    for i, frame in pairs(string.split(data, "/")) do
        table.insert(apple.frames, frame)
    end

    -- Setup first buffer
    apple.buffer = {}
    for y = 1, apple.height - 1 do
        for x = 1, apple.width - 1 do
            apple.buffer[x + y * apple.width] = false
        end
    end

    print("=== Done")
end

local function getBuffer(x, y)
    return apple.buffer[x + y * apple.width]
end

local function stopAnimation()
    timer:destroy("badApple")
    if apple.snd:isValid() then
        apple.snd:stop()
    end
end

local function playAnimation()
    print("Start bad apple!")
    stopAnimation()

    if apple.snd:isValid() then
        apple.snd:play()
    end

    timer:create("badApple", 1 / apple.frameRate, -1, function()
        local message = ""
        local rawData = table.remove(apple.frames, 1)
        local data = string.split(rawData, ":")

        -- Insert into buffer
        for i, v in pairs(data) do
            if v == "" then goto loop_end end
            local x, y = tonumber(v) % apple.width, math.floor(tonumber(v) / apple.width)
            local key = x + y * apple.width

            apple.buffer[key] = not apple.buffer[key]
            ::loop_end::
        end

        -- Render
        for y = 1, apple.height - 1 do
            for x = 1, apple.width - 1 do
                local char = " "
                if getBuffer(x, y) then char = "#" end
                message = message .. char
            end

            message = message .. "\n"
        end

        if apple.label and apple.label:isValid() then
            apple.label:setText(message)
        end

        if #apple.frames <= 0 then stopAnimation() end
    end)
end

function MOD:onLoad()
    print("---- MOD LOADING")
    resources:preLoad("./content/apple.ogg")
end

function MOD:onReady()
    print("Loading bad apple...")

    local mdl = test_model2()
    if mdl:isValid() then
        local ins = Instance:new()

        local mtx = Matrix:new()
        mtx:mtxSRT(Vector3:new(1, 1, 1), Vector4:new(0, 0, 0, 0), Vector3:new(1, 1, 1))

        ins:setMatrix(mtx)
        mdl:addInstance(ins)
    end

    --[[apple.snd = BASS:loadSound("./content/apple.ogg")
    if apple.snd:isValid() then
        print("Loading bad apple")
        http:request("https://ams3.digitaloceanspaces.com/failcake/public/badapple/caw.txt", HTTP.GET, {},
            function(err, data)
                if err then
                    print("ERROR: ", data)
                    return
                end

                parse(data.data)
                timer:simple("apple", 5000, function() playAnimation() end)
            end)
    end]]
end
