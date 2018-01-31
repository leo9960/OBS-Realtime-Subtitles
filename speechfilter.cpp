#include <windows.h>
#include <algorithm>
#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <cstdio>

#include <util/platform.h>
#include <util/util.hpp>
#include <util/threading.h>

#include <obs-module.h>
#include <media-io/audio-io.h>
#include <media-io/audio-math.h>
#include <media-io/audio-resampler.h>

#include "enum-tcspeech.hpp"

#include "hci_asr.h"
#include "common/FileReader.h"
#include "common/CommonTool.h"
#include "common/AccountInfo.h"

using namespace std;
#define do_log(level, format, ...) \
	blog(level, "[speech filter: '%s'] " format, \
			obs_source_get_name(gf->context), ##__VA_ARGS__)

#define warn(format, ...)  do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...)  do_log(LOG_INFO,    format, ##__VA_ARGS__)

#define S_GAIN_DB                      "db"
#define S_SPEECH                        "speech"
#define S_SPEECHEND                     "speechend"
#define T_(v)                           obs_module_text(v)
#define T_SPEECH			T_("speech")
#define T_SPEECHEND			T_("speechend")

#define MT_ obs_module_text
#define TEXT_GAIN_DB                   MT_("Gain.GainDB")

struct gain_data {
	obs_source_t *context;
	size_t channels;
	float multiple;
};
obs_source_t *source;
struct audio_output;
typedef struct audio_output audio_t;
struct gain_data *global_gf;
struct pthread_data {
	void*data;
	unsigned char * raw_audio_data;
	unsigned int len;
	FILE * file;
};
FILE *sptxt = NULL;
FILE *ft = NULL;
FILE *ft2 = NULL;
int global_startspeech = 0;
int global_speech_datalen = 0;
static uint8_t *global_speech_data[MAX_AV_PLANES];
int global_speech_count = 0;
obs_data_t *global_settings = {};
obs_source_t *global_source = {};
void *global_obj;
void *global_data;

bool CheckAndUpdataAuth();
bool init_speech(void *data);
static void *start_speech(void *sspthread_data);
void stop_speech();


static const char *gain_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("实时语音识别");
}

static void gain_destroy(void *data)
{

	stop_speech();
	struct gain_data *gf = (gain_data *)data;
	bfree(gf);
	//fclose(ft);
	//fclose(sptxt);
}

static void gain_update(void *data, obs_data_t *s)
{
	struct gain_data *gf = (gain_data *)data;
	double val = obs_data_get_double(s, S_GAIN_DB);
	gf->channels = audio_output_get_channels(obs_get_audio());
	gf->multiple = db_to_mul((float)val);
}

static void *gain_create(obs_data_t *settings, obs_source_t *filter)
{
	struct gain_data *gf = (gain_data *)bzalloc(sizeof(*gf));
	gf->context = filter;
	gain_update(gf, settings);
	return gf;
}
uint8_t *all_audio_data0 = NULL;
uint8_t *all_audio_data1 = NULL;
uint8_t *all_audio_data0_head = NULL;
uint8_t *all_audio_data1_head = NULL;
int count_cpynum = 0;
static struct obs_audio_data *gain_filter_audio(void *data,
	struct obs_audio_data *audio)
{
	audio_t *audiost = obs_get_audio();

	const audio_output_info * audio_info = audio_output_get_info(audiost);

	uint32_t sampleRate = audio_info->samples_per_sec;
	audio_format format = audio_info->format;
	speaker_layout speakers = audio_info->speakers;
	uint32_t frames = audio->frames;
	int count_num = 20;

	ft = fopen("./re.pcm", "ab+");
	ft2 = fopen("./re2.pcm", "ab+");


		if (global_startspeech == 1) {
			int audio_size = get_audio_size(format, speakers,frames);
			int all_data_len = count_num * (audio->frames);
			if (global_speech_datalen==0) {
				all_audio_data0 = (uint8_t*)malloc(count_num*audio_size);
				all_audio_data1 = (uint8_t*)malloc(count_num*audio_size);
				all_audio_data0_head = all_audio_data0;
				all_audio_data1_head = all_audio_data1;
			}
			if (global_speech_datalen <all_data_len) {
				fwrite(audio->data[0], 1, audio_size, ft);
				fwrite(audio->data[1], 1, audio_size, ft2);
				memcpy(&all_audio_data0 , &audio->data[0], audio_size);
				memcpy(&all_audio_data1 , &audio->data[1], audio_size);
				all_audio_data0 += audio_size;
				all_audio_data1 += audio_size;
				global_speech_datalen += (audio->frames);
				count_cpynum++;
			}
			else {
				all_audio_data0 = all_audio_data0_head;
				all_audio_data1 = all_audio_data1_head;
				FILE *test_file=NULL;
				test_file = fopen("./test_file.pcm", "ab+");
				fwrite(&all_audio_data0, 1, count_cpynum*audio_size, test_file);
				fclose(test_file);

				struct resample_info src = {};
				src.samples_per_sec = sampleRate;
				src.format = format;
				src.speakers = speakers;
				struct resample_info dst = {};
				dst.samples_per_sec = 16000;
				dst.format = AUDIO_FORMAT_16BIT;
				dst.speakers = SPEAKERS_MONO;
				uint8_t *re_audio[MAX_AV_PLANES];
				uint32_t resample_frames;
				uint64_t ts_offset;

				fseek(ft, 0L, SEEK_END);
				int len = ftell(ft);
				fseek(ft, 0L, SEEK_SET);
				fseek(ft2, 0L, SEEK_SET);
				global_speech_data[0] = (uint8_t*)malloc(len * sizeof(uint8_t));
				fread(global_speech_data[0], 1, len, ft);
				global_speech_data[1] = (uint8_t*)malloc(len * sizeof(uint8_t));
				fread(global_speech_data[1], 1, len, ft2);

				bool re_success;
				re_success = audio_resampler_resample(audio_resampler_create(&dst, &src), re_audio, &resample_frames, &ts_offset, (const uint8_t **)global_speech_data, global_speech_datalen);
				if (re_success) {

				int len = get_audio_size(dst.format, dst.speakers, resample_frames);
				pthread_data *spthread_data = new pthread_data();
				spthread_data->data = global_data;
				spthread_data->raw_audio_data = re_audio[0];
				spthread_data->len = len;
				pthread_t tid;
				pthread_create(&tid, NULL, &start_speech, spthread_data);

				global_speech_datalen = 0;
				count_cpynum = 0;
				fclose(ft);
				fclose(ft2);
				remove("./re.pcm");
				remove("./re2.pcm");
				return audio;
				}
		}
	}
	fclose(ft);
	fclose(ft2);
	return audio;
}

static void gain_defaults(obs_data_t *s)
{
}

static bool addbutton(obs_properties_t *props,
	obs_property_t *p, void *data)
{
	init_speech(data);
	return true;
}

static bool endbutton(obs_properties_t *props,
	obs_property_t *p, void *data)
{
	stop_speech();

	return true;
}
static obs_properties_t *gain_properties(void *data)
{
	obs_properties_t *ppts = obs_properties_create();


	obs_properties_add_button(ppts, S_SPEECH, T_SPEECH, addbutton);
	obs_properties_add_button(ppts, S_SPEECHEND, T_SPEECHEND, endbutton);

	UNUSED_PARAMETER(data);
	return ppts;
}

void Registerspeech_filter()
{
	struct obs_source_info speech_filter = {};

	speech_filter.id = "speech_filter",
		speech_filter.type = OBS_SOURCE_TYPE_FILTER,
		speech_filter.output_flags = OBS_SOURCE_AUDIO,
		speech_filter.get_name = gain_name,
		speech_filter.create = gain_create,
		speech_filter.destroy = gain_destroy,
		speech_filter.update = gain_update,
		speech_filter.filter_audio = gain_filter_audio,
		speech_filter.get_defaults = gain_defaults,
		speech_filter.get_properties = gain_properties,

	obs_register_source(&speech_filter);
}


bool CheckAndUpdataAuth()
{
	//获取过期时间
	int64 nExpireTime;
	int64 nCurTime = (int64)time(NULL);
	HCI_ERR_CODE errCode = hci_get_auth_expire_time(&nExpireTime);
	if (errCode == HCI_ERR_NONE)
	{
		//获取成功则判断是否过期
		if (nExpireTime > nCurTime)
		{
			//没有过期
			char format_buffer[1024];
			//int src_len = sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
			int src_len = sprintf(format_buffer, "auth can use continue\n");
			//obs_data_set_string(global_settings, S_TEXT, format_buffer);
			//gain_update(NULL, global_settings);
			return true;
		}
	}

	//获取过期时间失败或已经过期
	//手动调用更新授权
	errCode = hci_check_auth();
	if (errCode == HCI_ERR_NONE)
	{
		//更新成功
		char format_buffer[1024];
		//int src_len = sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
		int src_len = sprintf(format_buffer, "check auth success \n");
		//obs_data_set_string(global_settings, S_TEXT, format_buffer);
		//gain_update(NULL, global_settings);
		return true;
	}
	else
	{
		//更新失败
		char format_buffer[1024];
		//int src_len = sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
		int src_len = sprintf(format_buffer, "check auth return (%d:%s)\n", errCode, hci_get_error_info(errCode));
		//obs_data_set_string(global_settings, S_TEXT, format_buffer);
		//gain_update(NULL, global_settings);
		return false;
	}
}


int m_GrammarId = -1;
bool init_speech(void *data) {
	if (global_startspeech != 0) {
		return true;
	}
	remove("./re.pcm");
	remove("./re2.pcm");
	remove("./sptxt.txt");

	//memset(all_audio_data0, 0, 1747);
	//memset(all_audio_data1, 0, 1747);

	//ft = fopen("./re.pcm", "ab+");
	//FILE *sptxt = NULL;
	sptxt = fopen("./sptxt.txt", "a+");
	// 获取AccountInfo单例
	AccountInfo *account_info = AccountInfo::GetInstance();
	// 账号信息读取
	string account_info_file = "../../testdata/AccountInfo.txt";
	bool account_success = account_info->LoadFromFile(account_info_file);

	/*if (!account_success)
	{
		char format_buffer[1024];
		int buffer_len=sprintf(format_buffer, "AccountInfo read from %s failed\n", account_info_file.c_str());
		fwrite(format_buffer,1, buffer_len,sptxt);
		//obs_data_set_string(global_settings, S_TEXT, format_buffer);
		//gain_update(NULL, global_settings);
		return false;
	}
	else {
		char format_buffer[1024];
		int buffer_len=sprintf(format_buffer, "AccountInfo read from %s success\n", account_info_file.c_str());
		fwrite(format_buffer, 1, buffer_len, sptxt);
		//obs_data_set_string(global_settings, S_TEXT, format_buffer);
		//gain_update(NULL, global_settings);
		//return true;

	}*/
	
	// SYS初始化
	HCI_ERR_CODE speech_errCode = HCI_ERR_NONE;
	// 配置串是由"字段=值"的形式给出的一个字符串，多个字段之间以','隔开。字段名不分大小写。
	string speech_init_config = "";
	speech_init_config += "appKey=" + account_info->app_key();              //灵云应用序号
	speech_init_config += ",developerKey=" + account_info->developer_key(); //灵云开发者密钥
	speech_init_config += ",cloudUrl=" + account_info->cloud_url();         //灵云云服务的接口地址
	speech_init_config += ",authpath=" + account_info->auth_path();         //授权文件所在路径，保证可写
	speech_init_config += ",logfilepath=" + account_info->logfile_path();   //日志的路径
	speech_init_config += ",logfilesize=1024000,loglevel=5";
	// 其他配置使用默认值，不再添加，如果想设置可以参考开发手册
	speech_errCode = hci_init(speech_init_config.c_str());
	if (speech_errCode != HCI_ERR_NONE)
	{
		if (speech_errCode != HCI_ERR_SYS_ALREADY_INIT) {
			char format_buffer[1024];
			//int src_len = sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
			int buffer_len = sprintf(format_buffer, "hci_init return (%d:%s)\n", speech_errCode, hci_get_error_info(speech_errCode));
			fwrite(format_buffer, 1, buffer_len, sptxt);
			//obs_data_set_string(global_settings, S_TEXT, format_buffer);
			//gain_update(NULL, global_settings);
			return false;
		}
	}


	// 检测授权,必要时到云端下载授权。此处需要注意的是，这个函数只是通过检测授权是否过期来判断是否需要进行
	// 获取授权操作，如果在开发调试过程中，授权账号中新增了灵云sdk的能力，请到hci_init传入的authPath路径中
	// 删除HCI_AUTH文件。否则无法获取新的授权文件，从而无法使用新增的灵云能力。
	if (!CheckAndUpdataAuth())
	{
		hci_release();
		char format_buffer[1024];
		//int src_len = sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
		int buffer_len=sprintf(format_buffer, "CheckAndUpdateAuth failed\n");
		fwrite(format_buffer, 1, buffer_len, sptxt);
		//obs_data_set_string(global_settings, S_TEXT, format_buffer);
		//gain_update(NULL, global_settings);
		return false;
	}

	string initConfig = "initCapkeys=asr.local.freetalk;" + account_info->cap_key();
	initConfig += ",dataPath=" + account_info->data_path();
	HCI_ERR_CODE err_code = hci_asr_init(initConfig.c_str());
	if (err_code != HCI_ERR_NONE)
	{
		if (err_code != HCI_ERR_ASR_ALREADY_INIT) {
			hci_release();
			char format_buffer[200];
			int buffer_len=sprintf(format_buffer, "hci_asr_init return (%d:%s) \n", err_code, hci_get_error_info(err_code));
			fwrite(format_buffer, 1, buffer_len, sptxt);
			//obs_data_set_string(global_settings, S_TEXT, format_buffer);
			//gain_update(NULL, global_settings);
			return false;
		}
	}
	string strSessionConfig = "capKey=asr.local.freetalk";
	//此处也可以传入其他配置，参见开发手册，此处其他配置采用默认值
	err_code = hci_asr_session_start(strSessionConfig.c_str(), &m_GrammarId);
	if (err_code != HCI_ERR_NONE)
	{
		hci_release();
		char format_buffer[200];
		int buffer_len=sprintf(format_buffer, "hci_asr_session_start return (%d:%s)\n", err_code, hci_get_error_info(err_code));
		fwrite(format_buffer, 1, buffer_len, sptxt);
		//obs_data_set_string(global_settings, S_TEXT, format_buffer);
		//gain_update(NULL, global_settings);
	}
	if (sptxt == NULL) {
		sptxt = fopen("./sptxt.txt", "a+");
	}
	char format_buffer[200] = { 0 };
	int buffer_len = sprintf(format_buffer, "识别引擎初始化成功\n");
	fwrite(format_buffer, 1, buffer_len, sptxt);
	global_startspeech = 1;
	fclose(sptxt);
	return true;
}

static void *start_speech(void *sspthread_data) {

	pthread_data *param = (pthread_data *)sspthread_data;
	void*data = param->data;
	unsigned char * raw_audio_data = param->raw_audio_data;
	unsigned int len = param->len;
	//FILE *sptxt = param->file;

	sptxt = fopen("./sptxt.txt", "a+");

	ASR_RECOG_RESULT asrResult;
	string  recog_config = "audioFormat=pcm16k16bit,encode=none";
	HCI_ERR_CODE err_code = hci_asr_recog(m_GrammarId, raw_audio_data, len, recog_config.c_str(), NULL, &asrResult);
	global_speech_count++;

	if (err_code == HCI_ERR_NONE)
	{
		// 输出识别结果
		for (int index = 0; index < (int)asrResult.uiResultItemCount; ++index)
		{
			ASR_RECOG_RESULT_ITEM& item = asrResult.psResultItemList[index];
			char format_buffer[1024] = { 0 };
			//int src_len = sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
			/*if (src_len >= 1024) {
				char format_buffer[1024] = { 0 };
				src_len = 0;
			}*/
			int buffer_len = sprintf(format_buffer, "%s", item.pszResult);
			fwrite(format_buffer, 1, buffer_len, sptxt);
			//obs_data_set_string(global_settings, S_TEXT, format_buffer);
			//gain_update(NULL, global_settings);
		}
		// 释放识别结果
		hci_asr_free_recog_result(&asrResult);
	}
	else
	{
		char format_buffer[200];
		int buffer_len = sprintf(format_buffer, "%d:hci_asr_recog return (%d:%s)\n", global_speech_count, err_code, hci_get_error_info(err_code));
		fwrite(format_buffer, 1, buffer_len, sptxt);
		//obs_data_set_string(global_settings, S_TEXT, format_buffer);
		//gain_update(NULL, global_settings);
	}
	fclose(sptxt);
	pthread_exit(0);
	return NULL;
}

void stop_speech() {
	global_startspeech = 0;
	HCI_ERR_CODE errCode = HCI_ERR_NONE;
	//关闭会话
	errCode = hci_asr_session_stop(m_GrammarId);  //sessionID：已经开启的会话的session
						      //终止ASR能力
	errCode = hci_asr_release();
	//关闭灵云系统
	errCode = hci_release();
	//FILE *sptxt = NULL;
	//sptxt = fopen("./sptxt.txt", "a+");

	ft = fopen("./re.pcm", "ab+");
	fclose(ft);
	ft2 = fopen("./re2.pcm", "ab+");
	fclose(ft2);

	sptxt= fopen("./sptxt.txt", "a+");
	char format_buffer[200];
	int buffer_len = sprintf(format_buffer, "实时语音识别结束\n");
	fwrite(format_buffer, 1, buffer_len, sptxt);
	fclose(sptxt);

}
