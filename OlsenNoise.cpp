#include "SimplexNoise.h"

namespace OlsenNoise {

	std::vector<std::vector<int>> genNoise(int x_start, int y_start,
		int x_size, int y_size, int x_incr=1, int y_incr = 2)
	{
		
	}

	void convolve(std::vector<int>& pixels, int offset, int stride,
			int x, int y, int width, int height,
			const std::array<std::array<int,3>,3>& matrix)
	{
		int index = offset + x + (y*stride); //index is where we are in the pixels. All equal 0 for our use. Y=0, X=0, offset = 0.
		for (int j = 0; j < height; j++, index += stride) { // iterate the y values. adding stride to index each time.
			for (int k = 0; k < width; k++) {//iterate the x values
				int pos = index + k; //current position sum of the strides and the position in the x.
				pixels[pos] = priv::convolve(pixels,stride,pos, matrix); //convolves the matrix down and to the right from the current position.
				//this is somewhat non-standard, but that's because everybody's been doing it wrong for basically ever.
			}
		}
	}
	
	std::vector<int> trim(int width, int height,
		const std::vector<int>& workingpixels, int workingstride)
	{
		std::vector<int> pixels(width*height);
		for (int k = 0; k < height; k++) {
			for (int j = 0; j < width; j++) {
				 int index = j + (k * width);
				 int workingindex = j + (k * workingstride);
				 pixels[index] = workingpixels[workingindex];
			}
		}
		return pixels;
	}
	
	namespace priv {
		
		void olsennoise(std::vector<int>& pixels, int stride,
			int x_within_field, int y_within_field, int width, int height,
			int iteration)
		{
			if (iteration == 0) {
				clearValues(pixels, stride, width, height);
				applyNoise(pixels, stride, x_within_field, y_within_field,
					width, height, iteration);
				return;
			}
			int x_remainder = x_within_field & 1,
				y_remainder = y_within_field & 1;
			olsennoise(pixels, stride, 
				((x_within_field + x_remainder) / scaleFactor) - x_remainder,
				((y_within_field + y_remainder) / scaleFactor) - y_remainder,
				((width  + x_remainder) / scaleFactor) + blurEdge,
				((height + y_remainder) / scaleFactor) + blurEdge, iteration - 1
			);
			applyScale(pixels, stride, width+blurEdge, height + blurEdge,
				scaleFactor);
			applyShift(pixels, stride, x_remainder, y_remainder,
				width + blurEdge, height + blurEdge);
			applyBlur(pixels, stride, width + blurEdge, height + blurEdge);
			applyNoise(pixels, stride, x_within_field, y_within_field,
				width, height, iteration);
		}
		
		void applyNoise(std::vector<int>& pixels, int stride,
			int x_within_field, int y_within_field, int width, int height,
			int iteration)
		{
			int index = 0;
			for (int k = 0, n = height - 1; k <= n; k++, index += stride) { //iterate the y positions. Offsetting the index by stride each time.
				for (int j = 0, m = width - 1; j <= m; j++) { //iterate the x positions through width.
					int current = index + j; // The current position of the pixel is the index which will have added stride each, y iteration
					pixels[current] += (FNV({j + x_within_field, k + y_within_field, iteration}) & (1 << (7 - iteration)));
					//add on to this pixel the hash function with the set reduction.
					//The amount of randomness here somewhat arbitary. Just have it give self-normalized results 0-255.
					//It simply must scale down with the larger number of iterations.
				}
			}
		}
		
		void applyScale(std::vector<int>& pixels, int stride,
			int width, int height, int factor)
		{
			int index = (height - 1) * stride; //We must iteration backwards to scale so index starts at last Y position.
			for (int k = 0, n = height - 1; k <= n; n--, index -= stride) { // we iterate the y, removing stride from index.
				for (int j = 0, m = width - 1; j <= m; m--) { // iterate the x positions from width to 0.
					int current = index + m; //current position is the index (position of that scanline of Y) plus our current iteration in scale.
					int lower = ((n / factor) * stride) + (m / factor); //We find the position that is half that size. From where we scale them out.
					pixels[current] = pixels[lower]; // Set the outer position to the inner position. Applying the scale.
				}
			}
		}
		
		void applyShift(std::vector<int>& pixels, int stride,
			int shiftX, int shiftY, int width, int height)
		{
			if ((shiftX == 0) && (shiftY == 0)) { //if we aren't actually trying to move it.
				return; //return
			}

			int index;
			int indexoffset = shiftX + (shiftY * stride); //The offset within the array that that shift would correspond to.
			//Since down and to the right is still (stride + 1) every loop. We preset it.
			index = 0;
			for (int k = 0, n = height - 1; k <= n; k++, index += stride) { // iterate the y values, add stride to index.
				for (int j = 0, m = width - 1; j <= m; j++) { //iterate the x values with j.
					int current = index + j; // current position is all our added up stride values, and our position in the X.
					pixels[current] = pixels[current + indexoffset]; //set the current pixel equal to the one shifted by offset.
				}
			}
		}
		
		void clearValues(std::vector<int>& pixels, int stride,
			int width, int height)
		{
			int index;
			index = 0;
			for (int k = 0, n = height - 1; k <= n; k++, index += stride) { //iterate the y values.
				for (int j = 0, m = width - 1; j <= m; j++) { //iterate the x values.
					int current = index + j; // current position is the sum of the strides plus the position in the x.
					// int pixel = pixels[current]; //get the current pixel.
					pixels[current] = 0; //clears those values.
				}
			}
		}
		
		void applyBlur(std::vector<int>& pixels, int stride,
			int width, int height)
		{
			::OlsenNoise::convolve(pixels, 0, stride, 0, 0, width, height, blur3x3);
		}
		
		int convolve(std::vector<int>& pixels, int stride, int index,
			const std::array<std::array<int,3>,3>& matrix)
		{
			int parts = 0;
			int sum = 0;
			int factor;
			for (int j = 0, m = matrix.size(); j < m; j++, index+=stride) { //iterates the matrix
				for (int k = 0, n = matrix[j].size(); k < n; k++) { //iterates the matrix[] within.
					factor = matrix[j][k]; //gets the multiple from that matrix.
					parts += factor; //keeps a running total for the parts.
					sum += factor * pixels[index + k]; //keeps a total of the sum of the factors and the pixels they correspond to.
				}
			}
			if (parts == 0) {
				return crimp(sum);
			}
			return crimp(sum/parts);
		}
		
	};
};