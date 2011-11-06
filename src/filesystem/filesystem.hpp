#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <vector>
#include <string>
#include "../common/types.hpp"
#include "IFile.hpp"


namespace filesystem {

    IFile* OpenFile(const std::string& p, int mode = IFile::IN);
    bool   Exists(const std::string& p);
    bool   IsRegularFile(const std::string& p);
    bool   IsDirectory(const std::string& p);
    i32    GetLastWriteTime(const std::string& p);
    bool   CreateDirectory(const std::string& p);
    bool   Remove(const std::string& p);
    bool   Rename(const std::string& p1, const std::string& p2);
    bool   GetFileList(const std::string& p, std::vector<std::string>& out);
    // bool AddArchive(const std::string& filename);

    namespace internal {

        bool InitFilesystem(const Log& log);
        void DeinitFilesystem(const Log& log);
        const std::string& GetEnginePath();
        const std::string& GetCurrentPath();
        bool SetCurrentPath(const std::string& p);
        const std::string& GetDataPath();
        bool SetDataPath(const std::string& p);

    } // namespace internal

} // namespace filesystem


#endif
