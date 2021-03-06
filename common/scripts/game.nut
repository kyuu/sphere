RequireScript("/common/scripts/kbd")
RequireScript("/common/scripts/mouse")
RequireScript("/common/scripts/console")
RequireScript("/common/scripts/font")

Game <- {
    // callbacks (update and render callbacks must be defined)
    init   = null
    deinit = null
    update = null
    render = null

    function setWindowMode(width, height, fullScreen, scale = false) {
        Assert(width > 0)
        Assert(height > 0)
        Assert(typeof fullScreen == "bool")
        Assert(typeof scale == "bool")

        _width = width
        _height = height
        _fullScreen = fullScreen
        _scale = scale

        if (_isRunning) {
            SetWindowMode(_width * (_scale ? 2 : 1), _height * (_scale ? 2 : 1), _fullScreen)
        }
    }

    function getWidth() {
        return _width
    }

    function getHeight() {
        return _height
    }

    function getFullScreen() {
        return _fullScreen
    }

    function setFullScreen(fullScreen) {
        Assert(typeof fullScreen == "bool")
        if (_fullScreen != fullScreen) {
            _fullScreen = fullScreen
            if (_isRunning) {
                SetWindowMode(_width * (_scale ? 2 : 1), _height * (_scale ? 2 : 1), _fullScreen)
            }
        }
    }

    function getScale() {
        return _scale
    }

    function setScale(scale) {
        Assert(typeof scale == "bool")
        if (_scale != scale) {
            _scale = scale
            if (_isRunning) {
                SetWindowMode(_width * (_scale ? 2 : 1), _height * (_scale ? 2 : 1), _fullScreen)
            }
        }
    }

    function getFrameRate() {
        return _frameRate
    }

    function setFrameRate(frameRate) {
        if (frameRate > 0) {
            _frameRate = frameRate
            _idealTime = GetTicks() * frameRate + 1000
            _currentFPS = 0
            _fpsCounter = 0
            _fpsNextUpdate = GetTicks() + 1000
        } else {
            _frameRate = 0
            _idealTime = 0
            _currentFPS = 0
            _fpsCounter = 0
            _fpsNextUpdate = GetTicks() + 1000
        }
    }

    function getFPS() {
        return _currentFPS
    }

    function getShowFPS() {
        return _showFPS
    }

    function setShowFPS(showFPS) {
        Assert(typeof showFPS == "bool")
        _showFPS = showFPS
    }

    function addUpdateScript(name, script) {
        _updateScripts[name] <- script
    }

    function removeUpdateScript(name) {
        if (name in _updateScripts) {
            delete _updateScripts[name]
        }
    }

    function addPostRenderScript(name, script) {
        _postRenderScript[name] <- script
    }

    function removePostRenderScript(name) {
        if (name in _postRenderScript) {
            delete _postRenderScript[name]
        }
    }

    function getSystemFont() {
        return _systemFont
    }

    function getSystemCursor() {
        return _systemCursor
    }

    function enterConsoleMode() {
        if (!_console) {
            _console = Console(_systemFont)
        }
    }

    // private variables
    _isRunning = false
    _shouldQuit = false

    _width = 0
    _height = 0
    _fullScreen = false
    _scale = false
    _showFPS = false

    _frameRate = 0
    _idealTime = 0
    _currentFPS = 0
    _fpsCounter = 0
    _fpsNextUpdate = 0

    _updateScripts = {}
    _postRenderScripts = {}

    _systemFont = Font.FromFile("/common/system/font.rfn")
    _systemCursor = Texture.FromFile("/common/system/cursor.png")

    _console = null
}

function main(...) {
    // call init callback if provided
    if (Game.init) {
        Game.init(vargv)
    }

    // at this point width and height must be valid
    Assert(Game._width > 0)
    Assert(Game._height > 0)

    // set window mode
    SetWindowMode(Game._width * (Game._scale ? 2 : 1), Game._height * (Game._scale ? 2 : 1), Game._fullScreen)

    // the game loop
    Game._isRunning = true
    while (!Game._shouldQuit) {
        UpdateSystem()

        // dispatch window events
        while (PeekWindowEvent()) {
            local event = GetWindowEvent()
            switch (event.type) {
                case WE_WINDOW_CLOSE:
                    Game._shouldQuit = true
                    break
                case WE_KEY_PRESS:
                    local key = Kbd[Kbd._keyToSlot[event.key]]
                    key.pressed = true
                    if (event.key == KEY_CAPSLOCK) {
                        key.active = !key.active
                    }
                    Kbd.events.push({
                        key   = key
                        shift = Kbd.shift.pressed || Kbd.capslock.active
                        ctrl  = Kbd.ctrl.pressed
                        alt   = Kbd.alt.pressed
                    })
                    if (key.onPress) {
                        key.onPress()
                    }
                    break
                case WE_KEY_RELEASE:
                    local key = Kbd[Kbd._keyToSlot[event.key]]
                    key.pressed = false
                    if (key.onRelease) {
                        key.onRelease()
                    }
                    break
                case WE_MOUSE_BUTTON_PRESS:
                    local button = Mouse[Mouse._buttonToSlot[event.button]]
                    button.pressed = true
                    if (button.onPress) {
                        button.onPress()
                    }
                    break
                case WE_MOUSE_BUTTON_RELEASE:
                    local button = Mouse[Mouse._buttonToSlot[event.button]]
                    button.pressed = false
                    if (button.onRelease) {
                        button.onRelease()
                    }
                    break
                case WE_MOUSE_MOTION:
                    Mouse.x = event.x
                    Mouse.y = event.y
                    // if we are in scaled mode, the coordinates need to be scaled down
                    if (Game._scale) {
                        Mouse.x /= 2
                        Mouse.y /= 2
                    }
                    break
            }
        }

        if (Game._console) {
            if (Game._console.update() == false) {
                // quit console mode
                Game._console = null
            }
            if (Game._console) {
                Game._console.render()
            }
        } else {
            // execute update scripts
            foreach (updateScript in Game._updateScripts) {
                updateScript()
            }

            // call update callback
            if (Game.update() == false) {
                // the update callback requests to quit
                Game._shouldQuit = true
            }

            // call render callback
            Game.render()

            // execute post-render scripts
            foreach (postRenderScript in Game._postRenderScripts) {
                postRenderScript()
            }
        }

        // display FPS
        if (Game._showFPS) {
            Game._systemFont.drawString("FPS: " + Game._currentFPS, 5, 5)
        }

        // scale frame
        if (Game._scale) {
            local w = Game._width
            local h = Game._height
            local bm = GetBlendMode()
            SetBlendMode(BM_REPLACE)
            CaptureFrame(0, 0, w, h)
            DrawCaptureQuad(0, 0, w, h, 0, 0, w*2, 0, w*2, h*2, 0, h*2)
            SetBlendMode(bm)
        }

        // show the effects of drawing in the window
        SwapWindowBuffers()

        // throttle frame rate
        if (Game._frameRate > 0) {
            while (GetTicks() * Game._frameRate < Game._idealTime) {
                Sleep(1)
            }
            Game._idealTime += 1000
        }

        // update fps counter
        Game._fpsCounter++

        // update fps variable
        if (GetTicks() >= Game._fpsNextUpdate) {
            Game._currentFPS = Game._fpsCounter
            Game._fpsCounter = 0
            Game._fpsNextUpdate += 1000
        }
    }

    Game._isRunning = false

    // call deinit callback if provided
    if (Game.deinit) {
        Game.deinit()
    }
}
