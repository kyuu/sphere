#ifndef SPHERE_ITEXTURE_HPP
#define SPHERE_ITEXTURE_HPP

#include "../common/IRefCounted.hpp"
#include "../common/RefPtr.hpp"
#include "../base/Dim2.hpp"


namespace sphere {

    class ITexture : public IRefCounted {
    public:
        virtual const Dim2i& getTextureSize() const = 0;
        virtual const Dim2i& getSize() const = 0;

    protected:
        ~ITexture() { }
    };

    typedef RefPtr<ITexture> TexturePtr;

} // namespace sphere


#endif
