#ifndef MAPGEN_H_INCLUDED
#define MAPGEN_H_INCLUDED

#include <vector>
#include <array>
#include <string>
#include <tuple>

#include <png++/png.hpp>

#include "FRC.h"
#include "fnv.h"
#include "SimplexNoise.h"

#include "cmap.h"
//Increase by changing the FIRST size parameter of cmap and adding new lines
static constexpr size_t numMaps {sizeof(cmap)/sizeof(cmap[0])};
// enum class cmap {Greyscale, Natural, Arctic, Alien, Weird, Ratio, Elevation};
static constexpr std::array<const char*,numMaps> mapNames
	{
		"Greyscale",
		"Natural",
		"Arctic",
		"Alien",
		"Weird",
		"LandRatio",
		"ElevLines",
		"ElevColor",
		"map_8",
		"map_9",
		"map_A",
		"map_B",
		"map_C",
		"map_D",
		"map_E",
		"Random",
	};

//main interface, generates 1024x1024 map with all default settings
void genMap(std::vector<std::string> inList);

const int Greyscale=0, Natural=1, Arctic=2, Alien=3, Weird=4, Ratio=5, Elevation=6;
png::palette genColorMap(int type);
//Opens file and returns a set of palettes
png::color colorFromHex(std::string hex);
std::vector<std::pair<std::string,png::palette>> read_cmap(std::string maps);

extern bool verbose;

#endif //MAPGEN_H_INCLUDED
