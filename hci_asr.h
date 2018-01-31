/** 
* @file    hci_asr.h 
* @brief   HCI_ASR SDK 头文件  
*/  
#pragma once
#ifndef __HCI_ASR_HEADER__
#define __HCI_ASR_HEADER__

#include "hci_sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** @defgroup HCI_ASR 灵云ASR能力API */
    /* @{ */
    //////////////////////////////////////////////////////////////////////////

    /** @defgroup HCI_ASR_STRUCT  结构体 */
    /* @{ */
    //////////////////////////////////////////////////////////////////////////
    
    /**
	*@brief	模块名称
	*/
    #define ASR_MODULE    "HCI_ASR"
    
    /**
	*@brief	上传的ASR确认结果信息
	*/
	typedef struct _tag_ASR_CONFIRM_ITEM 
	{
		/// 确认的识别结果内容
		char * pszResult;		
	} ASR_CONFIRM_ITEM;

    /**
	*@brief	ASR识别候选结果条目
	*/
	typedef struct _tag_ASR_RECOG_RESULT_ITEM 
	{
		/// 候选结果分值, 分值越高，越可信
		unsigned int		uiScore;

		/// 候选结果字符串，UTF-8编码，以'\0'结束
		char *				pszResult;
	} ASR_RECOG_RESULT_ITEM;

	/**
	*@brief	ASR识别函数的返回结果
	*/
	typedef struct _tag_ASR_RECOG_RESULT 
	{
		/// 识别候选结果列表
		ASR_RECOG_RESULT_ITEM *	psResultItemList;

		/// 识别候选结果的数目
		unsigned int		uiResultItemCount;
	} ASR_RECOG_RESULT;

    /* @} */


    /** @defgroup HCI_ASR_FUNC  函数 */
    /* @{ */
    //////////////////////////////////////////////////////////////////////////    

	/**  
	* @brief	灵云ASR能力  初始化
	* @param	pszConfig	初始化配置串，ASCII字符串，可为NULL或以'\0'结束
	* @return
	* @n
	*	<table>
	*		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
	*		<tr><td>@ref HCI_ERR_SYS_NOT_INIT</td><td>HCI SYS 尚未初始化</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_ALREADY_INIT</td><td>已经初始化过了</td></tr>
	*		<tr><td>@ref HCI_ERR_CONFIG_INVALID</td><td>配置参数有误，如设定值非法、或格式错误等</td></tr>
	*		<tr><td>@ref HCI_ERR_CONFIG_DATAPATH_MISSING</td><td>缺少必需的dataPath配置项</td></tr>
	*		<tr><td>@ref HCI_ERR_CONFIG_CAPKEY_NOT_MATCH</td><td>CAPKEY与当前引擎不匹配</td></tr>
	*		<tr><td>@ref HCI_ERR_CAPKEY_NOT_FOUND</td><td>没有找到指定的能力</td></tr>
	*		<tr><td>@ref HCI_ERR_LOAD_FUNCTION_FROM_DLL</td><td>加载库函数失败</td></tr>
	*	</table>
	*
	* @par 配置串定义：
	* 配置串是由"字段=值"的形式给出的一个字符串，多个字段之间以','隔开。字段名不分大小写。
	* @n@n
	*	<table>
	*		<tr>
	*			<td><b>字段</b></td>
	*			<td><b>取值或示例</b></td>
	*			<td><b>缺省值</b></td>
	*			<td><b>含义</b></td>
	*			<td><b>详细说明</b></td>
	*		</tr>
	*		<tr>
	*			<td>dataPath</td>
	*			<td>字符串，如：./data/</td>
	*			<td>无</td>
	*			<td>语音识别本地资源所在路径</td>
	*			<td>配置路径下存放本地能力依赖资源</td>
	*		</tr>
	*		<tr>
	*			<td>initCapKeys</td>
	*			<td>字符串，参考 @ref hci_asr_page </td>
	*			<td>无</td>
	*			<td>准备能力列表</td>
	*			<td>需要准备的能力列表，多个能力以';'隔开</td>
	*		</tr>
	*		<tr>
	*			<td>fileFlag</td>
	*			<td>字符串，有效值{none, android_so}</td>
	*			<td>none</td>
	*			<td>获取本地文件名的特殊标记</td>
	*			<td>参见下面的注释</td>
	*		</tr>
	*	</table>
	*
	*  @note
	*  <b>Android特殊配置</b>
	*  @n
	*  当fileFlag为android_so时，加载本地资源文件时会将正常的库文件名更改为so文件名进行加载。
	*  例如，当使用的库为file.dat时，则实际打开的文件名为libfile.dat.so，这样在Android系统下，
	*  开发者可以按照此规则将本地资源改名后, 放在libs目录下打包入apk。在安装后，这些资源文件
	*  就会放置在/data/data/包名/lib目录下。则可以直接将dataPath配置项指为此目录即可。
	*  @n@n
	*
	*/ 

	HCI_ERR_CODE HCIAPI hci_asr_init(
		_MUST_ _IN_ const char * pszConfig
		);

	/**  
	* @brief	开始会话
	* @param	pszConfig		会话配置串，ASCII字符串，以'\0'结束;
	* @param	pnSessionId		成功时返回会话ID
	* @return
	* @n
	*	<table>
	*		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_NOT_INIT</td><td>HCI ASR尚未初始化</td></tr>
	*		<tr><td>@ref HCI_ERR_PARAM_INVALID</td><td>输入参数不合法</td></tr>
	*		<tr><td>@ref HCI_ERR_CONFIG_INVALID</td><td>配置项不合法</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_ENGINE_FAILED</td><td>本地引擎识别失败</td></tr>
	*		<tr><td>@ref HCI_ERR_TOO_MANY_SESSION</td><td>创建的Session数量超出限制</td></tr>
	*		<tr><td>@ref HCI_ERR_CONFIG_CAPKEY_MISSING</td><td>缺少必需的capKey配置项</td></tr>
	*		<tr><td>@ref HCI_ERR_URL_MISSING</td><td>找不到对应的网络服务地址（HCI能力服务地址)</td></tr>
	*		<tr><td>@ref HCI_ERR_CONFIG_CAPKEY_NOT_MATCH</td><td>CAPKEY与当前引擎不匹配</td></tr>
	*		<tr><td>@ref HCI_ERR_CAPKEY_NOT_FOUND</td><td>没有找到指定的能力</td></tr>
	*		<tr><td>@ref HCI_ERR_LOAD_FUNCTION_FROM_DLL</td><td>加载库函数失败</td></tr>
	*		<tr><td>@ref HCI_ERR_CONFIG_UNSUPPORT</td><td>配置项不支持,云端暂不支持grammar的实时识别</td></tr>
	*		<tr><td>@ref HCI_ERR_CONFIG_DATAPATH_MISSING</td><td>缺少必需的dataPath配置项</td></tr>
	*	</table>
	*
	* @par 配置串定义：
	* 配置串是由"字段=值"的形式给出的一个字符串，多个字段之间以','隔开。字段名不分大小写。
	* 支持云+端双路识别功能，如果要使用云+端双路识别，需要本地端能力配置和云端能力配置，配置之间使用'#'分割。
	* 例如：capkey=asr.cloud.freetalk,realtime=yes\#capkey=asr.local.grammar,realtime=yes,grammartype=id,grammarid=2
	* @n@n
	* 开启会话相关配置：
	*	<table>
	*		<tr>
	*			<td><b>字段</b></td>
	*			<td><b>取值或示例</b></td>
	*			<td><b>缺省值</b></td>
	*			<td><b>含义</b></td>
	*			<td><b>详细说明</b></td>
	*		</tr>
	*		<tr>
	*			<td>capKey</td>
	*			<td>字符串，参考 @ref hci_asr_page </td>
	*			<td>无</td>
	*			<td>语音识别能力key</td>
	*			<td>此项必填。每个session只能定义一种能力，并且过程中不能改变。</td>
	*		</tr>
    *		<tr>
    *			<td>resPrefix</td>
    *			<td>字符串，如：temp_</td>
    *			<td>无</td>
    *			<td>资源加载前缀</td>
    *			<td>不涉及多能力调用情况下，该项可忽略。如果不同会话需要使用同一路径下资源时可以使用该字段对统一路径下的资源进行区分</td>
    *		</tr>
	*		<tr>
	*			<td>intention</td>
	*			<td>字符串，如：poi</td>
	*			<td>无</td>
	*			<td>识别意图</td>
	*			<td>仅在dialog识别中有效，使用云端能力时必填。<br/>
	*				可传入多个领域，以分号分隔。<br/>
	*				例如：intention=weather;call，  相应的添加资源文件：weather_xxxx，call_xxxx<br/>
	*				有效配置详见：@ref nlu_intention</td>
	*		</tr>
	*		<tr>
	*			<td>realtime</td>
	*			<td>字符串，有效值{no, yes,rt}</td>
	*			<td>no</td>
	*			<td>识别模式</td>
	*			<td>no,非实时识别模式<br/>
	*				yes,实时识别模式,云端语法识别不支持，其他能力均支持<br/>
	*				rt,实时反馈识别结果,仅云端自由说asr.cloud.freetalk支持，其它能力均不支持，实时反馈结果文件格式参见 @ref hci_asr_realtime_rt "实时反馈"</td>
	*		</tr>
	*		<tr>
	*			<td>maxSeconds</td>
	*			<td>正整数，范围[1,60]</td>
	*			<td>30</td>
	*			<td>检测时输入的最大语音长度, 以秒为单位</td>
	*			<td>如果输入的声音超过此长，<br/>
	*				非实时识别返回(@ref HCI_ERR_DATA_SIZE_TOO_LARGE)<br/>
	*				实时识别返回(@ref HCI_ERR_ASR_REALTIME_END)<br/>
	*				端点检测认为超过maxseconds为缓冲区满<br/> 
	*               例外：云端非实时识别由于网速原因，暂时限定不能超过256k。</td>			
	*		</tr>
	*		<tr>
	*			<td>dialogMode</td>
	*			<td>字符串，有效值{freetalk,grammar}</td>
	*			<td>freetalk</td>
	*			<td>本地dialog的语音识别模式</td>
	*			<td>仅本地dialog生效，判断语音识别的部分使用的是本地语法识别，还是本地自由说识别。</td>			
	*		</tr>
	*		<tr>
	*			<td>netTimeout</td>
	*			<td>网络超时时间，范围[1,30]</td>
	*			<td>8</td>
	*			<td>网络请求超时时间，以秒为单位</td>
	*			<td>网络连接或网络请求超过设定值时，将返回失败。</td>			
	*		</tr>
	*	</table>
	* @n@n
	* 另外，这里还可以传入识别的配置项，作为默认配置项。参见 hci_asr_recog() 。
	*/ 

	HCI_ERR_CODE HCIAPI hci_asr_session_start(
		_MUST_ _IN_ const char * pszConfig,
		_MUST_ _OUT_ int * pnSessionId
		);

	/**  
	* @brief	本地语法识别加载语法
	* @param	pszConfig			识别参数配置串，ASCII字符串，可为NULL或以'\0'结束
	* @param	pszGrammarData		识别语法数据或文件，该数据必须以‘\0’结尾, 如果是语法数据，必须编码格式为UTF-8, 不可以为NULL
	* @param	pnGrammarId			返回的语法标识，可以用于 hci_asr_recog() 和 hci_asr_unload_grammar() 函数
	* @return
	* @n
	*	<table>
	*		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_NOT_INIT</td><td>HCI ASR尚未初始化</td></tr>
	*		<tr><td>@ref HCI_ERR_PARAM_INVALID</td><td>输入参数不合法</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_LOAD_GRAMMAR_FAILED</td><td>加载语法文件失败</td></tr>
	*		<tr><td>@ref HCI_ERR_CONFIG_INVALID</td><td>配置项不合法</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_OPEN_GRAMMAR_FILE</td><td>读取语法文件失败</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_GRAMMAR_OVERLOAD</td><td>加载语法数量已达上限值256</td></tr>
	*	</table>
	*
	* @par 配置串定义：
	* 配置串是由"字段=值"的形式给出的一个字符串，多个字段之间以','隔开。字段名不分大小写。
	* @n@n
	*	<table>
	*		<tr>
	*			<td><b>字段</b></td>
	*			<td><b>取值或示例</b></td>
	*			<td><b>缺省值</b></td>
	*			<td><b>含义</b></td>
	*			<td><b>详细说明</b></td>
	*		</tr>
    *		<tr>
    *			<td>capKey</td>
    *			<td>字符串，参考 @ref hci_asr_page </td>
    *			<td>无</td>
    *			<td>语音识别能力key</td>
    *			<td>此项必填。加载语法必须与开启会话的能力一致并且不能改变。</td>
    *		</tr>
    *		<tr>
    *			<td>resPrefix</td>
    *			<td>字符串，如：temp_</td>
    *			<td>无</td>
    *			<td>资源加载前缀</td>
    *			<td>不涉及多能力调用情况下，该项可忽略。如果不同会话需要使用同一路径下资源时可以使用该字段对统一路径下的资源进行区分</td>
    *		</tr>
	*		<tr>
	*			<td>grammarType</td>
	*			<td>字符串，有效值{wordlist, jsgf, wfst}</td>
	*			<td>jsgf</td>
	*			<td>使用的语法类型</td>
	*			<td>指定如何载入语法。<br/>
	*				- wordlist: 将 grammarData 数据当做以'\\r\\n'隔开的词表
	*				- jsgf: 将 grammarData 数据当做语法文件进行识别，语法文件格式请参见《捷通华声iSpeakGrammar语法规则说明.pdf》
	*				- wfst: 通过hci_asr_save_compiled_grammar接口保存的语法文件，只支持从文件加载
	*			</td>
	*		</tr>
	*		<tr>
	*			<td>isFile</td>
	*			<td>字符串，有效值{yes, no}</td>
	*			<td>no</td>
	*			<td>输入参数是文件名还是内存数据</td>
	*			<td>yes表示pszGrammarData表示的是一个语法文件名，no表示pszGrammarData表示的直接是内存中的语法数据</td>
	*		</tr>
	*	</table>
	*
	* @note
	* 可以用本函数加载本地语法文件，得到语法ID后，再使用 hci_asr_recog() 进行识别，
	* 识别时在 hci_asr_recog() 中将配置的grammarType设为id, grammarId设为本函数得到的语法ID，即可进行识别。
	* @n@n
	* 这里加载本地语法与session无关，多个session可以共享这里加载的语法
	* 
	*/ 
	HCI_ERR_CODE HCIAPI hci_asr_load_grammar(	
		_OPT_ _IN_ const char * pszConfig,   
		_MUST_ _IN_ const char * pszGrammarData,
		_MUST_ _OUT_ unsigned int * pnGrammarId
		);

	/**  
	* @brief	本地语法识别卸载语法
	* @param	nGrammarId			需要卸载的语法ID，从 hci_asr_load_grammar() 获得
	* @return
	* @n
	*	<table>
	*		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_NOT_INIT</td><td>HCI ASR尚未初始化</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_GRAMMAR_ID_INVALID</td><td>语法ID无效</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_GRAMMAR_USING</td><td>该语法正在使用中</td></tr>
	*		<tr><td>@ref HCI_ERR_UNSUPPORT</td><td>只编译云端版本时不支持</td></tr>
	*	</table>
	*/
	HCI_ERR_CODE HCIAPI hci_asr_unload_grammar(	
		_MUST_ _IN_ unsigned int nGrammarId
		);

	/**  
	* @brief	保存编译后的语法文件至WFST格式。编译后的文件可以通过WFST类型的方式载入，对于大的语法文件可以大大提高语法加载速度。
	* @param	nGrammarId			需要保存的语法ID，从 hci_asr_load_grammar() 获得
	* @param	pcsFileName			要保存的语法文件名称，不能为NULL或空串
	* @return	
	* @n
	*	<table>
	*		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_NOT_INIT</td><td>HCI ASR尚未初始化</td></tr>
	*		<tr><td>@ref HCI_ERR_PARAM_INVALID</td><td>输入参数不合法</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_GRAMMAR_ID_INVALID</td><td>语法ID无效</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_OPEN_GRAMMAR_FILE</td><td>写入语法文件失败</td></tr>
	*	</table>
	*/ 
	HCI_ERR_CODE hci_asr_save_compiled_grammar(
		_MUST_ _IN_ unsigned int nGrammarId,
		_MUST_ _IN_ const char *pcsFileName
		);

	/**  
	* @brief	语音识别
	* @param	nSessionId			会话ID
	* @param	pvVoiceData			要识别的音频数据，非实时识别中传入音频时长不应超过10s，否则建议使用实时识别
	* @param	uiVoiceDataLen		要识别的音频数据长度，以字节为单位
	* @param	pszConfig			识别参数配置串，ASCII字符串，可为NULL或以'\0'结束
	* @param	pszGrammarData		识别语法数据，该数据必须以‘\0’结尾,且编码格式为UTF-8,可以为NULL
	* @param	psAsrRecogResult	识别结果，使用完毕后，需使用 hci_asr_free_recog_result() 进行释放
	* @return
	* @n
	*	<table>
	*		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_NOT_INIT</td><td>HCI ASR尚未初始化</td></tr>
	*		<tr><td>@ref HCI_ERR_PARAM_INVALID</td><td>输入参数不合法</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_GRAMMAR_DATA_TOO_LARGE</td><td>语法数据过大(0,64K]</td></tr>
	*		<tr><td>@ref HCI_ERR_DATA_SIZE_TOO_LARGE</td><td>Encode转化后的音频数据和语法数据总大小超过限制(0,320K]，音频数据不宜过长，建议音频时长在60秒以内</td></tr>
	*		<tr><td>@ref HCI_ERR_SESSION_INVALID</td><td>传入的Session非法</td></tr>
	*		<tr><td>@ref HCI_ERR_URL_MISSING</td><td>找不到对应的网络服务地址（HCI能力服务地址)</td></tr>
	*		<tr><td>@ref HCI_ERR_CAPKEY_NOT_FOUND</td><td>没有找到指定的能力</td></tr>
	*		<tr><td>@ref HCI_ERR_SERVICE_CONNECT_FAILED</td><td>连接服务器失败，服务器无响应</td></tr>
	*		<tr><td>@ref HCI_ERR_SERVICE_TIMEOUT</td><td>服务器访问超时</td></tr>
	*		<tr><td>@ref HCI_ERR_SERVICE_DATA_INVALID</td><td>服务器返回的数据格式不正确</td></tr>
	*		<tr><td>@ref HCI_ERR_SERVICE_RESPONSE_FAILED</td><td>服务器返回识别失败</td></tr>
	*		<tr><td>@ref HCI_ERR_CONFIG_INVALID</td><td>配置参数无效</td></tr>
	*		<tr><td>@ref HCI_ERR_CONFIG_UNSUPPORT</td><td>配置项不支持</td></tr>
	*		<tr><td>@ref HCI_ERR_LOAD_CODEC_DLL</td><td>加载音频编解码库失败</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_OPEN_GRAMMAR_FILE</td><td>读取语法文件失败</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_GRAMMAR_ID_INVALID</td><td>语法ID无效</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_LOAD_GRAMMAR_FAILED</td><td>加载语法文件失败</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_ENGINE_FAILED</td><td>本地引擎识别失败</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_REALTIME_WAITING</td><td>实时识别时未检测到音频末端，继续等待数据</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_REALTIME_END</td><td>实时识别时检测到音频末端</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_REALTIME_NO_VOICE_INPUT</td><td>实时识别时未检测语音</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_VOICE_DATA_TOO_LARGE</td><td>音频片段太长，应在(0,32K)</td></tr>
	*	</table>
	*
	* @par 配置串定义：
	* 配置串是由"字段=值"的形式给出的一个字符串，多个字段之间以','隔开。字段名不分大小写。
	* @n@n
    * 音频相关配置
	*	<table>
	*		<tr>
	*			<td><b>字段</b></td>
	*			<td><b>取值或示例</b></td>
	*			<td><b>缺省值</b></td>
	*			<td><b>含义</b></td>
	*			<td><b>详细说明</b></td>
	*		</tr>
	*		<tr>
	*			<td>audioFormat</td>
	*			<td>字符串，有效值{pcm8k16bit, ulaw8k8bit, alaw8k8bit, pcm16k16bit , ulaw16k8bit, alaw16k8bit}</td>
	*			<td>pcm16k16bit</td>
	*			<td>传入的语音数据格式</td>
	*			<td>本地识别支持：pcm16k16bit<br/>
	*				云端grammar语法识别支持：pcm16k16bit，ulaw16k8bit，alaw16k8bit<br/> 
	*				云端freetalk和dialog支持：pcm16k16bit，pcm8k16bit，ulaw16k8bit，ulaw8k8bit，alaw16k8bit，alaw8k8bit<br/>
	*				注意，实时识别的端点检测暂不支持：alaw, ulaw<br/></td>
	*		</tr>
    *		<tr>
    *			<td>encode</td>
    *			<td>字符串，有效值{none, ulaw, alaw, speex,opus}</td>
    *			<td>none</td>
    *			<td>使用的编码格式</td>
    *			<td>只对云端识别有效：对传入的语音数据进行编码传输。<br/>
    *               具体audioFormat和encode的使用方法参见@ref codec_for_format </td>
    *		</tr> 
    *		<tr>
    *			<td>encLevel</td>
    *			<td>整数，范围[0,10]</td>
    *			<td>7</td>
    *			<td>压缩等级</td>
    *			<td>只对云端识别有效：</td>
    *		</tr>
    *	</table>
    * @n@n
    * 语法识别配置
    *	<table>
    *		<tr>
    *			<td><b>字段</b></td>
    *			<td><b>取值或示例</b></td>
    *			<td><b>缺省值</b></td>
    *			<td><b>含义</b></td>
    *			<td><b>详细说明</b></td>
    *		</tr>
	*		<tr>
	*			<td>grammarType</td>
	*			<td>字符串，有效值{id, wordlist, jsgf, wfst}</td>
	*			<td>id</td>
	*			<td>使用的语法类型</td>
	*			<td>只对grammar识别有效：指定语法格式。<br/>
	*				- id: 使用云端预置的或者本地加载的语法编号进行识别
	*				- wordlist: 将 grammarData 数据当做以'\\r\\n'隔开的词表进行识别
	*				- jsgf: 将 grammarData 数据当做语法文件进行识别，语法文件格式请参见《捷通华声iSpeakGrammar语法规则说明.pdf》
	*				- wfst: 通过hci_asr_save_compiled_grammar接口保存的语法文件，只支持文件加载
	*			</td>
	*		</tr>
	*		<tr>
	*			<td>grammarId</td>
	*			<td>正整数，如：1</td>
	*			<td>无</td>
	*			<td>识别使用的语法编号</td>
	*			<td>只对grammar识别中且grammarType设置为id时有效：<br/>
	*				- 此id可以是云端预置的某个语法Id，
	*				- 也可以是利用 hci_asr_load_grammar() 函数获得的本地的语法Id。这两类语法Id分别用于
	*				本地或者云端的语法识别能力，两者之间没有关系。灵云平台ASR云识别能力会预置语法，具体可用的语法编号可以在开发时咨询捷通华声。</td>
	*		</tr>
	*		<tr>
	*			<td>isFile</td>
	*			<td>字符串，有效值{yes, no}</td>
	*			<td>no</td>
	*			<td>输入参数是文件名还是内存数据</td>
	*			<td>只对本地grammar识别有效：<br/>
	*			    yes表示的是一个语法文件名<br/>
	*				no表示的直接是内存中的语法数据
	*			</td>
	*		</tr>
	*	</table>
    * @n@n
    * 端点检测配置
    *	<table>
    *		<tr>
    *			<td><b>字段</b></td>
    *			<td><b>取值或示例</b></td>
    *			<td><b>缺省值</b></td>
    *			<td><b>含义</b></td>
    *			<td><b>详细说明</b></td>
    *		</tr>
    *		<tr>
    *			<td>vadSwitch</td>
    *			<td>字符串，有效值{yes，no}</td>
    *			<td>yes</td>
    *			<td>识别前进行端点检测</td>
    *			<td>如果为yes则开启端点检测，如果no则关闭端点检测</td>
    *		</tr> 
	*		<tr>
	*			<td>vadHead</td>
	*			<td>正整数，范围[0,30000]</td>
	*			<td>10000</td>
	*			<td>开始的静音检测毫秒数</td>
	*			<td>当声音开始的静音超过此项指定的毫秒数时，认为没有检测到声音<br/>
	*				如果此值为0，则表示不进行起点检测</td>
	*		</tr> 
	*		<tr>
	*			<td>vadTail</td>
	*			<td>正整数，范围[0,30000]</td>
	*			<td>500</td>
	*			<td>端点检测的末尾毫秒数</td>
	*			<td>检测到起点后语音数据出现静音并且静音时间超过此项指定的毫秒数时，认为声音结束<br/>
	*				如果此值为0，则表示不进行末端检测</td>
	*		</tr>
	*		<tr>
	*			<td>vadSeg</td>
	*			<td>正整数，范围[0,30000]</td>
	*			<td>500</td>
	*			<td>端点检测的分段间隔毫秒数</td>
	*			<td>目前只对云端自由说实时反馈模式生效，<br/>
	*				检测到起点后语音数据出现静音并且静音时间超过此项指定的毫秒数时，认为音频进行分段<br/>
	*				如果此值为0或设置大于等于vadtail，则表示不进行分段</td>
	*		</tr>
    *		<tr>
    *			<td>vadThreshold</td>
    *			<td>正整数，范围[0,100]</td>
    *			<td>10</td>
    *			<td>端点检测灵敏度设置</td>
    *			<td>端点检测灵敏度设置，该值越小越灵敏</td>
    *		</tr>
    *	</table>
	*/
#ifndef PRIVATE_CLOUD__
	/**
    * @n@n
    * 识别结果相关配置
    *	<table>
    *		<tr>
    *			<td><b>字段</b></td>
    *			<td><b>取值或示例</b></td>
    *			<td><b>缺省值</b></td>
    *			<td><b>含义</b></td>
    *			<td><b>详细说明</b></td>
    *		</tr>
    *		<tr>
    *			<td>domain</td>
    *			<td>字符串，如：common, music, poi</td>
    *			<td>无</td>
    *			<td>识别使用的领域</td>
    *			<td>使用指定领域的模型进行识别。<br/>
    *				- 此项可选。具体可用的领域可以在开发时咨询捷通华声。<br/>
	*				特别的，使用公有云能力asr.cloud.freetalk.music或asr.cloud.freetalk.poi时<br/>
	*				需要传相对应的domain参数"music" 或 "poi"</td>
    *		</tr>
    *		<tr>
    *			<td>addPunc</td>
    *			<td>字符串，有效值{yes, no}</td>
    *			<td>no</td>
    *			<td>是否添加标点</td>
    *			<td>yes表示识别过程中添加标点，no表示不添加标点</td>
    *		</tr>
    *		<tr>
    *			<td>candNum</td>
    *			<td>正整数，范围[1,10]</td>
    *			<td>10</td>
    *			<td>识别候选结果个数</td>
    *			<td></td>
    *		</tr>
	*		<tr>
	*			<td>needContent</td>
	*			<td>字符串，有效值{yes, no}</td>
	*			<td>yes</td>
	*			<td>是否需要意图识别内容</td>
	*			<td>仅在dialog识别中有效：<br/>
	*				指定是否需要意图识别内容，即意图识别后获取相应的内容或答案。</td>
	*		</tr>
	*		<tr>
	*			<td>context</td>
	*			<td>字符串，有效字{yes,no}</td>
	*			<td>yes</td>
	*			<td>是否开启上下文功能</td>
	*			<td>仅在dialog识别中有效：<br/>
	*				yes 使用上下文，使NLU能记住对话的情景<br/>
	*				no 不使用上下文。
	*		</tr>
	*	</table>
	* @n@n
	* 这里没有定义的配置项，会使用 session_start 时的定义。如果 session_start 时也没有定义，则使用缺省值
	* 另外，这里还可以传入端点检测的配置项， 作为实时识别时进行端点检测的配置。
	*/ 
#else
	/**
    * @n@n
    * 识别结果相关配置
    *	<table>
    *		<tr>
    *			<td><b>字段</b></td>
    *			<td><b>取值或示例</b></td>
    *			<td><b>缺省值</b></td>
    *			<td><b>含义</b></td>
    *			<td><b>详细说明</b></td>
    *		</tr>
    *		<tr>
    *			<td>addPunc</td>
    *			<td>字符串，有效值{yes, no}</td>
    *			<td>no</td>
    *			<td>是否添加标点</td>
    *			<td>yes表示识别过程中添加标点，no表示不添加标点</td>
    *		</tr>
	*		<tr>
	*			<td>candNum</td>
	*			<td>正整数，范围[1,10]</td>
	*			<td>10</td>
	*			<td>识别候选结果个数</td>
	*			<td></td>
	*		</tr>
	*		<tr>
	*			<td>intention</td>
	*			<td>字符串，如：poi</td>
	*			<td>无</td>
	*			<td>识别意图</td>
	*			<td>仅在云端dialog识别中有效：<br/>
	*				- 云端识别会自动将识别结果进行意图分析，并返回json格式的意图结果。该项取值具体信息请咨询捷通华声公司</td>
	*		</tr>
	*		<tr>
	*			<td>needContent</td>
	*			<td>字符串，有效值{yes, no}</td>
	*			<td>yes</td>
	*			<td>是否需要意图识别内容</td>
	*			<td>仅在云端dialog识别中有效：<br/>
	*				指定是否需要意图识别内容，即意图识别后获取相应的内容或答案。</td>
	*		</tr>
	*		<tr>
	*			<td>property</td>
	*			<td>字符串，例如：chinese_8k_common</td>
	*			<td>无</td>
	*			<td>取值是语种lang、modeltype和领域domain组合。 </td>
	*			<td>必选，有效取值范围如下：<br/>
	*				lang: chinese，english <br/>
	*				modeltype: 8k(8k模型), 16k(16k模型) <br/>
	*				domain: common(通用领域), poi(导航领域), music(音乐领域), telecom(电信领域) <br/>
	*		</td>
	*		</tr>
	*	</table>
	* @n@n
	* 这里没有定义的配置项，会使用 session_start 时的定义。如果 session_start 时也没有定义，则使用缺省值
	* 另外，这里还可以传入端点检测的配置项， 作为实时识别时进行端点检测的配置。
	*/
#endif
	HCI_ERR_CODE HCIAPI hci_asr_recog(	
		_MUST_ _IN_ int nSessionId,
		_MUST_ _IN_ void * pvVoiceData,
		_MUST_ _IN_ unsigned int uiVoiceDataLen,
		_OPT_ _IN_ const char * pszConfig,   
		_OPT_ _IN_ const char * pszGrammarData,
		_MUST_ _OUT_ ASR_RECOG_RESULT * psAsrRecogResult
		);

	/**  
	* @brief	释放语音识别结果内存
	* @param	psAsrRecogResult	需要释放的识别结果内存
	* @return	
	* @n
	*	<table>
	*		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
	*		<tr><td>@ref HCI_ERR_PARAM_INVALID</td><td>输入参数不合法</td></tr>
	*	</table>
	*/ 
	HCI_ERR_CODE HCIAPI hci_asr_free_recog_result(	
		_MUST_ _IN_ ASR_RECOG_RESULT * psAsrRecogResult
		);

	/**  
	* @brief	提交确认结果函数
	* @param	nSessionId			会话ID
	* @param	pAsrConfirmItem		要提交的确认结果
	* @return
	* @n
	*	<table>
	*		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_NOT_INIT</td><td>HCI ASR尚未初始化</td></tr>
	*		<tr><td>@ref HCI_ERR_PARAM_INVALID</td><td>输入参数不合法</td></tr>
	*		<tr><td>@ref HCI_ERR_SESSION_INVALID</td><td>传入的Session非法</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_CONFIRM_NO_TASK</td><td>没有可用来提交的任务，例如尚未识别，就调用本函数</td></tr>
	*		<tr><td>@ref HCI_ERR_DATA_SIZE_TOO_LARGE</td><td>确认数据超过范围(0,2K]</td></tr>
	*		<tr><td>@ref HCI_ERR_SERVICE_CONNECT_FAILED</td><td>连接服务器失败，服务器无响应</td></tr>
	*		<tr><td>@ref HCI_ERR_SERVICE_TIMEOUT</td><td>服务器访问超时</td></tr>
	*		<tr><td>@ref HCI_ERR_SERVICE_DATA_INVALID</td><td>服务器返回的数据格式不正确</td></tr>
	*		<tr><td>@ref HCI_ERR_SERVICE_RESPONSE_FAILED</td><td>服务器返回识别失败</td></tr>
	*		<tr><td>@ref HCI_ERR_UNSUPPORT</td><td>本地识别不支持确认结果</td></tr>
	*	</table>
	*/ 
	HCI_ERR_CODE HCIAPI hci_asr_confirm(	
		_MUST_ _IN_ int nSessionId,
		_MUST_ _IN_ ASR_CONFIRM_ITEM * pAsrConfirmItem
		);

	/**  
	* @brief	结束会话
	* @param	nSessionId				会话ID
	* @return	错误码
	* @return
	* @n
	*	<table>
	*		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_NOT_INIT</td><td>HCI ASR尚未初始化</td></tr>
	*		<tr><td>@ref HCI_ERR_SESSION_INVALID</td><td>传入的Session非法</td></tr>
	*	</table>
	*/
	HCI_ERR_CODE HCIAPI hci_asr_session_stop(
		_MUST_ _IN_ int nSessionId
		);

	/**  
	* @brief	灵云ASR能力 反初始化
	* @return	错误码
	* @return
	* @n
	*	<table>
	*		<tr><td>@ref HCI_ERR_NONE</td><td>操作成功</td></tr>
	*		<tr><td>@ref HCI_ERR_ASR_NOT_INIT</td><td>HCI ASR尚未初始化</td></tr>
	*		<tr><td>@ref HCI_ERR_ACTIVE_SESSION_EXIST</td><td>尚有未stop的Sesssion，无法结束</td></tr>
	*	</table>
	*/ 
	HCI_ERR_CODE HCIAPI hci_asr_release();

    /* @} */
    //////////////////////////////////////////////////////////////////////////
	/* @} */
	//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
};
#endif


#endif
