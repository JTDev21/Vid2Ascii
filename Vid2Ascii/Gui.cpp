#include <iostream>
#include "Vid2Ascii.h"
#include <windows.h>
#include <cmath>

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
	/*
		[#] - Current
		[=] - Processed
		[ ] - Unprocessed
	*/
	int num_ths;
	(use_n_threads == 0) ? num_ths = 1 : num_ths = use_n_threads;

	float max_frames = total_frames / num_ths;

	int barWidth = 40;
	float sub_section = barWidth / num_ths;

	std::string bar;
	for (int i = 0; i < num_ths; i++) {
		float processed_frames = (frames_processed[i] * 1.0 / max_frames) * sub_section;
		processed_frames = round(processed_frames);
		float unprocessed_frames = ((max_frames - frames_processed[i]) * 1.0 / max_frames) * sub_section;
		unprocessed_frames = round(unprocessed_frames);

		std::string processed(processed_frames, '=');
		std::string unprocessed(unprocessed_frames, ' ');

		bar += processed;
		bar += unprocessed;
	}
	
	float progress = idx * 1.0 / total_frames;
	int pos = barWidth * progress;
	for (int i = 0; i < barWidth; i++) {
		if (i <= pos)
			bar[i] = '#';
	}
	bar = '[' + std::to_string((int)(idx * 100) / total_frames) + "%][" + bar + "] ";
	bar += std::to_string(idx) + '/' + std::to_string(total_frames) + "       ";
	std::cout << bar;
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

void Vid2Ascii::adjustOutput(float _resize_ratio, float _cell_height, float _cell_width, unsigned _use_n_threads)
{
	system("CLS");
	output_vid.resize_ratio = _resize_ratio;

	output_vid.cell_height = _cell_height;
	output_vid.cell_width = _cell_width;

	output_vid.height = input_vid.height * _resize_ratio;
	output_vid.width = input_vid.width * _resize_ratio;

	output_vid.ascii_height = output_vid.height / _cell_height;
	output_vid.ascii_width = output_vid.width / _cell_width;

	use_n_threads = _use_n_threads;

	int ascii_height = output_vid.ascii_height;
	int ascii_width = output_vid.ascii_width;
	display_adjustedOutputSize(ascii_height, ascii_width);

	setCursorPosition(0, ascii_height + 1);
	char res;
	std::cout << "Resize output? y/n -> ";
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

		if (max_threads != 0) {
			std::cout << "---" << std::endl;
			std::cout << "Current number of threads used: " << _use_n_threads << std::endl;
			std::cout << "Max number of threads supported by your hardware: " << max_threads << std::endl;
			std::cout << "Recommended number of threads to use: 1" << std::endl;
			std::cout << "Use: ";
			unsigned int num_ths;
			std::cin >> num_ths;
			if (num_ths > max_threads) {
				std::cout << "Cannot have " << num_ths << " threads" << std::endl;
			}
			else {
				_use_n_threads = num_ths;
			}
			std::cout << "---" << std::endl;
		}
		adjustOutput(_resize_ratio, _cell_height, _cell_width, _use_n_threads);
	}
}