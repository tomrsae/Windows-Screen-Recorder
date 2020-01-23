#pragma once

#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>
#include <string>

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")

class MP4File
{
private:
	// Fields used by the Microsoft Media Foundation interface
	HDC		m_hDC;
	IMFSinkWriter*	m_pSinkWriter;
	HRESULT		m_writeFrameResult;
	LPVOID		m_lpBitsBuffer;
	HANDLE		m_hHeap;
	DWORD		m_streamIndex;

	// Fields used to properly insert the recorded frames
	UINT64				m_frameDuration;
	IMFPresentationTimeSource	*m_pTimeSrc;
	IMFPresentationClock		*m_pPresentationClock;

	// File details
	std::wstring	m_name;
	std::wstring	m_path;
	UINT32		m_width;
	UINT32		m_height;
	UINT32		m_FPS;
	UINT32		m_bitrate;
	GUID		m_encoding;
	GUID		m_inputEncoding;

	bool		m_isInitialized;

	HRESULT InitializeFileCreation();

	HRESULT InitializeSinkWriter();

	HRESULT WriteFrame();

	void ReleaseMemory();

	// Helper function to release Media Foundation objects
	template <class T> void SafeRelease(T** ppT)
	{
		if (*ppT)
		{
			(*ppT)->Release();
			*ppT = NULL;
		}
	}
public:
	MP4File(std::wstring, UINT32, UINT32);
	~MP4File();

	void SetPath(std::wstring);

	HRESULT AppendFrame(HBITMAP);

	HRESULT Finalize();
};
