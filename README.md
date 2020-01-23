# Windows-Screen-Recorder
Small library for appending bitmaps as frames into a H.264 (mp4) encoded file.
A fun little project to work with to get a better understanding of the Microsoft Media Foundation, as well as GDI and video encoding in general.

# How does it work?
The library takes HBITMAP objects (obtained with GDI) to append as frames. The frames are then "deconstructed" into individual bits into an LPVOID buffer, to be used by the media foundation to write a new frame with the MF Sink Writer.

# Usability
I wouldn't personally recommend using this library over others, as I'm still very much (as of writing this) in the learning process. The program is not very CPU efficient, although I've tried my best to optimize it.

It does however, work as a decent reference for others that are just starting to work with the Media Foundation.

# Example usage
The `main.cpp` file in the `Screen Recorder` folder shows how easy it the library can be used in a regular console program.
