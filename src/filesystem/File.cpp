#include <cassert>
#include <stdexcept>
#include "../error.hpp"
#include "File.hpp"


//-----------------------------------------------------------------
File*
File::Create()
{
    try {
        return new File();
    } catch (const std::bad_alloc&) {
        ReportOutOfMemory();
        return 0;
    }
}

//-----------------------------------------------------------------
bool
File::open(const std::string& filename, int mode)
{
    assert(!filename.empty());
    close();
    if (filename.empty()) {
        return false;
    }
    switch (mode) {
    case IFile::IN:     _file = fopen(filename.c_str(), "rb"); break;
    case IFile::OUT:    _file = fopen(filename.c_str(), "wb"); break;
    case IFile::APPEND: _file = fopen(filename.c_str(), "ab"); break;
    default: return false;
    }
    if (_file) {
        _mode = mode;
        _name = filename;
        return true;
    }
    return false;
}

//-----------------------------------------------------------------
void
File::setName(const std::string& name)
{
    _name = name;
}

//-----------------------------------------------------------------
File::File()
    : _file(0)
    , _mode(-1)
{
}

//-----------------------------------------------------------------
File::~File()
{
    close();
}

//-----------------------------------------------------------------
const std::string&
File::getName() const
{
    return _name;
}

//-----------------------------------------------------------------
bool
File::isOpen() const
{
    return _file != 0;
}

//-----------------------------------------------------------------
bool
File::isReadable() const
{
    if (_file) {
        return (_mode == IFile::IN);
    }
    return false;
}

//-----------------------------------------------------------------
bool
File::isWriteable() const
{
    if (_file) {
        return (_mode == IFile::OUT || _mode == IFile::APPEND);
    }
    return false;
}

//-----------------------------------------------------------------
bool
File::close()
{
    if (_file) {
        int res = fclose(_file);
        if (res == 0) {
            _file = 0;
            _mode = -1;
            _name.clear();
            return true;
        }
        return false;
    }
    return true;
}

//-----------------------------------------------------------------
int
File::tell()
{
    if (!_file) {
        return -1;
    }
    return ftell(_file);
}

//-----------------------------------------------------------------
bool
File::seek(int offset, int origin)
{
    if (!_file) {
        return false;
    }
    switch (origin) {
    case IStream::BEG: return fseek(_file, offset, SEEK_SET) == 0;
    case IStream::CUR: return fseek(_file, offset, SEEK_CUR) == 0;
    case IStream::END: return fseek(_file, offset, SEEK_END) == 0;
    default: return false;
    }
}

//-----------------------------------------------------------------
int
File::read(void* buffer, int size)
{
    assert(buffer);
    assert(size >= 0);
    if (!_file || !isReadable()) {
        return -1;
    }
    if (size == 0) {
        return 0;
    }
    return fread(buffer, 1, size, _file);
}

//-----------------------------------------------------------------
int
File::write(const void* buffer, int size)
{
    assert(buffer);
    assert(size >= 0);
    if (!_file || !isWriteable()) {
        return -1;
    }
    if (size == 0) {
        return 0;
    }
    return fwrite(buffer, 1, size, _file);
}

//-----------------------------------------------------------------
bool
File::flush()
{
    if (!_file) {
        return false;
    }
    return fflush(_file) == 0;
}

//-----------------------------------------------------------------
bool
File::eof()
{
    if (!_file) {
        return true;
    }
    return feof(_file) != 0;
}
