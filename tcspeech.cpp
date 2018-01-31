#include <windows.h>
#include <gdiplus.h>
#include <algorithm>
#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <cstdio>

#include <obs.h>
#include <obs-module.h>

#include <util/circlebuf.h>
#include <util/darray.h>
#include <util/platform.h>
#include <util/util.hpp>
#include <util/platform.h>
#include <util/windows/HRError.hpp>
#include <util/windows/ComPtr.hpp>
#include <util/windows/WinHandle.hpp>
#include <util/windows/CoTaskMemPtr.hpp>
#include <util/threading.h>

#include <media-io/audio-io.h>
#include <media-io/audio-math.h>
#include <media-io/audio-resampler.h>

#include <graphics/matrix4.h>
#include <graphics/math-defs.h>
#include "enum-tcspeech.hpp"

#include "hci_asr.h"
#include "common/FileReader.h"
#include "common/CommonTool.h"
#include "common/AccountInfo.h"


using namespace std;
using namespace Gdiplus;

#define ACTUALLY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#define do_log(level, format, ...) \
	blog(level, "[compressor: '%s'] " format, \
			obs_source_get_name(cd->context), ##__VA_ARGS__)

#define warn(format, ...)  do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...)  do_log(LOG_INFO,    format, ##__VA_ARGS__)

#ifdef _DEBUG
#define debug(format, ...) do_log(LOG_DEBUG,   format, ##__VA_ARGS__)
#else
#define debug(format, ...)
#endif
#define OPT_DEVICE_ID         "device_id"
#define OPT_USE_DEVICE_TIMING "use_device_timing"

struct pthread_data {
	void*data;
	unsigned char * raw_audio_data;
	unsigned int len;
};
int global_startspeech = 0;
int global_speech_datalen = 0;
static uint8_t *global_speech_data;
static void GettcspeechDefaults(obs_data_t *settings);
bool CheckAndUpdataAuth();
static void *start_speech(void *sspthread_data);
void stop_speech();
int global_speech_count = 0;
obs_data_t *global_settings = {};
obs_source_t *global_source = {};
void *global_obj;
void *global_data;
struct audio_output;
typedef struct audio_output audio_t;
// Fix inconsistent defs of speaker_surround between avutil & tcspeech
#define KSAUDIO_SPEAKER_2POINT1 (KSAUDIO_SPEAKER_STEREO|SPEAKER_LOW_FREQUENCY)
#define KSAUDIO_SPEAKER_SURROUND_AVUTIL \
	(KSAUDIO_SPEAKER_STEREO|SPEAKER_FRONT_CENTER)
#define KSAUDIO_SPEAKER_4POINT1 (KSAUDIO_SPEAKER_QUAD|SPEAKER_LOW_FREQUENCY)

#define warning(format, ...) blog(LOG_WARNING, "[%s] " format, \
		obs_source_get_name(source), ##__VA_ARGS__)

#define warn_stat(call) \
	do { \
		if (stat != Ok) \
			warning("%s: %s failed (%d)", __FUNCTION__, call, \
					(int)stat); \
	} while (false)

#ifndef clamp
#define clamp(val, min_val, max_val) \
	if (val < min_val) val = min_val; \
	else if (val > max_val) val = max_val;
#endif

#define MIN_SIZE_CX 2
#define MIN_SIZE_CY 2
#define MAX_SIZE_CX 16384
#define MAX_SIZE_CY 16384

#define MAX_AREA (4096LL * 4096LL)

/* ------------------------------------------------------------------------- */

#define S_FONT                          "font"
#define S_USE_FILE                      "read_from_file"
#define S_FILE                          "file"
#define S_TEXT                          "text"
#define S_COLOR                         "color"
#define S_GRADIENT                      "gradient"
#define S_GRADIENT_COLOR                "gradient_color"
#define S_GRADIENT_DIR                  "gradient_dir"
#define S_GRADIENT_OPACITY              "gradient_opacity"
#define S_ALIGN                         "align"
#define S_VALIGN                        "valign"
#define S_OPACITY                       "opacity"
#define S_BKCOLOR                       "bk_color"
#define S_BKOPACITY                     "bk_opacity"
#define S_VERTICAL                      "vertical"
#define S_OUTLINE                       "outline"
#define S_OUTLINE_SIZE                  "outline_size"
#define S_OUTLINE_COLOR                 "outline_color"
#define S_OUTLINE_OPACITY               "outline_opacity"
#define S_CHATLOG_MODE                  "chatlog"
#define S_CHATLOG_LINES                 "chatlog_lines"
#define S_EXTENTS                       "extents"
#define S_EXTENTS_WRAP                  "extents_wrap"
#define S_EXTENTS_CX                    "extents_cx"
#define S_EXTENTS_CY                    "extents_cy"
#define S_SPEECH                        "speech"
#define S_SPEECHEND                     "speechend"

#define S_ALIGN_LEFT                    "left"
#define S_ALIGN_CENTER                  "center"
#define S_ALIGN_RIGHT                   "right"

#define S_VALIGN_TOP                    "top"
#define S_VALIGN_CENTER                 S_ALIGN_CENTER
#define S_VALIGN_BOTTOM                 "bottom"

#define T_(v)                           obs_module_text(v)
#define T_FONT                          T_("Font")
#define T_USE_FILE                      T_("ReadFromFile")
#define T_FILE                          T_("TextFile")
#define T_TEXT                          T_("Text")
#define T_COLOR                         T_("Color")
#define T_GRADIENT                      T_("Gradient")
#define T_GRADIENT_COLOR                T_("Gradient.Color")
#define T_GRADIENT_DIR                  T_("Gradient.Direction")
#define T_GRADIENT_OPACITY              T_("Gradient.Opacity")
#define T_ALIGN                         T_("Alignment")
#define T_VALIGN                        T_("VerticalAlignment")
#define T_OPACITY                       T_("Opacity")
#define T_BKCOLOR                       T_("BkColor")
#define T_BKOPACITY                     T_("BkOpacity")
#define T_VERTICAL                      T_("Vertical")
#define T_OUTLINE                       T_("Outline")
#define T_OUTLINE_SIZE                  T_("Outline.Size")
#define T_OUTLINE_COLOR                 T_("Outline.Color")
#define T_OUTLINE_OPACITY               T_("Outline.Opacity")
#define T_CHATLOG_MODE                  T_("ChatlogMode")
#define T_CHATLOG_LINES                 T_("ChatlogMode.Lines")
#define T_EXTENTS                       T_("UseCustomExtents")
#define T_EXTENTS_WRAP                  T_("UseCustomExtents.Wrap")
#define T_EXTENTS_CX                    T_("Width")
#define T_EXTENTS_CY                    T_("Height")
#define T_SPEECH			T_("speech")
#define T_SPEECHEND			T_("speechend")

#define T_FILTER_TEXT_FILES             T_("Filter.TextFiles")
#define T_FILTER_ALL_FILES              T_("Filter.AllFiles")

#define T_ALIGN_LEFT                    T_("Alignment.Left")
#define T_ALIGN_CENTER                  T_("Alignment.Center")
#define T_ALIGN_RIGHT                   T_("Alignment.Right")

#define T_VALIGN_TOP                    T_("VerticalAlignment.Top")
#define T_VALIGN_CENTER                 T_ALIGN_CENTER
#define T_VALIGN_BOTTOM                 T_("VerticalAlignment.Bottom")

/* ------------------------------------------------------------------------- */
#define obs_data_get_uint32 (uint32_t)obs_data_get_int
#define BUFFER_TIME_100NS (5*10000000)
#define RECONNECT_INTERVAL 3000
#define S_RATIO                         "ratio"
#define S_THRESHOLD                     "threshold"
#define S_ATTACK_TIME                   "attack_time"
#define S_RELEASE_TIME                  "release_time"
#define S_OUTPUT_GAIN                   "output_gain"
#define S_SIDECHAIN_SOURCE              "sidechain_source"

#define MT_ obs_module_text
#define TEXT_RATIO                      MT_("Compressor.Ratio")
#define TEXT_THRESHOLD                  MT_("Compressor.Threshold")
#define TEXT_ATTACK_TIME                MT_("Compressor.AttackTime")
#define TEXT_RELEASE_TIME               MT_("Compressor.ReleaseTime")
#define TEXT_OUTPUT_GAIN                MT_("Compressor.OutputGain")
#define TEXT_SIDECHAIN_SOURCE           MT_("Compressor.SidechainSource")

#define MIN_RATIO                       1.0f
#define MAX_RATIO                       32.0f
#define MIN_THRESHOLD_DB                -60.0f
#define MAX_THRESHOLD_DB                0.0f
#define MIN_OUTPUT_GAIN_DB              -32.0f
#define MAX_OUTPUT_GAIN_DB              32.0f
#define MIN_ATK_RLS_MS                  1
#define MAX_RLS_MS                      1000
#define MAX_ATK_MS                      500
#define DEFAULT_AUDIO_BUF_MS            10

#define MS_IN_S                         1000
#define MS_IN_S_F                       ((float)MS_IN_S)
#define set_vis(var, val, show) \
	do { \
		p = obs_properties_get(props, val); \
		obs_property_set_visible(p, var == show); \
	} while (false)

#define GET_ARRAY_LEN(array,len){len = (sizeof(array) / sizeof(array[0]));}
bool init_speech(void *data);
string ftoa(float *value)
{
	std::ostringstream o;
	if (!(o << value))
		return "";
	return o.str();
}
static inline DWORD get_alpha_val(uint32_t opacity)
{
	return ((opacity * 255 / 100) & 0xFF) << 24;
}

static inline DWORD calc_color(uint32_t color, uint32_t opacity)
{
	return color & 0xFFFFFF | get_alpha_val(opacity);
}

static inline wstring to_wide(const char *utf8)
{
	wstring text;

	size_t len = os_utf8_to_wcs(utf8, 0, nullptr, 0);
	text.resize(len);
	if (len)
		os_utf8_to_wcs(utf8, 0, &text[0], len + 1);

	return text;
}

static inline uint32_t rgb_to_bgr(uint32_t rgb)
{
	return ((rgb & 0xFF) << 16) | (rgb & 0xFF00) | ((rgb & 0xFF0000) >> 16);
}

/* ------------------------------------------------------------------------- */

template<typename T, typename T2, BOOL WINAPI deleter(T2)> class GDIObj {
	T obj = nullptr;

	inline GDIObj &Replace(T obj_)
	{
		if (obj) deleter(obj);
		obj = obj_;
		return *this;
	}

public:
	inline GDIObj() {}
	inline GDIObj(T obj_) : obj(obj_) {}
	inline ~GDIObj() { deleter(obj); }

	inline T operator=(T obj_) { Replace(obj_); return obj; }

	inline operator T() const { return obj; }

	inline bool operator==(T obj_) const { return obj == obj_; }
	inline bool operator!=(T obj_) const { return obj != obj_; }
};

using HDCObj = GDIObj<HDC, HDC, DeleteDC>;
using HFONTObj = GDIObj<HFONT, HGDIOBJ, DeleteObject>;
using HBITMAPObj = GDIObj<HBITMAP, HGDIOBJ, DeleteObject>;

/* ------------------------------------------------------------------------- */

enum class Align {
	Left,
	Center,
	Right
};

enum class VAlign {
	Top,
	Center,
	Bottom
};

struct compressor_data {
	obs_source_t *context;
	float *envelope_buf;
	size_t envelope_buf_len;

	float ratio;
	float threshold;
	float attack_gain;
	float release_gain;
	float output_gain;

	size_t num_channels;
	size_t sample_rate;
	float envelope;
	float slope;

	char *sidechain_name;
	obs_weak_source_t *weak_sidechain;
	pthread_mutex_t sidechain_mutex;
	struct circlebuf sidechain_data[MAX_AUDIO_CHANNELS];
	float *sidechain_buf[MAX_AUDIO_CHANNELS];
	size_t max_sidechain_frames;

	uint64_t sidechain_check_time;
};
static void sidechain_capture(void *param, obs_source_t *source,
	const struct audio_data *audio_data, bool muted)
{
	struct compressor_data *cd = (compressor_data *)param;

	UNUSED_PARAMETER(source);

	pthread_mutex_lock(&cd->sidechain_mutex);

	if (cd->max_sidechain_frames < audio_data->frames)
		cd->max_sidechain_frames = audio_data->frames;

	size_t expected_size = cd->max_sidechain_frames * sizeof(float);

	if (!expected_size)
		goto unlock;

	if (cd->sidechain_data[0].size > expected_size * 2) {
		for (size_t i = 0; i < cd->num_channels; i++) {
			circlebuf_pop_front(&cd->sidechain_data[i], NULL,
				expected_size);
		}
	}

	if (muted) {
		for (size_t i = 0; i < cd->num_channels; i++) {
			circlebuf_push_back_zero(&cd->sidechain_data[i],
				audio_data->frames * sizeof(float));
		}
	}
	else {
		for (size_t i = 0; i < cd->num_channels; i++) {
			circlebuf_push_back(&cd->sidechain_data[i],
				audio_data->data[i],
				audio_data->frames * sizeof(float));
		}
	}

unlock:
	pthread_mutex_unlock(&cd->sidechain_mutex);
}

static inline void swap_sidechain(struct compressor_data *cd, const char *name)
{

	obs_source_add_audio_capture_callback(global_source,
				sidechain_capture, cd);

	const char *parent_name = obs_source_get_name(global_source);

	blog(LOG_INFO, "Source '%s' now has sidechain "
				"compression from source '%s'",
				parent_name, cd->sidechain_name);

	cd->max_sidechain_frames =
		cd->sample_rate * DEFAULT_AUDIO_BUF_MS / MS_IN_S;

}

static inline float gain_coefficient(uint32_t sample_rate, float time)
{
	return (float)exp(-1.0f / (sample_rate * time));
}

static void resize_env_buffer(struct compressor_data *cd, size_t len)
{
	cd->envelope_buf_len = len;
	cd->envelope_buf = (float*)brealloc(cd->envelope_buf, len * sizeof(float));

	for (size_t i = 0; i < cd->num_channels; i++)
		cd->sidechain_buf[i] = (float*)brealloc(cd->sidechain_buf[i],
			len * sizeof(float));
}

struct tcspeechSource {
	ComPtr<IMMDevice>           device;
	ComPtr<IAudioClient>        client;
	ComPtr<IAudioCaptureClient> capture;
	ComPtr<IAudioRenderClient>  render;

	obs_source_t                *source;
	string                      device_id;
	string                      device_name;
	bool                        isInputDevice;
	bool                        useDeviceTiming = false;
	bool                        isDefaultDevice = false;

	bool                        reconnecting = false;
	bool                        previouslyFailed = false;
	WinHandle                   reconnectThread;

	bool                        active = false;
	WinHandle                   captureThread;

	WinHandle                   stopSignal;
	WinHandle                   receiveSignal;

	speaker_layout              speakers;
	audio_format                format;
	uint32_t                    sampleRate;

	gs_texture_t *tex = nullptr;

	HDCObj hdc;
	Graphics graphics;

	HFONTObj hfont;
	unique_ptr<Font> font;

	bool read_from_file = false;
	time_t file_timestamp = 0;
	bool update_file = false;
	float update_time_elapsed = 0.0f;

	wstring text;
	wstring face;
	int face_size = 0;
	uint32_t color = 0xFFFFFF;
	uint32_t color2 = 0xFFFFFF;
	float gradient_dir = 0;
	uint32_t opacity = 100;
	uint32_t opacity2 = 100;
	uint32_t bk_color = 0;
	uint32_t bk_opacity = 0;
	Align align = Align::Left;
	VAlign valign = VAlign::Top;
	bool gradient = false;
	bool bold = false;
	bool italic = false;
	bool underline = false;
	bool strikeout = false;
	bool vertical = false;

	bool use_outline = false;
	float outline_size = 0.0f;
	uint32_t outline_color = 0;
	uint32_t outline_opacity = 100;

	bool use_extents = false;
	bool wrap = false;
	uint32_t extents_cx = 0;
	uint32_t extents_cy = 0;

	bool chatlog_mode = false;
	int chatlog_lines = 6;

	static DWORD WINAPI ReconnectThread(LPVOID param);
	static DWORD WINAPI CaptureThread(LPVOID param);

	bool ProcessCaptureData();

	inline void Start();
	inline void Stop();
	void Reconnect();

	bool InitDevice(IMMDeviceEnumerator *enumerator);
	void InitName();
	void InitClient();
	void InitRender();
	void InitFormat(WAVEFORMATEX *wfex);
	void InitCapture();
	void Initialize();

	bool TryInitialize();

	void UpdateSettings(obs_data_t *settings);

public:
	uint32_t cx = 0;
	uint32_t cy = 0;
	string file;
	tcspeechSource(obs_data_t *settings, obs_source_t *source_, bool input);
	inline ~tcspeechSource();
	void Update(obs_data_t *settings);
	void UpdateFont();
	void GetStringFormat(StringFormat &format);
	void RemoveNewlinePadding(const StringFormat &format, RectF &box);
	void CalculateTextSizes(const StringFormat &format,
		RectF &bounding_box, SIZE &text_size);
	void RenderOutlineText(Graphics &graphics,
		const GraphicsPath &path,
		const Brush &brush);
	void RenderText();
	void LoadFileText();
	void changeText(const char * string);

	const char *GetMainString(const char *str);

	inline void Tick(float seconds);
	inline void Render(gs_effect_t *effect);
};

static time_t get_modified_timestamp(const char *filename)
{
	struct stat stats;
	if (os_stat(filename, &stats) != 0)
		return -1;
	return stats.st_mtime;
}

tcspeechSource::tcspeechSource(obs_data_t *settings, obs_source_t *source_,
	bool input)
	: source(source_),
	isInputDevice(input),
	hdc(CreateCompatibleDC(nullptr)),
	graphics(hdc)
{
	UpdateSettings(settings);

	stopSignal = CreateEvent(nullptr, true, false, nullptr);
	if (!stopSignal.Valid())
		throw "Could not create stop signal";

	receiveSignal = CreateEvent(nullptr, false, false, nullptr);
	if (!receiveSignal.Valid())
		throw "Could not create receive signal";

	Start();

	obs_source_update(source, settings);
}

inline void tcspeechSource::Start()
{
	if (!TryInitialize()) {
		blog(LOG_INFO, "[tcspeechSource::tcspeechSource] "
			"Device '%s' not found.  Waiting for device",
			device_id.c_str());
		Reconnect();
	}
}

inline void tcspeechSource::Stop()
{
	SetEvent(stopSignal);

	if (active) {
		blog(LOG_INFO, "tcspeech: Device '%s' Terminated",
			device_name.c_str());
		WaitForSingleObject(captureThread, INFINITE);
	}

	if (reconnecting)
		WaitForSingleObject(reconnectThread, INFINITE);

	ResetEvent(stopSignal);
}

inline tcspeechSource::~tcspeechSource()
{
	Stop();
	if (tex) {
		obs_enter_graphics();
		gs_texture_destroy(tex);
		obs_leave_graphics();
	}
}

void tcspeechSource::UpdateSettings(obs_data_t *settings)
{
	device_id = obs_data_get_string(settings, OPT_DEVICE_ID);
	useDeviceTiming = obs_data_get_bool(settings, OPT_USE_DEVICE_TIMING);
	isDefaultDevice = _strcmpi(device_id.c_str(), "default") == 0;
}

void tcspeechSource::UpdateFont()
{
	hfont = nullptr;
	font.reset(nullptr);

	LOGFONT lf = {};
	lf.lfHeight = face_size;
	lf.lfWeight = bold ? FW_BOLD : FW_DONTCARE;
	lf.lfItalic = italic;
	lf.lfUnderline = underline;
	lf.lfStrikeOut = strikeout;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfCharSet = DEFAULT_CHARSET;

	if (!face.empty()) {
		wcscpy(lf.lfFaceName, face.c_str());
		hfont = CreateFontIndirect(&lf);
	}

	if (!hfont) {
		wcscpy(lf.lfFaceName, L"Arial");
		hfont = CreateFontIndirect(&lf);
	}

	if (hfont)
		font.reset(new Font(hdc, hfont));
}

void tcspeechSource::GetStringFormat(StringFormat &format)
{
	UINT flags = StringFormatFlagsNoFitBlackBox |
		StringFormatFlagsMeasureTrailingSpaces;

	if (vertical)
		flags |= StringFormatFlagsDirectionVertical |
		StringFormatFlagsDirectionRightToLeft;

	format.SetFormatFlags(flags);
	format.SetTrimming(StringTrimmingWord);

	switch (align) {
	case Align::Left:
		if (vertical)
			format.SetLineAlignment(StringAlignmentFar);
		else
			format.SetAlignment(StringAlignmentNear);
		break;
	case Align::Center:
		if (vertical)
			format.SetLineAlignment(StringAlignmentCenter);
		else
			format.SetAlignment(StringAlignmentCenter);
		break;
	case Align::Right:
		if (vertical)
			format.SetLineAlignment(StringAlignmentNear);
		else
			format.SetAlignment(StringAlignmentFar);
	}

	switch (valign) {
	case VAlign::Top:
		if (vertical)
			format.SetAlignment(StringAlignmentNear);
		else
			format.SetLineAlignment(StringAlignmentNear);
		break;
	case VAlign::Center:
		if (vertical)
			format.SetAlignment(StringAlignmentCenter);
		else
			format.SetLineAlignment(StringAlignmentCenter);
		break;
	case VAlign::Bottom:
		if (vertical)
			format.SetAlignment(StringAlignmentFar);
		else
			format.SetLineAlignment(StringAlignmentFar);
	}
}

/* GDI+ treats '\n' as an extra character with an actual render size when
* calculating the texture size, so we have to calculate the size of '\n' to
* remove the padding.  Because we always add a newline to the string, we
* also remove the extra unused newline. */
void tcspeechSource::RemoveNewlinePadding(const StringFormat &format, RectF &box)
{
	RectF before;
	RectF after;
	Status stat;

	stat = graphics.MeasureString(L"W", 2, font.get(), PointF(0.0f, 0.0f),
		&format, &before);
	warn_stat("MeasureString (without newline)");

	stat = graphics.MeasureString(L"W\n", 3, font.get(), PointF(0.0f, 0.0f),
		&format, &after);
	warn_stat("MeasureString (with newline)");

	float offset_cx = after.Width - before.Width;
	float offset_cy = after.Height - before.Height;

	if (!vertical) {
		if (offset_cx >= 1.0f)
			offset_cx -= 1.0f;

		if (valign == VAlign::Center)
			box.Y -= offset_cy * 0.5f;
		else if (valign == VAlign::Bottom)
			box.Y -= offset_cy;
	}
	else {
		if (offset_cy >= 1.0f)
			offset_cy -= 1.0f;

		if (align == Align::Center)
			box.X -= offset_cx * 0.5f;
		else if (align == Align::Right)
			box.X -= offset_cx;
	}

	box.Width -= offset_cx;
	box.Height -= offset_cy;
}

void tcspeechSource::CalculateTextSizes(const StringFormat &format,
	RectF &bounding_box, SIZE &text_size)
{
	RectF layout_box;
	RectF temp_box;
	Status stat;

	if (!text.empty()) {
		if (use_extents && wrap) {
			layout_box.X = layout_box.Y = 0;
			layout_box.Width = float(extents_cx);
			layout_box.Height = float(extents_cy);

			if (use_outline) {
				layout_box.Width -= outline_size;
				layout_box.Height -= outline_size;
			}

			stat = graphics.MeasureString(text.c_str(),
				(int)text.size() + 1, font.get(),
				layout_box, &format,
				&bounding_box);
			warn_stat("MeasureString (wrapped)");

			temp_box = bounding_box;
		}
		else {
			stat = graphics.MeasureString(text.c_str(),
				(int)text.size() + 1, font.get(),
				PointF(0.0f, 0.0f), &format,
				&bounding_box);
			warn_stat("MeasureString (non-wrapped)");

			temp_box = bounding_box;

			bounding_box.X = 0.0f;
			bounding_box.Y = 0.0f;

			RemoveNewlinePadding(format, bounding_box);

			if (use_outline) {
				bounding_box.Width += outline_size;
				bounding_box.Height += outline_size;
			}
		}
	}

	if (vertical) {
		if (bounding_box.Width < face_size) {
			text_size.cx = face_size;
			bounding_box.Width = float(face_size);
		}
		else {
			text_size.cx = LONG(bounding_box.Width + EPSILON);
		}

		text_size.cy = LONG(bounding_box.Height + EPSILON);
	}
	else {
		if (bounding_box.Height < face_size) {
			text_size.cy = face_size;
			bounding_box.Height = float(face_size);
		}
		else {
			text_size.cy = LONG(bounding_box.Height + EPSILON);
		}

		text_size.cx = LONG(bounding_box.Width + EPSILON);
	}

	if (use_extents) {
		text_size.cx = extents_cx;
		text_size.cy = extents_cy;
	}

	text_size.cx += text_size.cx % 2;
	text_size.cy += text_size.cy % 2;

	int64_t total_size = int64_t(text_size.cx) * int64_t(text_size.cy);

	/* GPUs typically have texture size limitations */
	clamp(text_size.cx, MIN_SIZE_CX, MAX_SIZE_CX);
	clamp(text_size.cy, MIN_SIZE_CY, MAX_SIZE_CY);

	/* avoid taking up too much VRAM */
	if (total_size > MAX_AREA) {
		if (text_size.cx > text_size.cy)
			text_size.cx = (LONG)MAX_AREA / text_size.cy;
		else
			text_size.cy = (LONG)MAX_AREA / text_size.cx;
	}

	/* the internal text-rendering bounding box for is reset to
	* its internal value in case the texture gets cut off */
	bounding_box.Width = temp_box.Width;
	bounding_box.Height = temp_box.Height;
}

void tcspeechSource::RenderOutlineText(Graphics &graphics,
	const GraphicsPath &path,
	const Brush &brush)
{
	DWORD outline_rgba = calc_color(outline_color, outline_opacity);
	Status stat;

	Pen pen(Color(outline_rgba), outline_size);
	stat = pen.SetLineJoin(LineJoinRound);
	warn_stat("pen.SetLineJoin");

	stat = graphics.DrawPath(&pen, &path);
	warn_stat("graphics.DrawPath");

	stat = graphics.FillPath(&brush, &path);
	warn_stat("graphics.FillPath");
}

void tcspeechSource::RenderText()
{
	StringFormat format(StringFormat::GenericTypographic());
	Status stat;

	RectF box;
	SIZE size;

	GetStringFormat(format);
	CalculateTextSizes(format, box, size);

	unique_ptr<uint8_t> bits(new uint8_t[size.cx * size.cy * 4]);
	Bitmap bitmap(size.cx, size.cy, 4 * size.cx, PixelFormat32bppARGB,
		bits.get());

	Graphics graphics_bitmap(&bitmap);
	LinearGradientBrush brush(RectF(0, 0, (float)size.cx, (float)size.cy),
		Color(calc_color(color, opacity)),
		Color(calc_color(color2, opacity2)),
		gradient_dir, 1);
	DWORD full_bk_color = bk_color & 0xFFFFFF;

	if (!text.empty() || use_extents)
		full_bk_color |= get_alpha_val(bk_opacity);

	if ((size.cx > box.Width || size.cy > box.Height) && !use_extents) {
		stat = graphics_bitmap.Clear(Color(0));
		warn_stat("graphics_bitmap.Clear");

		SolidBrush bk_brush = Color(full_bk_color);
		stat = graphics_bitmap.FillRectangle(&bk_brush, box);
		warn_stat("graphics_bitmap.FillRectangle");
	}
	else {
		stat = graphics_bitmap.Clear(Color(full_bk_color));
		warn_stat("graphics_bitmap.Clear");
	}

	graphics_bitmap.SetTextRenderingHint(TextRenderingHintAntiAlias);
	graphics_bitmap.SetCompositingMode(CompositingModeSourceOver);
	graphics_bitmap.SetSmoothingMode(SmoothingModeAntiAlias);

	if (!text.empty()) {
		if (use_outline) {
			box.Offset(outline_size / 2, outline_size / 2);

			FontFamily family;
			GraphicsPath path;

			font->GetFamily(&family);
			stat = path.AddString(text.c_str(), (int)text.size(),
				&family, font->GetStyle(),
				font->GetSize(), box, &format);
			warn_stat("path.AddString");

			RenderOutlineText(graphics_bitmap, path, brush);
		}
		else {
			stat = graphics_bitmap.DrawString(text.c_str(),
				(int)text.size(), font.get(),
				box, &format, &brush);
			warn_stat("graphics_bitmap.DrawString");
		}
	}

	if (!tex || (LONG)cx != size.cx || (LONG)cy != size.cy) {
		obs_enter_graphics();
		if (tex)
			gs_texture_destroy(tex);

		const uint8_t *data = (uint8_t*)bits.get();
		tex = gs_texture_create(size.cx, size.cy, GS_BGRA, 1, &data,
			GS_DYNAMIC);

		obs_leave_graphics();

		cx = (uint32_t)size.cx;
		cy = (uint32_t)size.cy;

	}
	else if (tex) {
		obs_enter_graphics();
		gs_texture_set_image(tex, bits.get(), size.cx * 4, false);
		obs_leave_graphics();
	}
}

const char *tcspeechSource::GetMainString(const char *str)
{
	if (!str)
		return "";
	if (!chatlog_mode || !chatlog_lines)
		return str;

	int lines = chatlog_lines;
	size_t len = strlen(str);
	if (!len)
		return str;

	const char *temp = str + len;

	while (temp != str) {
		temp--;

		if (temp[0] == '\n' && temp[1] != 0) {
			if (!--lines)
				break;
		}
	}

	return *temp == '\n' ? temp + 1 : temp;
}

void tcspeechSource::LoadFileText()
{
	BPtr<char> file_text = os_quick_read_utf8_file(file.c_str());
	text = to_wide(GetMainString(file_text));

	if (!text.empty() && text.back() != '\n')
		text.push_back('\n');
}

void tcspeechSource::changeText(const char * string)
{
	text = to_wide(string);
	if (!text.empty() && text.back() != '\n')
		text.push_back('\n');
}


FILE *fp = NULL;
static audio_output_callback_t *saveaudio(void *param, size_t mix_idx, struct audio_data *data) {

	//pBes = new uint8_t[MAX_AV_PLANES];   //处理图像的指针  
	//memcpy(pBes, data->data, MAX_AV_PLANES*2);
	/*for (size_t i = 0; i < MAX_AV_PLANES; i++) {
		uint8_t* pBes = data->data[i];//指向类型的指针 
		fwrite(&pBes, 1, sizeof(pBes) / sizeof(pBes[0]), fp);
	}*/
	fwrite(&data->frames, 1, sizeof(uint32_t), fp);
	//fclose(fp);
	/*for (size_t i = 0; i < MAX_AV_PLANES; i++) {
		uint8_t* pBes = NULL;//指向类型的指针    
		pBes = new uint8_t[8];   //处理图像的指针  
		memcpy(pBes, data->data[i], 8);
		fwrite(pBes, sizeof(uint8_t)*8, 1, fp);
		uint8_t *m_pData;
		m_pData = (uint8_t *)malloc(8);
		memcpy(m_pData, &(data->data[i]),8);
		//int audio_len=sizeof(audio_sample);
		//fputs((const char *)&audio_len, fp);
		//fwrite(&audio_len, 1, 1, fp);
	}*/

	//fclose(fp);
	return NULL;
}

void tcspeechSource::Update(obs_data_t *settings)
{
	string newDevice = obs_data_get_string(settings, OPT_DEVICE_ID);
	bool restart = newDevice.compare(device_id) != 0;

	if (restart)
		Stop();

	UpdateSettings(settings);

	if (restart)
		Start();
	obs_data_t *s = settings;
	const char *new_text = obs_data_get_string(s, S_TEXT);
	obs_data_t *font_obj = obs_data_get_obj(s, S_FONT);
	const char *align_str = obs_data_get_string(s, S_ALIGN);
	const char *valign_str = obs_data_get_string(s, S_VALIGN);
	uint32_t new_color = obs_data_get_uint32(s, S_COLOR);
	uint32_t new_opacity = obs_data_get_uint32(s, S_OPACITY);
	bool gradient = obs_data_get_bool(s, S_GRADIENT);
	uint32_t new_color2 = obs_data_get_uint32(s, S_GRADIENT_COLOR);
	uint32_t new_opacity2 = obs_data_get_uint32(s, S_GRADIENT_OPACITY);
	float new_grad_dir = (float)obs_data_get_double(s, S_GRADIENT_DIR);
	bool new_vertical = obs_data_get_bool(s, S_VERTICAL);
	bool new_outline = obs_data_get_bool(s, S_OUTLINE);
	uint32_t new_o_color = obs_data_get_uint32(s, S_OUTLINE_COLOR);
	uint32_t new_o_opacity = obs_data_get_uint32(s, S_OUTLINE_OPACITY);
	uint32_t new_o_size = obs_data_get_uint32(s, S_OUTLINE_SIZE);
	bool new_use_file = obs_data_get_bool(s, S_USE_FILE);
	const char *new_file = obs_data_get_string(s, S_FILE);
	bool new_chat_mode = obs_data_get_bool(s, S_CHATLOG_MODE);
	int new_chat_lines = (int)obs_data_get_int(s, S_CHATLOG_LINES);
	bool new_extents = obs_data_get_bool(s, S_EXTENTS);
	bool new_extents_wrap = obs_data_get_bool(s, S_EXTENTS_WRAP);
	uint32_t n_extents_cx = obs_data_get_uint32(s, S_EXTENTS_CX);
	uint32_t n_extents_cy = obs_data_get_uint32(s, S_EXTENTS_CY);

	const char *font_face = obs_data_get_string(font_obj, "face");
	int font_size = (int)obs_data_get_int(font_obj, "size");
	int64_t font_flags = obs_data_get_int(font_obj, "flags");
	bool new_bold = (font_flags & OBS_FONT_BOLD) != 0;
	bool new_italic = (font_flags & OBS_FONT_ITALIC) != 0;
	bool new_underline = (font_flags & OBS_FONT_UNDERLINE) != 0;
	bool new_strikeout = (font_flags & OBS_FONT_STRIKEOUT) != 0;

	uint32_t new_bk_color = obs_data_get_uint32(s, S_BKCOLOR);
	uint32_t new_bk_opacity = obs_data_get_uint32(s, S_BKOPACITY);

	/* ----------------------------- */

	wstring new_face = to_wide(font_face);

	if (new_face != face ||
		face_size != font_size ||
		new_bold != bold ||
		new_italic != italic ||
		new_underline != underline ||
		new_strikeout != strikeout) {

		face = new_face;
		face_size = font_size;
		bold = new_bold;
		italic = new_italic;
		underline = new_underline;
		strikeout = new_strikeout;

		UpdateFont();
	}

	/* ----------------------------- */

	new_color = rgb_to_bgr(new_color);
	new_color2 = rgb_to_bgr(new_color2);
	new_o_color = rgb_to_bgr(new_o_color);
	new_bk_color = rgb_to_bgr(new_bk_color);

	color = new_color;
	opacity = new_opacity;
	color2 = new_color2;
	opacity2 = new_opacity2;
	gradient_dir = new_grad_dir;
	vertical = new_vertical;

	bk_color = new_bk_color;
	bk_opacity = new_bk_opacity;
	use_extents = new_extents;
	wrap = new_extents_wrap;
	extents_cx = n_extents_cx;
	extents_cy = n_extents_cy;

	if (!gradient) {
		color2 = color;
		opacity2 = opacity;
	}

	read_from_file = new_use_file;

	chatlog_mode = new_chat_mode;
	chatlog_lines = new_chat_lines;

	if (read_from_file) {
		file = new_file;
		file_timestamp = get_modified_timestamp(new_file);
		LoadFileText();

	}
	else {
		text = to_wide(GetMainString(new_text));

		/* all text should end with newlines due to the fact that GDI+
		* treats strings without newlines differently in terms of
		* render size */
		if (!text.empty())
			text.push_back('\n');
	}

	use_outline = new_outline;
	outline_color = new_o_color;
	outline_opacity = new_o_opacity;
	outline_size = roundf(float(new_o_size));

	if (strcmp(align_str, S_ALIGN_CENTER) == 0)
		align = Align::Center;
	else if (strcmp(align_str, S_ALIGN_RIGHT) == 0)
		align = Align::Right;
	else
		align = Align::Left;

	if (strcmp(valign_str, S_VALIGN_CENTER) == 0)
		valign = VAlign::Center;
	else if (strcmp(valign_str, S_VALIGN_BOTTOM) == 0)
		valign = VAlign::Bottom;
	else
		valign = VAlign::Top;

	RenderText();
	update_time_elapsed = 0.0f;

	/* ----------------------------- */

	obs_data_release(font_obj);
}

inline void tcspeechSource::Tick(float seconds)
{
	if (!read_from_file)
		return;

	update_time_elapsed += seconds;

	if (update_time_elapsed >= 1.0f) {
		time_t t = get_modified_timestamp(file.c_str());
		update_time_elapsed = 0.0f;

		if (update_file) {
			LoadFileText();
			RenderText();
			update_file = false;
		}

		if (file_timestamp != t) {
			file_timestamp = t;
			update_file = true;
		}
	}
}

inline void tcspeechSource::Render(gs_effect_t *effect)
{
	if (!tex)
		return;

	gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"), tex);
	gs_draw_sprite(tex, 0, cx, cy);
}

bool tcspeechSource::InitDevice(IMMDeviceEnumerator *enumerator)
{
	HRESULT res;

	if (isDefaultDevice) {
		res = enumerator->GetDefaultAudioEndpoint(
			isInputDevice ? eCapture : eRender,
			isInputDevice ? eCommunications : eConsole,
			device.Assign());
	}
	else {
		wchar_t *w_id;
		os_utf8_to_wcs_ptr(device_id.c_str(), device_id.size(), &w_id);

		res = enumerator->GetDevice(w_id, device.Assign());

		bfree(w_id);
	}

	return SUCCEEDED(res);
}


void tcspeechSource::InitClient()
{
	CoTaskMemPtr<WAVEFORMATEX> wfex;
	HRESULT                    res;
	DWORD                      flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;

	res = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
		nullptr, (void**)client.Assign());
	if (FAILED(res))
		throw HRError("Failed to activate client context", res);

	res = client->GetMixFormat(&wfex);
	if (FAILED(res))
		throw HRError("Failed to get mix format", res);

	InitFormat(wfex);

	if (!isInputDevice)
		flags |= AUDCLNT_STREAMFLAGS_LOOPBACK;

	res = client->Initialize(
		AUDCLNT_SHAREMODE_SHARED, flags,
		BUFFER_TIME_100NS, 0, wfex, nullptr);
	if (FAILED(res))
		throw HRError("Failed to get initialize audio client", res);
}

void tcspeechSource::InitRender()
{
	CoTaskMemPtr<WAVEFORMATEX> wfex;
	HRESULT                    res;
	LPBYTE                     buffer;
	UINT32                     frames;
	ComPtr<IAudioClient>       client;

	res = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
		nullptr, (void**)client.Assign());
	if (FAILED(res))
		throw HRError("Failed to activate client context", res);

	res = client->GetMixFormat(&wfex);
	if (FAILED(res))
		throw HRError("Failed to get mix format", res);

	res = client->Initialize(
		AUDCLNT_SHAREMODE_SHARED, 0,
		BUFFER_TIME_100NS, 0, wfex, nullptr);
	if (FAILED(res))
		throw HRError("Failed to get initialize audio client", res);

	/* Silent loopback fix. Prevents audio stream from stopping and */
	/* messing up timestamps and other weird glitches during silence */
	/* by playing a silent sample all over again. */

	res = client->GetBufferSize(&frames);
	if (FAILED(res))
		throw HRError("Failed to get buffer size", res);

	res = client->GetService(__uuidof(IAudioRenderClient),
		(void**)render.Assign());
	if (FAILED(res))
		throw HRError("Failed to get render client", res);

	res = render->GetBuffer(frames, &buffer);
	if (FAILED(res))
		throw HRError("Failed to get buffer", res);

	memset(buffer, 0, frames*wfex->nBlockAlign);

	render->ReleaseBuffer(frames, 0);
}

static speaker_layout ConvertSpeakerLayout(DWORD layout, WORD channels)
{
	switch (layout) {
	case KSAUDIO_SPEAKER_QUAD:             return SPEAKERS_QUAD;
	case KSAUDIO_SPEAKER_2POINT1:          return SPEAKERS_2POINT1;
	case KSAUDIO_SPEAKER_4POINT1:          return SPEAKERS_4POINT1;
	case KSAUDIO_SPEAKER_SURROUND_AVUTIL:  return SPEAKERS_SURROUND;
	case KSAUDIO_SPEAKER_5POINT1:          return SPEAKERS_5POINT1_SURROUND;
	case KSAUDIO_SPEAKER_5POINT1_SURROUND: return SPEAKERS_5POINT1;
	case KSAUDIO_SPEAKER_7POINT1:          return SPEAKERS_7POINT1_SURROUND;
	case KSAUDIO_SPEAKER_7POINT1_SURROUND: return SPEAKERS_7POINT1;
	}

	return (speaker_layout)channels;
}

void tcspeechSource::InitFormat(WAVEFORMATEX *wfex)
{
	DWORD layout = 0;

	if (wfex->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		WAVEFORMATEXTENSIBLE *ext = (WAVEFORMATEXTENSIBLE*)wfex;
		layout = ext->dwChannelMask;
	}

	/* tcspeech is always float */
	sampleRate = wfex->nSamplesPerSec;
	format = AUDIO_FORMAT_FLOAT;
	speakers = ConvertSpeakerLayout(layout, wfex->nChannels);
}

void tcspeechSource::InitCapture()
{
	HRESULT res = client->GetService(__uuidof(IAudioCaptureClient),
		(void**)capture.Assign());
	if (FAILED(res))
		throw HRError("Failed to create capture context", res);

	res = client->SetEventHandle(receiveSignal);
	if (FAILED(res))
		throw HRError("Failed to set event handle", res);

	captureThread = CreateThread(nullptr, 0,
		tcspeechSource::CaptureThread, this,
		0, nullptr);
	if (!captureThread.Valid())
		throw "Failed to create capture thread";

	client->Start();
	active = true;

	blog(LOG_INFO, "tcspeech: Device '%s' initialized", device_name.c_str());
}

void tcspeechSource::Initialize()
{
	ComPtr<IMMDeviceEnumerator> enumerator;
	HRESULT res;

	res = CoCreateInstance(__uuidof(MMDeviceEnumerator),
		nullptr, CLSCTX_ALL,
		__uuidof(IMMDeviceEnumerator),
		(void**)enumerator.Assign());
	if (FAILED(res))
		throw HRError("Failed to create enumerator", res);

	if (!InitDevice(enumerator))
		return;

	device_name = GetDeviceName(device);

	InitClient();
	if (!isInputDevice) InitRender();
	InitCapture();
}

bool tcspeechSource::TryInitialize()
{
	try {
		Initialize();

	}
	catch (HRError error) {
		if (previouslyFailed)
			return active;

		blog(LOG_WARNING, "[tcspeechSource::TryInitialize]:[%s] %s: %lX",
			device_name.empty() ?
			device_id.c_str() : device_name.c_str(),
			error.str, error.hr);

	}
	catch (const char *error) {
		if (previouslyFailed)
			return active;

		blog(LOG_WARNING, "[tcspeechSource::TryInitialize]:[%s] %s",
			device_name.empty() ?
			device_id.c_str() : device_name.c_str(),
			error);
	}

	previouslyFailed = !active;
	return active;
}

void tcspeechSource::Reconnect()
{
	reconnecting = true;
	reconnectThread = CreateThread(nullptr, 0,
		tcspeechSource::ReconnectThread, this,
		0, nullptr);

	if (!reconnectThread.Valid())
		blog(LOG_WARNING, "[tcspeechSource::Reconnect] "
			"Failed to initialize reconnect thread: %lu",
			GetLastError());
}

static inline bool WaitForSignal(HANDLE handle, DWORD time)
{
	return WaitForSingleObject(handle, time) != WAIT_TIMEOUT;
}


DWORD WINAPI tcspeechSource::ReconnectThread(LPVOID param)
{
	tcspeechSource *source = (tcspeechSource*)param;

	os_set_thread_name("tcspeech: reconnect thread");

	while (!WaitForSignal(source->stopSignal, RECONNECT_INTERVAL)) {
		if (source->TryInitialize())
			break;
	}

	source->reconnectThread = nullptr;
	source->reconnecting = false;
	return 0;
}
bool tcspeechSource::ProcessCaptureData()
{
	HRESULT res;
	LPBYTE  buffer;
	UINT32  frames;
	DWORD   flags;
	UINT64  pos, ts;
	UINT    captureSize = 0;
	FILE *ft = NULL;
	ft = fopen("./re.pcm", "ab+");
	while (true) {
		res = capture->GetNextPacketSize(&captureSize);

		if (FAILED(res)) {
			if (res != AUDCLNT_E_DEVICE_INVALIDATED)
				blog(LOG_WARNING,
					"[tcspeechSource::GetCaptureData]"
					" capture->GetNextPacketSize"
					" failed: %lX", res);
			return false;
		}

		if (!captureSize)
			break;

		res = capture->GetBuffer(&buffer, &frames, &flags, &pos, &ts);
		if (FAILED(res)) {
			if (res != AUDCLNT_E_DEVICE_INVALIDATED)
				blog(LOG_WARNING,
					"[tcspeechSource::GetCaptureData]"
					" capture->GetBuffer"
					" failed: %lX", res);
			return false;
		}

		obs_source_audio data = {};
		data.data[0] = (const uint8_t*)buffer;
		data.frames = (uint32_t)frames;
		data.speakers = speakers;
		data.samples_per_sec = sampleRate;
		data.format = format;
		data.timestamp = useDeviceTiming ?
			ts * 100 : os_gettime_ns();

		if (!useDeviceTiming){
			data.timestamp -= (uint64_t)frames * 1000000000ULL /
				(uint64_t)sampleRate;
		}
		obs_source_output_audio(source, &data);
		struct resample_info src = {};
		src.samples_per_sec = data.samples_per_sec;
		src.format = data.format;
		src.speakers = data.speakers;
		struct resample_info dst = {};
		dst.samples_per_sec = 16000;
		dst.format = AUDIO_FORMAT_16BIT;
		dst.speakers = SPEAKERS_MONO;
		uint8_t *re_audio[MAX_AV_PLANES];
		uint32_t resample_frames;
		uint64_t ts_offset;

		bool re_success;
		re_success=audio_resampler_resample(audio_resampler_create(&dst, &src), re_audio, &resample_frames, &ts_offset, (const uint8_t *const *)data.data, data.frames);
		if (re_success) {
			if (global_startspeech == 1) {
				int audio_size = get_audio_size(dst.format, dst.speakers, resample_frames);
				int all_data_len = 100 * audio_size;
				if (global_speech_datalen <all_data_len) {
					fwrite(re_audio[0], 1, audio_size, ft);
					global_speech_datalen += audio_size;
				}
				else {
					fseek(ft, 0L, SEEK_END);
					int len = ftell(ft);
					fseek(ft, 0L, SEEK_SET);
					global_speech_data= (uint8_t*)malloc(len * sizeof(uint8_t));
					fread(global_speech_data, 1, len, ft);
					pthread_data *spthread_data = new pthread_data();
					spthread_data->data = global_data;
					spthread_data->raw_audio_data = global_speech_data;
					spthread_data->len = len;
					pthread_t tid;
					pthread_create(&tid, NULL, &start_speech, spthread_data);
					global_speech_datalen=0;
					fclose(ft);
					remove("./re.pcm");
					ft = fopen("./re.pcm", "ab+");
				}
			}
		}
		capture->ReleaseBuffer(frames);
	}
	fclose(ft);

	return true;
}

static inline bool WaitForCaptureSignal(DWORD numSignals, const HANDLE *signals,
	DWORD duration)
{
	DWORD ret;
	ret = WaitForMultipleObjects(numSignals, signals, false, duration);

	return ret == WAIT_OBJECT_0 || ret == WAIT_TIMEOUT;
}

DWORD WINAPI tcspeechSource::CaptureThread(LPVOID param)
{
	tcspeechSource *source = (tcspeechSource*)param;
	bool         reconnect = false;

	/* Output devices don't signal, so just make it check every 10 ms */
	DWORD        dur = source->isInputDevice ? INFINITE : 10;

	HANDLE sigs[2] = {
		source->receiveSignal,
		source->stopSignal
	};

	os_set_thread_name("tcspeech: capture thread");

	while (WaitForCaptureSignal(2, sigs, dur)) {
		if (!source->ProcessCaptureData()) {
			reconnect = true;
			break;
		}
	}

	source->client->Stop();

	source->captureThread = nullptr;
	source->active = false;

	if (reconnect) {
		blog(LOG_INFO, "Device '%s' invalidated.  Retrying",
			source->device_name.c_str());
		source->Reconnect();
	}

	return 0;
}

/* ------------------------------------------------------------------------- */

static const char *GettcspeechInputName(void*)
{
	return obs_module_text("AudioInput");
}

static const char *GettcspeechOutputName(void*)
{
	return obs_module_text("AudioOutput");
}

static void GettcspeechDefaultsInput(obs_data_t *settings)
{
	obs_data_set_default_string(settings, OPT_DEVICE_ID, "default");
	obs_data_set_default_bool(settings, OPT_USE_DEVICE_TIMING, false);

	obs_data_t *font_obj = obs_data_create();
	obs_data_set_default_string(font_obj, "face", "Arial");
	obs_data_set_default_int(font_obj, "size", 36);

	obs_data_set_default_obj(settings, S_FONT, font_obj);
	obs_data_set_default_string(settings, S_ALIGN, S_ALIGN_LEFT);
	obs_data_set_default_string(settings, S_VALIGN, S_VALIGN_TOP);
	obs_data_set_default_int(settings, S_COLOR, 0xFFFFFF);
	obs_data_set_default_int(settings, S_OPACITY, 100);
	obs_data_set_default_int(settings, S_GRADIENT_COLOR, 0xFFFFFF);
	obs_data_set_default_int(settings, S_GRADIENT_OPACITY, 100);
	obs_data_set_default_double(settings, S_GRADIENT_DIR, 90.0);
	obs_data_set_default_int(settings, S_BKCOLOR, 0x000000);
	obs_data_set_default_int(settings, S_BKOPACITY, 0);
	obs_data_set_default_int(settings, S_OUTLINE_SIZE, 2);
	obs_data_set_default_int(settings, S_OUTLINE_COLOR, 0xFFFFFF);
	obs_data_set_default_int(settings, S_OUTLINE_OPACITY, 100);
	obs_data_set_default_int(settings, S_CHATLOG_LINES, 6);
	obs_data_set_default_bool(settings, S_EXTENTS_WRAP, true);
	obs_data_set_default_int(settings, S_EXTENTS_CX, 100);
	obs_data_set_default_int(settings, S_EXTENTS_CY, 100);

	obs_data_release(font_obj);
}

static void GettcspeechDefaultsOutput(obs_data_t *settings)
{
	obs_data_set_default_string(settings, OPT_DEVICE_ID, "default");
	obs_data_set_default_bool(settings, OPT_USE_DEVICE_TIMING, true);

	obs_data_t *font_obj = obs_data_create();
	obs_data_set_default_string(font_obj, "face", "Arial");
	obs_data_set_default_int(font_obj, "size", 36);

	obs_data_set_default_obj(settings, S_FONT, font_obj);
	obs_data_set_default_string(settings, S_ALIGN, S_ALIGN_LEFT);
	obs_data_set_default_string(settings, S_VALIGN, S_VALIGN_TOP);
	obs_data_set_default_int(settings, S_COLOR, 0xFFFFFF);
	obs_data_set_default_int(settings, S_OPACITY, 100);
	obs_data_set_default_int(settings, S_GRADIENT_COLOR, 0xFFFFFF);
	obs_data_set_default_int(settings, S_GRADIENT_OPACITY, 100);
	obs_data_set_default_double(settings, S_GRADIENT_DIR, 90.0);
	obs_data_set_default_int(settings, S_BKCOLOR, 0x000000);
	obs_data_set_default_int(settings, S_BKOPACITY, 0);
	obs_data_set_default_int(settings, S_OUTLINE_SIZE, 2);
	obs_data_set_default_int(settings, S_OUTLINE_COLOR, 0xFFFFFF);
	obs_data_set_default_int(settings, S_OUTLINE_OPACITY, 100);
	obs_data_set_default_int(settings, S_CHATLOG_LINES, 6);
	obs_data_set_default_bool(settings, S_EXTENTS_WRAP, true);
	obs_data_set_default_int(settings, S_EXTENTS_CX, 100);
	obs_data_set_default_int(settings, S_EXTENTS_CY, 100);

	obs_data_release(font_obj);
}

static void *CreatetcspeechSource(obs_data_t *settings, obs_source_t *source,
	bool input)
{
	global_settings = settings;
	global_source = source;
	try {
		return new tcspeechSource(settings, source, input);
	}
	catch (const char *error) {
		blog(LOG_ERROR, "[CreatetcspeechSource] %s", error);
	}

	return nullptr;
}
static void *CreatetcspeechInput(obs_data_t *settings, obs_source_t *source)
{
	return CreatetcspeechSource(settings, source, true);
}

static void *CreatetcspeechOutput(obs_data_t *settings, obs_source_t *source)
{
	return CreatetcspeechSource(settings, source, false);
}

static void DestroytcspeechSource(void *obj)
{
	stop_speech();
	delete static_cast<tcspeechSource*>(obj);
}



static void UpdatetcspeechSource(void *obj, obs_data_t *settings)
{
	global_obj = obj;
	static_cast<tcspeechSource*>(obj)->Update(settings);

}

static ULONG_PTR gdip_token = 0;
static bool use_file_changed(obs_properties_t *props, obs_property_t *p,
	obs_data_t *s)
{
	bool use_file = obs_data_get_bool(s, S_USE_FILE);

	set_vis(use_file, S_TEXT, false);
	set_vis(use_file, S_FILE, true);
	return true;
}

static bool outline_changed(obs_properties_t *props, obs_property_t *p,
	obs_data_t *s)
{
	bool outline = obs_data_get_bool(s, S_OUTLINE);

	set_vis(outline, S_OUTLINE_SIZE, true);
	set_vis(outline, S_OUTLINE_COLOR, true);
	set_vis(outline, S_OUTLINE_OPACITY, true);
	return true;
}

static bool chatlog_mode_changed(obs_properties_t *props, obs_property_t *p,
	obs_data_t *s)
{
	bool chatlog_mode = obs_data_get_bool(s, S_CHATLOG_MODE);

	set_vis(chatlog_mode, S_CHATLOG_LINES, true);
	return true;
}

static bool gradient_changed(obs_properties_t *props, obs_property_t *p,
	obs_data_t *s)
{
	bool gradient = obs_data_get_bool(s, S_GRADIENT);

	set_vis(gradient, S_GRADIENT_COLOR, true);
	set_vis(gradient, S_GRADIENT_OPACITY, true);
	set_vis(gradient, S_GRADIENT_DIR, true);
	return true;
}

static bool addbutton(obs_properties_t *props,
	obs_property_t *p, void *data)
{
	init_speech(data);
	//obs_data_set_string(global_settings, S_TEXT, "success");
	//tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
	//s->Update(global_settings);
	return true;
}

static bool endbutton(obs_properties_t *props,
	obs_property_t *p, void *data)
{
	stop_speech();
	obs_data_set_string(global_settings, S_TEXT, "实时语音识别结束");
	tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
	s->Update(global_settings);

	return true;
}

static bool checkmixer(obs_properties_t *props,
	obs_property_t *p, void *data)
{
	
	char format_buffer[200];
	sprintf(format_buffer, "\n");
	obs_data_set_string(global_settings, S_TEXT, format_buffer);
	tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
	s->Update(global_settings);

	return true;
}

static bool extents_modified(obs_properties_t *props, obs_property_t *p,
	obs_data_t *s)
{
	bool use_extents = obs_data_get_bool(s, S_EXTENTS);

	set_vis(use_extents, S_EXTENTS_WRAP, true);
	set_vis(use_extents, S_EXTENTS_CX, true);
	set_vis(use_extents, S_EXTENTS_CY, true);
	return true;
}

#undef set_vis

static obs_properties_t *GettcspeechProperties(bool input, void *data)
{
	//speech
	obs_properties_t *props = obs_properties_create();
	vector<AudioDeviceInfo> devices;

	obs_property_t *device_prop = obs_properties_add_list(props,
		OPT_DEVICE_ID, obs_module_text("Device"),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

	GettcspeechAudioDevices(devices, input);

	if (devices.size())
		obs_property_list_add_string(device_prop,
			obs_module_text("Default"), "default");

	for (size_t i = 0; i < devices.size(); i++) {
		AudioDeviceInfo &device = devices[i];
		obs_property_list_add_string(device_prop,
			device.name.c_str(), device.id.c_str());
	}

	obs_properties_add_bool(props, OPT_USE_DEVICE_TIMING,
		obs_module_text("UseDeviceTiming"));

	//text
	global_data = data;
	tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
	string path;

	obs_property_t *p;

	obs_properties_add_font(props, S_FONT, T_FONT);

	p = obs_properties_add_bool(props, S_USE_FILE, T_USE_FILE);
	obs_property_set_modified_callback(p, use_file_changed);

	string filter;
	filter += T_FILTER_TEXT_FILES;
	filter += " (*.txt);;";
	filter += T_FILTER_ALL_FILES;
	filter += " (*.*)";

	if (s && !s->file.empty()) {
		const char *slash;

		path = s->file;
		replace(path.begin(), path.end(), '\\', '/');
		slash = strrchr(path.c_str(), '/');
		if (slash)
			path.resize(slash - path.c_str() + 1);
	}

	obs_properties_add_text(props, S_TEXT, T_TEXT, OBS_TEXT_MULTILINE);
	obs_properties_add_path(props, S_FILE, T_FILE, OBS_PATH_FILE,
		filter.c_str(), path.c_str());

	obs_properties_add_button(props, S_SPEECH, T_SPEECH, addbutton);
	obs_properties_add_button(props, S_SPEECHEND, T_SPEECHEND, endbutton);
	obs_properties_add_button(props, "checkmixer", "checkmixer", checkmixer);
	obs_properties_add_bool(props, S_VERTICAL, T_VERTICAL);
	obs_properties_add_color(props, S_COLOR, T_COLOR);
	obs_properties_add_int_slider(props, S_OPACITY, T_OPACITY, 0, 100, 1);

	p = obs_properties_add_bool(props, S_GRADIENT, T_GRADIENT);
	obs_property_set_modified_callback(p, gradient_changed);

	obs_properties_add_color(props, S_GRADIENT_COLOR, T_GRADIENT_COLOR);
	obs_properties_add_int_slider(props, S_GRADIENT_OPACITY,
		T_GRADIENT_OPACITY, 0, 100, 1);
	obs_properties_add_float_slider(props, S_GRADIENT_DIR,
		T_GRADIENT_DIR, 0, 360, 0.1);

	obs_properties_add_color(props, S_BKCOLOR, T_BKCOLOR);
	obs_properties_add_int_slider(props, S_BKOPACITY, T_BKOPACITY,
		0, 100, 1);

	p = obs_properties_add_list(props, S_ALIGN, T_ALIGN,
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, T_ALIGN_LEFT, S_ALIGN_LEFT);
	obs_property_list_add_string(p, T_ALIGN_CENTER, S_ALIGN_CENTER);
	obs_property_list_add_string(p, T_ALIGN_RIGHT, S_ALIGN_RIGHT);

	p = obs_properties_add_list(props, S_VALIGN, T_VALIGN,
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, T_VALIGN_TOP, S_VALIGN_TOP);
	obs_property_list_add_string(p, T_VALIGN_CENTER, S_VALIGN_CENTER);
	obs_property_list_add_string(p, T_VALIGN_BOTTOM, S_VALIGN_BOTTOM);

	p = obs_properties_add_bool(props, S_OUTLINE, T_OUTLINE);
	obs_property_set_modified_callback(p, outline_changed);

	obs_properties_add_int(props, S_OUTLINE_SIZE, T_OUTLINE_SIZE, 1, 20, 1);
	obs_properties_add_color(props, S_OUTLINE_COLOR, T_OUTLINE_COLOR);
	obs_properties_add_int_slider(props, S_OUTLINE_OPACITY,
		T_OUTLINE_OPACITY, 0, 100, 1);

	p = obs_properties_add_bool(props, S_CHATLOG_MODE, T_CHATLOG_MODE);
	obs_property_set_modified_callback(p, chatlog_mode_changed);

	obs_properties_add_int(props, S_CHATLOG_LINES, T_CHATLOG_LINES,
		1, 1000, 1);

	p = obs_properties_add_bool(props, S_EXTENTS, T_EXTENTS);
	obs_property_set_modified_callback(p, extents_modified);

	obs_properties_add_int(props, S_EXTENTS_CX, T_EXTENTS_CX, 32, 8000, 1);
	obs_properties_add_int(props, S_EXTENTS_CY, T_EXTENTS_CY, 32, 8000, 1);
	obs_properties_add_bool(props, S_EXTENTS_WRAP, T_EXTENTS_WRAP);

	return props;
}

static obs_properties_t *GettcspeechPropertiesInput(void *data)
{
	return GettcspeechProperties(true, reinterpret_cast<tcspeechSource*>(data));
}

static obs_properties_t *GettcspeechPropertiesOutput(void *data)
{
	return GettcspeechProperties(false, reinterpret_cast<tcspeechSource*>(data));
}

struct gain_data {
	obs_source_t *context;
	size_t channels;
	float multiple;
};

string uitoa(uint8_t *value)
{
	std::ostringstream o;
	if (!(o << value))
		return "";
	return o.str();
}


void RegistertcspeechInput()
{
	obs_source_info info = {};
	info.id = "tcspeech_input_capture";
	info.type = OBS_SOURCE_TYPE_INPUT;
	info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO |
		OBS_SOURCE_DO_NOT_DUPLICATE |
		OBS_SOURCE_DO_NOT_SELF_MONITOR;
	info.get_name = GettcspeechInputName;
	info.create = CreatetcspeechInput;
	info.destroy = DestroytcspeechSource;
	info.update = UpdatetcspeechSource;
	info.get_defaults = GettcspeechDefaultsInput;
	info.get_properties = GettcspeechPropertiesInput;
	//info.audio_render = saveaudio;
	info.get_width = [](void *data)
	{
		return reinterpret_cast<tcspeechSource*>(data)->cx;
	};
	info.get_height = [](void *data)
	{
		return reinterpret_cast<tcspeechSource*>(data)->cy;
	};
	info.video_tick = [](void *data, float seconds)
	{
		reinterpret_cast<tcspeechSource*>(data)->Tick(seconds);
	};
	info.video_render = [](void *data, gs_effect_t *effect)
	{
		reinterpret_cast<tcspeechSource*>(data)->Render(effect);
	};

	obs_register_source(&info);

	const GdiplusStartupInput gdip_input;
	GdiplusStartup(&gdip_token, &gdip_input, nullptr);
}

void RegistertcspeechOutput()
{
	obs_source_info info = {};
	info.id = "tcspeech_output_capture";
	info.type = OBS_SOURCE_TYPE_INPUT;
	info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO |
		OBS_SOURCE_DO_NOT_DUPLICATE |
		OBS_SOURCE_DO_NOT_SELF_MONITOR;
	info.get_name = GettcspeechOutputName;
	info.create = CreatetcspeechOutput;
	info.destroy = DestroytcspeechSource;
	info.update = UpdatetcspeechSource;
	info.get_defaults = GettcspeechDefaultsOutput;
	info.get_properties = GettcspeechPropertiesOutput;
	//info.audio_render = saveaudio;
	info.get_width = [](void *data)
	{
		return reinterpret_cast<tcspeechSource*>(data)->cx;
	};
	info.get_height = [](void *data)
	{
		return reinterpret_cast<tcspeechSource*>(data)->cy;
	};
	info.video_tick = [](void *data, float seconds)
	{
		reinterpret_cast<tcspeechSource*>(data)->Tick(seconds);
	};
	info.video_render = [](void *data, gs_effect_t *effect)
	{
		reinterpret_cast<tcspeechSource*>(data)->Render(effect);
	};

	obs_register_source(&info);

	const GdiplusStartupInput gdip_input;
	GdiplusStartup(&gdip_token, &gdip_input, nullptr);
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
			int src_len = sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
			sprintf(format_buffer+src_len, "auth can use continue\n");
			obs_data_set_string(global_settings, S_TEXT, format_buffer);
			tcspeechSource *s = reinterpret_cast<tcspeechSource*>(global_data);
			s->Update(global_settings);
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
		int src_len = sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
		sprintf(format_buffer+src_len, "check auth success \n");
		obs_data_set_string(global_settings, S_TEXT, format_buffer);
		tcspeechSource *s = reinterpret_cast<tcspeechSource*>(global_data);
		s->Update(global_settings);
		return true;
	}
	else
	{
		//更新失败
		char format_buffer[1024];
		int src_len = sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
		sprintf(format_buffer+src_len, "check auth return (%d:%s)\n", errCode, hci_get_error_info(errCode));
		obs_data_set_string(global_settings, S_TEXT, format_buffer);
		tcspeechSource *s = reinterpret_cast<tcspeechSource*>(global_data);
		s->Update(global_settings);
		return false;
	}
}


int m_GrammarId = -1;
bool init_speech(void *data) {
	if (global_startspeech != 0) {
		return true;
	}
	remove("./re.pcm");
	// 获取AccountInfo单例
	AccountInfo *account_info = AccountInfo::GetInstance();
	// 账号信息读取
	string account_info_file = "../../testdata/AccountInfo.txt";//此处需要修改用户账户信息文件的位置
	bool account_success = account_info->LoadFromFile(account_info_file);
	
	if (!account_success)
	{
		char format_buffer[1024];
		sprintf(format_buffer, "AccountInfo read from %s failed\n", account_info_file.c_str());
		obs_data_set_string(global_settings, S_TEXT, format_buffer);
		tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
		s->Update(global_settings);
		return false;
	}
	else {
		char format_buffer[1024];
		sprintf(format_buffer, "AccountInfo read from %s success\n", account_info_file.c_str());
		obs_data_set_string(global_settings, S_TEXT, format_buffer);
		tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
		s->Update(global_settings);
		//return true;

	}
	
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
			int src_len=sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
			sprintf(format_buffer+ src_len, "hci_init return (%d:%s)\n", speech_errCode, hci_get_error_info(speech_errCode));
			obs_data_set_string(global_settings, S_TEXT, format_buffer);
			tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
			s->Update(global_settings);
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
		int src_len = sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
		sprintf(format_buffer+src_len, "CheckAndUpdateAuth failed\n");
		obs_data_set_string(global_settings, S_TEXT, format_buffer);
		tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
		s->Update(global_settings);
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
			sprintf(format_buffer, "hci_asr_init return (%d:%s) \n", err_code, hci_get_error_info(err_code));
			obs_data_set_string(global_settings, S_TEXT, format_buffer);
			tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
			s->Update(global_settings);
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
		sprintf(format_buffer, "hci_asr_session_start return (%d:%s)\n", err_code, hci_get_error_info(err_code));
		obs_data_set_string(global_settings, S_TEXT, format_buffer);
		tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
		s->Update(global_settings);
	}
	global_startspeech = 1;
	return true;
}

static void *start_speech(void *sspthread_data) {
	pthread_data *param = (pthread_data *)sspthread_data;
	void*data = param->data;
	unsigned char * raw_audio_data=param->raw_audio_data;
	unsigned int len=param->len;

	ASR_RECOG_RESULT asrResult;
	string  recog_config = "audioFormat=pcm16k16bit,encode=none";
	HCI_ERR_CODE err_code = hci_asr_recog(m_GrammarId, raw_audio_data,len, recog_config.c_str(), NULL, &asrResult);
	global_speech_count++;

	if (err_code == HCI_ERR_NONE)
	{
		// 输出识别结果
		for (int index = 0; index < (int)asrResult.uiResultItemCount; ++index)
		{
			ASR_RECOG_RESULT_ITEM& item = asrResult.psResultItemList[index];
			char format_buffer[1024] = { 0 };
			int src_len = sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
			if (src_len >= 1024) {
				char format_buffer[1024] = { 0 };
				src_len = 0;
			}
			sprintf(format_buffer+src_len, "%s",item.pszResult);
			obs_data_set_string(global_settings, S_TEXT, format_buffer);
			tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
			s->Update(global_settings);
		}
		// 释放识别结果
		hci_asr_free_recog_result(&asrResult);
	}
	else if (err_code == HCI_ERR_ASR_REALTIME_END) {
		err_code = hci_asr_recog(m_GrammarId, NULL, 0, NULL, NULL, &asrResult);
		for (int index = 0; index < (int)asrResult.uiResultItemCount; ++index)
		{
			ASR_RECOG_RESULT_ITEM& item = asrResult.psResultItemList[index];

			char format_buffer[1024] = { 0 };
			int src_len = sprintf(format_buffer, "%s", obs_data_get_string(global_settings, S_TEXT));
			if (src_len >= 1024) {
				char format_buffer[1024] = { 0 };
				src_len = 0;
			}
			sprintf(format_buffer + src_len, "%s", item.pszResult);
			obs_data_set_string(global_settings, S_TEXT, format_buffer);
			tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
			s->Update(global_settings);
		}
		// 释放识别结果
		hci_asr_free_recog_result(&asrResult);
	}
	else if (err_code == HCI_ERR_ASR_REALTIME_WAITING)
	{
		
	}
	else
	{
		char format_buffer[200];
		sprintf(format_buffer, "%d:hci_asr_recog return (%d:%s)\n", global_speech_count,err_code, hci_get_error_info(err_code));
		obs_data_set_string(global_settings, S_TEXT, format_buffer);
		tcspeechSource *s = reinterpret_cast<tcspeechSource*>(data);
		s->Update(global_settings);
	}
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
}
