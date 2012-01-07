#ifndef SPHERE_FILE_HPP
#define SPHERE_FILE_HPP

#include <cstdio>
#include <string>
#include "../common/RefImpl.hpp"
#include "IFile.hpp"


namespace sphere {

    class File : public RefImpl<IFile> {
    public:
        static File* Create();

        bool open(const std::string& filename, int mode = IFile::FM_IN);
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

} // namespace sphere


#endif
