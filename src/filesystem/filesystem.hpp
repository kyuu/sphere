#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <vector>
#include <string>
#include "../Log.hpp"
#include "../io/IFile.hpp"


bool   ComplementPath(std::string& path);
IFile* OpenFile(const std::string& filename, int mode = IFile::IN);
bool   DoesFileExist(const std::string& filename);
bool   IsRegularFile(const std::string& filename);
bool   IsDirectory(const std::string& filename);
int    GetFileModTime(const std::string& filename);
bool   CreateDirectory(const std::string& directory);
bool   RemoveFile(const std::string& filename);
bool   RenameFile(const std::string& filenameFrom, const std::string& filenameTo);
bool   GetFileList(const std::string& directory, std::vector<std::string>& fileList);

bool InitFilesystem(const Log& log);
void DeinitFilesystem(const Log& log);
const std::string& GetEnginePath();
const std::string& GetCurrentPath();
bool SetCurrentPath(const std::string& p);
const std::string& GetDataPath();
bool SetDataPath(const std::string& p);


#endif
