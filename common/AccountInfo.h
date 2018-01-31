#pragma once
#ifndef __ACCOUNT_INFO_H__
#define __ACCOUNT_INFO_H__

#include <string>
using std::string;

class AccountInfo
{
private:
    AccountInfo();
    ~AccountInfo();
    static AccountInfo *instance_;
public:
    //获取实例
    static AccountInfo *GetInstance();
	static void ReleaseInstance();

    bool LoadFromFile(const string &account_info_file);
    bool LoadFromCode();

    string app_key();
    string developer_key();
    string cloud_url();
    string auth_path();
    string logfile_path();
    string cap_key();
    string data_path();
    string test_data_path();
protected:
    bool IsValid();
private:
    string app_key_;        //灵云应用序号
    string developer_key_;  //灵云开发者密钥
    string cloud_url_;      //灵云云服务的接口地址
    string auth_path_;      //授权文件所在路径，在构造函数中赋值，可修改
    string logfile_path_;   //日志的路径，在构造函数中赋值，可修改
    string cap_key_;        //能力capkey
    string data_path_;      //本地资源路径
    string test_data_path_; //测试资源路径
};

#endif //__ACCOUNT_INFO_H__
