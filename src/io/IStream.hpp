#ifndef ISTREAM_HPP
#define ISTREAM_HPP

#include "../common/RefPtr.hpp"
#include "../common/IRefCounted.hpp"


class IStream : public IRefCounted {
public:
    enum Origin {
        BEG = 0,
        CUR,
        END,
    };

    virtual bool isOpen() const = 0;
    virtual bool isReadable() const = 0;
    virtual bool isWriteable() const = 0;
    virtual bool close() = 0;
    virtual int  tell() = 0;
    virtual bool seek(int offset, int origin = BEG) = 0;
    virtual int  read(void* buffer, int size) = 0;
    virtual int  write(const void* buffer, int size) = 0;
    virtual bool flush() = 0;
    virtual bool eof() = 0;

protected:
    ~IStream() { }
};

typedef RefPtr<IStream> StreamPtr;


#endif
