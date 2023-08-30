local apple = apple or {}
apple.frames = {}
apple.buffer = {}
apple.width = 96   -- 96 -- 40
apple.height = 128 -- 128 -- 60
apple.frameRate = 1000 / 15
apple.snd = {}

local tonumber = tonumber
local Color = Color
local stringSplit = string.split

local instances = {}
local colWhite = Color:new(255, 255, 255, 255)
local colBlack = Color:new(1, 1, 1, 255)

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
    timer:destroy("badApple")
    if apple.snd:isValid() then
        apple.snd:stop()
    end
end

local function playAnimation()
    print("Start bad apple!")
    stopAnimation()

    if apple.snd:isValid() then
        apple.snd:setVolume(0.2)
        apple.snd:play()
    end

    local currentFrame = 1
    timer:create("badApple", -1, apple.frameRate, function()
        local data = stringSplit(apple.frames[currentFrame], ":")

        local mdl = test_model2()
        if not mdl:isValid() then return end

        -- Insert into buffer
        for i, v in ipairs(data) do
            if v == "" then goto loop_end end
            apple.buffer[tonumber(v)] = not apple.buffer[tonumber(v)]
            ::loop_end::
        end

        -- render
        for k, v in ipairs(instances) do
            if apple.buffer[k - 1] then
                v:setColor(colWhite)
            else
                v:setColor(colBlack)
            end
        end

        mdl:updateInstance()

        currentFrame = currentFrame + 1
        if currentFrame > #apple.frames then stopAnimation() end
    end)
end

function MOD:onLoad()
    print("---- MOD LOADING")
    resources:preLoad("./content/apple.ogg")
end

function MOD:onReady()
    print("Loading bad apple...")

    --[[local mdl = test_model2()

    if not mdl or mdl:isValid() then
        mdl:setAutoUpload(false)

        for x = 1, apple.width do
            for y = 1, apple.height do
                local ins = Instance:new()
                local mtx = Matrix:new()
                mtx:mtxSRT(Vector3:new(1, 1, 1), Vector4:new(0, 0, 0, 0), Vector3:new(y * 0.1, 1, -x * 0.1))

                ins:setMatrix(mtx)
                ins:setColor(colBlack)

                mdl:addInstance(ins)
            end
        end

        mdl:updateInstance()
        mdl:setPos(Vector3:new(-2, 0, 2))

        for i = 1, apple.width * apple.height do
            table.insert(instances, mdl:getInstance(i - 1));
        end
    end


    ------------------------------------------------------
    apple.snd = BASS:loadSound("./content/apple.ogg")
    if apple.snd:isValid() then
        http:request("https://ams3.digitaloceanspaces.com/failcake/public/badapple/badapple.txt", HTTP.GET, {},
            function(err, data)
                if err then
                    print("ERROR: ", data)
                    return
                end

                parse(data.data)
                playAnimation()
            end, 50000)
    end]]
end
