//
//  fileManager.cpp
//  SourceStatistic
//
//  Created by L&L on 2017/8/14.
//  Copyright © 2017年 L&L. All rights reserved.
//

#include "fileManager.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>

FileManager::FileManager()
: _fileID(0),
  _running(true)
{
    _file_type_list = { {".c", FILETYPE::Type_C}, {".cpp", FILETYPE::Type_C},
        {".h", FILETYPE::Type_C}, {".hpp", FILETYPE::Type_C} /*{other types}*/};
}

FileManager::~FileManager()
{
    for (auto it : _file_list)
        if (it.second)
            delete it.second;
}

void FileManager::Run(const char *path)
{
    DfsFolder(path);
    InitWorkThreads();
    PrintResult();
}

void FileManager::InitWorkThreads()
{
    for (int i = 1; i <= thread::hardware_concurrency()/2 ; ++i)
    {
        _working_threads.push_back(thread(&FileManager::WorkingThread, this));
    }

    for_each(_working_threads.begin(), _working_threads.end(), mem_fn(&thread::join));
}

void FileManager::WorkingThread()
{
    while (_running)
    {
        int id = -1;
        
        _queue_mutex.lock();
        if (!_queue.empty())
        {
            id = _queue.front();
            _queue.pop();
        }
        else
        {
            _running = false;
        }
        _queue_mutex.unlock();
        
        if (id>=0)
        {
            fileInfo* info = _file_list[id];
            if (info != nullptr)
            {
                ProcessFile(info);
            }
        }
    } // end while
}

void FileManager::DfsFolder(const char *path)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if( (dp = opendir(path)) == NULL )
    {
        fprintf(stderr,"cannot open directory: %s\n", path);
        return;
    }
    
    chdir(path);
    while((entry = readdir(dp)) != NULL)
    {
        string filename = path;
        filename.append("/");
        filename.append(entry->d_name);
        lstat(entry->d_name, &statbuf);
        if(S_ISDIR(statbuf.st_mode))
        {
            // directory
            if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
                continue;
            DfsFolder(filename.c_str());
        }
        else
        {
            // file
            RecordFile(filename.c_str());
        }
    }
    chdir("..");
    closedir(dp);
}

void FileManager::RecordFile(const char *path)
{
    char* ext = strrchr(path, '.'); //get the extension of file
    if (ext != NULL)
    {
        auto it = _file_type_list.find(ext);
        if ( it != _file_type_list.end()) // check if need to process this file type
        {
            fileInfo* info = new fileInfo();
            _file_list[_fileID] = info;
            info->_id = _fileID;
            info->_file_name = path;
            info->_file_type = it->second;
            _queue.push(_fileID);
            ++_fileID;
        }
    }
}

void FileManager::ProcessFile(fileInfo *info)
{
    switch (info->_file_type)
    {
        case FILETYPE::Type_C:
            ProcessCFile(info);
            break;
        // other types
        default:
            break;
    }
}

void FileManager::ProcessCFile(fileInfo *info)
{
    chdir("./SourceStatistic");
    ifstream in_file(info->_file_name.c_str());
    if (!in_file.is_open())
    {
        fprintf(stderr, "Could not open CFile %s\n", info->_file_name.c_str());
        return;
    }
    char buf[MAX_CHARS] = { 0 };
    unsigned int total = 0;
    unsigned int empty = 0;
    unsigned int effective = 0;
    unsigned int comment = 0;
    bool isMultipleComment = false;
    bool isNewLine = true;
    while (in_file.getline(buf, MAX_CHARS))
    {
        ++total;
        isNewLine = true;
        char* p = buf;
        while (*p == ' ' || *p == '\t') ++p; // skip the leading spaces
        
        if (*p == '\0')
            ++empty;
        else
        {
            while (*p != '\0')
            {
                if (!isMultipleComment)
                {
                    // ingore those content between quotes
                    if (*p == '"')
                    {
                        do {
                            ++p;
                            if (*p == '\0')
                            {
                                ++effective;
                                break;
                            }
                        } while (*p != '"');
                        ++p;
                    }
                    
                    if (*p == '/')
                    {
                        ++p;
                        if (*p == '/')
                        {
                            ++comment;
                            break;
                        }
                        else if (*p == '*')
                        {
                            ++p;
                            ++comment;
                            bool isSingleComment = false;
                            while (*p != '\0')
                            {
                                if (*p == '*')
                                {
                                    if (*(p+1) == '/')
                                    {
                                        ++p;
                                        isSingleComment = true;
                                        break;
                                    }
                                }
                                ++p;
                            }
                            if (!isSingleComment) isMultipleComment = true;
                        }
                    }
                    else
                    {
                        if (isNewLine)
                        {
                            isNewLine = false;
                            ++effective;
                        }
                    }
                }
                else
                {
                    while (*p != '\0')
                    {
                        if (*p == '*')
                        {
                            if (*(p+1) == '/')
                            {
                                isMultipleComment = false;
                                ++comment;
                            }
                        }
                        ++p;
                    }
                    if (isMultipleComment) ++comment;
                }
                ++p;
            }
        }
        memset(buf, 0, MAX_CHARS);
    }
    
    info->_total = total;
    info->_empty = empty;
    info->_effective = effective;
    info->_comment = comment;
    in_file.close();
    chdir("..");
}

void FileManager::PrintResult()
{
    for (auto it : _file_list)
    {
        printf("%s total:%u empty:%u effective:%u comment:%u\n",
               it.second->_file_name.c_str(),
               it.second->_total,
               it.second->_empty,
               it.second->_effective,
               it.second->_comment);
    }
}



