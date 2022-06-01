#include "RScreenRecord.h"

#include "RCaptureThread.h"
#include "RAudioThread.h"
#include "RVideoWriter.h"
#include <iostream>
using namespace std;
bool RScreenRecord::Start(const char *filename)
{
	if (!filename)return false;
	Stop();
	mutex.lock();
	isExit = false;

	//初始化屏幕录制
	//fps是本类的一个成员变量 可以从外面传递过来
	//下面还有一个fps
	RCaptureThread::Get()->fps = fps; 
	RCaptureThread::Get()->Start();

	//初始化音频录制
	//音频的参数暂时先使用默认参数 也可以从外面传递进来
	RAudioThread::Get()->Start();

	//初始化编码器(RVideoWriter)
	//主要是一些参数的设置
	RVideoWriter::Get()->inWidth = RCaptureThread::Get()->width;   //输入的宽度 也就是全屏
	RVideoWriter::Get()->inHeight = RCaptureThread::Get()->height;  //输入的高度
	RVideoWriter::Get()->outWidth = outWidth;                 //rgb转化成yuv之后编码的宽度
	RVideoWriter::Get()->outHeight = outHeight;
	RVideoWriter::Get()->outFPS = fps;
	//初始化
	RVideoWriter::Get()->Init(filename);
	//添加视频流
	RVideoWriter::Get()->AddVideoStream();
	//添加音频流
	RVideoWriter::Get()->AddAudioStream();
	if (!RVideoWriter::Get()->WriteHead())
	{
		//如果失败，将线程stop
		mutex.unlock();
		Stop();
		return false;
	}
	//如果成功开启线程
	mutex.unlock();
	start();
	return true;
}
void RScreenRecord::Stop()
{
	mutex.lock();
	isExit = true;
	mutex.unlock();
	wait();
	mutex.lock();
	//写入尾部信息并关闭
	RVideoWriter::Get()->WriteEnd();
	RVideoWriter::Get()->Close();
	//屏幕和音频线程关闭
	RCaptureThread::Get()->Stop();
	RAudioThread::Get()->Stop();
	mutex.unlock();
}
void RScreenRecord::run()
{


	while (!isExit)
	{
		mutex.lock();

		//写入视频
		//首先，从RCaptureThread读原始数据
		if (RVideoWriter::Get()->IsVideoBefor())
		{
			char *rgb = RCaptureThread::Get()->GetRGB();
			if (rgb)
			{
				//读到数据之后 开始编码
				AVPacket *p = RVideoWriter::Get()->EncodeVideo((unsigned char*)rgb);
				delete rgb;
				//写入视频帧
				RVideoWriter::Get()->WriteFrame(p);
				//cout << "@";
			}
		}
		else
		{
			//写入音频
			//获取音频
			char *pcm = RAudioThread::Get()->GetPCM();
			if (pcm)
			{
				AVPacket *p = RVideoWriter::Get()->EncodeAudio((unsigned char*)pcm);
				delete pcm;
				RVideoWriter::Get()->WriteFrame(p);
				//cout << "#";
			}
		}
		/*
		char *rgb = RCaptureThread::Get()->GetRGB();
		if (rgb)
		{
			//读到数据之后 开始编码
			AVPacket *p = RVideoWriter::Get()->EncodeVideo((unsigned char*)rgb);
			delete rgb;
			//写入视频帧
			RVideoWriter::Get()->WriteFrame(p);
			cout << "@";
		}

		//写入音频
		//获取音频
		char *pcm = RAudioThread::Get()->GetPCM();
		if (pcm)
		{
			AVPacket *p = RVideoWriter::Get()->EncodeAudio((unsigned char*)pcm);
			delete pcm;
			RVideoWriter::Get()->WriteFrame(p);
			cout << "#";
		}
		*/
		msleep(10);
		mutex.unlock();
	}
}

RScreenRecord::RScreenRecord()
{
}


RScreenRecord::~RScreenRecord()
{
}
