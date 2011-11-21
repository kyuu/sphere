#include "../../error.hpp"
#include "DeflateStream.hpp"

#define DEFLATESTREAM_MIN_BUFFER_SIZE     32
#define DEFLATESTREAM_DEFAULT_BUFFER_SIZE 512


//-----------------------------------------------------------------
DeflateStream*
DeflateStream::Create()
{
    try {
        RefPtr<DeflateStream> stream = new DeflateStream();
        stream->internalInit();
        return stream.release();
    } catch (const std::bad_alloc& e) {
        ReportOutOfMemory();
        return 0;
    }
}

//-----------------------------------------------------------------
DeflateStream::DeflateStream()
    : _mode(-1)
    , _initialized(false)
{
}

//-----------------------------------------------------------------
DeflateStream::~DeflateStream()
{
    internalCleanup();
}

//-----------------------------------------------------------------
void
DeflateStream::internalInit()
{
    _buffer = Blob::Create(DEFLATESTREAM_DEFAULT_BUFFER_SIZE);
}

//-----------------------------------------------------------------
void
DeflateStream::internalCleanup()
{
    if (_initialized) {
        switch (_mode) {
        case ICompressionStream::COMPRESS:   deflateEnd(&_stream); break;
        case ICompressionStream::DECOMPRESS: inflateEnd(&_stream); break;
        default:
            break;
        }
        _initialized = false;
        _mode = -1;
    }
}

//-----------------------------------------------------------------
int
DeflateStream::getType() const
{
    return ICompressionStream::DEFLATE;
}

//-----------------------------------------------------------------
bool
DeflateStream::init(int mode)
{
    internalCleanup();

    switch (_mode) {
    case ICompressionStream::COMPRESS:
        _stream.zalloc = Z_NULL;
        _stream.zfree  = Z_NULL;
        _stream.opaque = Z_NULL;
        _initialized = deflateInit(&_stream, Z_DEFAULT_COMPRESSION) == Z_OK;
        break;
    case ICompressionStream::DECOMPRESS:
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
DeflateStream::isInitialized() const
{
    return _initialized;
}

//-----------------------------------------------------------------
int
DeflateStream::getMode() const
{
    return _mode;
}

//-----------------------------------------------------------------
int
DeflateStream::getBufferSize() const
{
    return _buffer->getSize();
}

//-----------------------------------------------------------------
void
DeflateStream::setBufferSize(int size)
{
    assert(size > 0);
    if (size >= DEFLATESTREAM_MIN_BUFFER_SIZE) {
        _buffer->resize(size);
    }
}

//-----------------------------------------------------------------
bool
DeflateStream::consume(const u8* buf, int len, Blob* out)
{
    assert(buf);
    assert(len > 0);
    assert(out);

    out->resize(0); // start with an empty output buffer

    // initialize input parameters
    _stream.next_in  = buf;
    _stream.avail_in = len;

    // intialize output parameters
    _stream.next_out  = _buffer->getBuffer();
    _stream.avail_out = _buffer->getSize();

    while (true) {
        int ret;

        switch (_mode) {
        case ICompressionStream::COMPRESS:   ret = deflate(&_stream, Z_NO_FLUSH); break;
        case ICompressionStream::DECOMPRESS: ret = inflate(&_stream, Z_NO_FLUSH); break;
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
                out->resize(old_data_size + new_data_size);
                memcpy(out->getBuffer() + old_data_size, _buffer->getBuffer(), new_data_size);
            }

            // and return, since there is nothing to do here for now
            return true;

        } else if (_stream.avail_out == 0) { // output buffer is full
            // feed the data in the output buffer to out
            int new_data_size = _buffer->getSize();
            int old_data_size = out->getSize();
            out->resize(old_data_size + new_data_size);
            memcpy(out->getBuffer() + old_data_size, _buffer->getBuffer(), new_data_size);

            // update output parameters and resume input processing
            _stream.next_out  = _buffer->getBuffer();
            _stream.avail_out = _buffer->getSize();
        }
    }
}

//-----------------------------------------------------------------
bool
DeflateStream::close(Blob* out)
{
    assert(out);

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
        case ICompressionStream::COMPRESS:   ret = deflate(&_stream, Z_FINISH); break;
        case ICompressionStream::DECOMPRESS: ret = inflate(&_stream, Z_FINISH); break;
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
            out->resize(old_data_size + new_data_size);
            memcpy(out->getBuffer() + old_data_size, _buffer->getBuffer(), new_data_size);

            // update output parameters and resume input processing
            _stream.next_out  = _buffer->getBuffer();
            _stream.avail_out = _buffer->getSize();

        } else { // all output has been produced
            // feed anything left in the output buffer to out
            if (_stream.avail_out < _buffer->getSize()) {
                int new_data_size = _buffer->getSize() - _stream.avail_out;
                int old_data_size = out->getSize();
                out->resize(old_data_size + new_data_size);
                memcpy(out->getBuffer() + old_data_size, _buffer->getBuffer(), new_data_size);
            }

            // we are done
            internalCleanup();
            return true;
        }
    }
}
