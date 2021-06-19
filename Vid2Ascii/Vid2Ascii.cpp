#include <chrono>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include "Vid2Ascii.h"

std::mutex mu;

Vid2Ascii::Vid2Ascii(std::string _filepath, std::string intensity_chars)
{
	max_threads = std::thread::hardware_concurrency();
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
	//input_vid.total_frames = 500;

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

void Vid2Ascii::resizeNStor(unsigned int th_id, unsigned int max_ths, bool resize) {
	int total_frames = input_vid.total_frames; // Number of frames in video
	int max_frames = total_frames / max_ths; // Number of frames to be shared among threads

	int start = (th_id - 1) * max_frames;
	int end = (th_id != max_ths ?
		(th_id - 1) * max_frames + max_frames - 1 :
		total_frames - 1);

	int output_width = output_vid.width;
	int output_height = output_vid.height;

	cv::VideoCapture video(filepath);
	video.set(cv::CAP_PROP_POS_FRAMES, start);
	cv::Mat frame;

	for (int i = start; i <= end; i++) {
		video >> frame;
		if (frame.empty())
			break;
		if (!resize)
			cv::resize(frame, frame, cv::Size(output_width, output_height));
		vid_frames[i] = frame.clone();

		mu.lock();
		setCursorPosition(0, 0 + th_id);
		frames_processed++;
		std::cout << "Threads[" << th_id << "]";
		progressBar(i - start, end - start);

		setCursorPosition(0, 1 + max_ths);
		std::cout << "[Progress]";
		progressBar(frames_processed, total_frames);
		mu.unlock();
	}
	// Deallocate memory and clear *capture pointer
	video.release();
}

void Vid2Ascii::readFrames() 
{
	bool resize = ((int)output_vid.resize_ratio == 1);
	system("CLS");
	if (!resize)
		std::cout << "Reading/Resizing frames" << std::endl;
	else
		std::cout << "Reading frames" << std::endl;

	if (max_threads == 0) {
		resizeNStor(1, 1, resize);
	}
	else {
		std::thread* threads = new std::thread[max_threads];
		for (int i = 0; i < max_threads; i++) {
			threads[i] = std::thread([&] { resizeNStor(i + 1, max_threads, resize); });
			cv::waitKey(100);
		}

		for (int i = 0; i < max_threads; i++) {
			threads[i].join();
		}
		delete[] threads;
	}
	frames_processed = 0;
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


void Vid2Ascii::generateAscii(unsigned int th_id, unsigned int max_ths)
{
	if (th_id <= 0 || max_ths <= 0) {
		std::cout << "You must have nonzero positive integers for th_id/max_ths!" << std::endl;
		exit(0);
	}
	int total_frames = input_vid.total_frames;
	int max_frames = total_frames / max_ths;

	int start = (th_id - 1) * max_frames;
	int end = (th_id != max_ths ?
		(th_id - 1) * max_frames + max_frames - 1 :
		total_frames - 1);

	int ascii_output_height = output_vid.ascii_height;
	int ascii_output_width = output_vid.ascii_width;
	float cell_width = output_vid.cell_width;
	float cell_height = output_vid.cell_height;

	for (int i = start; i <= end; i++) {
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

		mu.lock();
		setCursorPosition(0, 3 + max_ths + th_id);
		frames_processed++;
		std::cout << "Threads[" << th_id << "]";
		progressBar(i - start, end-start);
		std::cout << " start_idx: " << start << " end_idx: " << end << std::endl;

		setCursorPosition(0, 4 + (2*max_ths));
		std::cout << "[Progress]";
		progressBar(frames_processed, total_frames);
		mu.unlock();
	}
}

void Vid2Ascii::convert(float resize_ratio, float cell_width, float cell_height)
{
	adjustOutputSize(resize_ratio, cell_height, cell_width);

	auto start = std::chrono::steady_clock::now();
	readFrames();

	if (max_threads == 0)
		setCursorPosition(0, 4);
	else
		setCursorPosition(0, 3 + max_threads);

	std::cout << "Video to Ascii Conversion" << std::endl;

	if (max_threads == 0) {
		generateAscii(1, 1);
		playBack();
	}
	else {
		std::thread* threads = new std::thread[max_threads];
		for (int i = 0; i < max_threads; i++) {
			threads[i] = std::thread([&] { generateAscii(i + 1, max_threads); });
			cv::waitKey(100);
		}

		for (int i = 0; i < max_threads; i++) {
			threads[i].join();
		}
		delete[] threads;
	}
	frames_processed = 0;
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;

	if(max_threads == 0)
		setCursorPosition(0, 7);
	else
		setCursorPosition(0, 5 + (2 * max_threads));
	std::cout << "---" << std::endl;
	std::cout << "Processing time: " << elapsed_seconds.count() << "s" << std::endl;
	std::cout << "---" << std::endl;

	std::cout << "Press any key to start ..." << std::endl;
	char wait_input;
	std::cin >> wait_input;

	playBack();
}