#ifndef FILE_HPP
#define FILE_HPP

#include <cstdio>
#include <string>
#include "../common/types.hpp"
#include "../common/RefImpl.hpp"
#include "../io/IFile.hpp"


class File : public RefImpl<IFile> {
public:
    static File* Create();

    bool open(const std::string& filename, int mode = IFile::IN);
    void setName(const std::string& name);

    // IFile implementation
    const std::string& getName() const;

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
    File();
    virtual ~File();

private:
    FILE* _file;
    int _mode;
    std::string _name;
};


#endif
