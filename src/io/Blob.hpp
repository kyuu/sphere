#ifndef BLOB_HPP
#define BLOB_HPP

#include "../common/types.hpp"
#include "../common/RefPtr.hpp"
#include "../common/RefImpl.hpp"
#include "../filesystem/IStream.hpp"


class Blob : public RefImpl<IStream> {
public:
    static Blob* Create(int size = 0);
    static Blob* Create(const void* buf, int size);

    int   getSize() const;
    int   getCapacity() const;
    u8*   getBuffer();
    u8&   at(int pos);
    void  clear();
    void  reset(u8 val = 0);
    bool  assign(const void* buf, int size);
    bool  append(const void* buf, int size);
    Blob* concat(const void* buf, int size);
    bool  resize(int size);
    void  bloat();
    bool  reserve(int size);
    bool  doubleCapacity();

    // IStream implementation
    bool isOpen() const;
    bool isReadable() const;
    bool isWriteable() const;
    bool close();
    int  tell();
    bool seek(int offset, int origin = IStream::BEG);
    int  read(void* buffer, int size);
    int  write(const void* buffer, int size);
    bool flush();
    bool eof();

private:
    Blob();
    virtual ~Blob();

private:
    u8* _buffer;
    int _reserved;
    int _size;
    int _streampos;
    bool _eof;
};

typedef RefPtr<Blob> BlobPtr;

//-----------------------------------------------------------------
inline int
Blob::getSize() const
{
    return _size;
}

//-----------------------------------------------------------------
inline int
Blob::getCapacity() const
{
    return _reserved;
}

//-----------------------------------------------------------------
inline u8*
Blob::getBuffer()
{
    return _buffer;
}


#endif
