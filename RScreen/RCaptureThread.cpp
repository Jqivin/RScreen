#include "RCaptureThread.h"
#include <iostream>
#include <QTime>
#include <d3d9.h>     
#pragma comment (lib,"d3d9.lib")

RCaptureThread::RCaptureThread()
{

}


RCaptureThread::~RCaptureThread()
{
}

//截取全屏
void CaptureScreen(void *data)
{
	//1 创建directx3d对象
	static IDirect3D9 *d3d = NULL;
	if (!d3d)
	{
		d3d = Direct3DCreate9(D3D_SDK_VERSION);
	}
	if (!d3d) return;

	//2 创建显卡的设备对象
	static IDirect3DDevice9 *device = NULL;
	if (!device)
	{
		D3DPRESENT_PARAMETERS pa;
		ZeroMemory(&pa, sizeof(pa));
		pa.Windowed = true;
		pa.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		pa.SwapEffect = D3DSWAPEFFECT_DISCARD;
		pa.hDeviceWindow = GetDesktopWindow();
		d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 0,
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &pa, &device
			);
	}
	if (!device)return;



	//3创建离屏表面
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);
	static IDirect3DSurface9 *sur = NULL;
	if (!sur)
	{
		device->CreateOffscreenPlainSurface(w, h,
			D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &sur, 0);
	}
	if (!sur)return;
	//如果data是一个空，就返回，也就是如果传null，将前面初始化，然后返回
	if (!data)return;

	//4 抓屏
	device->GetFrontBufferData(0, sur);

	//5 取出数据
	D3DLOCKED_RECT rect;
	ZeroMemory(&rect, sizeof(rect));
	if (sur->LockRect(&rect, 0, 0) != S_OK)
	{
		return;
	}
	memcpy(data, rect.pBits, w * h * 4);
	sur->UnlockRect();
	//cout << ".";
}

void RCaptureThread::Start()
{
	Stop();
	mutex.lock();
	isExit = false;
	//做一个初始化 因为data为NULL
	CaptureScreen(0);
	//全屏
	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);
	mutex.unlock();
	start();
}
void RCaptureThread::Stop()
{
	mutex.lock();
	isExit = true;
	//保证下一次开始录制的时候，上次的数据不会对新数据造成污染
	while (!rgbs.empty())
	{
		delete rgbs.front();
		rgbs.pop_front();
	}
	mutex.unlock();
	wait();
}
//对外的接口
char* RCaptureThread::GetRGB()
{
	mutex.lock();
	if (rgbs.empty())
	{
		mutex.unlock();
		return NULL;
	}
	char* re = rgbs.front();
	rgbs.pop_front();
	mutex.unlock();
	//std::cout << "V";
	return re;
}
void RCaptureThread::run()
{
	QTime t;
	//如果正在运行
	while (!isExit)
	{
		//在锁之前就开始计时，因为锁也有花费时间
		t.restart();
		mutex.lock();
		int s = 1000 / fps;
		//rgbs是char* 的，这一步保证rgbs里保存的rgb数据<=3个
		if (rgbs.size() < cacheSize)
		{
			//data的大小就是一个rgb空间的大小
			char *data = new char[width * height * 4];
			CaptureScreen(data);
			rgbs.push_back(data);
		}
		mutex.unlock();
		s = s - t.restart();
		//异常数据的处理
		if (  s <= 0 || s > 10000)
		{
			s = 10;
		}
		//std::cout << s;
		msleep(s);
	}
}
