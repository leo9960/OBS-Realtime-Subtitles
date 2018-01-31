#include <obs-module.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("tcspeech", "en-US")

void Registerspeech_filter();

bool obs_module_load(void)
{
	Registerspeech_filter();
	return true;
}
