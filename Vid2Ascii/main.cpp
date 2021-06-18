#include <iostream>
#include <string>
#include "Vid2Ascii.h"

int main(int argc, char** argv)
{
	// 1920 x 1080 - width: 236; height: 63, recommended width: 57
	/*std::string filepath;
	std::cout << "Video Path: ";
	std::cin >> filepath;

	float cell_width = 0;
	float cell_height = 0;

	std::cout << "Cell Width: ";
	std::cin >> cell_width;

	std::cout << "Cell Height: ";
	std::cin >> cell_height;

	asciiArt(filepath, cell_width, cell_height);*/

	//asciiArt("C:/Users/joshk/OneDrive/Desktop/BadApple.mp4", 2.5, 7);



	//" .'`^\",:;Il!i><~+_-?][}{1)(|\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$" // 70 char
	Vid2Ascii vid("C:/Users/joshk/OneDrive/Desktop/bad-apple.mp4", "@#%xo;:,. ");
	vid.convert(1, 2.5, 7);
	return 0;
}