local apple = apple or {}
apple.frames = {}
apple.buffer = {}
apple.width = 96   -- 96 -- 40
apple.height = 128 -- 128 -- 60
apple.frameRate = 1000 / 15
apple.snd = {}

local instances = {}
local colWhite = Color(1, 1, 1, 1)
local colBlack = Color(0, 0, 0, 1)

local function parse(data)
    apple.data = data

    -- Calculate all frames
    apple.frames = {}
    for i, frame in pairs(string.split(data, "/")) do
        table.insert(apple.frames, frame)
    end

    -- Setup first buffer
    apple.buffer = {}
    print("=== Done")
end

local function stopAnimation()
    timer.destroy("badApple")
    if apple.snd then
        apple.snd:stop()
    end
end

local function playAnimation()
    print("Start bad apple!")
    stopAnimation()

    if apple.snd then
        apple.snd:setVolume(0.2)
        apple.snd:play()
    end

    local currentFrame = 1
    timer.create("badApple", -1, apple.frameRate, function()
        local data = string.split(apple.frames[currentFrame], ":")

        local mdl = test_model2()
        if not mdl then return end

        -- Insert into buffer
        for i, v in ipairs(data) do
            if v == "" then continue end

            local vv = tonumber(v)
            apple.buffer[vv] = not apple.buffer[vv]
        end

        -- render
        for k, v in ipairs(instances) do
            if not v then continue end
            if apple.buffer[k - 1] then
                v:setColor(colWhite)
            else
                v:setColor(colBlack)
            end
        end

        mdl:updateBuffers()

        currentFrame = currentFrame + 1
        if currentFrame > #apple.frames then stopAnimation() end
    end)
end

function MOD:onLoad()
    if not resources then
        error("resources module not available, cannot continue")
        return
    end

    print("---- MOD LOADING")
    resources.preLoad("./assets/apple.ogg")
end


function MOD:onReady()
    if not bass then
        error("BASS plugin not available, cannot continue")
        return
    end

    if not http then
        error("http plugin not available, cannot continue")
        return
    end

    print("Loading bad apple...")

    local mdl = test_model2()
    if mdl then
        mdl:setAutoUpload(false)
        print("model is ok, yey")

        for x = 1, apple.width do
            for y = 1, apple.height do
                local ins = Instance()

                ins:setMatrix(Matrix.mtxSRT(Vector3(1, 1, 1), Vector4(0, 0, 0, 0), Vector3(y * 0.1, 1, -x * 0.1)))
                ins:setColor(colBlack)

                mdl:addInstance(ins)
            end
        end

        mdl:setPos(Vector3(-2, 0, 2))

        for i = 1, apple.width * apple.height do
            table.insert(instances, mdl:getInstance(i - 1));
        end
    end

    ------------------------------------------------------
    apple.snd = bass.loadSound("./assets/apple.ogg")
    if apple.snd then
        http.request("https://ams3.digitaloceanspaces.com/failcake/public/badapple/badapple.txt", _G.HTTP_METHOD.GET, {},
            function(success, data)
                if not success then
                    print("ERROR", data)
                    return
                end

                parse(data.data)
                playAnimation()
            end, 50000)
    end
end
