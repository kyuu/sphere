// ---------------------------------------------------
class SpriteSet {

    constructor(frame_width, frame_height, base_x1, base_y1, base_x2, base_y2, images_, directions_) {
        frameWidth  = frame_width;
        frameHeight = frame_height;
        baseX1      = base_x1;
        baseY1      = base_y1;
        baseX2      = base_x2;
        baseY2      = base_y2;
        images      = images_;
        directions  = directions_;
    }

    frameWidth  = null;
    frameHeight = null;
    baseX1      = null;
    baseY1      = null;
    baseX2      = null;
    baseY2      = null;
    images      = null;
    directions  = null;
}

// ---------------------------------------------------
function SpriteSet::Load(filename) {
    Assert(typeof this == "class");

    // open input file
    local file = File.Open(filename);

    // read the spriteset signature
    local signature = file.readString(4);
    Assert(signature == ".rss");

    // read the spriteset version
    local version = file.readNumber('w');
    Assert(version == 3);

    // read the number of images
    local num_images = file.readNumber('w');
    Assert(num_images > 0);

    // read the frame dimensions
    local frame_width = file.readNumber('w');
    local frame_height = file.readNumber('w');
    Assert(frame_width * frame_height > 0);

    // read the number of directions
    local num_directions = file.readNumber('w');
    Assert(num_directions > 0);

    // read the base
    local base_x1 = file.readNumber('w');
    local base_y1 = file.readNumber('w');
    local base_x2 = file.readNumber('w');
    local base_y2 = file.readNumber('w');
    Assert(base_x1 < 0 && base_x1 >= frame_width  &&
           base_x2 < 0 && base_x2 >= frame_width  &&
           base_y1 < 0 && base_y1 >= frame_height &&
           base_y2 < 0 && base_y2 >= frame_height);

    // skip reserved bytes
    file.seek(106, Stream.CUR);

    // read images
    local images = array(num_images);
    for (local i = 0; i < num_images; i++) {
        // read image pixels
        local pixels = file.read(frame_width * frame_height * 4);
        local image = Canvas(frame_width, frame_height, pixels);
        images[i] = Texture(image);
    }

    // read directions
    local directions = array(num_directions);
    for (local i = 0; i < num_directions; i++) {
        // read number of frames
        local num_frames = file.readNumber('w');

        // skip reserved bytes
        file.seek(6, Stream.CUR);

        // read name
        local name_length = file.readNumber('w');
        local name = file.readString(name_length);

        // read frames
        local frames = array(num_frames);
        for (local j = 0; j < num_frames; j++) {
            // read index
            local index = file.readNumber('w');

            // read delay
            local delay = file.readNumber('w');

            // skip reserved bytes
            file.seek(4, Stream.CUR);

            frames[j] = {index = index, delay = delay};
        }

        directions[i] = {name = name, frames = frames}

    }

    return Spriteset(frame_width, frame_height, base_x1, base_y1, base_x2, base_y2, images, directions);
}
