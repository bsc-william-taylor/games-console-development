
#ifndef __LOG__H_
#define __LOG__H_

#include <string>
#include <sstream>
#include <cstring>
#include <cstdio>

inline char * __fn__(const char *path)
{
    const char *filename = strrchr(path, '/');
    if (filename == NULL)
        filename = path;
    else
        filename++;
    return (char*)filename;
}

inline std::string __format__(char * fmt)
{
    std::stringstream ss;
    ss << "%s(%d) - " << fmt << "\n";
    return ss.str();
}

#define LOG(format, ...) printf(__format__(format).c_str(), __fn__(__FILE__), __LINE__, __VA_ARGS__); 

#endif
