#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include "Vid2Ascii.h"


Vid2Ascii::Vid2Ascii(std::string _filepath, std::string intensity_chars)
{
	frames_processed = 0;

	cv::VideoCapture video(_filepath);
	// Check if video file is opened
	if (!video.isOpened())
	{
		std::cout << _filepath << " does not exists!" << std::endl;
		std::exit(0);
	}
	filepath = _filepath;

	input_vid.total_frames = video.get(cv::CAP_PROP_FRAME_COUNT) - 1;
	//input_vid.total_frames = 100;

	int total_frames = input_vid.total_frames;
	input_vid.fps = video.get(cv::CAP_PROP_FPS);
	input_vid.total_duration = total_frames / input_vid.fps;

	cv::Mat frame;
	video >> frame;
	input_vid.height = frame.size().height;
	input_vid.width = frame.size().width;

	/* Change this and read/store character mapping in a txt file */
	num_map_chars = intensity_chars.length();
	map = new char[num_map_chars];

	for (int i = 0; i < num_map_chars; i++) {
		map[i] = intensity_chars[i];
	}

	vid_frames = new cv::Mat[total_frames];
	ascii_frames = new std::string[total_frames];
	video.release();
}

Vid2Ascii::~Vid2Ascii() 
{
	std::cout << "Deallocating memory" << std::endl;
	// Deallocate memory from heap
	delete[] map;
	delete[] vid_frames;
	delete[] ascii_frames;
}



void Vid2Ascii::readFrames() 
{
	bool resize = ((int)output_vid.resize_ratio == 1);
	system("CLS");
	if (!resize)
		std::cout << "Reading/Resizing frames" << std::endl;
	else
		std::cout << "Reading frames" << std::endl;

	cv::VideoCapture video(filepath);
	cv::Mat frame;

	int output_width = output_vid.width;
	int output_height = output_vid.height;
	int total_frames = input_vid.total_frames;

	for(int i = 0; i < total_frames; i++) {
		video >> frame;
		if(!resize)
			cv::resize(frame, frame, cv::Size(output_width, output_height));
		vid_frames[i] = frame.clone();

		setCursorPosition(0,1);
		progressBar(i, total_frames);
	}
	// Deallocate memory and clear *capture pointer
	video.release();
}

void Vid2Ascii::playBack()
{
	system("CLS");
	setCursorPosition(0, 0);
	std::cout << ascii_frames[0] << std::endl;
	cv::resize(vid_frames[0], vid_frames[0], cv::Size(320, 180));
	cv::imshow("Bad Apple", vid_frames[0]);

	cv::waitKey(2000);

	auto start = std::chrono::steady_clock::now();
	float curr_fps = 0;
	int total_frames = input_vid.total_frames;
	int total_duration = input_vid.total_duration;
	for (int i = 0; i < total_frames; i++) {
		setCursorPosition(0, 0);
		std::cout << ascii_frames[i] << std::endl;

		cv::resize(vid_frames[i], vid_frames[i], cv::Size(320, 180));
		cv::imshow("Bad Apple", vid_frames[i]);

		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		progressBar(i, total_frames);
		std::cout << "Duration " << (int)elapsed_seconds.count() << "s --- " << total_duration << "s       " << std::endl;


		curr_fps = i / elapsed_seconds.count();
		std::cout << "FPS " << curr_fps << "       " << std::endl;
		if (input_vid.fps < curr_fps)
			cv::waitKey(5);

		if (input_vid.fps > curr_fps)
			i++;

		char c = (char)cv::waitKey(24);
		if (c == 27)
			break;
	}

	// Release VideoCapture && destroy all windows
	cv::destroyAllWindows();
}

void Vid2Ascii::convert(float resize_ratio, float cell_width = 2.5, float cell_height = 7)
{
	adjustOutputSize(resize_ratio, cell_height, cell_width);
	readFrames();

	setCursorPosition(0, 3);
	std::cout << "Video to Ascii Conversion" << std::endl;

	auto start = std::chrono::steady_clock::now();
	int total_frames = input_vid.total_frames;
	int ascii_output_height = output_vid.ascii_height;
	int ascii_output_width = output_vid.ascii_width;
	cell_width = output_vid.cell_width;
	cell_height = output_vid.cell_height;
	for (int i = 0; i < total_frames; i++) {
		std::string placeholder;
		for (int hnum = 0; hnum < ascii_output_height; hnum++) {
			for (int wnum = 0; wnum < ascii_output_width; wnum++) {
				// Display ROI(region of interest) bounding box
				/*cv::Mat canvas = vid_frames[i].clone();
				cv::rectangle(canvas, cv::Point(wnum*cell_width, hnum*cell_height), cv::Point(wnum * cell_width+ cell_width, hnum * cell_height+ cell_height), cv::Scalar(0,0,255));
				cv::imshow("crop", canvas);
				cv::waitKey(1);*/

				// Get ROI position
				int roi_x = wnum * cell_width;
				int roi_y = hnum * cell_height;

				// Get ROI by cropping
				cv::Mat roi(vid_frames[i], cv::Rect(roi_x, roi_y, cell_width, cell_height));

				// Calculate intensity by pixel/area
				float intensity = cv::mean(roi)[0];
				placeholder += map[(int)((255 - intensity) * num_map_chars / 256)];
			}
			placeholder += "\n";
		}
		ascii_frames[i] = placeholder;

		setCursorPosition(0, 4);
		progressBar(i, total_frames);
	}
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "---" << std::endl;
	std::cout << "Processing time: " << elapsed_seconds.count() << "s" << std::endl;
	std::cout << "---" << std::endl;
	
	std::cout << "Press any key to start ..." << std::endl;
	char wait_input;
	std::cin >> wait_input;

	playBack();
}