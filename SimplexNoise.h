#ifndef SIMPLEXNOISE_H
#define SIMPLEXNOISE_H

#include <vector>
#include <array>
#include <cstddef>
// #include "Array2D.h"
#include "fnv.h"

class SimplexNoise {
public:
	SimplexNoise(unsigned Octaves, float Gain,
		float Lacunarity, float Frequency = 0.002f,
		float Amplitude = 50.0f);
	// void makeSomeNoise(Array_2D<float>& height_map, int z);
	std::vector<std::vector<float>> genNoise(
		const std::vector<float>& xgrid,
		const std::vector<float>& ygrid,
		float z);
private:
	std::array<uint8_t,256> permutation_table;//randomized tables
	float 		gradient_table[8][3];
	
	unsigned 	max_octaves; //Fractional Brownian Motion variables
	float		lacunarity;
	float		gain;
	float		start_frequency;
	float		start_amplitude;
	
	float make_point(float x, float y, float z);
	
	
};

namespace OlsenNoise {
	// std::vector<std::vector<float>> genNoise(std::vector<float> xgrid, std::vector<float> ygrid);
	std::vector<std::vector<int>> genNoise(int x_start, int y_start,
		int x_incr=1, int y_incr = 1);
	
	constexpr int maxIterations = 7;
	namespace priv {
		constexpr std::array<std::array<int,3>,3> blur3x3 =
			{{{1,1,1},
			  {1,1,1},
			  {1,1,1}}};
		
		constexpr int blurEdge = 2, scaleFactor = 2;
	};
	
	inline void olsennoise(int iterations, std::vector<int>& pixels,
		int stride, int x, int y, int width, int height);
	inline void olsennoise(std::vector<int>& pixels, int stride,
		int x, int y, int width, int height);
	
	inline constexpr int getRequiredDim(int dim) {
		return dim+priv::blurEdge+priv::scaleFactor;
	}
	
	void convolve(std::vector<int>& pixels, int offset, int stride,
		int x, int y, int width, int height,
		const std::array<std::array<int,3>,3>& matrix=priv::blur3x3);
	
	std::vector<int> trim(int width, int height,
		const std::vector<int>& workingpixels, int workingstride);
		
	inline unsigned hashrandom(const std::vector<int>& elements);
	
	inline unsigned FNV(const std::vector<int>& data) {
		return FNV32a(data.data(), data.size() * sizeof(int));
	}
	
	namespace priv {
		void olsennoise(std::vector<int>& pixels, int stride,
			int x_within_field, int y_within_field, int width, int height,
			int iteration);
		
		void applyNoise(std::vector<int>& pixels, int stride,
			int x_within_field, int y_within_field, int width, int height,
			int iteration);
		
		void applyScale(std::vector<int>& pixels, int stride,
			int width, int height, int factor);
		
		void applyShift(std::vector<int>& pixels, int stride,
			int shiftX, int shiftY, int width, int height);
		
		void clearValues(std::vector<int>& pixels, int stride,
			int width, int height);
		
		void applyBlur(std::vector<int>& pixels, int stride,
			int width, int height);
		
		inline constexpr int crimp(int color) {
			return (color >= 0xFF) ? 0xFF : (color < 0) ? 0 : color;
		}
		
		int convolve(std::vector<int>& pixels, int stride, int index,
			const std::array<std::array<int,3>,3>& matrix);
		
		inline unsigned long long hash(unsigned long long hash) {
        unsigned long long h = hash;

        switch (static_cast<int>(hash) & 3) {
			case 3:
				 hash += h;
				 hash ^= hash << 32;
				 hash ^= h << 36;
				 hash += hash >> 22;
				 break;
			case 2:
				 hash += h;
				 hash ^= hash << 22;
				 hash += hash >> 34;
				 break;
			case 1:
				 hash += h;
				 hash ^= hash << 20;
				 hash += hash >> 2;
        }
        hash ^= hash << 6;
        hash += hash >> 10;
        hash ^= hash << 8;
        hash += hash >> 34;
        hash ^= hash << 50;
        hash += hash >> 12;
        return hash;
		}
	};
	
	inline void olsennoise(int iterations, std::vector<int>& pixels,
		int stride, int x, int y, int width, int height) {
		priv::olsennoise(pixels, stride, x, y, width, height, iterations); //Calls the main routine.
	}
	
	inline void olsennoise(std::vector<int>& pixels, int stride, int x,
		int y, int width, int height) {
		priv::olsennoise(pixels, stride, x, y, width, height, maxIterations);
	}
	
	unsigned hashrandom(const std::vector<int>& elements) {
		unsigned long long hash {0};
		for (auto i : elements) {
			hash ^= i;
			hash = priv::hash(hash);
		}
		return static_cast<int>(hash);
	}
};
#endif
