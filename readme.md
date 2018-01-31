### 实时字幕插件

#### 适用于OBS Studio

该项目使用的离线语音识别库为灵云(www.hcicloud.com)

##### 思路：

该插件作为滤镜，用OBS提供的API获取到音频流，再将拼接后时长为1秒的音频流传入离线语音识别(多线程)，将返回的文字保存至临时文件中，在OBS中新建文字读取该文件，做到延迟较低的实时字幕输出。

##### 问题：

使用过程中音频流获取速度会越来越慢，暂未有解决方案

#### 编译方法：

1.先去 https://github.com/jp9000/obs-studio 上下载整个项目，再将本项目放入plugins文件夹下

2.下载安装CMake(https://cmake.org/)，Visual Studio 2017

3.打开CMake，source code选择obs源码的文件夹，build the binaries选择新建一个obs编译文件夹，然后点击Configure，等进度条走完后再点击一次Configure，最后点击Generate，生成Visual Studio 2017工程文件

4.在Visual Studio 2017中打开生成的工程，点生成解决方案即可

#### 使用方法：

1.先到灵云开发者平台申请账户

2.下载灵云离线语音识别SDK(https://www.aicloud.com/dev/sdk/viewsdk/typeid/1)以及离线语音识别库(https://www.aicloud.com/dev/application/viewres)要选择asr.local.freetalk下的资源文件

3.在源码中修改appkey和识别库文件的位置