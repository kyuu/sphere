// ---------------------------------------------------
class WindowStyle {

    constructor(background_mode, corner_colors, edge_offsets, images_) {
        backgroundMode = background_mode;
        cornerColors   = corner_colors;
        edgeOffsets    = edge_offsets;
        images         = images_;
    }

    backgroundMode = null; // 0 = tiled, 1 = stretched, 2 = gradient (uses corner colors), 3 = tiled with gradient, 4 = stretched with gradient
    cornerColors   = null; // upper left, upper right, lower left, lower right
    edgeOffsets    = null; // left, top, right, bottom
    images         = null; // upper-left, top, upper-right, right, lower-right, bottom, lower-left, left, background
}

// ---------------------------------------------------
function WindowStyle::Load(filename) {
    Assert(typeof this == "class");

    // open input file
    local file = File.Open(filename);

    // read the windowstyle signature
    local signature = file.readString(4);
    Assert(signature == ".rws");

    // read the spriteset version
    local version = file.readNumber('w');
    Assert(version == 2);

    // skip edge width (not valid for version 2)
    file.seek(1, Stream.CUR);

    // read the background mode
    local background_mode = file.readNumber('b');
    Assert(background_mode >= 0 && background_mode <= 4);

    // read corner colors
    local corner_colors = array(4);
    for (local i = 0; i < 4; i++) {
        local rgba = file.read(4);
        corner_colors[i] = CreateColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    }

    // read edge offsets
    local edge_offsets = array(4);
    for (local i = 0; i < 4; i++) {
        edge_offsets[i] = file.readNumber('b');
    }

    // skip reserved bytes
    file.seek(36, Stream.CUR);

    // read images
    local images = array(9);
    for (local i = 0; i < 9; i++) {
        // read dimensions
        local width = file.readNumber('w');
        local height = file.readNumber('w');
        Assert(width * height > 0);

        // read image
        local pixels = file.read(width * height * 4);
        local image = Canvas(width, height, pixels);

        images[i] = Texture(image);
    }

    return WindowStyle(background_mode, corner_colors, edge_offsets, images);
}

// ---------------------------------------------------
function WindowStyle::drawWindow(x, y, width, height) {
    Assert(typeof this == "instance");

    // draw background
    _drawBackground(x, y, width, height);

    // draw edges
    _drawVerticalEdge(  1, x,                   y - images[1].height, width, height);
    _drawVerticalEdge(  5, x,                   y + height,           width, height);
    _drawHorizontalEdge(7, x - images[7].width, y,                    width, height);
    _drawHorizontalEdge(3, x + width,           y,                    width, height);

    // draw corners
    DrawImage(images[0], x - images[0].width, y - images[0].height);
    DrawImage(images[2], x + width,           y - images[2].height);
    DrawImage(images[4], x + width,           y + height          );
    DrawImage(images[6], x - images[6].width, y + height          );
}

// ---------------------------------------------------
function WindowStyle::_drawBackground(x, y, width, height) {
    local background = images[8];

    x -= edgeOffsets[0];
    y -= edgeOffsets[1];
    width  += edgeOffsets[0] + edgeOffsets[2];
    height += edgeOffsets[1] + edgeOffsets[3];

    if (backgroundMode == 0 || backgroundMode == 3) {
        local orig_scissor = GetFrameScissor();
        SetFrameScissor(Rect(x, y, width, height));
        for (local ix = 0; ix < width / background.width + 1; ix++) {
            for (local iy = 0; iy < height / background.height + 1; iy++) {
                DrawImage(background, x + ix * background.width, y + iy * background.height);
            }
        }
        SetFrameScissor(orig_scissor);
    } else if (backgroundMode == 1 || backgroundMode == 4) {
        DrawImageQuad(background, x, y, x + width, y, x + width, y + height, x, y + height);
    }

    if (backgroundMode >= 2 || backgroundMode <= 4) {
        DrawRect(Rect(x, y, width, height), cornerColors[0], cornerColors[1], cornerColors[3], cornerColors[2]);
    }
}

// ---------------------------------------------------
function WindowStyle::_drawHorizontalEdge(index, x, y, width, height) {
    local edge = images[index];
    local edge_w = edge.width;
    local edge_h = edge.height;
    local orig_scissor = GetFrameScissor();
    SetFrameScissor(Rect(x, y, edge_w, height));
    for (local i = 0; i < height / edge_h + 1; i++) {
        DrawImage(edge, x, y + i * edge_h);
    }
    SetFrameScissor(orig_scissor);
}

// ---------------------------------------------------
function WindowStyle::_drawVerticalEdge(index, x, y, width, height) {
    local edge = images[index];
    local edge_w = edge.width;
    local edge_h = edge.height;
    local orig_scissor = GetFrameScissor();
    SetFrameScissor(Rect(x, y, width, edge_h));
    for (local i = 0; i < width / edge_w + 1; i++) {
        DrawImage(edge, x + i * edge_w, y);
    }
    SetFrameScissor(orig_scissor);
}
