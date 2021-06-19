#include <iostream>
#include <string>
#include "Vid2Ascii.h"

int main(int argc, char** argv)
{
	std::string filepath;
	std::cout << "Video Path: ";
	std::cin >> filepath;

	float resize_ratio = 0;
	float cell_width = 0;
	float cell_height = 0;
	
	std::cout << "Resize ratio: ";
	std::cin >> resize_ratio;

	std::cout << "Cell Width: ";
	std::cin >> cell_width;

	std::cout << "Cell Height: ";
	std::cin >> cell_height;

	Vid2Ascii vid(filepath, "@#%xo;:,. ");
	vid.convert(resize_ratio, cell_width, cell_height);

	return 0;
}