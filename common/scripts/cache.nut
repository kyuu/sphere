RequireScript("/common/scripts/font")
RequireScript("/common/scripts/windowstyle")

Cache <- {
    function texture(filename) {
        if (!(filename in _textures)) {
            _textures[filename] <- Texture.FromFile(filename)
        }
        return _textures[filename]
    }

    function removeTexture(filename) {
        if (filename in _textures) {
            delete _textures[filename]
        }
    }

    function canvas(filename) {
        if (!(filename in _canvases)) {
            _canvases[filename] <- Canvas.FromFile(filename)
        }
        return _canvases[filename]
    }

    function removeCanvas(filename) {
        if (filename in _canvases) {
            delete _canvases[filename]
        }
    }

    function sound(filename) {
        if (!(filename in _sounds)) {
            _sounds[filename] <- Sound.FromFile(filename)
        }
        return _sounds[filename]
    }

    function removeSound(filename) {
        if (filename in _sounds) {
            delete _sounds[filename]
        }
    }

    function soundEffect(filename) {
        if (!(filename in _soundEffects)) {
            _soundEffects[filename] <- SoundEffect.FromFile(filename)
        }
        return _soundEffects[filename]
    }

    function removeSoundEffect(filename) {
        if (filename in _soundEffects) {
            delete _soundEffects[filename]
        }
    }

    function font(filename) {
        if (!(filename in _fonts)) {
            _fonts[filename] <- Font.FromFile(filename)
        }
        return _fonts[filename]
    }

    function removeFont(filename) {
        if (filename in _fonts) {
            delete _fonts[filename]
        }
    }

    function windowStyle(filename) {
        if (!(filename in _windowStyles)) {
            _windowStyles[filename] <- WindowStyle.FromFile(filename)
        }
        return _windowStyles[filename]
    }

    function removeWindowStyle(filename) {
        if (filename in _windowStyles) {
            delete _windowStyles[filename]
        }
    }

    function saveState() {
        local state = {
            textures = []
            canvases = []
            sounds = []
            soundEffects = []
            fonts = []
            windowStyles = []
        }
        foreach (filename, texture in _textures) {
            state.textures.push(filename)
        }
        foreach (filename, canvas in _canvases) {
            state.canvases.push(filename)
        }
        foreach (filename, sound in _sounds) {
            state.sounds.push(filename)
        }
        foreach (filename, soundEffect in _soundEffects) {
            state.soundEffects.push(filename)
        }
        foreach (filename, font in _fonts) {
            state.fonts.push(filename)
        }
        foreach (filename, windowStyle in _windowStyles) {
            state.windowStyles.push(filename)
        }
        return state
    }

    function loadState(state) {
        foreach (filename in state.textures) {
            if (!(filename in _textures) {
                _textures[filename] <- Texture.FromFile(filename)
            }
        }
        foreach (filename in state.canvases) {
            if (!(filename in _canvases) {
                _canvases[filename] <- Canvas.FromFile(filename)
            }
        }
        foreach (filename in state.sounds) {
            if (!(filename in _sounds) {
                _sounds[filename] <- Sound.FromFile(filename)
            }
        }
        foreach (filename in state.soundEffects) {
            if (!(filename in _soundEffects) {
                _soundEffects[filename] <- SoundEffect.FromFile(filename)
            }
        }
        foreach (filename in state.fonts) {
            if (!(filename in _fonts) {
                _fonts[filename] <- Font.FromFile(filename)
            }
        }
        foreach (filename in state.windowStyles) {
            if (!(filename in _windowStyles) {
                _windowStyles[filename] <- WindowStyle.FromFile(filename)
            }
        }
    }

    // private variables
    _textures = {}
    _canvases = {}
    _sounds = {}
    _soundEffects = {}
    _fonts = {}
    _windowStyles = {}
}
