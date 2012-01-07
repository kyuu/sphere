#include <cstring>
#include "ZStream.hpp"

#define ZSTREAM_MIN_BUFFER_SIZE     128
#define ZSTREAM_DEFAULT_BUFFER_SIZE 1024
#define ZSTREAM_MODE_INVALID  -1
#define ZSTREAM_MODE_DEFLATE   0
#define ZSTREAM_MODE_INFLATE   1


namespace sphere {

    //-----------------------------------------------------------------
    ZStream*
    ZStream::Create()
    {
        return new ZStream();
    }

    //-----------------------------------------------------------------
    ZStream::ZStream()
        : _mode(ZSTREAM_MODE_INVALID)
    {
        _buffer = Blob::Create(ZSTREAM_DEFAULT_BUFFER_SIZE);
    }

    //-----------------------------------------------------------------
    ZStream::~ZStream()
    {
        deinit();
    }

    //-----------------------------------------------------------------
    void
    ZStream::deinit()
    {
        switch (_mode) {
        case ZSTREAM_MODE_INFLATE: deflateEnd(&_stream); break;
        case ZSTREAM_MODE_DEFLATE: inflateEnd(&_stream); break;
        }
        _mode = ZSTREAM_MODE_INVALID;
    }

    //-----------------------------------------------------------------
    bool
    ZStream::initForCompression()
    {
        _stream.zalloc = Z_NULL;
        _stream.zfree  = Z_NULL;
        _stream.opaque = Z_NULL;
        if (deflateInit(&_stream, Z_DEFAULT_COMPRESSION) == Z_OK) {
            _mode = ZSTREAM_MODE_DEFLATE;
            return true;
        }
        return false;
    }

    //-----------------------------------------------------------------
    bool
    ZStream::initForDecompression()
    {
        deinit();
        _stream.zalloc   = Z_NULL;
        _stream.zfree    = Z_NULL;
        _stream.opaque   = Z_NULL;
        _stream.next_in  = Z_NULL;
        _stream.avail_in = 0;
        if (inflateInit(&_stream) == Z_OK) {
            _mode = ZSTREAM_MODE_INFLATE;
            return true;
        }
        return false;
    }

    //-----------------------------------------------------------------
    int
    ZStream::getBufferSize() const
    {
        return _buffer->getSize();
    }

    //-----------------------------------------------------------------
    void
    ZStream::setBufferSize(int size)
    {
        assert(size > 0);
        if (size >= ZSTREAM_MIN_BUFFER_SIZE) {
            _buffer->resize(size);
        }
    }

    //-----------------------------------------------------------------
    bool
    ZStream::compress(const u8* buf, int len, Blob* out)
    {
        assert(buf);
        assert(len > 0);
        assert(out);

        if (_mode != ZSTREAM_MODE_DEFLATE) {
            if (!initForCompression()) {
                return false;
            }
        }
        return consume(buf, len, out);
    }

    //-----------------------------------------------------------------
    bool
    ZStream::decompress(const u8* buf, int len, Blob* out)
    {
        assert(buf);
        assert(len > 0);
        assert(out);
        if (_mode != ZSTREAM_MODE_INFLATE) {
            if (!initForDecompression()) {
                return false;
            }
        }
        return consume(buf, len, out);
    }

    //-----------------------------------------------------------------
    bool
    ZStream::consume(const u8* buf, int len, Blob* out)
    {
        out->resize(0); // start with an empty output buffer

        // initialize input parameters
        _stream.next_in  = (Bytef*)buf;
        _stream.avail_in = len;

        // intialize output parameters
        _stream.next_out  = _buffer->getBuffer();
        _stream.avail_out = _buffer->getSize();

        while (true) {
            int ret;

            switch (_mode) {
            case ZSTREAM_MODE_DEFLATE: ret = deflate(&_stream, Z_NO_FLUSH); break;
            case ZSTREAM_MODE_INFLATE: ret = inflate(&_stream, Z_NO_FLUSH); break;
            }

            if (ret == Z_STREAM_ERROR) {
                return false;
            }

            if (_stream.avail_in == 0) { // all input has been consumed
                // feed anything left in the output buffer to out
                if ((int)_stream.avail_out < _buffer->getSize()) {
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
    ZStream::finish(Blob* out)
    {
        assert(out);

        if (_mode == ZSTREAM_MODE_INVALID) {
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
            case ZSTREAM_MODE_DEFLATE: ret = deflate(&_stream, Z_FINISH); break;
            case ZSTREAM_MODE_INFLATE: ret = inflate(&_stream, Z_FINISH); break;
            }

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
                if ((int)_stream.avail_out < _buffer->getSize()) {
                    int new_data_size = _buffer->getSize() - _stream.avail_out;
                    int old_data_size = out->getSize();
                    out->resize(old_data_size + new_data_size);
                    memcpy(out->getBuffer() + old_data_size, _buffer->getBuffer(), new_data_size);
                }

                // we are done
                deinit();
                return true;
            }
        }
    }

} // namespace sphere
