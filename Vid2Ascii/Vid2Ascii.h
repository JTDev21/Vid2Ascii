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
				int total_frames;
				float fps;
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

		int num_map_chars;
		int frames_processed;
		char* map;
		cv::Mat* vid_frames;
		std::string* ascii_frames;



		/** @brief Displays progress bar.
		*
		*	@param n_frame Index of the current frame being processed.
		*	@param total_frames	The total number of frames inside a video file.
		*
		*	@return Void.
		*/
		void progressBar(int idx);

		void display_adjustedOutputSize(int ascii_height, int ascii_width);

		/** @brief Displays the skeleton frame of an ascii image.
		*
		*	The function primary use is to prevent wasting time/energy
		*	when processing a video file only to have the user realize
		*	the size of the output is undesirable.
		*
		*	@param filepath
		*	@param cell_width
		*	@param cell_height
		*
		*	@return Void.
		*/
		void adjustOutputSize(float resize_ratio, float _cell_height, float _cell_width);

		/** @brief Changes the brightness/contrast of an image.
		 *
		 *  @param input Input array(type: cv::Mat).
		 *  @param output Output array(type: cv::Mat).
		 *	@param alpha Alpha controls the contrast of an image.
		 *	@param beta Beta controls the brightness of an image.
		 *
		 *  @return Void.
		 */
		void ctrl_brightness_contrast(cv::Mat input, cv::Mat& output, float alpha, int beta);
		void playBack();
		
	public:
		void generateAscii(int th_id, int num_ths);
		/** @brief Constructor
		*
		*	@param filepath
		*	@param intensity_chars
		*/
		Vid2Ascii(std::string filepath, std::string intensity_chars);

		/** @brief Deconstructor - Deallocates map, vid_frames, ascii_frames */
		~Vid2Ascii();

		void readFrames();


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