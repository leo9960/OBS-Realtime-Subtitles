#pragma once
#ifndef __COMMON_TOOL_H__
#define __COMMON_TOOL_H__

namespace HciExampleComon{
    //设置控制台打印颜色
    void SetSpecialConsoleTextAttribute();
    //恢复控制台打印颜色
    void SetOriginalConsoleTextAttribute();
    //控制台打印UTF8字符（windows转化为GBK）
    void PrintUtf8String(char *pUTF8Str);

    //转码函数
    int UTF8ToGBK(unsigned char * pUTF8Str,unsigned char ** pGBKStr);
	int GBKToUTF8(unsigned char * pGBKStr,unsigned char ** pUTF8Str);
    //转码内存释放函数
    void FreeConvertResult(unsigned char * pConvertResult);

}

#endif // __FILE_UTIL_H__
