#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>
//#include <opencv2/core/utility.hpp>
#include <string>
#include <thread>
#include "Vid2Ascii.h"

#include <mutex>
/* Use ratio 0-1 to resize video, cell height/width should be fixed */

Vid2Ascii::Vid2Ascii(std::string _filepath, std::string intensity_chars)
{
	output_vid.cell_height = 7;
	output_vid.cell_width = 2.5;
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
	//input_vid.total_frames = 1000;

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

/* -------------------------------------------------------- */
void Vid2Ascii::ctrl_brightness_contrast(cv::Mat input, cv::Mat& output, float alpha, int beta)
{
	/*
		alpha - Simple contrast control
		beta - Simple brightness control
	*/
	output = input.clone();

	int rows = output.rows;
	int cols = output.cols;
	int channels = output.channels();

	for (int y = 0; y < rows; y++) {
		cv::Vec3b* ptr = output.ptr<cv::Vec3b>(y);
		for (int x = 0; x < cols; x++) {
			for (int c = 0; c < channels; c++) {
				ptr[x][c] = alpha * ptr[x][c] + beta;
			}
		}
	}
}
/* -------------------------------------------------------- */


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
		progressBar(i);
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
		progressBar(i);
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

/*
	Add a function for preprocessing options
*/

///* -------------------------------------------------------------- */
//void Vid2Ascii::convert(float resize_ratio = 1, float cell_width = 2.5, float cell_height = 7) 
//{
//	adjustOutputSize(resize_ratio, cell_height, cell_width);
//	readFrames();
//
//	//setCursorPosition(0, 0);
//	std::cout << "\nVideo to Ascii Conversion" << std::endl;
//	auto start = std::chrono::steady_clock::now();
//	int total_frames = input_vid.total_frames;
//	int ascii_output_height = output_vid.ascii_height;
//	int ascii_output_width = output_vid.ascii_width;
//	cell_width = output_vid.cell_width;
//	cell_height = output_vid.cell_height;
//	for (int i = 0; i < total_frames; i++) {
//		std::string placeholder;
//		for (int hnum = 0; hnum < ascii_output_height; hnum++) {
//			for (int wnum = 0; wnum < ascii_output_width; wnum++) {
//				// Display ROI(region of interest) bounding box
//				/*cv::Mat canvas = vid_frames[i].clone();
//				cv::rectangle(canvas, cv::Point(wnum*cell_width, hnum*cell_height), cv::Point(wnum * cell_width+ cell_width, hnum * cell_height+ cell_height), cv::Scalar(0,0,255));
//				cv::imshow("crop", canvas);
//				cv::waitKey(1);*/
//
//				// Get ROI position
//				int roi_x = wnum * cell_width;
//				int roi_y = hnum * cell_height;
//
//				// Get ROI by cropping
//				cv::Mat roi(vid_frames[i], cv::Rect(roi_x, roi_y, cell_width, cell_height));
//
//				// Calculate intensity by pixel/area
//				int intensity = cv::mean(roi)[0];
//				placeholder += map[(255 - intensity) * num_map_chars / 256];
//			}
//			placeholder += "\n";
//		}
//		ascii_frames[i] = placeholder;
//
//		setCursorPosition(0, 4);
//		progressBar(i);
//	}
//	auto end = std::chrono::steady_clock::now();
//	std::chrono::duration<double> elapsed_seconds = end - start;
//	std::cout << "---" << std::endl;
//	std::cout << "Processing time: " << elapsed_seconds.count() << "s" << std::endl;
//	std::cout << "---" << std::endl;
//	
//	std::cout << "Press any key to start ..." << std::endl;
//	char wait_input;
//	std::cin >> wait_input;
//
//	playBack();
//}
///* -------------------------------------------------------------- */

std::mutex mu;
void Vid2Ascii::generateAscii(int th_id, int num_ths)
{
	if (th_id == 0 || num_ths == 0) {
		std::cout << "You cannot have zero threads!" << std::endl;
		exit(0);
	}
	int total_frames = input_vid.total_frames;
	int max_frames = total_frames / num_ths;

	int start = (th_id-1) * max_frames;
	int end = (th_id != num_ths ? 
								(th_id-1) * max_frames + max_frames - 1 :
								total_frames);

	int ascii_output_height = output_vid.ascii_height;
	int ascii_output_width = output_vid.ascii_width;
	int cell_width = output_vid.cell_width;
	int cell_height = output_vid.cell_height;
	for (int i = start; i < end; i++) {
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
				int intensity = cv::mean(roi)[0];
				placeholder += map[(255 - intensity) * num_map_chars / 256];
			}
			placeholder += "\n";
		}
		ascii_frames[i] = placeholder;

		mu.lock();
		setCursorPosition(0, 4 + th_id);
		frames_processed++;
		std::cout << "Threads[" << th_id << "]";
		progressBar(frames_processed);
		mu.unlock();
	}
}



void Vid2Ascii::convert(float resize_ratio = 1, float cell_width = 2.5, float cell_height = 7)
{
	adjustOutputSize(resize_ratio, cell_height, cell_width);
	readFrames();

	/*
		std::thread::hardware_concurrency() will return the 
		number of concurrent threads supported by the hardware.
		If the number of concurrent threads not supported in the
		hardware implementation the function will return 0.
	*/
	
	//unsigned int max_threads = std::thread::hardware_concurrency();
	
	unsigned int max_threads = 8;
	std::cout << "\nVideo to Ascii Conversion" << std::endl;
	auto start = std::chrono::steady_clock::now();

	// generateAscii(1,1);

	std::thread th1([&] { generateAscii(1, max_threads); });
	std::thread th2([&] { generateAscii(2, max_threads); });
	std::thread th3([&] { generateAscii(3, max_threads); });
	std::thread th4([&] { generateAscii(4, max_threads); });
	std::thread th5([&] { generateAscii(5, max_threads); });
	std::thread th6([&] { generateAscii(6, max_threads); });
	std::thread th7([&] { generateAscii(7, max_threads); });
	std::thread th8([&] { generateAscii(8, max_threads); });

	th1.join();
	th2.join();
	th3.join();
	th4.join();
	th5.join();
	th6.join();
	th7.join();
	th8.join();

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;

	setCursorPosition(0, 15);
	std::cout << "---" << std::endl;
	std::cout << "Processing time: " << elapsed_seconds.count() << "s" << std::endl;
	std::cout << "---" << std::endl;
	std::cout << "frames_processed: " << frames_processed << std::endl;

	std::cout << "Press any key to start ..." << std::endl;
	char wait_input;
	std::cin >> wait_input;

	playBack();
}