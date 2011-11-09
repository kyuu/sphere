#include <cassert>
#include <cstring>
#include <boost/filesystem.hpp>
#include "../filesystem.hpp"
#include "File.hpp"

namespace fs = boost::filesystem;


//-----------------------------------------------------------------
// globals
static fs::path g_engine_path;
static fs::path g_common_path;
static fs::path g_data_path;

//-----------------------------------------------------------------
static bool make_path(const std::string& p, std::string& out)
{
    if ((!p.empty() && p[0] == '\\') ||     // path starts with a backslash
        p.find(':') != std::string::npos || // path contains a ":"
        p.find("..") != std::string::npos)  // path contains a ".."
    {
        return false;
    }

    if (p.empty()) { // path is empty, so simply defaults to the data path
        out = g_data_path.string();
    } else if (p[0] == '/') {
        if (p.find("/engine") == 0) { // path begins with "/engine", so is relative to the engine path
            if (p.size() == strlen("/engine")) {
                out = g_engine_path.string();
            } else {
                out = (g_engine_path / p.substr(strlen("/engine"))).string();
            }
        } else if (p.find("/common") == 0) { // path begins with "/common", so is relative to the common path
            if (p.size() == strlen("/common")) {
                out = g_common_path.string();
            } else {
                out = (g_common_path / p.substr(strlen("/common"))).string();
            }
        } else {
            return false;
        }
    } else { // path begins not with a forwardslash, so is relative to the data path
        out = (g_data_path / p).string();
    }

    return true;
}

namespace filesystem {

    //-----------------------------------------------------------------
    IFile* OpenFile(const std::string& p, int mode)
    {
        std::string _p;
        if (make_path(p, _p)) {
            RefPtr<File> file = File::Create();
            if (file && file->open(_p, mode)) {
                file->setName(p);
                return file.release();
            }
        }
        return 0;
    }

    //-----------------------------------------------------------------
    bool Exists(const std::string& p)
    {
        std::string _p;
        if (make_path(p, _p)) {
            try {
                return fs::exists(_p);
            } catch (...) { }
        }
        return false;
    }

    //-----------------------------------------------------------------
    bool IsRegularFile(const std::string& p)
    {
        std::string _p;
        if (make_path(p, _p)) {
            try {
                return fs::is_regular_file(_p);
            } catch (...) { }
        }
        return false;
    }

    //-----------------------------------------------------------------
    bool IsDirectory(const std::string& p)
    {
        std::string _p;
        if (make_path(p, _p)) {
            try {
                return fs::is_directory(_p);
            } catch (...) { }
        }
        return false;
    }

    //-----------------------------------------------------------------
    int GetLastWriteTime(const std::string& p)
    {
        std::string _p;
        if (make_path(p, _p)) {
            try {
                return (int)fs::last_write_time(_p);
            } catch (...) { }
        }
        return -1;
    }

    //-----------------------------------------------------------------
    bool CreateDirectory(const std::string& p)
    {
        std::string _p;
        if (make_path(p, _p)) {
            try {
                return fs::create_directory(_p);
            } catch (...) { }
        }
        return false;
    }

    //-----------------------------------------------------------------
    bool Remove(const std::string& p)
    {
        std::string _p;
        if (make_path(p, _p)) {
            try {
                fs::remove(_p);
                return true;
            } catch (...) { }
        }
        return false;
    }

    //-----------------------------------------------------------------
    bool Rename(const std::string& p1, const std::string& p2)
    {
        std::string _p1;
        std::string _p2;
        if (make_path(p1, _p1) && make_path(p2, _p2)) {
            try {
                fs::rename(_p1, _p2);
                return true;
            } catch (...) { }
        }
        return false;
    }

    //-----------------------------------------------------------------
    bool GetFileList(const std::string& p, std::vector<std::string>& out)
    {
        std::string _p;
        if (make_path(p, _p)) {
            std::string dir_path(p);
            if (!dir_path.empty() && dir_path[dir_path.size()-1] != '/') {
                dir_path += "/";
            }
            try {
                out.clear();
                fs::directory_iterator end_iter;
                for (fs::directory_iterator iter(_p); iter != end_iter; ++iter) {
                    out.push_back(dir_path + iter->path().filename());
                }
                return true;
            } catch (...) { }
        }
        return false;
    }

    namespace internal {

        //-----------------------------------------------------------------
        bool InitFilesystem(const Log& log)
        {
            g_engine_path = fs::initial_path();
            g_common_path = g_engine_path / "common";
            return true;
        }

        //-----------------------------------------------------------------
        void DeinitFilesystem(const Log& log)
        {
            // NO-OP
        }

        //-----------------------------------------------------------------
        const std::string& GetEnginePath()
        {
            return g_engine_path.string();
        }

        //-----------------------------------------------------------------
        std::string GetCurrentPath()
        {
            return fs::current_path().string();
        }

        //-----------------------------------------------------------------
        bool SetCurrentPath(const std::string& p)
        {
            try {
                fs::current_path(path);
                return true;
            } catch (const fs::filesystem_error& e) {
                return false;
            }
        }

        //-----------------------------------------------------------------
        const std::string& GetDataPath()
        {
            return g_data_path.string();
        }

        //-----------------------------------------------------------------
        void SetDataPath(const std::string& p)
        {
            g_data_path = p;
        }

    } // namespace internal

} // namespace filesystem
