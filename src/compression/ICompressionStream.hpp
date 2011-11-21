#ifndef ICOMPRESSIONSTREAM_HPP
#define ICOMPRESSIONSTREAM_HPP

#include "../../common/IRefCounted.hpp"
#include "../../common/RefPtr.hpp"
#include "../../common/types.hpp"
#include "../../io/Blob.hpp"


class ICompressionStream : public IRefCounted {
public:
    enum Type {
        DEFLATE = 0,
        LZMA2,
        BZIP2,
    };

    enum Mode {
        COMPRESS = 0,
        DECOMPRESS,
    };

    virtual bool init(int mode) = 0;
    virtual bool isInitialized() const = 0;
    virtual int  getMode() const = 0;
    virtual int  getBufferSize() const = 0;
    virtual bool setBufferSize(int size) = 0;
    virtual bool consume(const u8* buf, int len, Blob* out) = 0;
    virtual bool close(Blob* out) = 0;

protected:
    ~ICompressionStream() { }
};

typedef RefPtr<ICompressionStream> CompressionStreamPtr;


#endif
