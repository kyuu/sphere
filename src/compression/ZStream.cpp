#include "ZStream.hpp"

#define ZSTREAM_MIN_BUFFER_SIZE     32
#define ZSTREAM_DEFAULT_BUFFER_SIZE 512


//-----------------------------------------------------------------
ZStream*
ZStream::Create()
{
    try {
        ZStreamPtr stream = new ZStream();
        if (stream->internalInit()) {
            return stream.release();
        }
    } catch (const std::bad_alloc& e) { }
    return 0;
}

//-----------------------------------------------------------------
ZStream::ZStream()
    : _mode(-1)
    , _initialized(false)
{
}

//-----------------------------------------------------------------
ZStream::~ZStream()
{
    internalCleanup();
}

//-----------------------------------------------------------------
void
ZStream::internalCleanup()
{
    if (_initialized) {
        switch (_mode) {
        case ZStream::DEFLATE: deflateEnd(&_stream); break;
        case ZSTream::INFLATE: inflateEnd(&_stream); break;
        default:
            break;
        }
        _initialized = false;
        _mode = -1;
    }
}

//-----------------------------------------------------------------
bool
ZStream::internalInit()
{
    if (!_buffer) {
        _buffer = Blob::Create(ZSTREAM_DEFAULT_BUFFER_SIZE);
        return _buffer.get() != 0;
    }
}

//-----------------------------------------------------------------
bool
ZStream::init(int mode)
{
    internalCleanup();

    int err;
    switch (_mode) {
    case ZStream::DEFLATE:
        _stream.zalloc = Z_NULL;
        _stream.zfree  = Z_NULL;
        _stream.opaque = Z_NULL;
        _initialized = deflateInit(&_stream, Z_DEFAULT_COMPRESSION) == Z_OK;
        break;
    case ZSTream::INFLATE:
        _stream.zalloc   = Z_NULL;
        _stream.zfree    = Z_NULL;
        _stream.opaque   = Z_NULL;
        _stream.next_in  = Z_NULL;
        _stream.avail_in = 0;
        _initialized = inflateInit(&_stream, Z_DEFAULT_COMPRESSION) == Z_OK;
        break;
    default:
        return false;
    }
    return _initialized;
}

//-----------------------------------------------------------------
bool
ZStream::isInitialized() const
{
    return _initialized;
}

//-----------------------------------------------------------------
int
ZStream::getMode() const
{
    return _mode;
}

//-----------------------------------------------------------------
int
ZStream::getBufferSize() const
{
    if (_buffer) {
        return _buffer->getSize();
    }
    return 0;
}

//-----------------------------------------------------------------
bool
ZStream::setBufferSize(int size)
{
    if (_buffer && size >= ZSTREAM_MIN_BUFFER_SIZE) {
        return _buffer->resize(size);
    }
    return false;
}

//-----------------------------------------------------------------
bool
ZStream::process(const u8* in, int len, Blob* out)
{
    if (!_initialized || !in || len <= 0 || !out) {
        return false;
    }

    out->resize(0); // start with an empty output buffer

    // initialize input parameters
    _stream.next_in  = in;
    _stream.avail_in = len;

    // intialize output parameters
    _stream.next_out  = _buffer->getBuffer();
    _stream.avail_out = _buffer->getSize();

    while (true) {
        int ret;

        switch (_mode) {
        case ZStream::DEFLATE: ret = deflate(&_stream, Z_NO_FLUSH); break;
        case ZSTream::INFLATE: ret = inflate(&_stream, Z_NO_FLUSH); break;
        default:
            return false; // this should not happen
        }

        // this should not happen
        if (ret == Z_STREAM_ERROR) {
            return false;
        }

        if (_stream.avail_in == 0) { // all input has been consumed
            // feed anything left in the output buffer to out
            if (_stream.avail_out < _buffer->getSize()) {
                int new_data_size = _buffer->getSize() - _stream.avail_out;
                int old_data_size = out->getSize();
                if (!out->resize(old_data_size + new_data_size)) {
                    return false;
                }
                memcpy(out->getBuffer() + old_data_size, _buffer->getBuffer(), new_data_size);
            }

            // and return, since there is nothing to do here for now
            return true;

        } else if (_stream.avail_out == 0) { // output buffer is full
            // feed the data in the output buffer to out
            int new_data_size = _buffer->getSize();
            int old_data_size = out->getSize();
            if (!out->resize(old_data_size + new_data_size)) {
                return false;
            }
            memcpy(out->getBuffer() + old_data_size, _buffer->getBuffer(), new_data_size);

            // update output parameters and resume input processing
            _stream.next_out  = _buffer->getBuffer();
            _stream.avail_out = _buffer->getSize();
        }
    }
}

//-----------------------------------------------------------------
bool
ZStream::finish(Blob* out)
{
    if (!_initialized || !out) {
        return false;
    }

    out->resize(0); // start with an empty output buffer

    // initialize input parameters
    _stream.next_in  = Z_NULL;
    _stream.avail_in = 0;

    // intialize output parameters
    _stream.next_out  = _buffer->getBuffer();
    _stream.avail_out = _buffer->getSize();

    while (true) {
        int ret;

        // process input
        switch (_mode) {
        case ZStream::DEFLATE: ret = deflate(&_stream, Z_FINISH); break;
        case ZSTream::INFLATE: ret = inflate(&_stream, Z_FINISH); break;
        default:
            return false; // this should not happen
        }

        // this should not happen
        if (ret == Z_STREAM_ERROR) {
            return false;
        }

        if (_stream.avail_out == 0) { // output buffer is full
            // feed the data in the output buffer to out
            int new_data_size = _buffer->getSize();
            int old_data_size = out->getSize();
            if (!out->resize(old_data_size + new_data_size)) {
                return false;
            }
            memcpy(out->getBuffer() + old_data_size, _buffer->getBuffer(), new_data_size);

            // update output parameters and resume input processing
            _stream.next_out  = _buffer->getBuffer();
            _stream.avail_out = _buffer->getSize();

        } else { // all output has been produced
            // feed anything left in the output buffer to out
            if (_stream.avail_out < _buffer->getSize()) {
                int new_data_size = _buffer->getSize() - _stream.avail_out;
                int old_data_size = out->getSize();
                if (!out->resize(old_data_size + new_data_size)) {
                    return false;
                }
                memcpy(out->getBuffer() + old_data_size, _buffer->getBuffer(), new_data_size);
            }

            // we are done
            internalCleanup();
            return true;
        }
    }
}
