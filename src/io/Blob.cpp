#include <cassert>
#include <cstring>
#include <cmath>
#include "../error.hpp"
#include "endian.hpp"
#include "Blob.hpp"


//-----------------------------------------------------------------
Blob*
Blob::Create(int size)
{
    assert(size >= 0);
    try {
        BlobPtr blob = new Blob();
        if (size > 0) {
            blob->resize(size);
        }
        return blob.release();
    } catch (const std::bad_alloc&) {
        ReportOutOfMemory();
        return 0;
    }
}

//-----------------------------------------------------------------
Blob*
Blob::Create(const void* buf, int size)
{
    assert(buf);
    assert(size >= 0);
    try {
        BlobPtr blob = new Blob();
        blob->assign(buf, size);
        return blob.release();
    } catch (const std::bad_alloc& e) {
        ReportOutOfMemory();
        return 0;
    }
}

//-----------------------------------------------------------------
Blob::Blob()
    : _buffer(0)
    , _reserved(0)
    , _size(0)
    , _streampos(0)
    , _eof(false)
{
}

//-----------------------------------------------------------------
Blob::~Blob()
{
    if (_buffer) {
        delete[] _buffer;
    }
}

//-----------------------------------------------------------------
u8&
Blob::at(int idx)
{
    assert(_size > 0);
    assert(idx >= 0 && idx < _size);
    return _buffer[idx];
}

//-----------------------------------------------------------------
void
Blob::clear()
{
    if (_buffer) {
        delete[] _buffer;
        _buffer   = 0;
        _reserved = 0;
        _size     = 0;
    }
}

//-----------------------------------------------------------------
void
Blob::reset(u8 val)
{
    if (_buffer && _size > 0) {
        memset(_buffer, val, _size);
    }
}

//-----------------------------------------------------------------
void
Blob::assign(const void* buf, int size)
{
    assert(buf);
    assert(size > 0);
    resize(size);
    memcpy(_buffer, buf, size);
}

//-----------------------------------------------------------------
void
Blob::append(const void* buf, int size)
{
    assert(buf);
    assert(size > 0);
    int old_size = _size;
    resize(old_size + size);
    memcpy(_buffer + old_size, buf, size);
}

//-----------------------------------------------------------------
Blob*
Blob::concat(const void* buf, int size)
{
    assert(buf);
    assert(size > 0);
    Blob* result = Create(_size + size);
    if (_size > 0) {
        memcpy(result->getBuffer(), _buffer, _size);
    }
    memcpy(result->getBuffer() + _size, buf, size);
    return result;
}

//-----------------------------------------------------------------
void
Blob::resize(int size)
{
    assert(size >= 0);
    reserve(size);
    _size = size;
}

//-----------------------------------------------------------------
void
Blob::bloat()
{
    _size = _reserved;
}

//-----------------------------------------------------------------
void
Blob::reserve(int size)
{
    assert(size >= 0);
    if (size > _reserved) {
        try {
            int new_reserved = 1 << ((int)ceil(log10((double)size) / log10((double)2)));
            u8* new_buffer = new u8[new_reserved];
            if (_size > 0) {
                memcpy(new_buffer, _buffer, _size);
            }
            delete[] _buffer;
            _buffer = new_buffer;
            _reserved = new_reserved;
        } catch (const std::bad_alloc& e) {
            ReportOutOfMemory();
        }
    }
}

//-----------------------------------------------------------------
void
Blob::doubleCapacity()
{
    reserve(_reserved * 2);
}

//-----------------------------------------------------------------
void
Blob::swap2()
{
    if (_buffer && _size % 2 == 0) {
        ::swap2(_buffer, _size / 2);
    }
}

//-----------------------------------------------------------------
void
Blob::swap4()
{
    if (_buffer && _size % 4 == 0) {
        ::swap4(_buffer, _size / 4);
    }
}

//-----------------------------------------------------------------
void
Blob::swap8()
{
    if (_buffer && _size % 8 == 0) {
        ::swap8(_buffer, _size / 8);
    }
}

//-----------------------------------------------------------------
bool
Blob::isOpen() const
{
    return true;
}

//-----------------------------------------------------------------
bool
Blob::isReadable() const
{
    return true;
}

//-----------------------------------------------------------------
bool
Blob::isWriteable() const
{
    return true;
}

//-----------------------------------------------------------------
bool
Blob::close()
{
    return false;
}

//-----------------------------------------------------------------
int
Blob::tell()
{
    return _streampos;
}

//-----------------------------------------------------------------
bool
Blob::seek(int offset, int origin)
{
    // clear end-of-stream flag
    _eof = false;

    switch (origin) {
    case IStream::BEG: {
        if (offset >= 0 && offset <= _size) {
            _streampos = offset;
        } else {
            return false;
        }
        break;
    }
    case IStream::CUR: {
        int newstreampos = _streampos + offset;
        if (newstreampos >= 0 && newstreampos <= _size) {
            _streampos = newstreampos;
        } else {
            return false;
        }
        break;
    }
    case IStream::END: {
        if (offset <= 0) {
            int newstreampos = _size + offset;
            if (newstreampos >= 0 && newstreampos <= _size) {
                _streampos = newstreampos;
            } else {
                return false;
            }
        } else {
            return false;
        }
        break;
    }
    default:
        return false;
    }
    return true;
}

//-----------------------------------------------------------------
uint
Blob::read(void* buffer, uint size)
{
    assert(buffer);
    if (_eof || size == 0) {
        return 0;
    }
    if (_streampos >= _size) {
        _eof = true;
        return 0;
    }
    uint num_read = ((size <= _size - _streampos) ? size : _size - _streampos);
    memcpy(buffer, _buffer + _streampos, num_read);
    _streampos += num_read;
    if (num_read < size) {
        _eof = true;
    }
    return num_read;
}

//-----------------------------------------------------------------
uint
Blob::write(const void* buffer, uint size)
{
    assert(buffer);
    if (size == 0) {
        return 0;
    }
    if (_streampos + size > _size) {
        resize(_streampos + size);
    }
    memcpy(_buffer + _streampos, buffer, size);
    _streampos += size;
    return size;
}

//-----------------------------------------------------------------
bool
Blob::flush()
{
    return true;
}

//-----------------------------------------------------------------
bool
Blob::eof()
{
    return _eof;
}
