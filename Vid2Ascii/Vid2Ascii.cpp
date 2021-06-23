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
	cv::VideoCapture video(_filepath);
	// Check if video file is opened
	if (!video.isOpened()) {
		std::cout << _filepath << " does not exists!" << std::endl;
		std::cin.ignore();
		std::cin.get();
		std::exit(0);
	}
	
	filepath = _filepath;
	frames_processed = 0;
	max_threads = std::thread::hardware_concurrency();
	(max_threads == 0) ? use_n_threads = 0 : use_n_threads = 1;


	input_vid.total_frames = video.get(cv::CAP_PROP_FRAME_COUNT) - 1;

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

	ascii_frames = new std::string[total_frames];
	video.release();
}

Vid2Ascii::~Vid2Ascii() 
{
	std::cout << "Deallocating memory" << std::endl;
	// Deallocate memory from heap
	delete[] map;
	delete[] ascii_frames;
	delete[] frames_processed;
}


/* ---------------------------------------------- */
void Vid2Ascii::playBack()
{
	cv::waitKey(1000);
	system("CLS");
	cv::VideoCapture video(filepath);
	cv::Mat frame;

	float curr_fps = 0;
	int total_frames = input_vid.total_frames;
	int total_duration = input_vid.total_duration;
	float TIMEOUT = 1000 / input_vid.fps; // miliseconds per frame

	auto start = std::chrono::steady_clock::now();
	for (int i = 1; i < total_frames; i++) {
		video >> frame;
		if (frame.empty())
			break;
		cv::resize(frame, frame, cv::Size(320, 180));

		setCursorPosition(0, 0);
		std::cout << ascii_frames[i];
		cv::imshow(" ", frame);

		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		progressBar(i, total_frames);
		std::cout << "[Current time] " << (int)elapsed_seconds.count() << "s       [Duration] " << total_duration << "s       " << std::endl;
		std::cout << "[# threads] " << use_n_threads << "       ";

		curr_fps = i / elapsed_seconds.count();
		std::cout << "[Target FPS] "<< input_vid.fps << "fps       [Current FPS] " << curr_fps << "       " << std::endl;

		if (curr_fps > input_vid.fps)
			cv::waitKey(TIMEOUT);
		
		if (curr_fps < input_vid.fps) {
			i++;
			video >> frame;
		}

		char c = (char)cv::waitKey(24);
		if (c == 27)
			break;
	}

	video.release();
	cv::destroyAllWindows();
}


void Vid2Ascii::generateAscii(unsigned int _th_id)
{
	int th_id, max_ths;
	(use_n_threads == 0) ? max_ths = 1 : max_ths = use_n_threads;
	(_th_id == 0) ? th_id = 1 : th_id = _th_id;

	int total_frames = input_vid.total_frames; // Number of frames in video
	int max_frames = total_frames / max_ths; // Number of frames to be shared among threads
	bool resize = ((int)output_vid.resize_ratio == 1);

	int start = (th_id-1) * max_frames;
	int end = (th_id != max_ths) ? ((th_id-1) * max_frames + max_frames - 1) : (total_frames - 1);

	int ascii_output_height = output_vid.ascii_height;
	int ascii_output_width = output_vid.ascii_width;
	int output_width = output_vid.width;
	int output_height = output_vid.height;
	float cell_width = output_vid.cell_width;
	float cell_height = output_vid.cell_height;

	cv::VideoCapture video(filepath);
	video.set(cv::CAP_PROP_POS_FRAMES, start);
	cv::Mat frame;

	for (int i = start; i <= end; i++) {
		video >> frame;
		if (frame.empty())
			break;

		if (!resize)
			cv::resize(frame, frame, cv::Size(output_width, output_height));

		std::string placeholder;
		for (int hnum = 0; hnum < ascii_output_height; hnum++) {
			for (int wnum = 0; wnum < ascii_output_width; wnum++) {
				// Get ROI position
				int roi_x = wnum * cell_width;
				int roi_y = hnum * cell_height;

				// Get ROI by cropping
				cv::Mat roi(frame, cv::Rect(roi_x, roi_y, cell_width, cell_height));

				// Calculate intensity by pixel/area
				float intensity = cv::mean(roi)[0];
				placeholder += map[(int)((255 - intensity) * num_map_chars / 256)];
			}
			placeholder += "\n";
		}
		ascii_frames[i] = placeholder;

		if (use_n_threads == 0) {
			mu.lock();
			setCursorPosition(0, 1);
			frames_processed[0]++;
			std::cout << "[Progress]";
			progressBar(i, total_frames);
			mu.unlock();
		}

		if (_th_id != 0)
			frames_processed[_th_id - 1]++;
	}
	// Deallocate memory and clear *capture pointer
	video.release();
}

void Vid2Ascii::convert(float resize_ratio, float cell_width, float cell_height)
{
	adjustOutput(resize_ratio, cell_height, cell_width, use_n_threads);
	system("CLS");

	auto start = std::chrono::steady_clock::now();
	
	if (use_n_threads == 0) {
		std::cout << "Processing frames ..." << std::endl;
		frames_processed = new int[1];
		frames_processed[0] = 0;
		generateAscii(0);
		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;

		std::cout << "\n\n--- Processing time: " << elapsed_seconds.count() << "s ---\n";
		std::cout << "Press any key to start ..." << std::endl;
		std::cin.ignore();
		std::cin.get();

		playBack();
	}
	else {
		frames_processed = new int[use_n_threads];
		for (int i = 0; i < use_n_threads; i++) {
			frames_processed[i] = 0;
		}

		std::thread* threads = new std::thread[use_n_threads];
		for (int i = 0; i < use_n_threads; i++) {
			threads[i] = std::thread([&] { generateAscii(i+1); });
			cv::waitKey(100);
		}
		playBack();
		for (int i = 0; i < use_n_threads; i++) {
			threads[i].join();
		}
		delete[] threads;
	}
}