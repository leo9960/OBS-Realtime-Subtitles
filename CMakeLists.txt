project(tcspeech)

set(tcspeech_HEADERS
	enum-tcspeech.hpp
	hci_asr.h
	hci_asr_recorder.h
	hci_sys.h
	common/AccountInfo.h
	common/CommonTool.h
	common/FileReader.h)

set(tcspeech_SOURCES
	tcspeech.cpp
	speechfilter.cpp
	enum-tcspeech.cpp
	plugin-main.cpp
	common/AccountInfo.cpp
	common/CommonTool.cpp
	common/FileReader.cpp)

set(obs-text_PLATFORM_DEPS
	gdiplus)

link_directories(lib) 

add_library(tcspeech MODULE
	${tcspeech_SOURCES}
	${tcspeech_HEADERS})
target_link_libraries(tcspeech
	libobs
	${obs-text_PLATFORM_DEPS}
	w32-pthreads
	hci_asr
	hci_asr_recorder
	hci_sys)

install_obs_plugin_with_data(tcspeech data)
