#include <stdio.h>
#include "stdafx.h"
#include <malloc.h>
#include <string.h>
#include <Windows.h>
#include <strsafe.h>
#include <Shlwapi.h>
#include <stdint.h>
#include <crtdbg.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "FileIoHelperClass.h"
#include <mmio.h>

class StopWatch
{
private:
	LARGE_INTEGER   mFreq, mStart, mEnd;
	float                   mTimeforDuration;
public:
	StopWatch() : mTimeforDuration(0)
	{
		mFreq.LowPart = mFreq.HighPart = 0;
		mStart = mFreq;
		mEnd = mFreq;
		QueryPerformanceFrequency(&mFreq);
	}
	~StopWatch()
	{
	}

public:
	void Start(){ QueryPerformanceCounter(&mStart); }
	void Stop()
	{
		QueryPerformanceCounter(&mEnd);
		mTimeforDuration = (mEnd.QuadPart - mStart.QuadPart) / (float)mFreq.QuadPart;
	}
	float GetDurationSecond() { return mTimeforDuration; }
	float GetDurationMilliSecond() { return mTimeforDuration * 1000.f; }

};

int _tmain(int argc, _TCHAR* argv[])
{
#define KB 1024
	uint64_t uFileSize = 4 * KB;
	_ASSERTE(create_very_big_file(L"bigf.txt", uFileSize));

	StopWatch sw;
	sw.Start();
	_ASSERTE(file_copy_using_read_write(L"bigf.txt", L"bigf2.txt"));
	sw.Stop();
	print("info] time elapsed = %f", sw.GetDurationSecond());

#if 0 // 湲곗〈�� Memory_Mapped_IO
	StopWatch sw2;
	sw2.Start();
	_ASSERTE(file_copy_using_memory_map(L"bigf.txt", L"bigf3.txt"));
	sw2.Stop();
	print("info] time elapsed = %f", sw2.GetDurationSecond());
#endif

	StopWatch sw2;
	sw2.Start();
	LARGE_INTEGER *currOffset = new LARGE_INTEGER;
	currOffset->QuadPart = (LONGLONG)0;

	LARGE_INTEGER *destOffset = new LARGE_INTEGER;
	destOffset->QuadPart = (LONGLONG)(1024 * 1024) * (LONGLONG)uFileSize;

	FileIoHelper *aFileIoHelper = new FileIoHelper;
	aFileIoHelper->FIOpenForRead(L"bigf.txt"); // read
	aFileIoHelper->FIOCreateFile(L"bigf3.txt", *destOffset); // createfile

	LONGLONG nResult;
	uint64_t uFileSize_Copy = uFileSize;

	unsigned char *buffer = new unsigned char[uFileSize];

	while (1){
		nResult = (destOffset->QuadPart - currOffset->QuadPart);

		if (nResult <= 0) break; // infinite loop escape

		(nResult > uFileSize_Copy) ? uFileSize_Copy = uFileSize : uFileSize_Copy = nResult;

		aFileIoHelper->FIOReadFromFile(*currOffset, uFileSize_Copy, buffer);
		aFileIoHelper->FIOWriteToFile(*currOffset, uFileSize_Copy, buffer);

		currOffset->QuadPart += (LONGLONG)uFileSize_Copy;
	}
	sw2.Stop();
	print("info] time elapsed = %f", sw2.GetDurationSecond());

	delete aFileIoHelper;
	delete buffer;
	delete currOffset;
	delete destOffset;
	return 0;
}