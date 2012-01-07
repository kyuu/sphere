#ifndef SPHERE_IFILE_HPP
#define SPHERE_IFILE_HPP

#include <string>
#include "../common/RefPtr.hpp"
#include "IStream.hpp"


namespace sphere {

    class IFile : public IStream {
    public:
        enum Mode {
            FM_IN = 0,
            FM_OUT,
            FM_APPEND,
        };

        virtual const std::string& getName() const = 0;

    protected:
        ~IFile() { }
    };

    typedef RefPtr<IFile> FilePtr;

} // namespace sphere


#endif
