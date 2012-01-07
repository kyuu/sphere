#ifndef SPHERE_BLOB_HPP
#define SPHERE_BLOB_HPP

#include "../common/types.hpp"
#include "../common/RefPtr.hpp"
#include "../common/RefImpl.hpp"
#include "../io/IStream.hpp"


namespace sphere {

    class Blob : public RefImpl<IStream> {
    public:
        static Blob* Create(int size = 0);
        static Blob* Create(const void* buffer, int size);

        int   getSize() const;
        int   getCapacity() const;
        u8*   getBuffer();
        u8&   at(int idx);
        void  clear();
        void  reset(u8 val = 0);
        void  assign(const void* buffer, int size);
        void  append(const void* buffer, int size);
        Blob* concat(const void* buffer, int size);
        void  resize(int size);
        void  bloat();
        void  reserve(int size);
        void  doubleCapacity();
        void  swap2();
        void  swap4();
        void  swap8();

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

} // namespace sphere


#endif
