class Color {
    constructor(r, g, b, a = 255) {
        packed = (r << 24) | (g << 16) | (b << 8) | a
    }

    // the RGBA value
    packed = null

    // predefined common RGBA values
    static Black   = 0x000000FF
    static White   = 0xFFFFFFFF
    static Red     = 0xFF0000FF
    static Lime    = 0x00FF00FF
    static Blue    = 0x0000FFFF
    static Yellow  = 0xFFFF00FF
    static Cyan    = 0x00FFFFFF
    static Magenta = 0xFF00FFFF
    static Gray    = 0x808080FF
    static Maroon  = 0x800000FF
    static Green   = 0x008000FF
    static Navy    = 0x000080FF
    static Olive   = 0x808000FF
    static Teal    = 0x008080FF
    static Purple  = 0x800080FF
    static Silver  = 0xC0C0C0FF
}

function Color::_set(idx, val) {
    switch (idx) {
    case "r":
        packed = (packed & 0x00FFFFFF) | (val << 24)
        break
    case "g":
        packed = (packed & 0xFF00FFFF) | (val << 16)
        break
    case "b":
        packed = (packed & 0xFFFF00FF) | (val << 8)
        break
    case "a":
        packed = packed & 0xFFFFFF00 |  val
        break
    default:
        throw null // index not found
    }
}

function Color::_get(idx) {
    switch (idx) {
    case "r":
        return packed >> 24
    case "g":
        return (packed & 0x00FF0000) >> 16
    case "b":
        return (packed & 0x0000FF00) >> 8
    case "a":
        return packed & 0x000000FF
    default:
        throw null // index not found
    }
}
