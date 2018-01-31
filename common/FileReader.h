#pragma once
#ifndef __FILE_UTIL_H__
#define __FILE_UTIL_H__

namespace HciExampleComon{

class FileReader
{
public:
    unsigned char * buff_;
    int       buff_len_;

public:
    FileReader();
    ~FileReader();
    virtual bool Load(const char * pszLibName, int nExtraBytes = 0);
    virtual void Free();
};

}

#endif // __FILE_UTIL_H__
