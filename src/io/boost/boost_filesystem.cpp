#include <cassert>
#include <cstring>
#include <boost/filesystem.hpp>
#include "../filesystem.hpp"
#include "../File.hpp"

namespace fs = boost::filesystem;


//-----------------------------------------------------------------
// globals
static fs::path g_EnginePath;
static fs::path g_CommonPath;
static fs::path g_DataPath;

//-----------------------------------------------------------------
static bool process_path(const std::string& raw, std::string& abs)
{
    // see if path is invalid
    if (raw.empty()      || // path is empty
        raw[0] == '\\'   || // path starts with a "\"
        raw[0] == '~'    || // path starts with a "~"
        raw[0] == '.'    || // path starts with a "."
        raw.find(':') != std::string::npos || // path contains ":"
        raw.find("..") != std::string::npos)  // path contains ".."
    {
        return false;
    }

    if (raw.find("/data") == 0) { // path begins with "/data", so is relative to the data path
        if (raw.size() == strlen("/data")) { // path is equal to "/data"
            abs = g_DataPath.string();
        } else {
            abs = (g_DataPath / raw.substr(strlen("/data"))).string();
        }
    } else if (raw.find("/common") == 0) { // path begins with "/common", so is relative to the common path
        if (raw.size() == strlen("/common")) { // path is equal to "/common"
            abs = g_CommonPath.string();
        } else {
            abs = (g_CommonPath / raw.substr(strlen("/common"))).string();
        }
    } else if (raw.find("/engine") == 0) { // path begins with "/engine", so is relative to the engine path
        if (raw.size() == strlen("/engine")) { // path is equal to "/engine"
            abs = g_EnginePath.string();
        } else {
            abs = (g_EnginePath / raw.substr(strlen("/engine"))).string();
        }
    } else {
        return false;
    }

    return true;
}

namespace sphere {
    namespace io {
        namespace filesystem {

            //-----------------------------------------------------------------
            IFile* OpenFile(const std::string& filename, int mode)
            {
                std::string abs;
                if (process_path(filename, abs)) {
                    RefPtr<File> file = File::Create();
                    if (file->open(abs, mode)) {
                        file->setName(filename);
                        return file.release();
                    }
                }
                return 0;
            }

            //-----------------------------------------------------------------
            bool FileExists(const std::string& filename)
            {
                std::string abs;
                if (process_path(filename, abs)) {
                    try {
                        return fs::exists(abs);
                    } catch (...) { }
                }
                return false;
            }

            //-----------------------------------------------------------------
            bool IsFile(const std::string& filename)
            {
                std::string abs;
                if (process_path(filename, abs)) {
                    try {
                        return fs::is_regular_file(abs);
                    } catch (...) { }
                }
                return false;
            }

            //-----------------------------------------------------------------
            bool IsDirectory(const std::string& filename)
            {
                std::string abs;
                if (process_path(filename, abs)) {
                    try {
                        return fs::is_directory(abs);
                    } catch (...) { }
                }
                return false;
            }

            //-----------------------------------------------------------------
            int GetFileSize(const std::string& filename)
            {
                std::string abs;
                if (process_path(filename, abs)) {
                    try {
                        return (int)fs::file_size(abs);
                    } catch (...) { }
                }
                return -1;
            }

            //-----------------------------------------------------------------
            int GetFileModTime(const std::string& filename)
            {
                std::string abs;
                if (process_path(filename, abs)) {
                    try {
                        return (int)fs::last_write_time(abs);
                    } catch (...) { }
                }
                return -1;
            }

            //-----------------------------------------------------------------
            bool CreateDirectory(const std::string& directory)
            {
                std::string abs;
                if (process_path(directory, abs)) {
                    try {
                        return fs::create_directory(abs);
                    } catch (...) { }
                }
                return false;
            }

            //-----------------------------------------------------------------
            bool RemoveFile(const std::string& filename)
            {
                std::string abs;
                if (process_path(filename, abs)) {
                    try {
                        fs::remove(abs);
                        return true;
                    } catch (...) { }
                }
                return false;
            }

            //-----------------------------------------------------------------
            bool RenameFile(const std::string& filenameFrom, const std::string& filenameTo)
            {
                std::string absFrom;
                std::string absTo;
                if (process_path(filenameFrom, absFrom) &&
                    process_path(filenameTo, absTo))
                {
                    try {
                        fs::rename(absFrom, absTo);
                        return true;
                    } catch (...) { }
                }
                return false;
            }

            //-----------------------------------------------------------------
            bool EnumerateFiles(const std::string& directory, std::vector<std::string>& fileList)
            {
                std::string abs;
                if (process_path(directory, abs)) {
                    try {
                        fileList.clear();
                        fs::directory_iterator end_iter;
                        for (fs::directory_iterator iter(abs); iter != end_iter; ++iter) {
                            fileList.push_back(iter->path().filename());
                        }
                        return true;
                    } catch (...) { }
                }
                return false;
            }

            namespace internal {

                //-----------------------------------------------------------------
                bool InitFileSystem(const Log& log, const std::string& commonPath, const std::string& dataPath)
                {
                    g_EnginePath = fs::initial_path();
                    log.info() << "Engine path: '" << g_EnginePath.string() << "'";

                    // set up common path
                    if (fs::path(commonPath).is_complete()) {
                        g_CommonPath = commonPath;
                    } else {
                        g_CommonPath = g_EnginePath / commonPath;
                    }
                    log.info() << "Common path: '" << g_CommonPath.string() << "'";

                    // set up data path
                    if (fs::path(dataPath).is_complete()) {
                        g_DataPath = dataPath;
                    } else {
                        g_DataPath = g_EnginePath / dataPath;
                    }
                    log.info() << "Data path: '" << g_DataPath.string() << "'";

                    // enter data path
                    try {
                        fs::current_path(g_DataPath);
                    } catch (...) {
                        log.error() << "Could not set current path to: '" << g_DataPath << "'";
                        return false;
                    }

                    return true;
                }

                //-----------------------------------------------------------------
                void DeinitFileSystem()
                {
                    // NO-OP
                }

                //-----------------------------------------------------------------
                const std::string& GetEnginePath()
                {
                    return g_EnginePath.string();
                }

                //-----------------------------------------------------------------
                const std::string& GetCommonPath()
                {
                    return g_CommonPath.string();
                }

                //-----------------------------------------------------------------
                const std::string& GetDataPath()
                {
                    return g_DataPath.string();
                }

                //-----------------------------------------------------------------
                std::string GetCurrentPath()
                {
                    return fs::current_path().string();
                }

                //-----------------------------------------------------------------
                bool SetCurrentPath(const std::string& path)
                {
                    try {
                        fs::current_path(path);
                        return true;
                    } catch (...) {
                        return false;
                    }
                }

            } // namespace internal
        } // namespace filesystem
    } // namespace io
} // namespace sphere
