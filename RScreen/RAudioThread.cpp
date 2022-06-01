#include "RAudioThread.h"
#include <iostream>
#include <QAudioInput>
using namespace std;
static QAudioInput *input = NULL;
static QIODevice *io = NULL;


void RAudioThread::Start()
{
	Stop();
	mutex.lock();
	isExit = false;
	//初始化
	QAudioFormat fmt;
	fmt.setSampleRate(sampleRate);
	fmt.setChannelCount(channels);
	fmt.setSampleSize(sampleByte * 8);
	fmt.setSampleType(QAudioFormat::UnSignedInt);
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setCodec("audio/pcm");
	input = new QAudioInput(fmt);
	io = input->start();
	mutex.unlock();
	start();
}

void RAudioThread::Stop()
{
	mutex.lock();
	isExit = true;
	while (!pcms.empty())
	{
		delete pcms.front();
		pcms.pop_front();
	}
	//把new的空间清理掉
	if (input)
	{
		io->close();
		input->stop();
		delete input;
		input = NULL;
		io = NULL;
	}
	mutex.unlock();
	wait();
}


char *RAudioThread::GetPCM()
{
	mutex.lock();
	if (pcms.empty())
	{
		mutex.unlock();
		return NULL;
	}
	char *re = pcms.front();
	pcms.pop_front();
	mutex.unlock();
	cout << "A";
	return re;
}
void RAudioThread::run()
{
	int size = nbSample*channels*sampleByte;
	while (!isExit)
	{
		mutex.lock();
		if (pcms.size() > cacheSize)
		{
			mutex.unlock();
			msleep(5);
			continue;
		}

		char *data = new char[size];
		int readedSize = 0;   //已经读到的字节数  
		//读慢一帧音频，放到内存当中
		while (readedSize < size)
		{
			//获得已经准备好的字节数
			int br = input->bytesReady();
			//满1024才开始读
			if (br < 1024)
			{
				msleep(1);
				continue;
			}
			int s = 1024;

			//最后一次，剩下的不到1024  剩多少赌多少
			if (size - readedSize < s)
			{
				s = size - readedSize;
			}
			//开始读，+readedSize表示指针移动位置
			int len = io->read(data + readedSize, s);
			readedSize += len;
		}
		//读到之后，将数据放到data中
		pcms.push_back(data);
		mutex.unlock();
	}
}

RAudioThread::RAudioThread()
{
}


RAudioThread::~RAudioThread()
{
}
