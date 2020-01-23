#include "MP4File.h"

MP4File::MP4File(std::wstring fileName, UINT32 videoWidth, UINT32 videoHeight) :
	m_hDC(NULL),
	m_pSinkWriter(NULL),
	m_writeFrameResult(FVE_E_AD_ATTR_NOT_SET),
	m_lpBitsBuffer(NULL),
	m_hHeap(NULL),

	m_frameDuration(0),
	m_pTimeSrc(NULL),
	m_pPresentationClock(NULL),

	m_name(fileName),
	m_path(),
	m_width(videoWidth),
	m_height(videoHeight),
	m_FPS(30),
	m_bitrate(15000000),
	m_encoding(MFVideoFormat_H264),
	m_inputEncoding(MFVideoFormat_RGB32),

	m_isInitialized(false)
{
	MFFrameRateToAverageTimePerFrame(m_FPS, 1, &m_frameDuration);
	
	HRESULT hr = InitializeFileCreation();

	if (SUCCEEDED(hr))
	{
		m_isInitialized = true;
	}
}

MP4File::~MP4File()
{
	ReleaseMemory();
}

HRESULT MP4File::InitializeFileCreation()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (SUCCEEDED(hr))
	{
		hr = MFStartup(MF_VERSION);
		if (SUCCEEDED(hr))
		{
			hr = InitializeSinkWriter();
			if (SUCCEEDED(hr))
			{
				m_hDC = CreateCompatibleDC(NULL);
				if (m_hDC == NULL)
				{
					return E_FAIL;
				}

				m_hHeap = HeapCreate(HEAP_NO_SERIALIZE, m_width * m_height * 4, 0);
				if (m_hHeap == NULL)
				{
					return E_FAIL;
				}

				m_lpBitsBuffer = HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, m_width * m_height * 4);
				if (m_lpBitsBuffer == NULL)
				{
					return E_FAIL;
				}

				hr = MFCreateSystemTimeSource(&m_pTimeSrc);
				if (SUCCEEDED(hr))
				{
					hr = MFCreatePresentationClock(&m_pPresentationClock);
					if (SUCCEEDED(hr))
					{
						hr = m_pPresentationClock->SetTimeSource(m_pTimeSrc);
						if (SUCCEEDED(hr))
						{
							hr = m_pPresentationClock->Start(0);
						}
					}
				}
			}
		}
	}

	return hr;
}

void MP4File::ReleaseMemory()
{
	if (m_hDC)
	{
		DeleteDC(m_hDC);
		m_hDC = NULL;
	}

	if (m_lpBitsBuffer)
	{
		HeapFree(m_hHeap, HEAP_NO_SERIALIZE, m_lpBitsBuffer);
		m_lpBitsBuffer = NULL;
	}

	if (m_hHeap)
	{
		HeapDestroy(m_hHeap);
		m_hHeap = NULL;
	}

	m_pPresentationClock->Stop();

	SafeRelease(&m_pPresentationClock);
	SafeRelease(&m_pTimeSrc);

	SafeRelease(&m_pSinkWriter);
	MFShutdown();
	CoUninitialize();
}

HRESULT MP4File::InitializeSinkWriter()
{
	IMFSinkWriter   *pSinkWriter = NULL;
	IMFMediaType    *pMediaTypeOut = NULL;
	IMFMediaType    *pMediaTypeIn = NULL;
	DWORD           streamIndex;

	HRESULT hr = MFCreateSinkWriterFromURL((m_path + m_name + L".mp4").c_str(), NULL, NULL, &pSinkWriter);

	// Set the output media type.
	if (SUCCEEDED(hr))
	{
		hr = MFCreateMediaType(&pMediaTypeOut);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeOut->SetGUID(MF_MT_SUBTYPE, m_encoding);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeOut->SetUINT32(MF_MT_AVG_BITRATE, m_bitrate);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeOut->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeSize(pMediaTypeOut, MF_MT_FRAME_SIZE, m_width, m_height);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_FRAME_RATE, m_FPS, 1);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	}
	if (SUCCEEDED(hr))
	{
		hr = pSinkWriter->AddStream(pMediaTypeOut, &streamIndex);
	}

	// Set the input media type.
	if (SUCCEEDED(hr))
	{
		hr = MFCreateMediaType(&pMediaTypeIn);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeIn->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeIn->SetGUID(MF_MT_SUBTYPE, m_inputEncoding);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeIn->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeSize(pMediaTypeIn, MF_MT_FRAME_SIZE, m_width, m_height);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_FRAME_RATE, m_FPS, 1);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	}
	if (SUCCEEDED(hr))
	{
		hr = pSinkWriter->SetInputMediaType(streamIndex, pMediaTypeIn, NULL);
	}

	// Tell the sink writer to start accepting data.
	if (SUCCEEDED(hr))
	{
		hr = pSinkWriter->BeginWriting();
	}

	// Assign pointers to members upon complete success
	if (SUCCEEDED(hr))
	{
		m_pSinkWriter = pSinkWriter;
		m_pSinkWriter->AddRef();
		m_streamIndex = streamIndex;

	}

	SafeRelease(&pSinkWriter);
	SafeRelease(&pMediaTypeOut);
	SafeRelease(&pMediaTypeIn);
	return hr;
}

void MP4File::SetPath(std::wstring path)
{
	m_path = path + L"\\";
}

HRESULT MP4File::WriteFrame()
{
	IMFSample *pSample = NULL;
	IMFMediaBuffer *pBuffer = NULL;

	const LONG cbWidth = 4 * m_width;
	const DWORD cbBufferSize = cbWidth * m_height;

	BYTE *pData = NULL;
	MFTIME sampleTime = NULL;

	// Create a new memory buffer.
	HRESULT hr = MFCreateMemoryBuffer(cbBufferSize, &pBuffer);

	// Lock the buffer and copy the video frame to the buffer.
	if (SUCCEEDED(hr))
	{
		hr = pBuffer->Lock(&pData, NULL, NULL);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFCopyImage(
			pData,                      // Destination buffer.
			cbWidth,                    // Destination stride.
			(BYTE*)m_lpBitsBuffer,		// First row in source image.
			cbWidth,                    // Source stride.
			cbWidth,                    // Image width in bytes.
			m_height	                // Image height in pixels.
		);
	}
	if (pBuffer)
	{
		pBuffer->Unlock();
	}

	// Set the data length of the buffer.
	if (SUCCEEDED(hr))
	{
		hr = pBuffer->SetCurrentLength(cbBufferSize);
	}

	// Create a media sample and add the buffer to the sample.
	if (SUCCEEDED(hr))
	{
		hr = MFCreateSample(&pSample);
	}
	if (SUCCEEDED(hr))
	{
		hr = pSample->AddBuffer(pBuffer);
	}

	// Set the time stamp and the duration.
	if (SUCCEEDED(hr))
	{
		hr = m_pPresentationClock->GetTime(&sampleTime);
		if (SUCCEEDED(hr))
		{
			hr = pSample->SetSampleTime(sampleTime);
		}
	}
	if (SUCCEEDED(hr))
	{
		hr = pSample->SetSampleDuration(m_frameDuration);
	}

	// Send the sample to the Sink Writer and update the timestamp
	if (SUCCEEDED(hr))
	{
		hr = m_pSinkWriter->WriteSample(m_streamIndex, pSample);
	}

	SafeRelease(&pSample);
	SafeRelease(&pBuffer);

	return hr;
}

HRESULT MP4File::AppendFrame(HBITMAP frame)
{
	HRESULT hr = NULL;

	if (m_isInitialized) // Make sure buffer is initialized
	{
		BITMAPINFO bmpInfo;
		bmpInfo.bmiHeader.biBitCount = 0;
		bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

		// Get individual bits from bitmap and load it into the buffer used by `WriteFrame`
		GetDIBits(m_hDC, frame, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS);
		bmpInfo.bmiHeader.biCompression = BI_RGB;
		GetDIBits(m_hDC, frame, 0, bmpInfo.bmiHeader.biHeight, m_lpBitsBuffer, &bmpInfo, DIB_RGB_COLORS);

		hr = WriteFrame();
	}

	return m_writeFrameResult = hr;
}

HRESULT MP4File::Finalize()
{
	if (SUCCEEDED(m_writeFrameResult))
	{
		m_pSinkWriter->Finalize();

		return S_OK;
	}

	return E_FAIL;
}
