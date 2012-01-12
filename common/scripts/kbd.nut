Kbd <- {
    events = []

    a = {
        code      = KEY_A
        pressed   = false
        onPress   = null
        onRelease = null
    }

    b = {
        code      = KEY_B
        pressed   = false
        onPress   = null
        onRelease = null
    }

    c = {
        code      = KEY_C
        pressed   = false
        onPress   = null
        onRelease = null
    }

    d = {
        code      = KEY_D
        pressed   = false
        onPress   = null
        onRelease = null
    }

    e = {
        code      = KEY_E
        pressed   = false
        onPress   = null
        onRelease = null
    }

    f = {
        code      = KEY_F
        pressed   = false
        onPress   = null
        onRelease = null
    }

    g = {
        code      = KEY_G
        pressed   = false
        onPress   = null
        onRelease = null
    }

    h = {
        code      = KEY_H
        pressed   = false
        onPress   = null
        onRelease = null
    }

    i = {
        code      = KEY_I
        pressed   = false
        onPress   = null
        onRelease = null
    }

    j = {
        code      = KEY_J
        pressed   = false
        onPress   = null
        onRelease = null
    }

    k = {
        code      = KEY_K
        pressed   = false
        onPress   = null
        onRelease = null
    }

    l = {
        code      = KEY_L
        pressed   = false
        onPress   = null
        onRelease = null
    }

    m = {
        code      = KEY_M
        pressed   = false
        onPress   = null
        onRelease = null
    }

    n = {
        code      = KEY_N
        pressed   = false
        onPress   = null
        onRelease = null
    }

    o = {
        code      = KEY_O
        pressed   = false
        onPress   = null
        onRelease = null
    }

    p = {
        code      = KEY_P
        pressed   = false
        onPress   = null
        onRelease = null
    }

    q = {
        code      = KEY_Q
        pressed   = false
        onPress   = null
        onRelease = null
    }

    r = {
        code      = KEY_R
        pressed   = false
        onPress   = null
        onRelease = null
    }

    s = {
        code      = KEY_S
        pressed   = false
        onPress   = null
        onRelease = null
    }

    t = {
        code      = KEY_T
        pressed   = false
        onPress   = null
        onRelease = null
    }

    u = {
        code      = KEY_U
        pressed   = false
        onPress   = null
        onRelease = null
    }

    v = {
        code      = KEY_V
        pressed   = false
        onPress   = null
        onRelease = null
    }

    w = {
        code      = KEY_W
        pressed   = false
        onPress   = null
        onRelease = null
    }

    x = {
        code      = KEY_X
        pressed   = false
        onPress   = null
        onRelease = null
    }

    y = {
        code      = KEY_Y
        pressed   = false
        onPress   = null
        onRelease = null
    }

    z = {
        code      = KEY_Z
        pressed   = false
        onPress   = null
        onRelease = null
    }

    num0 = {
        code      = KEY_0
        pressed   = false
        onPress   = null
        onRelease = null
    }

    num1 = {
        code      = KEY_1
        pressed   = false
        onPress   = null
        onRelease = null
    }

    num2 = {
        code      = KEY_2
        pressed   = false
        onPress   = null
        onRelease = null
    }

    num3 = {
        code      = KEY_3
        pressed   = false
        onPress   = null
        onRelease = null
    }

    num4 = {
        code      = KEY_4
        pressed   = false
        onPress   = null
        onRelease = null
    }

    num5 = {
        code      = KEY_5
        pressed   = false
        onPress   = null
        onRelease = null
    }

    num6 = {
        code      = KEY_6
        pressed   = false
        onPress   = null
        onRelease = null
    }

    num7 = {
        code      = KEY_7
        pressed   = false
        onPress   = null
        onRelease = null
    }

    num8 = {
        code      = KEY_8
        pressed   = false
        onPress   = null
        onRelease = null
    }

    num9 = {
        code      = KEY_9
        pressed   = false
        onPress   = null
        onRelease = null
    }

    f1 = {
        code      = KEY_F1
        pressed   = false
        onPress   = null
        onRelease = null
    }

    f2 = {
        code      = KEY_F2
        pressed   = false
        onPress   = null
        onRelease = null
    }

    f3 = {
        code      = KEY_F3
        pressed   = false
        onPress   = null
        onRelease = null
    }

    f4 = {
        code      = KEY_F4
        pressed   = false
        onPress   = null
        onRelease = null
    }

    f5 = {
        code      = KEY_F5
        pressed   = false
        onPress   = null
        onRelease = null
    }

    f6 = {
        code      = KEY_F6
        pressed   = false
        onPress   = null
        onRelease = null
    }

    f7 = {
        code      = KEY_F7
        pressed   = false
        onPress   = null
        onRelease = null
    }

    f8 = {
        code      = KEY_F8
        pressed   = false
        onPress   = function() {
            Game.enterConsoleMode()
        }
        onRelease = null
    }

    f9 = {
        code      = KEY_F9
        pressed   = false
        onPress   = function() {
            local showFPS = Game.getShowFPS()
            Game.setShowFPS(!showFPS)
        }
        onRelease = null
    }

    f10 = {
        code      = KEY_F10
        pressed   = false
        onPress   = function() {
            local frameRate = Game.getFrameRate()
            Game.setFrameRate((frameRate == 0 ? 60 : 0))
        }
        onRelease = null
    }

    f11 = {
        code      = KEY_F11
        pressed   = false
        onPress   = function() {
            local scale = Game.getScale()
            Game.setScale(!scale)
        }
        onRelease = null
    }

    f12 = {
        code      = KEY_F12
        pressed   = false
        onPress   = function() {
            local fullScreen = Game.getFullScreen()
            Game.setFullScreen(!fullScreen)
        }
        onRelease = null
    }

    tab = {
        code      = KEY_TAB
        pressed   = false
        onPress   = null
        onRelease = null
    }

    backspace = {
        code      = KEY_BACKSPACE
        pressed   = false
        onPress   = null
        onRelease = null
    }

    enter = {
        code      = KEY_ENTER
        pressed   = false
        onPress   = null
        onRelease = null
    }

    escape = {
        code      = KEY_ESCAPE
        pressed   = false
        onPress   = null
        onRelease = null
    }

    space = {
        code      = KEY_SPACE
        pressed   = false
        onPress   = null
        onRelease = null
    }

    pgup = {
        code      = KEY_PAGEUP
        pressed   = false
        onPress   = null
        onRelease = null
    }

    pgdn = {
        code      = KEY_PAGEDOWN
        pressed   = false
        onPress   = null
        onRelease = null
    }

    home = {
        code      = KEY_HOME
        pressed   = false
        onPress   = null
        onRelease = null
    }

    end = {
        code      = KEY_END
        pressed   = false
        onPress   = null
        onRelease = null
    }

    left = {
        code      = KEY_LEFT
        pressed   = false
        onPress   = null
        onRelease = null
    }

    up = {
        code      = KEY_UP
        pressed   = false
        onPress   = null
        onRelease = null
    }

    right = {
        code      = KEY_RIGHT
        pressed   = false
        onPress   = null
        onRelease = null
    }

    down = {
        code      = KEY_DOWN
        pressed   = false
        onPress   = null
        onRelease = null
    }

    ins = {
        code      = KEY_INSERT
        pressed   = false
        onPress   = null
        onRelease = null
    }

    del = {
        code      = KEY_DELETE
        pressed   = false
        onPress   = null
        onRelease = null
    }

    plus = {
        code      = KEY_PLUS
        pressed   = false
        onPress   = null
        onRelease = null
    }

    comma = {
        code      = KEY_COMMA
        pressed   = false
        onPress   = null
        onRelease = null
    }

    minus = {
        code      = KEY_MINUS
        pressed   = false
        onPress   = null
        onRelease = null
    }

    period = {
        code      = KEY_PERIOD
        pressed   = false
        onPress   = null
        onRelease = null
    }

    capslock = {
        code      = KEY_CAPSLOCK
        pressed   = false
        active    = false
        onPress   = null
        onRelease = null
    }

    shift = {
        code      = KEY_SHIFT
        pressed   = false
        onPress   = null
        onRelease = null
    }

    ctrl = {
        code      = KEY_CTRL
        pressed   = false
        onPress   = null
        onRelease = null
    }

    alt = {
        code      = KEY_ALT
        pressed   = false
        onPress   = null
        onRelease = null
    }

    oem1 = {
        code      = KEY_OEM1
        pressed   = false
        onPress   = null
        onRelease = null
    }

    oem2 = {
        code      = KEY_OEM2
        pressed   = false
        onPress   = null
        onRelease = null
    }

    oem3 = {
        code      = KEY_OEM3
        pressed   = false
        onPress   = null
        onRelease = null
    }

    oem4 = {
        code      = KEY_OEM4
        pressed   = false
        onPress   = null
        onRelease = null
    }

    oem5 = {
        code      = KEY_OEM5
        pressed   = false
        onPress   = null
        onRelease = null
    }

    oem6 = {
        code      = KEY_OEM6
        pressed   = false
        onPress   = null
        onRelease = null
    }

    oem7 = {
        code      = KEY_OEM7
        pressed   = false
        onPress   = null
        onRelease = null
    }

    oem8 = {
        code      = KEY_OEM8
        pressed   = false
        onPress   = null
        onRelease = null
    }

    _keyToSlot = [
        "a"
        "b"
        "c"
        "d"
        "e"
        "f"
        "g"
        "h"
        "i"
        "j"
        "k"
        "l"
        "m"
        "n"
        "o"
        "p"
        "q"
        "r"
        "s"
        "t"
        "u"
        "v"
        "w"
        "x"
        "y"
        "z"
        "num0"
        "num1"
        "num2"
        "num3"
        "num4"
        "num5"
        "num6"
        "num7"
        "num8"
        "num9"
        "f1"
        "f2"
        "f3"
        "f4"
        "f5"
        "f6"
        "f7"
        "f8"
        "f9"
        "f10"
        "f11"
        "f12"
        "tab"
        "backspace"
        "enter"
        "escape"
        "space"
        "pgup"
        "pgdn"
        "home"
        "end"
        "left"
        "up"
        "right"
        "down"
        "ins"
        "del"
        "plus"
        "comma"
        "minus"
        "period"
        "capslock"
        "shift"
        "ctrl"
        "alt"
        "oem1"
        "oem2"
        "oem3"
        "oem4"
        "oem5"
        "oem6"
        "oem7"
        "oem8"
    ]

    function keyToString(code, shift, alt) {
        switch (code) {
        case KEY_A:
            if (shift) {
                return "A"
            } else {
                return "a"
            }
        case KEY_B:
            if (shift) {
                return "B"
            } else {
                return "b"
            }
        case KEY_C:
            if (shift) {
                return "C"
            } else {
                return "c"
            }
        case KEY_D:
            if (shift) {
                return "D"
            } else {
                return "d"
            }
        case KEY_E:
            if (shift) {
                return "E"
            } else {
                return "e"
            }
        case KEY_F:
            if (shift) {
                return "F"
            } else {
                return "f"
            }
        case KEY_G:
            if (shift) {
                return "G"
            } else {
                return "g"
            }
        case KEY_H:
            if (shift) {
                return "H"
            } else {
                return "h"
            }
        case KEY_I:
            if (shift) {
                return "I"
            } else {
                return "i"
            }
        case KEY_J:
            if (shift) {
                return "J"
            } else {
                return "j"
            }
        case KEY_K:
            if (shift) {
                return "K"
            } else {
                return "k"
            }
        case KEY_L:
            if (shift) {
                return "L"
            } else {
                return "l"
            }
        case KEY_M:
            if (shift) {
                return "M"
            } else {
                return "m"
            }
        case KEY_N:
            if (shift) {
                return "N"
            } else {
                return "n"
            }
        case KEY_O:
            if (shift) {
                return "O"
            } else {
                return "o"
            }
        case KEY_P:
            if (shift) {
                return "P"
            } else {
                return "p"
            }
        case KEY_Q:
            if (shift) {
                return "Q"
            } else {
                return "q"
            }
        case KEY_R:
            if (shift) {
                return "R"
            } else {
                return "r"
            }
        case KEY_S:
            if (shift) {
                return "S"
            } else {
                return "s"
            }
        case KEY_T:
            if (shift) {
                return "T"
            } else {
                return "t"
            }
        case KEY_U:
            if (shift) {
                return "U"
            } else {
                return "u"
            }
        case KEY_V:
            if (shift) {
                return "V"
            } else {
                return "v"
            }
        case KEY_W:
            if (shift) {
                return "W"
            } else {
                return "w"
            }
        case KEY_X:
            if (shift) {
                return "X"
            } else {
                return "x"
            }
        case KEY_Y:
            if (shift) {
                return "Y"
            } else {
                return "y"
            }
        case KEY_Z:
            if (shift) {
                return "Z"
            } else {
                return "z"
            }
        case KEY_0:
            if (shift) {
                return "="
            } else if (alt) {
                return "}"
            } else {
                return "0"
            }
        case KEY_1:
            if (shift) {
                return "!"
            } else {
                return "1"
            }
        case KEY_2:
            if (shift) {
                return "\""
            } else {
                return "2"
            }
        case KEY_3:
            if (shift) {
                return "§"
            } else {
                return "3"
            }
        case KEY_4:
            if (shift) {
                return "$"
            } else {
                return "4"
            }
        case KEY_5:
            if (shift) {
                return "%"
            } else {
                return "5"
            }
        case KEY_6:
            if (shift) {
                return "&"
            } else {
                return "6"
            }
        case KEY_7:
            if (shift) {
                return "/"
            } else if (alt) {
                return "{"
            } else {
                return "7"
            }
        case KEY_8:
            if (shift) {
                return "("
            } else if (alt) {
                return "["
            } else {
                return "8"
            }
        case KEY_9:
            if (shift) {
                return ")"
            } else if (alt) {
                return "]"
            } else {
                return "9"
            }
        case KEY_SPACE:
            return " "
        case KEY_PLUS:
            if (shift) {
                return "*"
            } else if (alt) {
                return "~"
            } else {
                return "+"
            }
        case KEY_COMMA:
            if (shift) {
                return ";"
            } else {
                return ","
            }
        case KEY_MINUS:
            if (shift) {
                return "_"
            } else {
                return "-"
            }
        case KEY_PERIOD:
            if (shift) {
                return ":"
            } else {
                return "."
            }
        case KEY_OEM2:
            if (shift) {
                return "'"
            } else {
                return "#"
            }
        case KEY_OEM4:
            if (shift) {
                return "?"
            } else if (alt) {
                return "\\"
            } else {
                return ""
            }
        case KEY_OEM5:
            return "^"
        case KEY_OEM6:
            if (shift) {
                return "`"
            } else {
                return "´"
            }
        case KEY_OEM8:
            if (shift) {
                return ">"
            } else if (alt) {
                return "|"
            } else {
                return "<"
            }
        default:
            return ""
        }
    }
}
