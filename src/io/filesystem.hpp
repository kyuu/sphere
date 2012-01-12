#ifndef SPHERE_FILESYSTEM_HPP
#define SPHERE_FILESYSTEM_HPP

#include <vector>
#include <string>
#include "../Log.hpp"
#include "IFile.hpp"


namespace sphere {
    namespace io {
        namespace filesystem {

            IFile* OpenFile(const std::string& filename, int mode = IFile::FM_IN);
            bool   FileExists(const std::string& filename);
            bool   IsFile(const std::string& filename);
            bool   IsDirectory(const std::string& filename);
            int    GetFileSize(const std::string& filename);
            int    GetFileModTime(const std::string& filename);
            bool   CreateDirectory(const std::string& directory);
            bool   RemoveFile(const std::string& filename);
            bool   RenameFile(const std::string& filenameFrom, const std::string& filenameTo);
            bool   EnumerateFiles(const std::string& directory, std::vector<std::string>& fileList);

            namespace internal {

                bool InitFileSystem(const Log& log, const std::string& commonPath, const std::string& dataPath);
                void DeinitFileSystem();
                const std::string& GetEnginePath();
                const std::string& GetCommonPath();
                const std::string& GetDataPath();
                std::string GetCurrentPath();
                bool SetCurrentPath(const std::string& p);

            } // namespace internal
        } // namespace filesystem
    } // namespace io
} // namespace sphere


#endif
