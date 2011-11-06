#ifndef IFILE_HPP
#define IFILE_HPP

#include <string>
#include "../common/RefPtr.hpp"
#include "IStream.hpp"

class IFile : public IStream {
public:
    enum Mode {
        IN = 0,
        OUT,
        APPEND,
    };

    virtual const std::string& getName() const = 0;

protected:
    ~IFile() { }
};

typedef RefPtr<IFile> FilePtr;


#endif
