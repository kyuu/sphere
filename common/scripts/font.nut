class Font {
    constructor(images) {
        chars = array(images.len())
        maxHeight = 0
        for (local i = 0; i < images.len(); i++) {
            if (images[i].height > maxHeight) {
                maxHeight = images[i].height
            }
            chars[i] = {
                image = images[i]
                width = images[i].width
            }
        }
    }

    chars     = null
    maxHeight = null
}

function Font::FromFile(filename) {
    // open input file
    local file = File.Open(filename)

    // read the font signature
    local signature = file.readString(4)
    Assert(signature == ".rfn")

    // read the font version
    local version = file.readNumber('w')
    Assert(version == 2)

    // read number of characters
    local num_chars = file.readNumber('w')
    Assert(num_chars > 0)

    // skip reserved bytes
    file.seek(248, Stream.CUR)

    // read character images
    local images = array(num_chars)
    for (local i = 0; i < num_chars; i++) {
        // read dimensions
        local width  = file.readNumber('w')
        local height = file.readNumber('w')
        Assert(width * height > 0)

        // skip reserved bytes
        file.seek(28, Stream.CUR)

        // read image
        local pixels = file.read(width * height * 4)
        local image = Canvas.FromBuffer(width, height, pixels)

        images[i] = Texture.FromCanvas(image)
    }

    return Font(images)
}

function Font::getStringWidth(text) {
    local width = 0
    foreach (char in text) {
        width += chars[char].width
    }
    return width
}

function Font::drawString(x, y, text, scale = 1.0) {
    if (scale == 1.0) {
        foreach (char in text) {
            DrawImage(chars[char].image, x, y)
            x += chars[char].width
        }
    } else {
        local cx = x
        foreach (char in text) {
            local char_w = chars[char].width
            local char_h = chars[char].height
            DrawImageQuad(
                chars[char].image,
                cx,                  y,
                cx + char_w * scale, y,
                cx + char_w * scale, y + char_h * scale,
                cx,                  y + char_h * scale
            )
            cx += char_w * scale
        }
    }
}

function Font::drawTextBox(x, y, width, height, text, offset = 0) {
    local orig_scissor = GetFrameScissor()
    SetFrameScissor(Rect(x, y, width, height))

    y += offset
    local dy = 0

    // word-wrap and draw strings
    local lines = wordWrapString(text, width)
    foreach (line in lines) {
        drawString(x, y + dy, line)
        dy += maxHeight
    }

    SetFrameScissor(orig_scissor)
}

function Font::wordWrapString(text, width) {
    local lines = []
    local lc = 0 // line char count
    local lw = 0 // line width
    local wc = 0 // word char count
    local ww = 0 // word width
    local len = text.len()
    local i = 0

    for (; i < len; ++i) {
        local c  = text[i]
        local cw = chars[c].width

        if (c == ' ') {
            if (lw + ww > width) {
                lines.push(text.slice(i-(lc+wc), i-wc))
                lc = 0
                lw = 0
            }
            lc += wc
            lw += ww
            wc = 0
            ww = 0
            if (lw + cw > width) {
                lines.push(text.slice(i-lc, i))
                lc = 0
                lw = 0
            }
            ++lc
            lw += cw
        } else if (c == '\n') {
            if (lw + ww > width) {
                lines.push(text.slice(i-(lc+wc), i-wc))
                lc = 0
                lw = 0
            }
            lc += wc
            lw += ww
            wc = 0
            ww = 0
            if (lw > 0) {
                lines.push(text.slice(i-lc, i))
                lc = 0
                lw = 0
            } else {
                lines.push("")
            }
        } else {
            if (ww + cw > width) {
                if (lw > 0) {
                    lines.push(text.slice(i-(lc+wc), i-wc))
                    lc = 0
                    lw = 0
                }
                lines.push(text.slice(i-wc, i))
                wc = 0
                ww = 0
            }
            ++wc
            ww += cw
        }
    }

    if (lw + ww > 0) {
        lines.push(text.slice(i-(lc+wc), i))
    }

    return lines
}
