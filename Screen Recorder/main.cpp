#include "MP4File.h"

#include <iostream>

// An example of how the library can be used.
// This file is not in any way required to use the library.

const UINT32 VIDEO_WIDTH = 1920;
const UINT32 VIDEO_HEIGHT = 1080;

HBITMAP GetBitmap()
{
	HDC hScreen = GetDC(NULL);
	HDC hDC = CreateCompatibleDC(hScreen);

	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, VIDEO_WIDTH, VIDEO_HEIGHT);
	HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
	BOOL bRet = BitBlt(hDC, 0, 0, VIDEO_WIDTH, VIDEO_HEIGHT, hScreen, 0, 0, SRCCOPY);

	SelectObject(hDC, old_obj);
	DeleteDC(hDC);
	ReleaseDC(NULL, hScreen);

	return hBitmap;
}

int main()
{
	HRESULT hr = NULL;
	MP4File file(L"test", VIDEO_WIDTH, VIDEO_HEIGHT); // Do not specify file extension. You will get an mp4 file regardless

	file.SetPath(L"C:\\Users\\lille\\Desktop");       // Completely optional. If unset, file will appear in project directory

	std::cout << "Recording initiated..\n";
	bool isRecording = true;
	while (isRecording)
	{
		HBITMAP hBitmap = GetBitmap();
		hr = file.AppendFrame(hBitmap);

		if (FAILED(hr))
			break;

		if (GetAsyncKeyState(0x51)) // Detects 'Q' keypress to stop recording
		{
			std::cout << "Stopped recording\n";
			isRecording = false;
		}
	}

	file.Finalize();
	
	return 0;
}
