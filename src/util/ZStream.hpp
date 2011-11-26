#ifndef ZSTREAM_HPP
#define ZSTREAM_HPP

#include <zlib.h>
#include "../common/IRefCounted.hpp"
#include "../common/RefImpl.hpp"
#include "../common/RefPtr.hpp"
#include "../io/Blob.hpp"


class ZStream : public RefImpl<IRefCounted> {
public:
    static ZStream* Create();

    int  getBufferSize() const;
    void setBufferSize(int size);
    bool compress(const u8* buf, int len, Blob* out);
    bool decompress(const u8* buf, int len, Blob* out);
    bool finish(Blob* out);

private:
    ZStream();
    ~ZStream();
    void initBuffer();
    void cleanup();
    bool initForCompression();
    bool initForDecompression();
    bool consume(const u8* buf, int len, Blob* out);

private:
    int _mode;
    z_stream _stream;
    BlobPtr _buffer;
};

typedef RefPtr<ZStream> ZStreamPtr;


#endif
