#ifndef VID2ASCII_H
#define VID2ASCII_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

class Vid2Ascii {
	private:
		struct base_vid_struct {
			public:
				int height;
				int width;
		};

		struct input_vid_struct : base_vid_struct {
			public:
				double total_frames;
				double fps;
				float total_duration;

		};

		struct output_vid_struct : base_vid_struct {
			public:
				float resize_ratio;
				float cell_height;
				float cell_width;

				int ascii_height;
				int ascii_width;
		};

		input_vid_struct input_vid;
		output_vid_struct output_vid;

		std::string filepath;

		size_t num_map_chars;
		int frames_processed;
		char* map;
		cv::Mat* vid_frames;
		std::string* ascii_frames;

		/** @brief Displays progress bar.
		*
		*	@param idx Index of the current frame being processed.
		*	@param total_frames	The total number of frames being processed.
		*
		*	@return Void.
		*/
		void progressBar(int idx, int total_frames);

		/** @brief Displays an ascii skeleton frame of the output.
		*
		*	The function primary use is to prevent wasting time/energy
		*	when processing a video file only to have the user realize
		*	the size of the output is undesirable.
		* 
		*	@param ascii_height
		*	@param ascii_width
		*
		*	@return Void.
		*/
		void display_adjustedOutputSize(int ascii_height, int ascii_width);

		/** @brief Adjusts params based on user preference
		*
		*	Adjust params based on user request(resize_ratio, _cell_height, _cell_width).
		*
		*	@param resize_ratio
		*	@param cell_width
		*	@param cell_height
		*
		*	@return Void.
		*/
		void adjustOutputSize(float resize_ratio, float _cell_height, float _cell_width);

		/** @brief Displays ascii_frames on console and actively adjusts current frame rate to match original. 
		 *
		 *  @return Void.
		 */
		void playBack();
		
		/** @brief Extract frames from video file and resize frames if necessary. Store cv::Mat frame to dynamic array.
		* 
		*	@return Void.
		*/
		void readFrames();

		/**	@brief Generates ascii characters based on video frame.
		*
		*	This algorithm is uses the pixel/area intensity method (which takes shading into account).
		*	To map pixel/area intensity to ascii characters we use
		*	(255 - intensity) * number of characters in map / 256.
		*	cv::Mat roi crops the frame based on x,y position and crop height/width.
		*
		*	*Note: 0 < th_id and 0 < max_ths. th_id and max_ths should equal 1 if you don't know how
		*	many threads your hardware supports.
		*
		*	@param th_id Assigned thread id, id should be unique.
		*	@param max_ths Max number of threads supported by your hardware.
		*
		*	@return Void.
		*/
		void generateAscii(int th_id, int max_ths);
	public:
		
		/** @brief Constructor
		*
		*	@param filepath
		*	@param intensity_chars
		*/
		Vid2Ascii(std::string filepath, std::string intensity_chars);

		/** @brief Deconstructor - Deallocates map, vid_frames, ascii_frames */
		~Vid2Ascii();

		
		void convert(float resize_ratio, float cell_width, float cell_height);

		/** @brief Moves cursor around in the console. Using (x,y) position.
		*
		*	Instead of using system("CLS"), which increases latency and flickering of
		*	console when clearing the console, we can limit the I/O by overwriting it.
		*
		*	@param x
		*	@param y
		*
		*	@return Void.
		*/
		void setCursorPosition(int x, int y);
};

#endif