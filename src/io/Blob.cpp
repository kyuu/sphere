#include <cassert>
#include <cstring>
#include <cmath>
#include "Blob.hpp"


//-----------------------------------------------------------------
Blob*
Blob::Create(int size)
{
    assert(size >= 0);
    if (size >= 0) {
        try {
            BlobPtr blob = new Blob();
            if (size > 0 && !blob->resize(size)) {
                return 0;
            }
            return blob.release();
        } catch (const std::bad_alloc& e) {
        }
    }
    return 0;
}

//-----------------------------------------------------------------
Blob*
Blob::Create(const void* buf, int size)
{
    assert(buf);
    assert(size >= 0);
    if (buf && size > 0) {
        try {
            BlobPtr blob = new Blob();
            if (!blob->assign(buf, size)) {
                return 0;
            }
            return blob.release();
        } catch (const std::bad_alloc& e) {
            return 0;
        }
    }
    return 0;
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
Blob::at(int pos)
{
    assert(_size > 0);
    assert(pos >= 0 && pos < _size);
    return _buffer[pos];
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
bool
Blob::assign(const void* buf, int size)
{
    assert(buf);
    assert(size > 0);
    if (buf && size > 0 && resize(size)) {
        memcpy(_buffer, buf, size);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------
bool
Blob::append(const void* buf, int size)
{
    assert(buf);
    assert(size > 0);
    if (buf && size > 0) {
        int old_size = getSize();
        if (resize(old_size + size)) {
            memcpy(_buffer + old_size, buf, size);
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------
Blob*
Blob::concat(const void* buf, int size)
{
    assert(buf);
    assert(size > 0);
    if (buf && size > 0) {
        Blob* result = New(_size + size);
        if (getSize() > 0) {
            memcpy(result->getBuffer(), _buffer, _size);
        }
        memcpy(result->getBuffer() + _size, buf, size);
        return result;
    }
    return 0;
}

//-----------------------------------------------------------------
bool
Blob::resize(int size)
{
    assert(size >= 0);
    if (size >= 0 && reserve(size)) {
        _size = size;
        return true;
    }
    return false;
}

//-----------------------------------------------------------------
bool
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
            return false;
        }
    }
    return true;
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
