class Console {
    constructor(font) {
        _command = ""
        _cursorLeft = 0
        _cursorRight = 0
        _cursorTimer = GetTicks() + 600
        _showCursor = true
        _log = []
        _logIndex = 0
        _output = []
        _font = font
    }

    function update() {
        if (GetTicks() >= _cursorTimer) {
            _cursorTimer += 600
            _showCursor = !_showCursor
        }

        foreach (event in Kbd.events) {
            switch (event.key.code) {
            case KEY_ESCAPE:
                return false
            case KEY_ENTER:
                if (_command.len() > 0) {
                    _output.push(">>> " + _command)
                    try {
                        local result = EvaluateString("return " + _command)
                        if (result != null) {
                            _output.push("" + result)
                        }
                    } catch (e) {
                        _output.push(e)
                    }
                    _log.push(_command)
                    _logIndex = _log.len()
                    _command = ""
                    _cursorLeft = 0
                    _cursorRight = 0
                }
                break
            case KEY_BACKSPACE:
                if (_cursorRight > 0) {
                    _command = _command.slice(0, _cursorRight-1) + _command.slice(_cursorRight, _command.len())
                    _cursorRight--
                    if (_cursorLeft > _cursorRight) {
                        _cursorLeft--
                    }
                }
                break
            case KEY_UP:
                local idx = _logIndex - 1
                if (idx >= 0 && idx < _log.len()) {
                    _logIndex = idx
                    _command = _log[idx]
                    _cursorLeft = 0
                    _cursorRight = 0
                }
                break
            case KEY_DOWN:
                local idx = _logIndex + 1
                if (idx >= 0 && idx < _log.len()) {
                    _logIndex = idx
                    _command = _log[idx]
                    _cursorLeft = 0
                    _cursorRight = 0
                }
                break
            case KEY_LEFT:
                if (_cursorRight > _cursorLeft) {
                    _cursorRight--
                } else if (_cursorLeft > 0) {
                    _cursorLeft--
                    _cursorRight--
                }
                break
            case KEY_RIGHT:
                if (_cursorRight < _command.len()) {
                    _cursorRight++
                    if (_font.getStringWidth(_command.slice(_cursorLeft, _cursorRight)) > Game.getWidth() - 10) {
                        _cursorLeft++
                    }
                }
                break
            default:
                local char = Kbd.keyToString(event.key.code, event.shift, event.alt)
                if (char.len() > 0) {
                    _command = _command.slice(0, _cursorRight) + char + _command.slice(_cursorRight, _command.len())
                    _cursorRight++
                    if (_font.getStringWidth(_command.slice(_cursorLeft, _cursorRight)) > Game.getWidth() - 10) {
                        _cursorLeft++
                    }
                }
            }
        }
        Kbd.events.clear()
    }

    function render() {
        // draw command line
        local command_y = Game.getHeight() - (_font.maxHeight + 5)
        DrawRect(0, command_y - 5, Game.getWidth(), _font.maxHeight + 10, CreateColor(100, 100, 100))
        _font.drawString(_command.slice(_cursorLeft, _command.len()), 5, command_y)
        if (_showCursor) {
            local cursor_x = 5 + _font.getStringWidth(_command.slice(_cursorLeft, _cursorRight))
            _font.drawString("|", cursor_x, command_y)
        }

        // draw output
        local cy = Game.getHeight() - (_font.maxHeight * 2 + 15)
        for (local i = _output.len()-1; i >= 0; --i) {
            local lines = _font.wordWrapString(_output[i], Game.getWidth()-10)
            for (local l = lines.len()-1; l >= 0; --l) {
                _font.drawString(lines[l], 5, cy)
                cy -= _font.maxHeight + 5
                if (cy < 0) {
                    break
                }
            }
            if (cy < 0) {
                break
            }
        }
    }

    _command = null
    _cursorLeft = null
    _cursorRight = null
    _cursorTimer = null
    _showCursor = null
    _log = null
    _logIndex = null
    _output = null
    _font = null
}
