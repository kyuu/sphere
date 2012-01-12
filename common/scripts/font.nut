// ---------------------------------------------------
class Font {

    constructor(images_) {
        images    = images_
        maxHeight = 0
        foreach (image in images) {
            if (image.height > maxHeight) {
                maxHeight = image.height
            }
        }
    }

    images    = null
    maxHeight = null
}

// ---------------------------------------------------
function Font::FromFile(filename) {
    Assert(typeof this == "class")

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

// ---------------------------------------------------
function Font::getStringWidth(text) {
    Assert(typeof this == "instance")

    local width = 0
    foreach (char in text) {
        width += images[char].width
    }
    return width
}

// ---------------------------------------------------
function Font::drawString(x, y, text, scale = 1.0) {
    Assert(typeof this == "instance")

    if (scale == 1.0) {
        foreach (char in text) {
            DrawImage(images[char], x, y)
            x += images[char].width
        }
    } else {
        local cx = x
        foreach (char in text) {
            local char_w = images[char].width
            local char_h = images[char].height
            DrawImageQuad(
                images[char],
                cx,                  y,
                cx + char_w * scale, y,
                cx + char_w * scale, y + char_h * scale,
                cx,                  y + char_h * scale
            )
            cx += char_w * scale
        }
    }
}

// ---------------------------------------------------
function Font::drawTextBox(x, y, width, height, text, offset = 0) {
    Assert(typeof this == "instance")

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

// ---------------------------------------------------
function Font::wrapString(text, width) {
    Assert(typeof this == "instance")

    local lines = []
    local l = 0
    local r = 0
    local w = 0
    local cw = 0
    local len = text.len()

    for (; r < len; ++r) {
        cw = images[text[r]].width
        if (w + cw > width) {
            lines.push(text.slice(l, r))
            l = r
            w = cw
        } else {
            w += cw
        }
    }

    if (w > 0) {
        lines.push(text.slice(l, r))
    }

    return lines
}

// ---------------------------------------------------
function Font::wordWrapString(text, width) {
    Assert(typeof this == "instance")

    local space_w = getStringWidth(" ");
    local tab_w   = getStringWidth("    ");
    local lines = [""];
    local dx = 0;
    local word = "";
    local word_w = 0;

    foreach (char in text) {
        if (char == ' ') { // space
            if (dx + word_w + space_w > width) { // word goes on a new line
                dx = word_w + space_w;
                lines.append(word + " ");
            } else { // word is tacked on to the last line
                lines[lines.len()-1] += word + " ";
                dx += word_w + space_w;
            }
            word = "";
            word_w = 0;
        } else if (char == '\t') { // tab
            if (dx + word_w + tab_w > width) { // word goes on a new line
                dx = word_w + tab_w;
                lines.append(word + "    ");
            } else { // word is tacked on to the last line
                lines[lines.len()-1] += word + "    ";
                dx += word_w + tab_w;
            }
            word = "";
            word_w = 0;
        } else if (char == '\n') { // new line
            lines[lines.len()-1] += word;
            dx = 0;
            lines.append("");
            word = "";
            word_w = 0;
        } else {
            local char_w = images[char].width;
            if (word_w + char_w > width && dx == 0) { // split word if it's too wide for one line
                lines[lines.len()-1] += word;
                lines.append("");
                word = "";
                word_w = 0;
            } else if (dx + word_w + char_w > width) { // just start a new line
                dx = 0;
                lines.append("");
            }
            word += char.tochar();
            word_w += char_w;
        }
    }
    lines[lines.len()-1] += word;
/*
    local lines = []
    local l = 0
    local r = 0
    local w = 0
    local ws = 0
    local ww = 0
    local c = 0
    local cw = 0

    local len = text.len()

    for (; r < len; ++r) {
        c  = text[r]
        cw = images[c].width

        if (c == ' ') {
            if (w > 0) {
                if (w + cw > width) {
                    lines.push(text.slice(l, r))
                    w = 0
                    ww = 0
                } else {
                    w += cw
                    ww = 0
                }
            }
        } else if (c == '\n') {
            if (w > 0) {
                lines.push(text.slice(l, r))
                w = 0
                ww = 0
            } else {
                lines.push("")
            }
        } else {
            if (w > 0) {
                if (w + cw > width) {
                    if (ww > 0) {
                        lines.push(text.slice(l, ws))
                        l = ws
                        w = ww + cw
                        ww += cw
                    } else {
                        lines.push(text.slice(l, r))
                        l = r
                        w = cw
                        ws = r
                        ww = cw
                    }
                } else {
                    w += cw
                    if (ww > 0) {
                        ww += cw
                    } else {
                        ws = r
                        ww = cw
                    }
                }
            } else {
                l = r
                w = cw
                ws = r
                ww = cw
            }
        }
    }

    if (w > 0) {
        lines.push(text.slice(l, r))
    }
*/
    return lines
}
