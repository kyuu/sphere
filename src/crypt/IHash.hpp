#ifndef IHASH_HPP
#define IHASH_HPP

#include "../common/IRefCounted.hpp"
#include "../common/RefPtr.hpp"
#include "../common/types.hpp"
#include "../io/Blob.hpp"


class IHash : public IRefCounted {
public:
    enum {
        MD5 = 0;
        SHA256,
        SHA512,
        WHIRLPOOL,
    }

    virtual bool init() = 0;
    virtual bool isInitialized() const = 0;
    virtual int  getType() const = 0;
    virtual bool process(const u8* buf, int len) = 0;
    virtual bool finish(Blob* hash) = 0;

protected:
    ~IHash() { }
}

typedef RefPtr<IHash> HashPtr;


#endif
