#ifndef ZSTREAM_HPP
#define ZSTREAM_HPP

#include <zlib.h>
#include "../common/IRefCounted.hpp"
#include "../common/RefImpl.hpp"
#include "../common/RefPtr.hpp"
#include "../common/types.hpp"


class ZStream : public RefImpl<IRefCounted> {
public:
    enum {
        INVALID = 0,
        DEFLATE,
        INFLATE,
    }

    static ZStream* Create();

    bool init(int mode);
    bool isInitialized() const;
    int  getMode() const;
    int  getBufferSize() const;
    bool setBufferSize(int size);
    bool process(const u8* in, int len, Blob* out);
    bool finish(Blob* out);

private:
    ZStream();
    ~ZStream();
    void internalCleanup();
    bool internalInit();

private:
    int _mode;
    bool _initialized;
    z_stream _stream;
    BlobPtr _buffer;
};

typedef RefPtr<ZStream> ZStreamPtr;


#endif
