//
//  fileManager.hpp
//  SourceStatistic
//
//  Created by L&L on 2017/8/14.
//  Copyright © 2017年 L&L. All rights reserved.
//

#ifndef fileManager_hpp
#define fileManager_hpp

#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <queue>

#include <thread>
#include <mutex>

using namespace std;

#define MAX_CHARS           1024

enum class FILETYPE
{
    Type_C
    // other types
};

struct fileInfo
{
    string _file_name;
    FILETYPE _file_type;
    unsigned int _id;
    unsigned int _total;
    unsigned int _empty;
    unsigned int _effective;
    unsigned int _comment;
    
    fileInfo()
    {
        _total = 0;
        _empty = 0;
        _effective = 0;
        _comment = 0;
    }
};

class FileManager
{
public:
    FileManager();
    ~FileManager();
    void Run(const char* path);
    
private:
    void InitWorkThreads();
    void WorkingThread();
    
    void DfsFolder(const char* path);
    void RecordFile(const char* path);
    
    void ProcessFile(fileInfo* info);
    void ProcessCFile(fileInfo* info);
    
    void PrintResult();
    
private:
    vector<thread> _working_threads;
    bool _running;
    
    unsigned int _fileID;
    map<unsigned int, fileInfo*> _file_list;
    map<string, FILETYPE> _file_type_list;
    
    queue<unsigned int> _queue;
    mutex _queue_mutex;
};

#endif /* fileManager_hpp */
