#include <cassert>
#include <cstring>
#include <boost/filesystem.hpp>
#include "../filesystem.hpp"
#include "../File.hpp"

namespace fs = boost::filesystem;


//-----------------------------------------------------------------
// globals
static fs::path g_engine_path;
static fs::path g_common_path;
static fs::path g_data_path;

//-----------------------------------------------------------------
static bool process_path(const std::string& raw, std::string& rel, std::string& abs)
{
    // see if path contains forbidden characters
    if ((!raw.empty() && raw[0] == '\\')     || // path starts with a "\"
        (!raw.empty() && raw[0] == '~')      || // path starts with a "~"
        (!raw.empty() && raw[0] == '.')      || // path starts with a "."
        raw.find(':') != std::string::npos || // path contains ":"
        raw.find("..") != std::string::npos)  // path contains ".."
    {
        return false;
    }

    if (raw.empty()) { // path is empty, so simply defaults to the data path
        rel = "/data";
        abs = g_data_path.string();
    } else if (raw[0] == '/') {
        if (raw.find("/data") == 0) { // path begins with "/data", so is relative to the data path
            rel = raw;
            if (raw.size() == strlen("/data")) { // path is equal to "/data"
                abs = g_data_path.string();
            } else {
                abs = (g_data_path / raw.substr(strlen("/data"))).string();
            }
        } else if (raw.find("/common") == 0) { // path begins with "/common", so is relative to the common path
            rel = raw;
            if (raw.size() == strlen("/common")) { // path is equal to "/common"
                abs = g_common_path.string();
            } else {
                abs = (g_common_path / raw.substr(strlen("/common"))).string();
            }
        } else if (raw.find("/engine") == 0) { // path begins with "/engine", so is relative to the engine path
            rel = raw;
            if (raw.size() == strlen("/engine")) { // path is equal to "/engine"
                abs = g_engine_path.string();
            } else {
                abs = (g_engine_path / raw.substr(strlen("/engine"))).string();
            }
        } else {
            return false;
        }
    } else { // path begins not with a forwardslash, so is relative to the data path
        rel = std::string("/data/") + raw;
        abs = (g_data_path / raw).string();
    }

    return true;
}

//-----------------------------------------------------------------
bool ComplementPath(std::string& path)
{
    std::string rel;
    std::string abs;
    if (process_path(path, rel, abs)) {
        path = rel;
        return true;
    }
    return false;
}

//-----------------------------------------------------------------
IFile* OpenFile(const std::string& filename, int mode)
{
    std::string rel;
    std::string abs;
    if (process_path(filename, rel, abs)) {
        RefPtr<File> file = File::Create();
        if (file->open(abs, mode)) {
            file->setName(rel);
            return file.release();
        }
    }
    return 0;
}

//-----------------------------------------------------------------
bool DoesFileExist(const std::string& filename)
{
    std::string rel;
    std::string abs;
    if (process_path(filename, rel, abs)) {
        try {
            return fs::exists(abs);
        } catch (...) { }
    }
    return false;
}

//-----------------------------------------------------------------
bool IsRegularFile(const std::string& filename)
{
    std::string rel;
    std::string abs;
    if (process_path(filename, rel, abs)) {
        try {
            return fs::is_regular_file(abs);
        } catch (...) { }
    }
    return false;
}

//-----------------------------------------------------------------
bool IsDirectory(const std::string& filename)
{
    std::string rel;
    std::string abs;
    if (process_path(filename, rel, abs)) {
        try {
            return fs::is_directory(abs);
        } catch (...) { }
    }
    return false;
}

//-----------------------------------------------------------------
int GetFileModTime(const std::string& filename)
{
    std::string rel;
    std::string abs;
    if (process_path(filename, rel, abs)) {
        try {
            return (int)fs::last_write_time(abs);
        } catch (...) { }
    }
    return -1;
}

//-----------------------------------------------------------------
bool CreateDirectory(const std::string& directory)
{
    std::string rel;
    std::string abs;
    if (process_path(directory, rel, abs)) {
        try {
            return fs::create_directory(abs);
        } catch (...) { }
    }
    return false;
}

//-----------------------------------------------------------------
bool RemoveFile(const std::string& filename)
{
    std::string rel;
    std::string abs;
    if (process_path(filename, rel, abs)) {
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
    std::string relFrom;
    std::string absFrom;
    std::string relTo;
    std::string absTo;
    if (process_path(filenameFrom, relFrom, absFrom) &&
        process_path(filenameTo, relTo, absTo))
    {
        try {
            fs::rename(absFrom, absTo);
            return true;
        } catch (...) { }
    }
    return false;
}

//-----------------------------------------------------------------
bool GetFileList(const std::string& directory, std::vector<std::string>& fileList)
{
    std::string rel;
    std::string abs;
    if (process_path(directory, rel, abs)) {
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
