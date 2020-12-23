#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <thread>
#include <chrono>
#include <time.h>
#include <iomanip>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <hyperion/Hyperion.h>
#include <hyperion/HyperionIManager.h>

#include <QDirIterator>
#include <QFileInfo>

#include "grabber/MFWorker.h"



volatile bool	MFWorker::_isActive = false;

MFWorkerManager::MFWorkerManager():	
	workers(nullptr)	
{
	int select = QThread::idealThreadCount();
	
	if (select >= 2 && select <= 3)
		select = 2;
	else if (select > 3 && select <= 5)
		select = 3;	
	else if (select > 5)
		select = 4;
	
	workersCount = std::max(select, 1);
}

MFWorkerManager::~MFWorkerManager()
{
	if (workers!=nullptr)
	{
		for(unsigned i=0; i < workersCount; i++)
			if (workers[i]!=nullptr)
			{
				workers[i]->deleteLater();
				workers[i] = nullptr;
			}
		delete[] workers;
		workers = nullptr;
	}	
}

void MFWorkerManager::Start()
{
	MFWorker::_isActive = true;
}

void MFWorkerManager::InitWorkers()
{	
	if (workersCount>=1)
	{
		workers = new MFWorker*[workersCount];
					
		for (unsigned i=0;i<workersCount;i++)
		{								
			workers[i] = new MFWorker();
		}
	}
}

void MFWorkerManager::Stop()
{
	MFWorker::_isActive =  false;

	if (workers != nullptr)
	{
		for(unsigned i=0; i < workersCount; i++)
			if (workers[i]!=nullptr)
			{
				workers[i]->quit();
				workers[i]->wait();				
			}
	}
}

bool MFWorkerManager::isActive()
{
	return MFWorker::_isActive;
}

MFWorker::MFWorker():

		_workerIndex(0),
		_lineLength(0),
		_pixelFormat(PixelFormat::NO_CHANGE),
		_size(0),
		_width(0),
		_height(0),
		_subsamp(0),
		_cropLeft(0),
		_cropTop(0),
		_cropBottom(0),
		_cropRight(0),
		_currentFrame(0),
		_frameBegin(0),
		_hdrToneMappingEnabled(0),
		lutBuffer(0),

		_localData(nullptr),
		_localDataSize(0),
		_decompress(nullptr),
		_isBusy(false),
		_semaphore(1)
{
	
}

MFWorker::~MFWorker(){
	
	if (_decompress == nullptr)
		tjDestroy(_decompress);
	
	if (_localData != NULL)
	{
		free(_localData);
		_localData = NULL;
		_localDataSize = 0;
	}
}

void MFWorker::setup(unsigned int __workerIndex, PixelFormat __pixelFormat, 
			uint8_t * _sharedData, int __size,int __width, int __height, int __lineLength,
			int __subsamp, 
			unsigned  __cropLeft, unsigned  __cropTop, 
			unsigned __cropBottom, unsigned __cropRight,int __currentFrame, quint64	__frameBegin,
			int __hdrToneMappingEnabled,unsigned char* _lutBuffer)
{
	_workerIndex = __workerIndex;  	
	_lineLength = __lineLength;
	_pixelFormat = __pixelFormat;		
	_size = __size;
	_width = __width;
	_height = __height;
	_subsamp = __subsamp;
	_cropLeft = __cropLeft;
	_cropTop = __cropTop;
	_cropBottom = __cropBottom;
	_cropRight = __cropRight;
	_currentFrame = __currentFrame;
	_frameBegin = __frameBegin;
	_hdrToneMappingEnabled = __hdrToneMappingEnabled;
	lutBuffer = _lutBuffer;	
	
	if (__size > _localDataSize)
	{
		if (_localData != NULL)
		{
			free(_localData);
			_localData = NULL;
			_localDataSize = 0;
		}
		_localData = (uint8_t *) malloc(__size+1);
		_localDataSize = __size;		
	}

	if (_localData != NULL)
		memcpy(_localData, _sharedData, __size);	
}

void MFWorker::run()
{
	runMe();	
}

void MFWorker::runMe()
{
	if (_isActive && _width > 0 && _height >0)
	{
		if (_pixelFormat == PixelFormat::MJPEG)
		{
			process_image_jpg_mt();								
		}
		else
		{
			int outputWidth = (_width - _cropLeft - _cropRight);
			int outputHeight = (_height - _cropTop - _cropBottom);

			Image<ColorRgb> image(outputWidth, outputHeight);
			
			ImageResampler::processImage(				
				_cropLeft, _cropRight, _cropTop, _cropBottom,
				_localData , _width, _height, _lineLength, _pixelFormat, lutBuffer, image);
				
			
			emit newFrame(_workerIndex, image, _currentFrame, _frameBegin);		
		}
	}		
}

void MFWorker::startOnThisThread()
{	
	runMe();
}

bool MFWorker::isBusy()
{	
	bool temp;
	_semaphore.acquire();
	if (_isBusy)
		temp = true;
	else
	{
		temp = false;	
		_isBusy = true;
	}
	_semaphore.release();
	return temp;
}

void MFWorker::noBusy()
{	
	_semaphore.acquire();
	_isBusy = false;
	_semaphore.release();
}


void MFWorker::process_image_jpg_mt()
{										
	if (_decompress == nullptr)	
		_decompress = tjInitDecompress();	
	
	
	if (tjDecompressHeader2(_decompress, _localData, _size, &_width, &_height, &_subsamp) != 0)
	{	
		if (tjGetErrorCode(_decompress) == TJERR_FATAL)
		{
			QString info = QString(tjGetErrorStr());
			emit newFrameError(_workerIndex, info,_currentFrame);				
			return;
		}
	}
	
	Image<ColorRgb> srcImage(_width, _height);
	
	if (tjDecompress2(_decompress, _localData , _size, (unsigned char*)srcImage.memptr(), _width, 0, _height, TJPF_RGB, TJFLAG_FASTDCT | TJFLAG_FASTUPSAMPLE) != 0)
	{		
		if (tjGetErrorCode(_decompress) == TJERR_FATAL)
		{
			QString info = QString(tjGetErrorStr());
			emit newFrameError(_workerIndex, info,_currentFrame);
			return;		
		}
	}		
	
	// got image, process it	
	if ( !(_cropLeft > 0 || _cropTop > 0 || _cropBottom > 0 || _cropRight > 0))
	{
		// apply LUT	
		ImageResampler::applyLUT((unsigned char*)srcImage.memptr(), srcImage.width(), srcImage.height(), lutBuffer, _hdrToneMappingEnabled);;
		
		// exit
		emit newFrame(_workerIndex, srcImage, _currentFrame, _frameBegin);    		
	}
	else
    {    	
    		// calculate the output size
		int outputWidth = (_width - _cropLeft - _cropRight);
		int outputHeight = (_height - _cropTop - _cropBottom);
		
		if (outputWidth <= 0 || outputHeight <= 0)
		{
			QString info = QString("Invalid cropping");
			emit newFrameError(_workerIndex, info,_currentFrame);
			return;	
		}
		
		Image<ColorRgb> destImage(outputWidth, outputHeight);
		
		for (unsigned int y = 0; y < destImage.height(); y++)
		{
			unsigned char* source = (unsigned char*)srcImage.memptr() + (static_cast<size_t>(y) + _cropTop) * srcImage.width() * 3 + static_cast<size_t>(_cropLeft) * 3;
			unsigned char* dest = (unsigned char*)destImage.memptr() + static_cast<size_t>(y) * destImage.width() * 3;
			memcpy(dest, source, static_cast<size_t>(destImage.width()) * 3);
		}
		
		// apply LUT	
		ImageResampler::applyLUT((unsigned char*)destImage.memptr(), destImage.width(), destImage.height(), lutBuffer, _hdrToneMappingEnabled);
    		
    	// exit		
		emit newFrame(_workerIndex, destImage, _currentFrame, _frameBegin);	
	}
}
