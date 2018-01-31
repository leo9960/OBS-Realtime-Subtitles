/**
 * @file    hci_asr_recorder.h
 * @brief   HCI_ASR_RECORDER SDK 头文件
 */
#pragma once
#ifndef __HCI_ASR_RECORDER_HEADER__
#define __HCI_ASR_RECORDER_HEADER__

#include "hci_sys.h"
#include "hci_asr.h"

#ifdef __cplusplus
extern "C"
{
#endif
    
    /** @defgroup HCI_ASR_RECORDER_API 灵云ASR录音机API */
    /* @{ */
    
    /** @defgroup HCI_RECORDER_STRUCT 结构体 */
    /* @{ */
    
    /**
	*@brief	模块名称
	*/
    #define MODULE_NAME    "HCI_ASR_RECORDER"
    
    /**
     * @brief	录音机回调时的通知事件
     */
    typedef enum
    {
        RECORDER_EVENT_BEGIN_RECORD,		///< 录音开始
        RECORDER_EVENT_HAVING_VOICE,		///< 听到声音 检测到始端的时候会触发该事件
        RECORDER_EVENT_NO_VOICE_INPUT,		///< 没有听到声音
        RECORDER_EVENT_BUFF_FULL,			///< 缓冲区已填满
        RECORDER_EVENT_END_RECORD,			///< 录音完毕（自动或手动结束）
        RECORDER_EVENT_BEGIN_RECOGNIZE,		///< 开始识别
        RECORDER_EVENT_RECOGNIZE_COMPLETE,	///< 识别完毕
        RECORDER_EVENT_ENGINE_ERROR,		///< 引擎出错
        RECORDER_EVENT_DEVICE_ERROR,		///< 设备出错
        RECORDER_EVENT_MALLOC_ERROR,		///< 分配空间失败
        RECORDER_EVENT_INTERRUPTED,         ///< iOS收到其他声音中断会触发此事件，同时停止录音机，开发者必须处理此事件
        RECORDER_EVENT_PERMISSION_DENIED,   ///< iOS没有麦克风使用权限(ios7以上版本会有此事件，开发者必须处理此事件)
        RECORDER_EVENT_TASK_FINISH,			///< 识别任务结束
        RECORDER_EVENT_RECOGNIZE_PROCESS,	///< 识别中间状态
    }
    RECORDER_EVENT;
    
    /**
     * @brief	录音机错误码
     */
    typedef enum
    {
        RECORDER_ERR_UNKNOWN = -1,					///< -1: 未知错误，不会出现
        
        RECORDER_ERR_NONE = 0,						///< 0: 成功
        
        RECORDER_ERR_NOT_INIT,						///< 1: 没有初始化
        RECORDER_ERR_ALREADY_INIT,					///< 2: 已经初始化
        RECORDER_ERR_ALREADY_BEGIN,					///< 3: 已经开始录音
        RECORDER_ERR_NOT_BEGIN,						///< 4: 没有开始录音
        RECORDER_ERR_OUT_OF_MEMORY,					///< 5: 分配空间失败
        RECORDER_ERR_ENGINE_ERROR,					///< 6: 调用引擎出错
		RECORDER_ERR_CONFIG_ITEM,					///< 7: 配置串错误
    }
    RECORDER_ERR_CODE;
    
    /**
     * @brief	录音机事件变化回调
     * @note
     * RECORDER_EVENT_RECOGNIZE_COMPLETE 事件不会在该回调中触发，它只会在 Callback_RecorderEventRecogFinish 中触发
     * RECORDER_EVENT_ENGINE_ERROR、RECORDER_EVENT_DEVICE_ERROR 事件不会在该回调中触发，只会在 Callback_RecorderErr 中触发
     * @param	eRecorderEvent		回调时的通知事件
     * @param	pUsrParam			用户自定义参数
     */
    typedef void (HCIAPI * Callback_RecorderEventStateChange)(
                                                              _MUST_ _IN_ RECORDER_EVENT eRecorderEvent,
                                                              _OPT_ _IN_ void * pUsrParam );
    
    /**
     * @brief	录音音频数据回调
     * @note
     * @param    pVoiceData      音频数据
     * @param    uiVoiceLen      音频数据长度
     * @param    pUsrParam		用户自定义参数
     */
    typedef void (HCIAPI * Callback_RecorderRecording)(
                                                       _MUST_ _IN_ unsigned char * pVoiceData,
                                                       _MUST_ _IN_ unsigned int uiVoiceLen,
                                                       _OPT_ _IN_ void * pUsrParam
                                                       );
    
    /**
     * @brief	录音机识别完成回调
     * @note
     * 识别成功后回调
     * @param	eRecorderEvent		识别完成事件
     * @param	pRecogResult		识别结果
     * @param	pUsrParam			用户自定义参数
     */
    typedef void (HCIAPI * Callback_RecorderEventRecogFinish)(
                                                              _MUST_ _IN_ RECORDER_EVENT eRecorderEvent,
                                                              _MUST_ _IN_ ASR_RECOG_RESULT *pRecogResult,
                                                              _OPT_ _IN_ void * pUsrParam );
    
    
    /**
     * @brief	录音机识别中间结果回调
     * @note
     * 识别中间结果回调
     * @param	eRecorderEvent		识别进行中事件
     * @param	pRecogResult		识别结果
     * @param	pUsrParam			用户自定义参数
     */
    typedef void (HCIAPI * Callback_RecorderEventRecogProcess)(
                                                               _MUST_ _IN_ RECORDER_EVENT eRecorderEvent,
                                                               _MUST_ _IN_ ASR_RECOG_RESULT *pRecogResult,
                                                               _OPT_ _IN_ void * pUsrParam );
    
    
    /**
     * @brief	ASR SDK出错回调
     * @note
     * 发生事件 RECORDER_EVENT_ENGINE_ERROR 或 RECORDER_EVENT_DEVICE_ERROR 时回调
     * @param	eRecorderEvent		回调时的通知事件
     * @param	nErrorCode			错误码
     * @param	pUsrParam			用户自定义参数
     */
    typedef void (HCIAPI * Callback_RecorderEventError)(
                                                        _MUST_ _IN_ RECORDER_EVENT eRecorderEvent,
                                                        _MUST_ _IN_ HCI_ERR_CODE eErrorCode,
                                                        _OPT_ _IN_ void * pUsrParam );
    
    /**
     * @brief	设置audisession回调
     * @note
     * 只在iOS平台使用
     * @param	pExtendParam		扩展参数，当前无效
     * @param	pUsrParam			用户数据
     */
    typedef bool (HCIAPI * Callback_RecorderSetAudioSession)(
                                                            _MUST_ _IN_ void * pExtendParam,
                                                            _OPT_ _IN_ void * pUsrParam);

    /**
     * @brief	录音机回调函数的结构体
     */
    typedef struct _RECORDER_CALLBACK_PARAM {
        Callback_RecorderEventStateChange pfnStateChange;       ///< 录音机状态回调
        void * pvStateChangeUsrParam;                           ///< 录音机状态回调自定义参数
        Callback_RecorderRecording pfnRecording;                ///< 录音机数据回调
        void * pvRecordingUsrParam;                             ///< 录音机数据回调自定义参数
        Callback_RecorderEventRecogFinish pfnRecogFinish;       ///< 录音机识别结束回调
        void * pvRecogFinishUsrParam;                           ///< 录音机识别结束回调自定义参数
        Callback_RecorderEventError pfnError;                   ///< 录音机错误回调
        void *pvErrorUsrParam;                                  ///< 录音机错误回调自定义参数
        Callback_RecorderEventRecogProcess pfnRecogProcess;     ///< 录音机识别中间结果回调
        void *pvRecogProcessParam;                              ///< 录音机识别中间结果回调自定义参数
    } RECORDER_CALLBACK_PARAM;

    /* @} */

    /** @defgroup HCI_RECOREDER_FUNC 函数 */
    /* @{ */
    
    /**
     * @brief	录音机SDK初始化
     * @param	pszAsrSdkConfig		录音机初台化配置串 参见 hci_asr_init
     * @param	psCallbackParam		回调函数的集合
     * @return
     * @n
     *	<table>
     *		<tr><td>@ref RECORDER_ERR_NONE</td><td>操作成功</td></tr>
	 *		<tr><td>@ref RECORDER_ERR_CONFIG_ITEM</td><td>配置串错误</td></tr>
     *		<tr><td>@ref RECORDER_ERR_ENGINE_ERROR</td><td>ASR SDK 初始化失败，失败的原因会通过 Callback_RecorderErr 回调返回</td></tr>
     *		<tr><td>@ref RECORDER_ERR_ALREADY_INIT</td><td>已经初始化</td></tr>
     *	</table>
	 * @par 配置串定义：
	 * 这里定义录音机对配置串的配置，其它配置 参见 hci_asr_init。
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
	 *			<td>initCapKeys</td>
	 *			<td>字符串，参考 hci_asr_init </td>
	 *			<td>无</td>
	 *			<td>准备能力列表</td>
	 *			<td>录音机不支持云端语法识别</td>
	 *		</tr>
	 *	</table>
	 *
     */
    RECORDER_ERR_CODE HCIAPI hci_asr_recorder_init(
                                                   _MUST_ _IN_ const char * pszAsrSdkConfig,
                                                   _MUST_ _IN_ RECORDER_CALLBACK_PARAM *psCallbackParam);
    
    /**
     * @brief	设置audiosession回调
     * @param	pfnCallBack		audiosession回调
     * @param	pUsrParam		用户数据
     * @return
     * @n
     *	<table>
     *		<tr><td>@ref RECORDER_ERR_NONE</td><td>操作成功</td></tr>
     *		<tr><td>@ref RECORDER_ERR_NOT_INIT</td><td>recorder尚未初始化</td></tr>
     *	</table>
     * @n
     * @note
     * 此接口只在iOS平台使用
     * recorder默认不处理中断。如果你的应用需要响应中断进行相应的处理，
     * 可以调用此函数，设置Callback_RecorderSetAudioSession回调，在回调中进行audiosession的设置。如何设置audiosession，请参
     * 考苹果AudioToolBox的官方文档。下面是一个设置audiosession的示例
     @code
     *  void playerSetAudioSessionCallback(void * pExtendParam, void * pUsrParam)
     *	{
     *		//初始化audiosession
     *		AudioSessionInitialize(NULL, NULL, MyAudioSessionInterruptionListener, pUsrParam);
     *		//设置kAudioSessionProperty_AudioCategory
     *		UInt32 sessionCategory = kAudioSessionCategory_PlayAndRecord;
     *		AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(sessionCategory), &sessionCategory);
     *	}
     @endcode
     */
    RECORDER_ERR_CODE hci_asr_recorder_set_audio_session_callback(
                                                                  _MUST_ _IN_ Callback_RecorderSetAudioSession pfnCallBack ,
                                                                  _OPT_ _IN_ void * pUsrParam);
    
    /**
     * @brief	录音机SDK加载语法
     * @param	pszAsrSdkConfig		录音机初台化配置串 参见 hci_asr_load_grammar
     * @param	pszGrammarData		语法数据或者语法文件路径
     * @param   pnGrammarId         语法id
     * @return
     * @n
     *	<table>
     *		<tr><td>@ref RECORDER_ERR_NONE</td><td>操作成功</td></tr>
     *		<tr><td>@ref RECORDER_ERR_ENGINE_ERROR</td><td>ASR SDK 初始化失败，失败的原因会通过 Callback_RecorderErr 回调返回</td></tr>
     *		<tr><td>@ref RECORDER_ERR_NOT_INIT</td><td>录音机没有初始化</td></tr>
     *	</table>
     */
    RECORDER_ERR_CODE HCIAPI hci_asr_recorder_load_grammar(
                                                           _OPT_ _IN_ const char * pszAsrSdkConfig,
                                                           _MUST_ _IN_ const char * pszGrammarData,
                                                           _MUST_ _OUT_ unsigned int * pnGrammarId
                                                           );
    
    /**
     * @brief	录音机SDK卸载语法
     * @param   nGrammarId         语法id
     * @return
     * @n
     *	<table>
     *		<tr><td>@ref RECORDER_ERR_NONE</td><td>操作成功</td></tr>
     *		<tr><td>@ref RECORDER_ERR_ENGINE_ERROR</td><td>ASR SDK 初始化失败，失败的原因会通过 Callback_RecorderErr 回调返回</td></tr>
     *		<tr><td>@ref RECORDER_ERR_NOT_INIT</td><td>录音机没有初始化</td></tr>
     *	</table>
     */
    RECORDER_ERR_CODE HCIAPI hci_asr_recorder_unload_grammar(
                                                             _MUST_ _IN_ unsigned int nGrammarId
                                                             );
    
    /**  
     * @brief	录音机SDK启动
     * @details
     * 配置中包含端点检测配制项
     * @note
     * 录音机仅提供对 pcm8k16bit pcm16k16bit 格式的支持
     * 如果不是录音机支持的格式，会在 Callback_RecorderEventStateChange 中触发打开设备失败的事件
     * 
     * @param	pszConfig			配置串 可通过deviceId指定录音设备，其余参数参见 hci_asr_session_start
     * @param	pszGrammarData		语法文件(只有当配置串grammarType为wordlist、jsgf或wfst时有效）
     * @return	
     * @n
     *	<table>
     *		<tr><td>@ref RECORDER_ERR_NONE</td><td>操作成功</td></tr>
     *		<tr><td>@ref RECORDER_ERR_NOT_INIT</td><td>没有初始化</td></tr>
	 *		<tr><td>@ref RECORDER_ERR_CONFIG_ITEM</td><td>配置串错误</td></tr>
     *		<tr><td>@ref RECORDER_ERR_ALREADY_BEGIN</td><td>已经开录音</td></tr>
     *	</table>
	  * @par 配置串定义：
	 * 配置串是由"字段=值"的形式给出的一个字符串，多个字段之间以','隔开。字段名不分大小写。
	 * @n@n
	 * 开启录音机相关配置：
	 *	<table>
	 *		<tr>
	 *			<td><b>字段</b></td>
	 *			<td><b>取值或示例</b></td>
	 *			<td><b>缺省值</b></td>
	 *			<td><b>含义</b></td>
	 *			<td><b>详细说明</b></td>
	 *		</tr>
	 *		<tr>
	 *			<td>realtime</td>
	 *			<td>字符串，有效值{yes, rt}</td>
	 *			<td>yes</td>
	 *			<td>实时识别模式</td>
	 *			<td>yes,实时识别模式<br/>
	 *				rt,实时反馈识别结果</td>
	 *		</tr>
	 *		<tr>
	 *			<td>continuous</td>
	 *			<td>字符串，有效值{yes, no}</td>
	 *			<td>yes</td>
	 *			<td>是否连续录音</td>
	 *			<td>yes,连续录音<br/>
	 *				no,间断录音，检测到一次端点末尾即停止，需要再次开启，才能进行录音</td>			
	 *		</tr>
	 *		<tr>
	 *			<td>audioformat</td>
	 *			<td>字符串，有效值{pcm16k16bit, pcm8k16bit}</td>
	 *			<td>pcm16k16bit</td>
	 *			<td>录音音频格式</td>
	 *			<td>pcm16k16bit,16k16bit pcm录音<br/>
	 *				pcm8k16bit,8k16bit pcm录音</td>
	 *		</tr>
	 *	</table>
	 * @n@n
	 * 另外，这里还可以传入识别的配置项，作为默认配置项。参见 hci_asr_recog() 。
	 * 如果pszConfig中没有asr识别的capkey，则仅启动录音功能，通过语音回调接口向外抛语音数据。
     */ 
    RECORDER_ERR_CODE HCIAPI hci_asr_recorder_start(
                                                    _MUST_ _IN_ const char * pszConfig,
                                                    _OPT_ _IN_ const char * pszGrammarData);
    
    /**  
     * @brief	结束录音但不识别
     * @details 此操作表示结束录音但不识别，如果希望结束录音并识别，请调用 hci_asr_recorder_stop_and_recog()
     * @return	
     * @n
     *	<table>
     *		<tr><td>@ref RECORDER_ERR_NONE</td><td>操作成功</td></tr>
     *		<tr><td>@ref RECORDER_ERR_NOT_INIT</td><td>没有初始化</td></tr>
     *		<tr><td>@ref RECORDER_ERR_NOT_BEGIN</td><td>没有开始录音</td></tr>
     *	</table>
     */ 
    RECORDER_ERR_CODE HCIAPI hci_asr_recorder_cancel();
    
    /**  
     * @brief	结束录音并启动识别
     * @details 此操作表示结束录音同时启动识别过程，如果希望结束录音时不去识别，请调用 hci_asr_recorder_cancel()
     * @return	
     * @n
     *	<table>
     *		<tr><td>@ref RECORDER_ERR_NONE</td><td>操作成功</td></tr>
     *		<tr><td>@ref RECORDER_ERR_NOT_INIT</td><td>没有初始化</td></tr>
     *		<tr><td>@ref RECORDER_ERR_NOT_BEGIN</td><td>没有开始录音</td></tr>
     *	</table>
     */ 
    RECORDER_ERR_CODE HCIAPI hci_asr_recorder_stop_and_recog();
    
    /**  
     * @brief	上传识别的确认结果
     * @return	
     * @n
     *	<table>
     *		<tr><td>@ref RECORDER_ERR_NONE</td><td>操作成功</td></tr>
     *		<tr><td>@ref RECORDER_ERR_NOT_INIT</td><td>没有初始化</td></tr>
     *		<tr><td>@ref RECORDER_ERR_OUT_OF_MEMORY</td><td>内存分配失败</td></tr>
     *	</table>
     * @note
     * iOS 平台不支持此接口，调用后不进行任何操作并返回 RECORDER_ERR_NONE
     */ 
    RECORDER_ERR_CODE HCIAPI hci_asr_recorder_confirm(
                                                      _MUST_ _IN_ ASR_CONFIRM_ITEM * psAsrConfirmItem);
    
    /**  
     * @brief	ASR 录音机反初始化
     * @return	
     * @n
     *	<table>
     *		<tr><td>@ref RECORDER_ERR_NONE</td><td>操作成功</td></tr>
     *		<tr><td>@ref RECORDER_ERR_NOT_INIT</td><td>没有初始化</td></tr>
     *	</table>
     */ 
    RECORDER_ERR_CODE HCIAPI hci_asr_recorder_release();
    
     /* @} */
    /* @} */
    //////////////////////////////////////////////////////////////////////////
    
#ifdef __cplusplus
}
#endif

#endif // _hci_cloud_asr_recorder_api_header_
