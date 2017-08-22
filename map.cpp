#include "map.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstdint>

#include <chrono>
using namespace std::chrono;

#include <deque>

using std::vector;
using std::string;
using std::cin;
using std::cout;
using std::cerr;
using std::clog;
using std::endl;

extern char** environ;

constexpr inline float plateau(const float r)
{
	return r*r/(r*r+1);
}

template<typename T>
constexpr inline T square(const T x)
{
	return x*x;
}

vector<vector<float>>& genSpiral(
		const bool logarithmic,
		vector<vector<float>>& buf,
		const vector<float>& x_vals,
		const vector<float>& y_vals);

//filename left lower width pixels
void genMap(vector<string> inList)
{
	float seed = 5867;
	std::deque<std::pair<high_resolution_clock::time_point,const char*>> ts;
	ts.push_back({high_resolution_clock::now(),"Begin"});
	
	//generate a square image tile
	//const static unsigned short tileSize = 256;
	unsigned short tileSize = fromStr<unsigned short int>(inList[5]);
	png::image<png::index_pixel> image(tileSize,tileSize);
	png::palette cmap;

#ifdef _DEBUG
	clog<<"called as: "
		<<inList.at(0)<<", "
		<<inList.at(1)<<", "
		<<inList.at(2)<<", "
		<<inList.at(3)<<", "
		<<inList.at(4)<<", "
		<<inList.at(5)
		<<std::endl;
#endif
	auto a = string::npos;
	if ((a = inList[0].find_first_of("0123456789ABCDEFabcdef")) != string::npos) {
		auto map = stoi(inList[0].substr(a,1), nullptr, 16);
		cmap = genColorMap(map);
#ifdef _DEBUG
		clog<<"Using colormap "<<map<<" ("<<mapNames[map]<<")"<<endl;
#endif
	} else {
		cmap = genColorMap(Natural);
	}
	image.set_palette(cmap);
	// ts.push_back({high_resolution_clock::now(),"Setup image"});
	
	float xmin = fromStr<float>(inList[2]), ymin = fromStr<float>(inList[3]),
		width = fromStr<float>(inList[4]);
	float xmax = xmin+width, ymax = ymin+width;
	
	vector<float> x_vals(tileSize), y_vals(tileSize);
	float xdiff = (xmax-xmin)/tileSize, ydiff = (ymax-ymin)/tileSize;
	for (int i = 0; i < tileSize; ++i) {
		x_vals[i] = xmin + xdiff*i;
		y_vals[i] = ymin + ydiff*i;
	}
	// ts.push_back({high_resolution_clock::now(),"Setup noise"});
	
	SimplexNoise noise(10, 0.65, 2.06, 0.0009f, 65);
	vector<vector<float>> imgBuf(tileSize,vector<float>(tileSize,0.f));
	//noise.makeSomeNoise(imgBuf, seed);
	imgBuf = noise.genNoise(x_vals, y_vals, seed);
	
	ts.push_back({high_resolution_clock::now(),"Simplex"});
	
	genSpiral(inList[0][1] != 'l', imgBuf, x_vals, y_vals);
	
	ts.push_back({high_resolution_clock::now(),"Apply spiral"});
	
	png::index_pixel V = 0;
	for (size_t x = 0; x < tileSize; ++x) {
		for (size_t y = 0; y < tileSize; ++y) {
			// double temp = imgBuf[x][y] + 255 - fabs(imgBuf[x][y]-255);
			// V = (temp + fabs(temp)) * 0.25;
			V = (imgBuf[x][y] < 0.5f)
				? 0 : (
					(imgBuf[x][y] > 254.5f)
					? 255 : static_cast<png::byte>(imgBuf[x][y])
				);
			image[x][y] = V;
		}
	}
	// ts.push_back({high_resolution_clock::now(),"Fill image"});//*/
	
	if (inList[1] != "-")
		image.write(inList[1]);
	else
		image.write_stream<std::ostream>(cout);
	if (verbose) {
		ts.push_back({high_resolution_clock::now(),"Write Image"});
		auto prev = ts[0];
		ts.pop_front();
		for (auto i : ts) {
			// cerr<<i.second<<": "<<(i.first-prev).count()/1000000ull<<"; ";
			clog<<duration_cast<milliseconds>(i.first-prev.first).count()<<"ms\t"<<i.second<<std::endl;
			prev = i;
		}
	}
}

vector<vector<float>>& genSpiral(
		const bool logarithmic,
		vector<vector<float>>& buf,
		const vector<float>& x_vals,
		const vector<float>& y_vals)
{
	
	//Some constants
	constexpr float
		pi_up = 3.141593, //Pi, rounded slightly up
		pi_down = 3.141592, //and down
		//Spiral term (shared)
		spiral_strength = 60, //How strong the spiral motif is
		//Spiral term (archimedian)
		spiral_tightness = 0.05, //turning pitch of spiral
		spiral_sharpness = 1.40, //how much to exaggerate peaks by (exponential)
		spiral_negative_bias = 1.2, //how much to subtract from spiral term
		spiral_flatness = 1/1.25, //how much to dampen peaks by (linear)
		//Plateau term
		plt_radius = 150, //Radius of the central plateau in ADU
		// plt_height = 24, //How high the central plateau is (logarithmic)
		plt_height_a = 48, //How high the central plateau is (archimedian)
		plt_cliffiness = 20, //How smooth the slope to the central plateau is
			plt_cliffiness_x = 1/plt_cliffiness,
		// plt_prescale = 1.15, //Scales the plateau term before normalization
		plt_flatness = 1.3, //How much to flatten the central plateau by
		// plt_l10n_adj = plt_height_a/35-pi_down, //Reduces the large-scale effect of the plateau term
		plateau_hinge = 160,
		// plt_l10n_adj2 = plt_l10n_adj/24,
		//Noise term
		noise_hinge = 155, //Hinge which noise will be scaled away from
		noise_scale = 1.20, //How much to scale noise by
		//Adjustment
		sea_level = 135,
		sea_adjustment = noise_hinge - 8; //Overall factor to adjust the map height by
	const float
		//Spiral term (log)
		spiral_curviness = 1.12, //Base of the logarithm used for the spiral
			spiral_curviness_x = 1/log(spiral_curviness);
	
	// vector<vector<std::pair<float,float>>> pc(x_vals.size(), vector<std::pair<float,float>>(y_vals.size()));
	// ts.push_back({high_resolution_clock::now(),"Process setup"});
	for (size_t x = 0; x < x_vals.size(); ++x) {
		for (size_t y = 0; y < y_vals.size(); ++y) {
			float r = sqrt(x_vals[x]*x_vals[x]*.1f+y_vals[y]*y_vals[y]*.1f);
			float t = atan2(x_vals[x],y_vals[y]);
			buf[x][y] = 
				(buf[x][y]-noise_hinge)*noise_scale
				+sea_adjustment;
			if (logarithmic) {
				buf[x][y] +=
					spiral_strength
					*(pow(
						(sin(
							t
							+spiral_tightness*r
						)+1.f),
						spiral_sharpness)
					*spiral_flatness
					-spiral_negative_bias);
			} else {
				buf[x][y] +=
					spiral_strength
					*(pow(
						(sin(
							t
							+spiral_curviness_x*log(r)
						)+1.f),
						spiral_sharpness)
					*spiral_flatness
					-spiral_negative_bias);
			}	
			auto p = atan(plt_cliffiness_x*(r-plt_radius))/(pi_up)+0.5;
			buf[x][y] =
				(buf[x][y]-plateau_hinge)
				/((1+square(plt_flatness))-square(plt_flatness*p))
				+plt_height_a*(1-p)+plateau_hinge;
		}
	}
	return buf;
}

png::color colorFromHex(std::string hexStr)
{
	png::byte r, g, b;
	uint_fast32_t c;
	std::stringstream(hexStr)>>std::hex>>c;
	r = (c&0xFF0000)>>16;
	g = (c&0x00FF00)>>8;
	b = (c&0x0000FF);
	return png::color(r,g,b);
}

std::vector<std::pair<std::string,png::palette>> read_cmap(std::string file)
{
	std::ifstream cmapFile {file};
	return {};
	/* Format of data file:
	 * 
	 * e := {empty}
	 * delim := {1st char}
	 * char := [^delim] {Anything but delim}
	 * string := char* {Empty, or a char plus a string}
	 * digit := [0-9]
	 * number := digit+
	 * hexdigit := [0-9A-Fa-f]
	 * byte := hexdigit . hexdigit
	 * color := byte . byte . byte
	 * lsep := ([:punct:] | [:space:])*
	 * clist := color . space* {colors may have arbitrary spacing after}
	 * cmap := string . delim . number . space+ . clist
	 * 
	 * file := delim . cmap . (delim . cmap)*
	 * 
	 */
	// A clist must have as many colors as the preceding number
	if (!cmapFile) {
		//file not opened
	} else if (cmapFile.eof()) {
		//file is empty
	}
	
	//state machine:
	char delim;
	
	enum class states : uint8_t {
		START,
		CMAP,
		STRING,
		NUMBER,
		CLIST,
		HEXCOLOR,
	};
	
}

png::palette genColorMap(int type)
{
	png::palette ret(256);
	if (type >= numMaps)
		return ret;
	switch (type) {
	case 0://Greyscale
		for (short x = 0; x < 256; ++x) {
			ret[x] = png::color(x,x,x);
		}
		break;
	case 1://Natural
		for (short x = 0; x < 256; ++x) {
			if (x < 132) {//ocean
				ret[x] = png::color(
					0,x/3+4*static_cast<char>(static_cast<double>(x)*x/768.),x+70
				);
			} else if (x < 136) {//beach
				ret[x] = png::color(240,240,64);
			} else if (x < 200) {//grass, forests
				ret[x] = png::color(48-(x-168),180-(x-168),(x-100)/2);
			} else if (x < 248) {//Prevent underflow of red
				ret[x] = png::color(0         ,180-(x-168),(x-160));
			} else if (x < 252) {//snowy forest
				ret[x] = png::color(x-64,248,x-24);//*/
			} else {             //snow
				ret[x] = png::color(x-24,x-12,x-8);//Make snow bluish
			}
		}
		break;
	case 2://Arctic
	//Broken
		for (short x = 0; x < 256; ++x) {
			if (x < 149) {
				ret[x] = png::color(0,0,x+77);
			} else if (x == 149) {
				ret[x] = png::color(0,128,255);
			} else if (x < 155) {
				ret[x] = png::color(240-3*(x-155),240,64+4*(x-155));
			} else if (x < 162) {
				ret[x] = png::color(32-(x-162),180-(x-162),0+(x-162));
			} else if (x < 210) {
				ret[x] = png::color(0,128,32);//*/
			} else {
				ret[x] = png::color(x-4,x-2,x);//Make snow bluish
			}
		}
		break;
	case 3://Alien
	//Broken
		for (short x = 0; x < 256; ++x) {
			if (x < 149) {//0+
				ret[x] = png::color(0,0,x+77);
			} else if (x == 149) {//149+
				ret[x] = png::color(0,128,255);
			} else if (x < 162) {//150+
				ret[x] = png::color(240-3*(x-155),240,64+4*(x-155));
			} else if (x < 182) {//162+
				ret[x] = png::color(32-(x-162),180-(x-162),(x-162));
			} else if (x < 210) {
				ret[x] = png::color(0,128,32);//*/
			} else {//182+
				ret[x] = png::color(x-4,x-2,x);//Make snow bluish
			}
		}
		break;
	case 4://Purple, red (Natural * RGB=>GRB)
		for (short x = 0; x < 256; ++x) {
			ret[x] = png::color(cmap[1][x][1],cmap[1][x][0],cmap[1][x][2]);
		}
		break;
	case 5://Water ratio map
		for (short x = 0; x < 256; ++x) {
			if (x < 132) {//ocean
				ret[x] = png::color(0,0,100);
			} else {//land
				ret[x] = png::color(48,180,48);
			}
		}
		break;
	case 6://Elevation
		for (short x = 0; x < 256; ++x) {
			if (x >= 132 && x <= 135) {//shore
				ret[x] = png::color(64,64,64);
			} else if (x < 132 && ((x-132)%10 == 0)) {//ocean depth marks
				ret[x] = png::color(128,128,128);
			} else if (x > 135 && ((x-135)%10 == 0)) {//land height marks
				ret[x] = png::color(192,192,192);
			} else {
				ret[x] = png::color(255,255,255);
			}
		}
		break;
	case 7://Elevation-color
		for (short x = 0; x < 256; ++x) {
			if (x >= 132 && x <= 135) {//shore
				ret[x] = png::color(64,64,64);
			} else if (x < 132 && ((x-132)%10 == 0)) {//ocean depth marks
				ret[x] = png::color(128,128,128);
			} else if (x > 135 && ((x-135)%10 == 0)) {//land height marks
				ret[x] = png::color(192,192,192);
			} else {
				ret[x] = png::color(255,255,255);
			}
		}
		break;
	case 15: //Hash
		{
			int h = -1;
			for (size_t x = 0; x < 256; ++x) {
				h = FNV32a(&h,sizeof(h));
				ret[x] = png::color(h&255,(h>>8)&255,(h>>16)&255);
			}
		}
		break;
	default: //Fill cmap.h yourself and rebuild to get new maps
		for (short x = 0; x < 256; ++x) {
			ret[x] = png::color(cmap[type][x][0],cmap[type][x][1],cmap[type][x][2]);
		}
		break;
	}
	return ret;
}