#ifndef DEFLATESTREAM_HPP
#define DEFLATESTREAM_HPP

#include <zlib.h>
#include "../../common/RefImpl.hpp"
#include "../ICompressionStream.hpp"


class DeflateStream : public RefImpl<ICompressionStream> {
public:
    static DeflateStream* Create();

    // ICompressionStream implementation
    int  getType() const;
    bool init(int mode);
    bool isInitialized() const;
    int  getMode() const;
    int  getBufferSize() const;
    void setBufferSize(int size);
    bool consume(const u8* buf, int len, Blob* out);
    bool close(Blob* out);

private:
    DeflateStream();
    ~DeflateStream();
    void internalInit();
    void internalCleanup();

private:
    int _mode;
    bool _initialized;
    z_stream _stream;
    BlobPtr _buffer;
};

typedef RefPtr<DeflateStream> ZStreamPtr;


#endif
