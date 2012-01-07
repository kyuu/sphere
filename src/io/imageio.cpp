#include <cassert>
#include <memory>
#include <corona.h>
#include "imageio.hpp"


namespace sphere {
    namespace io {

        //-----------------------------------------------------------------
        struct CoronaFileAdapter : public corona::DLLImplementation<corona::File>
        {
            CoronaFileAdapter(IStream* stream) : _stream(0) {
                assert(stream);
                stream->grab();
                _stream = stream;
            }

            ~CoronaFileAdapter() {
                _stream->drop();
            }

            int COR_CALL read(void* buffer, int size) {
                return _stream->read(buffer, size);
            }

            int COR_CALL write(const void* buffer, int size) {
                return _stream->write(buffer, size);
            }

            bool COR_CALL seek(int pos, SeekMode mode) {
                switch (mode) {
                case BEGIN:   return _stream->seek(pos, IStream::BEG);
                case CURRENT: return _stream->seek(pos, IStream::CUR);
                case END:     return _stream->seek(pos, IStream::END);
                default: return false;
                }
            }

            int COR_CALL tell() {
                return _stream->tell();
            }

        private:
            IStream* _stream;
        };

        //-----------------------------------------------------------------
        Canvas* LoadImage(IStream* stream)
        {
            assert(stream);
            assert(stream->isReadable());
            CoronaFileAdapter cfa(stream);
            std::auto_ptr<corona::Image> img(corona::OpenImage(&cfa, corona::PF_R8G8B8A8));
            if (!img.get()) {
                return 0;
            }
            return Canvas::Create(img->getWidth(), img->getHeight(), (const RGBA*)img->getPixels());
        }

        //-----------------------------------------------------------------
        bool SaveImage(Canvas* image, IStream* stream)
        {
            assert(stream);
            assert(stream->isWriteable());
            std::auto_ptr<corona::Image> img(corona::CreateImage(image->getWidth(), image->getHeight(), corona::PF_R8G8B8A8));
            if (!img.get()) {
                return false;
            }
            memcpy(img->getPixels(), image->getPixels(), image->getNumPixels() * Canvas::GetNumBytesPerPixel());
            CoronaFileAdapter cfa(stream);
            return corona::SaveImage(&cfa, corona::FF_PNG, img.get());
        }

    } // namespace io
} // namespace sphere
