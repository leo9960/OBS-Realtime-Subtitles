#include "FileReader.h"
#include <stdio.h>
#include <malloc.h>
#include <memory.h>

namespace HciExampleComon{

FileReader::FileReader()
{
    buff_ = NULL;
    buff_len_ = 0;
}

FileReader::~FileReader()
{
    Free();
}

bool FileReader::Load(const char * pszLibName, int nExtraBytes)
{
    FILE * fp = fopen(pszLibName, "rb");
    if (fp == NULL)
        return false;

    fseek(fp,0,SEEK_END);
    buff_len_=ftell(fp);
    fseek(fp,0,SEEK_SET);

    if (buff_len_ == 0)
    {
        fclose(fp);
        return false;
    }

    buff_ = (unsigned char*)malloc(buff_len_ + nExtraBytes);
    if (buff_ == NULL)
    {
        fclose(fp);
        return false;
    }

    fread( buff_, 1, buff_len_, fp);
    if (ferror(fp) != 0)
    {
        fclose(fp);
        free(buff_);
        buff_ = NULL;
        return false;
    }

    fclose(fp);

    if (nExtraBytes != 0)
    {
        memset(buff_ + buff_len_, 0, nExtraBytes);
        buff_len_ += nExtraBytes;
    }
    return true;
}

void FileReader::Free()
{
    if (buff_ != NULL)    
    {                    
        free(buff_);
        buff_ = NULL;
    }

    buff_len_ = 0;
}

}