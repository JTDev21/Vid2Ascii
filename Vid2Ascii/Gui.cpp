#include <iostream>
#include "Vid2Ascii.h"
#include <windows.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

void Vid2Ascii::setCursorPosition(int x, int y)
{
	static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(hOut, coord);
}

void Vid2Ascii::progressBar(int idx, int total_frames)
{
	float barWidth = 40;
	float progress = idx * 1.0 / total_frames;
	float pos = barWidth * progress;

	std::cout << "[" << (int)(idx * 100) / total_frames << "%]";

	std::string bar;
	bar += "[";
	for (int i = 0; i < barWidth; i++) {
		(i <= pos ? bar += "#" : bar += "=");
	}
	bar += "]";
	std::cout << bar << " " << idx << "/" << total_frames << "    ";
}

void Vid2Ascii::display_adjustedOutputSize(int ascii_height, int ascii_width) 
{
	for (int i = 0; i < ascii_height; i++) {
		std::cout << "#\n";
	}

	setCursorPosition(0, 0);
	for (int i = 0; i < ascii_width; i++) {
		std::cout << "#";
	}
}


/* -------------------------------------------------------- */
void Vid2Ascii::adjustOutputSize(float _resize_ratio, float _cell_height, float _cell_width)
{
	system("CLS");
	output_vid.resize_ratio = _resize_ratio;

	output_vid.cell_height = _cell_height;
	output_vid.cell_width = _cell_width;

	output_vid.height = input_vid.height * _resize_ratio;
	output_vid.width = input_vid.width * _resize_ratio;

	output_vid.ascii_height = output_vid.height / _cell_height;
	output_vid.ascii_width = output_vid.width / _cell_width;

	int ascii_height = output_vid.ascii_height;
	int ascii_width = output_vid.ascii_width;
	display_adjustedOutputSize(ascii_height, ascii_width);

	setCursorPosition(0, ascii_height + 1);
	char res;
	std::cout << "Resize output size? y/n -> ";
	std::cin >> res;
	res = std::tolower(res);

	if (res == 'y') {
		std::cout << "Current resize ratio: " << _resize_ratio << std::endl;
		std::cout << "New ratio: ";
		std::cin >> _resize_ratio;

		std::cout << "Current cell width: " << _cell_width << std::endl;
		std::cout << "New cell width: ";
		std::cin >> _cell_width;

		std::cout << "Current cell height: " << _cell_height << std::endl;
		std::cout << "New cell height: ";
		std::cin >> _cell_height;

		adjustOutputSize(_resize_ratio, _cell_height, _cell_width);
	}
}