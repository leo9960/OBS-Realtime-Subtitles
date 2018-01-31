/** 
 * @file    hci_sys.h 
 * @brief   HCI_SYS 公共头文件  
 */  
#pragma once
#ifndef _hci_sys_header_
#define _hci_sys_header_

#ifdef __ANDROID__
#include <jni.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/** @defgroup HCI_SYS 灵云系统API */
/* @{ */

/* parameter remark definition */
#ifndef _IN_
#define _IN_
#endif
#ifndef _OUT_
#define _OUT_
#endif
#ifndef _IN_OUT_
#define _IN_OUT_
#endif
#ifndef _MUST_
#define _MUST_
#endif
#ifndef _OPT_
#define _OPT_
#endif

#ifdef _WIN32
#define HCIAPI _stdcall
#else
#define HCIAPI
#endif

/** @defgroup HCI_SYS_STRUCT  结构体 */
/* @{ */

/**
*@brief	定义64位int
*/
#if !defined _MSC_VER && !defined __BORLANDC__
#  include <stdint.h>
#endif

#if defined _MSC_VER || defined __BORLANDC__
typedef __int64 int64;
#else
typedef int64_t int64;
#endif

/**
*@brief	模块名称
*/
#define SYS_MODULE    "HCI_SYS"

/**
*HCI_ERR_CODE  返回错误码定义
*/
typedef enum {
	HCI_ERR_UNKNOWN = -1,					///< -1: 未知错误，通常不会出现
	HCI_ERR_NONE = 0,						///< 0: 正确
	HCI_ERR_PARAM_INVALID,					///< 1: 函数的传入参数错误
	HCI_ERR_OUT_OF_MEMORY,					///< 2: 申请内存失败
	HCI_ERR_CONFIG_INVALID,					///< 3: 配置串参数错误
	HCI_ERR_CONFIG_CAPKEY_MISSING,			///< 4: 缺少必需的capKey配置项
	HCI_ERR_CONFIG_CAPKEY_NOT_MATCH,		///< 5: CAPKEY与当前引擎不匹配
	HCI_ERR_CONFIG_DATAPATH_MISSING,		///< 6: 缺少必需的dataPath配置项
	HCI_ERR_CONFIG_UNSUPPORT,				///< 7: 配置项不支持
	HCI_ERR_SERVICE_CONNECT_FAILED,			///< 8: 连接服务器失败，服务器无响应
	HCI_ERR_SERVICE_TIMEOUT,				///< 9: 服务器访问超时
	HCI_ERR_SERVICE_DATA_INVALID,			///< 10: 服务器返回的数据格式不正确
	HCI_ERR_SERVICE_RESPONSE_FAILED,		///< 11: 服务器返回操作失败
	HCI_ERR_CAPKEY_NOT_FOUND,				///< 12: 没有找到指定的能力
	HCI_ERR_NOT_LOCAL_CAPKEY,				///< 13: 不是本地能力的KEY
	HCI_ERR_LOCAL_LIB_MISSING,				///< 14: 本地能力引擎缺失必要的库资源
	HCI_ERR_URL_MISSING,					///< 15: 找不到对应的网络服务地址（可能是HCI能力服务地址，下载资源库地址等）
	HCI_ERR_SESSION_INVALID,				///< 16: 无效的会话
	HCI_ERR_TOO_MANY_SESSION,				///< 17: 开启会话过多(目前每种HCI能力的最大会话数为256)
	HCI_ERR_ACTIVE_SESSION_EXIST,			///< 18: 还有会话没有停止
	HCI_ERR_START_LOG_FAILED,				///< 19: 启动日志错误, 可能是日志配置参数错误，路径不存在或者没有写权限等造成
	HCI_ERR_DATA_SIZE_TOO_LARGE,			///< 20: 传入的数据量超过可处理的上限
	HCI_ERR_LOAD_CODEC_DLL,					///< 21: 加载codec编码库失败
	HCI_ERR_UNSUPPORT,						///< 22: 暂不支持
	HCI_ERR_LOAD_FUNCTION_FROM_DLL,			///< 23: 加载库失败
    HCI_ERR_TXACCOUNT_NOT_FOUND,            ///< 24: 天行账号获取失败

	//HCI_SYS 
	HCI_ERR_SYS_NOT_INIT = 100,				///< 100: HCI_SYS未初始化
	HCI_ERR_SYS_ALREADY_INIT,				///< 101: HCI_SYS多次初始化
	HCI_ERR_SYS_CONFIG_AUTHPATH_MISSING,	///< 102: 缺少必需的authPath配置项
	HCI_ERR_SYS_CONFIG_CLOUDURL_MISSING,	///< 103: 缺少必需的cloudUrl配置项
	HCI_ERR_SYS_CONFIG_USERID_MISSING,		///< 104: 缺少必需的userId配置项
	HCI_ERR_SYS_CONFIG_PASSWORD_MISSING,	///< 105: 缺少必需的password配置项
	HCI_ERR_SYS_CONFIG_PLATFORMID_MISSING,	///< 106: 缺少必需的platformId配置项
	HCI_ERR_SYS_CONFIG_DEVELOPERID_MISSING,	///< 107: 缺少必需的developerId配置项
	HCI_ERR_SYS_CONFIG_DEVELOPERKEY_MISSING,///< 108: 缺少必需的developerKey配置项
	HCI_ERR_SYS_CONFIG_APPNO_MISSING,		///< 109: 缺少必需的appNo配置项
	HCI_ERR_SYS_USERINFO_INVALID,			///< 110: 读写用户信息文件错误
	HCI_ERR_SYS_AUTHFILE_INVALID,			///< 111: 读取授权文件错误
	HCI_ERR_SYS_CHECKAUTH_RESPONSE_FAILED,	///< 112: 服务器返回获取云端授权失败
	HCI_ERR_SYS_REGISTER_RESPONSE_FAILED,	///< 113: 服务器返回注册用户失败（已废弃，废弃版本3.0）
	HCI_ERR_SYS_USING,						///< 114: 仍然有能力在使用（尚未反初始化）
	HCI_ERR_SYS_CONFIG_APPKEY_MISSING,      ///< 115: 缺少必需的appkey配置项

	/* HCI_ASR */
	HCI_ERR_ASR_NOT_INIT = 200,				///< 200: HCI_ASR没有初始化
	HCI_ERR_ASR_ALREADY_INIT,				///< 201: HCI_ASR多次初始化
	HCI_ERR_ASR_CONFIRM_NO_TASK,			///< 202: 使用confirm，但没有确认任务
	HCI_ERR_ASR_PARAM_CHECK_FLAG_INVALID,	///< 203: 错误的CheckFlag项，例如已经检测到端点仍然再发送数据，或尚未开启端点检测就发送flag为（CHECK_FLAG_END，CHECK_FLAG_PROGRESS）
	HCI_ERR_ASR_GRAMMAR_DATA_TOO_LARGE,		///< 204: 语法数据太大
	HCI_ERR_ASR_ENGINE_NOT_INIT,			///< 205: ASR本地引擎尚未初始化(已废弃，废弃版本3.8)
	HCI_ERR_ASR_ENGINE_INIT_FAILED,			///< 206: ASR本地引擎初始化失败
	HCI_ERR_ASR_OPEN_GRAMMAR_FILE,			///< 207: 读取语法文件失败
	HCI_ERR_ASR_LOAD_GRAMMAR_FAILED,		///< 208: 加载语法文件失败
	HCI_ERR_ASR_ENGINE_FAILED,				///< 209: ASR本地引擎识别失败
	HCI_ERR_ASR_GRAMMAR_ID_INVALID,			///< 210: 语法ID无效
	HCI_ERR_ASR_REALTIME_WAITING,			///< 211: 实时识别时未检测到音频末端，继续等待数据
	HCI_ERR_ASR_GRAMMAR_OVERLOAD,			///< 212: 加载语法数量已达上限
	HCI_ERR_ASR_GRAMMAR_USING,				///< 213: 该语法正在使用中
	HCI_ERR_ASR_REALTIME_END,				///< 214: 实时识别时检测到末端，或者缓冲区满，需要使用NULL获取结果
	HCI_ERR_ASR_UPLOAD_NO_DATA,				///< 215: 上传本地数据时，无用于上传的数据 
	HCI_ERR_ASR_REALTIME_NO_VOICE_INPUT,	///< 216: 实时识别时未检测语音
	HCI_ERR_ASR_VOICE_DATA_TOO_LARGE,		///< 217: 音频片段太长，应在(0,32K)
	
	/* HCI_HWR */
	HCI_ERR_HWR_NOT_INIT = 300,				///< 300: HCI_HWR没有初始化
	HCI_ERR_HWR_ALREADY_INIT,				///< 301: HCI_HWR多次初始化
	HCI_ERR_HWR_CONFIRM_NO_TASK,			///< 302: 使用confirm，但没有确认任务
	HCI_ERR_HWR_ENGINE_INIT_FAILED,			///< 303: HWR本地引擎初始化失败
	HCI_ERR_HWR_ENGINE_FAILED,				///< 304: HWR本地引擎操作（识别、获取拼音、获取联想结果）失败
	HCI_ERR_HWR_UPLOAD_NO_DATA,				///< 305: 没有可用于上传的数据
	HCI_ERR_HWR_ENGINE_SESSION_START_FAILED,///< 306: HWR本地引擎开启会话失败
	HCI_ERR_HWR_ENGINE_NOT_INIT,			///< 307: SDK初始化时未传入本地能力却在创建会话时使用了本地能力(已废弃，废弃版本3.8)
	HCI_ERR_HWR_CONFIG_SUBLANG_MISSING,		///< 308: 单字能力、多语种字典时，未传入sublang
	HCI_ERR_HWR_TOO_MANY_DOMAIN,			///< 309: 传入了领域数目超过了4个

	/* HCI_OCR */
	HCI_ERR_OCR_NOT_INIT = 400,				///< 400: HCI_OCR没有初始化
	HCI_ERR_OCR_ALREADY_INIT,				///< 401: HCI_OCR多次初始化
	HCI_ERR_OCR_ENGINE_INIT_FAILED,			///< 402: OCR本地引擎初始化失败
	HCI_ERR_OCR_ENGINE_FAILED,				///< 403: OCR本地引擎操作（倾斜校正、版面分析、识别、预处理、压缩）失败
	HCI_ERR_OCR_ENGINE_NOT_INIT,			///< 404: SDK初始化时未传入本地能力却在创建会话时使用了本地能力(已废弃，废弃版本3.8)
	HCI_ERR_OCR_LOAD_IMAGE,					///< 405: 载入本地文件或者本地图片缓冲失败
	HCI_ERR_OCR_SAVE_IMAGE,					///< 406: 保存OCR_IMAGE到本地文件失败
	HCI_ERR_OCR_IMAGE_NOT_SET,				///< 407: 未设置要处理的图片就进行了倾斜校正、版面分析、识别等操作
	HCI_ERR_OCR_LOAD_TEMPLATE_FAILED,		///< 408: 加载模板文件失败
	HCI_ERR_OCR_TEMPLATE_OVERLOAD,		    ///< 409: 加载模板数量已达上限（1024）
	HCI_ERR_OCR_TEMPLATE_ID_INVALID,		///< 410: 传入的模板ID不在加载的模板列表中
	HCI_ERR_OCR_TEMPLATE_USING,			    ///< 411: 还有会话在使用要卸载的模板
	HCI_ERR_OCR_DETECT_CORNER_FAILED,		///< 412: OCR获取切边信息失败
	HCI_ERR_OCR_NORMALIZE_FAILED,			///< 413: OCR切边失败
	HCI_ERR_OCR_RECOGNIZABLE_FAILED,		///< 414: OCR识别判断返回失败
	HCI_ERR_OCR_IMAGE_NOT_CLEAR,			///< 415: OCR识别图片不清晰
	HCI_ERR_OCR_CONFIG_TEMPLATE_ID_MISSING,	///< 416: OCR本地模板 templateid参数缺失
	HCI_ERR_OCR_CONFIG_TEMPLATE_INDEX_MISSING,	///< 417: OCR模板 templateindex参数缺失
	HCI_ERR_OCR_CONFIG_TEMPLATE_PAGE_INDEX_MISSING,	///< 418: OCR模板 templatepageindex参数缺失
	HCI_ERR_OCR_CONFIG_PROPERTY_MISSING,	///< 419: OCR云端模板 property参数缺失
	HCI_ERR_OCR_EDGE_IS_NOT_EXIST,			///< 420: OCR识别图片边缘不存在

	/* HCI_TTS */
	HCI_ERR_TTS_NOT_INIT = 500,				///< 500: HCI_TTS没有初始化
	HCI_ERR_TTS_ALREADY_INIT,				///< 501: HCI_TTS多次初始化
	HCI_ERR_TTS_SESSION_BUSY,				///< 502: TTS会话正忙，例如在合成回调函数中又调用了合成
	HCI_ERR_TTS_ENGINE_SESSION_START_FAILED,///< 503: TTS本地引擎开启会话失败
	HCI_ERR_TTS_ENGINE_FAILED,				///< 504: TTS本地引擎合成失败
    HCI_ERR_TTS_ENGINE_INIT_FAILED,	        ///< 505: TTS(NU)本地引擎初始化失败
    HCI_ERR_TTS_ENGINE_NOT_INIT,			///< 506: TTS(NU)本地引擎尚未初始化
	HCI_ERR_TTS_CONFIG_PROPERTY_MISSING,	///< 507: TTS 云端property参数缺失

	/* HCI_MT */
	HCI_ERR_MT_NOT_INIT = 600,				///< 600: HCI_MT没有初始化
	HCI_ERR_MT_ALREADY_INIT,				///< 601: HCI_MT多次初始化

	/*HCI_NLU*/
	HCI_ERR_NLU_NOT_INIT = 700,				///< 700: HCI_NLU没有初始化
	HCI_ERR_NLU_ALREADY_INIT,				///< 701: HCI_NLU多次初始化
	HCI_ERR_NLU_ENGINE_SESSION_START_FAILED,///< 702: NLU本地引擎开启会话失败
	HCI_ERR_NLU_ENGINE_FAILED,				///< 703: NLU本地引擎识别失败
	
	/*HCI_KB*/
	HCI_ERR_KB_NOT_INIT = 800,				///< 800: HCI_KB没有初始化
	HCI_ERR_KB_ALREADY_INIT,				///< 801: HCI_KB多次初始化
	HCI_ERR_KB_ENGINE_SESSION_START_FAILED,	///< 802: KB本地引擎开启会话失败
	HCI_ERR_KB_ENGINE_FAILED,				///< 803: KB本地引擎识别失败
	HCI_ERR_KB_SYLLABLE_INVALID,			///< 804: 容错音节无法判断类型
	HCI_ERR_KB_UDB_WORD_EXIST,				///< 805: 已经在用户词库中存在
	HCI_ERR_KB_CONFIRM_NO_TASK,				///< 806: 使用confirm，但没有确认任务

	/*HCI_VPR*/
	HCI_ERR_VPR_NOT_INIT = 900,				///< 900: HCI_VPR没有初始化
	HCI_ERR_VPR_ALREADY_INIT,				///< 901: HCI_VPR多次初始化
	HCI_ERR_VPR_ENGINE_INIT_FAILED,			///< 902: VPR本地引擎初始化失败
	HCI_ERR_VPR_ENGINE_FAILED,				///< 903: VPR本地引擎处理失败
	HCI_ERR_VPR_USERID_NOT_EXIST,			///< 904: VPR用户不存在
	HCI_ERR_VPR_ENGINE_SESSION_START_FAILED,///< 905: VPR本地引擎开启会话失败
	HCI_ERR_VPR_CONFIG_USER_ID_MISSING,		///< 906: VPR缺少userId配置项
    HCI_ERR_VPR_CONFIG_GROUP_ID_MISSING,	///< 907: VPR缺少groupId配置项
	HCI_ERR_VPR_REALTIME_WAITING,           ///< 908: 实时识别检测到末端，等待
	HCI_ERR_VPR_VOICE_SHORT,			    ///< 909: 语音太短
	HCI_ERR_VPR_VOLUMN_HIGH,			    ///< 910: 音量太高
	HCI_ERR_VPR_VOLUMN_LOW,				    ///< 911: 音量太低
	HCI_ERR_VPR_NOISE_HIGH,				    ///< 912: 噪音太大
	HCI_ERR_VPR_VAD_DETECT_RESULT_IS_EMPTY, ///< 913: 端点检测结果为空


	/*HCI_FPR*/
	HCI_ERR_FPR_NOT_INIT = 1000,			///< 1000: HCI_FPR没有初始化
	HCI_ERR_FPR_ALREADY_INIT,				///< 1001: HCI_FPR多次初始化
	HCI_ERR_FPR_ENGINE_INIT_FAILED,			///< 1002: FPR本地引擎初始化失败
	HCI_ERR_FPR_ENGINE_FAILED,				///< 1003: FPR本地引擎处理失败
	HCI_ERR_FPR_USERID_NOT_EXIST,			///< 1004: FPR用户不存在
	HCI_ERR_FPR_ENGINE_SESSION_START_FAILED,///< 1005: FPR本地引擎开启会话失败
	HCI_ERR_FPR_CONFIG_USER_ID_MISSING,		///< 1006: FPR缺少userId配置项
	HCI_ERR_FPR_CONFIG_DPI_MISSING,			///< 1007: FPR缺少dpi配置项
	HCI_ERR_FPR_CONFIG_BGCOLOR_MISSING,		///< 1008: FPR缺少bgColor配置项
	HCI_ERR_FPR_CONFIG_WIDTH_MISSING,		///< 1009: FPR缺少width配置项
	HCI_ERR_FPR_CONFIG_HEIGHT_MISSING,		///< 1010: FPR缺少height配置项
    HCI_ERR_FPR_CONFIG_GROUP_ID_MISSING,	///< 1011: FPR缺少groupId配置项

	/*HCI_AFR*/
	HCI_ERR_AFR_NOT_INIT = 1100,				///< 1100: HCI_AFR没有初始化
	HCI_ERR_AFR_ALREADY_INIT,					///< 1101: HCI_AFR多次初始化
	HCI_ERR_AFR_ENGINE_INIT_FAILED,				///< 1102: AFR本地引擎初始化失败
	HCI_ERR_AFR_ENGINE_FAILED,					///< 1103: AFR本地引擎处理失败
	HCI_ERR_AFR_USERID_NOT_EXIST,				///< 1104: AFR用户不存在
	HCI_ERR_AFR_ENGINE_SESSION_START_FAILED,	///< 1105: AFR本地引擎开启会话失败
	HCI_ERR_AFR_CONFIG_USER_ID_MISSING,			///< 1106: AFR缺少userId配置项
	HCI_ERR_AFR_CONFIG_FACE_ID_MISSING,			///< 1107: AFR缺少faceid配置项
	HCI_ERR_AFR_CONFIG_GROUP_ID_MISSING,		///< 1108: AFR缺少groupId配置项
	HCI_ERR_AFR_ENGINE_SESSION_IMAGE_MISSING,	///< 1109: AFR探测未设置图像信息
	HCI_ERR_AFR_LOAD_IMAGE,						///< 1110: AFR无法打开文件
	HCI_ERR_AFR_IMAGE_INVALID,					///< 1111: AFR输入图像非法
	HCI_ERR_AFR_MODEL_INVALID,					///< 1112: AFR输入的人脸模型非法
	HCI_ERR_AFR_CONFIG_INVALID,					///< 1113: AFR输入的配置串不合法
	HCI_ERR_AFR_PARAM_KEY,						///< 1114: AFR不正确的参数Key
	HCI_ERR_AFR_SAVE_MODEL,						///< 1115: 保存用户模型错误
	HCI_ERR_AFR_ENGINE,							///< 1116: 引擎错误, 一般不会发生
	HCI_ERR_AFR_FACEID_NOT_EXIST,				///< 1117: AFR人脸不存在
	HCI_ERR_AFR_MODEL_NOT_EXIST					///< 1118: AFR人脸模型不存在
	
} HCI_ERR_CODE;


/**
 * @brief	能力项条目
 */
typedef struct _tag_CAPABILITY_ITEM
{
	/// 能力的唯一标识
	char* pszCapKey;
} CAPABILITY_ITEM;

/**
 * @brief	能力列表
 */
typedef struct _tag_CAPABILITY_LIST 
{
	/// 能力条目列表
	CAPABILITY_ITEM *	pItemList;

	/// 能力个数
	unsigned int	uiItemCount;
} CAPABILITY_LIST;

/* @} */
/** @defgroup HCI_SYS_FUNC 函数 */
/* @{ */

/**
 * @brief	灵云系统SDK 公共初始化 
 * @param	pszConfig	初始化配置串，ASCII字符串，以'\0'结束
 * @return	
 * @n
 *	<table>
 *		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref HCI_ERR_SYS_ALREADY_INIT</td><td>HCI SYS 已经初始化过了</td></tr>
 *		<tr><td>@ref HCI_ERR_PARAM_INVALID</td><td>输入参数不合法,如参数pszConfig传入NULL或字符串传入""</td></tr>
 *		<tr><td>@ref HCI_ERR_CONFIG_INVALID</td><td>配置参数有误，如设定值非法或格式错误等</td></tr>
 *		<tr><td>@ref HCI_ERR_START_LOG_FAILED</td><td>启动日志错误, 一般为日志路径不存在或者没有写权限等造成</td></tr>
 *		<tr><td>@ref HCI_ERR_SYS_CONFIG_AUTHPATH_MISSING</td><td>缺少必需的authPath配置项</td></tr>
 *		<tr><td>@ref HCI_ERR_SYS_CONFIG_CLOUDURL_MISSING</td><td>缺少必需的cloudUrl配置项</td></tr>
 *		<tr><td>@ref HCI_ERR_SYS_CONFIG_DEVELOPERKEY_MISSING</td><td>缺少必需的developerKey配置项</td></tr>
 *		<tr><td>@ref HCI_ERR_SYS_CONFIG_APPKEY_MISSING</td><td>缺少必需的appKey配置项</td></tr>
 *	</table>
 *
 * @par 配置串定义：
 * 配置串是由"字段=值"的形式给出的一个字符串，多个字段之间以','隔开。字段名不分大小写。
 * @n@n
 * 灵云账号配置（必选）
 *	<table>
 *		<tr>
 *			<td><b>字段</b></td>
 *			<td><b>取值或示例</b></td>
 *			<td><b>缺省值</b></td>
 *			<td><b>含义</b></td>
 *			<td><b>详细说明</b></td>
 *		</tr>
 *		<tr>
 *			<td>developerKey</td>
 *			<td>字符串</td>
 *			<td>无</td>
 *			<td>开发者密钥</td>
 *			<td>此项必填，由捷通华声提供。</td>
 *		</tr>
 *		<tr>
 *			<td>appKey</td>
 *			<td>字符串</td>
 *			<td>无</td>
 *			<td>应用秘钥</td>
 *			<td>此项必填，由捷通华声提供。</td>
 *		</tr>
 *		<tr>
 *			<td>cloudUrl</td>
 *			<td>字符串，url地址，如：http://test.api.hcicloud.com:8888</td>
 *			<td>无</td>
 *			<td>灵云服务地址</td>
 *			<td>此项必填,由捷通华声提供。</td>
 *		</tr>
 *		<tr>
 *			<td>authPath</td>
 *			<td>字符串，如：./auth/</td>
 *			<td>无</td>
 *			<td>授权文件所在路径</td>
 *			<td>此项必填，此目录下将会存放<b>授权文件</b>。<br/>
 *				SDK还需要在此目录下写一些数据文件，因此此目录必须有可读写权限, 
 *              但强烈建议不是在一个可移除的介质上，例如SD卡上，防止SD卡被拔出。<br/>
 *              Android系统下建议使用/data/data/包名/file 路径
 *			</td>
 *		</tr>
 *		<tr>
 *			<td>appChannel</td>
 *			<td>正整数，参考@subpage app_channel</td>
 *			<td>1</td>
 *			<td>应用渠道</td>
 *			<td>可选配置，开发者通过指定不同的值来编译不同的版本，这样在灵云开发者社区可以获取不同渠道下载安装使用的一个统计。</td>
 *		</tr>
 *		<tr>
 *			<td>udidType</td>
 *			<td>
 *				字符串，根据平台不同，可选取值为
 *				windows：{"unknown","macadress","uuid"}
 *				linux：{"unknown","macadress","uuid"}
 *				ios：{"unknown","macadress","uuid","openudid"}
 *				android：{"unknown","androidid","imei","serial","wifiid","randomnumber"}
 *			</td>
 *			<td>unknown</td>
 *			<td>获取udid类型</td>
 *			<td>
 *				可选配置。当为unknown时，各平台默认授权方式为
 *				windows："macadress"
 *				linux："macadress"
 *				ios："macadress"
 *				android："androidid"
 *			</td>
 *		</tr>
 *	</table>
 * @n@n
 * 日志配置（可选）
 *	<table>
 *		<tr>
 *			<td><b>字段</b></td>
 *			<td><b>取值或示例</b></td>
 *			<td><b>缺省值</b></td>
 *			<td><b>含义</b></td>
 *			<td><b>详细说明</b></td>
 *		</tr>
 *		<tr>
 *			<td>logFilePath</td>
 *			<td>字符串，如：./log/</td>
 *			<td>无</td>
 *			<td>日志文件存放路径</td>
 *			<td>可选，如果不传或者为空则不生成日志，日志名称为hci.log</td>
 *		</tr>
 *		<tr>
 *			<td>logFileSize</td>
 *			<td>正整数，范围[1,1048576]</td>
 *			<td>32</td>
 *			<td>日志大小</td>
 *			<td>默认一个日志文件写多大，单位为K</td>
 *		</tr>
 *		<tr>
 *			<td>logFileCount</td>
 *			<td>整数，范围[0,1024]</td>
 *			<td>5</td>
 *			<td>日志数目</td>
 *			<td>默认保留多少个备份的日志文件，超过则覆盖最旧的日志。为0表示当前日志存满后不做任何备份</td>
 *		</tr>
 * 		<tr>
 *			<td>logLevel</td>
 *			<td>整数，[0，5]</td>
 *			<td>1</td>
 *			<td>日志等级</td>
 *			<td>SDK将输出小于等于logLevel的日志信息,日志级别为：<br/>
 *              0=无<br/>
 *              1=错误<br/>
 *              2=警告<br/>
 *              3=信息<br/>
 *              4=细节<br/>
 *              5=调试</td>
 *		</tr>
 *	</table>
 * HTTP代理配置（可选）
 *	<table>
 *		<tr>
 *			<td><b>字段</b></td>
 *			<td><b>取值或示例</b></td>
 *			<td><b>缺省值</b></td>
 *			<td><b>含义</b></td>
 *			<td><b>详细说明</b></td>
 *		</tr>
 * 		<tr>
 *			<td>netProxy</td>
 *			<td>字符串，一般为IP地址，如：192.168.10.2</td>
 *			<td>无</td>
 *			<td>HTTP代理服务器IP地址</td>
 *			<td>可选</td>
 *		</tr>
 * 		<tr>
 *			<td>netProxyPort</td>
 *			<td>正整数，有效的端口号，如：1080</td>
 *			<td>1080</td>
 *			<td>HTTP代理服务器端口</td>
 *			<td>可选,如果netPorxy未设置则该项也不起作用</td>
 *		</tr>
 * 		<tr>
 *			<td>netProxyUser</td>
 *			<td>字符串，如：user</td>
 *			<td>无</td>
 *			<td>HTTP代理服务器访问用户名</td>
 *			<td>可选,如果netPorxy未设置则该项也不起作用</td>
 *		</tr>
 * 		<tr>
 *			<td>netProxyPwd</td>
 *			<td>字符串，如：pwd</td>
 *			<td>无</td>
 *			<td>HTTP代理服务器访问密码</td>
 *			<td>可选,如果netPorxy未设置则该项也不起作用</td>
 *		</tr>
 *	</table>
 * @note
 * @n@n			
 * <b>应用信息</b>
 * @n@n
 * 配置项中developerKey, appKey两项内容由捷通华声提供，用于标识不同的开发者和应用程序，给予不同的授权和服务。<br/>
 * 如果不提供或者提供内容和捷通华声给出的不一致，会导致服务失败。
 * @n@n
 * <b>授权</b>
 * @n@n
 * 授权是用来确定应用可以使用的云端或本地的HCI能力，即CapKey列表，具有过期时间，过期时间可以为永久可用。<br/>
 * 授权的能力列表可以通过接口\ref hci_get_capability_list 获取，过期时间可以通过接口\ref hci_get_auth_expire_time 获取。
 * @n@n
 * <b>CapKey</b>
 * @n@n
 * 灵云SDK用来标识具体能力，如asr.cloud.freetalk为云端自由说语音识别，应用只能使用包含在授权文件中的CapKey。
 * @n@n
 */ 
#ifdef __ANDROID__
HCI_ERR_CODE HCIAPI hci_init( 
		_MUST_ _IN_ const char * pszConfig ,
		_MUST_ _IN_ JavaVM *vm, 
		_MUST_ _IN_ jobject context);
#else
HCI_ERR_CODE HCIAPI hci_init(
		_MUST_ _IN_ const char * pszConfig
		);
#endif

/**  
 * @brief	手动访问云授权
 * @details	当正常返回时，可以通过 hci_get_auth_expire_time()得到新授权的过期时间，
 *			通过 hci_get_capability() 得到新授权的可使用的HCI能力。
 * @return
 * @n
 *	<table>
 *		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref HCI_ERR_SYS_NOT_INIT</td><td>HCI SYS 尚未初始化</td></tr>
 *		<tr><td>@ref HCI_ERR_SERVICE_CONNECT_FAILED</td><td>连接服务器失败，需要检查设备网络</td></tr>
 *		<tr><td>@ref HCI_ERR_SERVICE_TIMEOUT</td><td>服务器访问超时</td></tr>
 *		<tr><td>@ref HCI_ERR_SERVICE_DATA_INVALID</td><td>服务器返回的数据格式不正确，服务异常请联系捷通华声</td></tr>
 *		<tr><td>@ref HCI_ERR_SYS_CHECKAUTH_RESPONSE_FAILED</td><td>获取云端授权失败</td></tr>
 *	</table>
 */
HCI_ERR_CODE HCIAPI hci_check_auth();

/**  
 * @brief	获得授权过期时间
 * @details	得到的时间和 time() 返回概念一致，指 1970-01-01 00:00:00 UTC 之后的秒数。
 * @param	pnExpireTime		结果缓冲区指针，存放过期时间
 * @return
 * @n
 *	<table>
 *		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref HCI_ERR_SYS_NOT_INIT</td><td>HCI SYS 尚未初始化</td></tr>
 *		<tr><td>@ref HCI_ERR_PARAM_INVALID</td><td>输入参数不合法</td></tr>
 *		<tr><td>@ref HCI_ERR_SYS_AUTHFILE_INVALID</td><td>读取授权文件错误或本地无授权</td></tr>
 *	</table>
 */
HCI_ERR_CODE HCIAPI hci_get_auth_expire_time(
		_MUST_ _OUT_ int64 * pnExpireTime
		);


/**
 * @brief	获取某种HCI类型可用的能力列表
 * @param	pszHciType				所查询的HCI能力（例如 hwr, tts, ocr, asr)，多种能力用';'隔开，
 *									如果为NULL或者空串，表示全部查询
 * @param	psCapabilityList		结果缓冲区指针，存放返回的能力列表。使用完毕后，应该使用 hci_free_capability_list() 释放
 * @return	
 * @n
 *	<table>
 *		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref HCI_ERR_SYS_NOT_INIT</td><td>HCI SYS 尚未初始化</td></tr>
 *		<tr><td>@ref HCI_ERR_PARAM_INVALID</td><td>输入参数不合法</td></tr>
 *	</table>
 */ 
HCI_ERR_CODE HCIAPI hci_get_capability_list(
	    _OPT_ _IN_ const char * pszHciType,
		_MUST_ _OUT_ CAPABILITY_LIST	* psCapabilityList
		);

/**
 * @brief	释放能力列表的内存
 * @param	psCapabilityList		需要释放的能力列表内存
 * @return	
 * @n
 *	<table>
 *		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref HCI_ERR_PARAM_INVALID</td><td>输入参数不合法</td></tr>
 *	</table>
 */ 
HCI_ERR_CODE HCIAPI hci_free_capability_list(
		_MUST_ _IN_ CAPABILITY_LIST	* psCapabilityList
		);


/**  
* @brief	上传本地用户录入数据
* @return	错误码
* @return	
* @n
*	<table>
*		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
*		<tr><td>@ref HCI_ERR_SYS_NOT_INIT</td><td>HCI SYS尚未初始化</td></tr>
*		<tr><td>@ref HCI_ERR_URL_MISSING</td><td>未找到有效的云服务地址</td></tr>
*		<tr><td>@ref HCI_ERR_SERVICE_CONNECT_FAILED</td><td>连接服务器失败，需要检查设备网络</td></tr>
*		<tr><td>@ref HCI_ERR_SERVICE_TIMEOUT</td><td>服务器访问超时</td></tr>
*		<tr><td>@ref HCI_ERR_SERVICE_DATA_INVALID</td><td>服务器返回的数据格式不正确，服务异常请联系捷通华声</td></tr>
*		<tr><td>@ref HCI_ERR_SERVICE_RESPONSE_FAILED</td><td>服务器返回上传失败</td></tr>
*	</table>
*/ 
HCI_ERR_CODE HCIAPI hci_upload_user_history();

/**  
 * @brief	灵云系统SDK 反初始化
 * @return	错误码
 * @return	
 * @n
 *	<table>
 *		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref HCI_ERR_SYS_NOT_INIT</td><td>HCI SYS 尚未初始化</td></tr>
  *		<tr><td>@ref HCI_ERR_SYS_USING</td><td>仍然有能力在使用（尚未反初始化）</td></tr>
 *	</table>
 */ 
HCI_ERR_CODE HCIAPI hci_release();


/**  
 * @brief	获取错误码对应的详细错误信息
 * @return	错误信息
 * @note
 * 错误信息是utf8编码的英文字符串,如果传入的错误码没有在灵云sdk中定义
 * 会返回“Unkown Error”
 */ 
const char * HCIAPI hci_get_error_info(HCI_ERR_CODE nErrorCode);

/**  
 * @brief	获取sdk版本信息
 * @return	sdk版本号
 * 
 */ 
const char * HCIAPI hci_get_sdk_version();

/* @} */
/* @} */
//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
};
#endif

#endif
