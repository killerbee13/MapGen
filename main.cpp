#include <iostream>
#include <iomanip>

#include <vector>
#include <array>
#include <string>
#include <tuple>
#include <cmath>

#include <png++/png.hpp>

#include "FRC.h"
#include "input.h"
#include "map.h"

#include "version.h"

//HelpStr, iHelpStr
#include "strings.h"

using std::string;
using std::vector;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

//main interface, generates 1024x1024 map with all default settings
// void genMap(vector<string> inList);


bool Continue = true;
#ifdef _DEBUG
bool verbose = true;
#else
bool verbose = false;
#endif
input_trie commands;

void runCommand(input_trie commands, string in);

int main(int argc, char** argv)
{
	string in;
	commands.insert(vector<std::pair<string, functdata*>>({{
		std::pair<string, functdata*>(string("h"), new functdata(
			[](vector<string> inList){
				if (inList.size() > 1) {
					int len = 0;
					functdata *found = commands.longestPrefix(toLower(inList[1]), &len);
					if (found) {
						cout<<found->help<<endl;
					} else {
						cout<<"Nothing found for "<<toLower(inList[1])<<endl;
					}
				} else {
					//from strings.h
					cout<<iHelpStr;
			}},
			0, string("Get help"))),
		std::pair<string, functdata*>(string("q"), new functdata(
			[](vector<string> inList){
				Continue = false;
			},
			0, string("Quit"))),
		std::pair<string, functdata*>(string("e"), new functdata(
			[](vector<string> inList){
				cout<<'[';
				for (unsigned i = 0; i != inList.size(); i++) {
					cout<<inList[i]<<':';
				}
				cout<<']'<<endl;
			},
			0, string("Echo inputs"))),
		std::pair<string, functdata*>(string("x"), new functdata(
			[](vector<string> inList){
				inList.erase(inList.begin());
				int len = 0;
				functdata *found = commands.longestPrefix(toLower(inList[0]), &len);
				if (found) {
					if (inList.size() > (unsigned)found->argC) {
						(*found)(inList);
					} else {
						cout<<"Please specify all "<<found->argC<<" arguments for "<<inList[0]<<endl;
						cout<<"Tokens received (e notation): "<<endl<<'[';
						for (auto i : inList) {
							cout<<i<<':';
						}
						cout<<']'<<endl;
					}
				} else {
					cout<<"Nothing found for "<<toLower(inList[0])<<endl;
			}},
			1, string("Execute Command (Very sophisticated no-op)"))),
		std::pair<string, functdata*>(string("p"), new functdata(
			[](vector<string> inList){
				if (inList.size() == 1) {
				auto dump = commands.getIndex();
					for (unsigned i = 0; i != dump.size(); i++) {
						cout<<i<<": ";
						for (unsigned char j = 0; j != 128; j++) {
							if (dump[i][j].first || dump[i][j].second) {
								cout<<(char)(j?j:'/')<<dump[i][j].first<<(dump[i][j].second?'*':' ')<<' '<<std::flush;
							}
						}
						cout<<endl;
						if (inList[0][1] == 'l') {
							for (unsigned char j = 0; j != 128; j++) {
								cout<<dump[i][j].first<<(dump[i][j].second?'*':' ')<<' '<<std::flush;
								if (j%16 == 15)
									cout<<endl;
							}
							cout<<endl;
						}
					}
				} else {
					int len = 0;
					functdata *found = commands.longestPrefix(toLower(inList[1]), &len);
					if (found)
						cout<<"Matched \""<<inList[1].substr(0,len)<<'"'<<endl
							<<found->help<<" [takes "<<found->argC<<']'<<(found->call?' ':'!')<<endl;
					else
						cout<<len<<";Nothing found for "<<toLower(inList[1])<<endl;
			}},
			0, string("Print/Probe input trie"))),
		std::pair<string, functdata*>(string("m"), new functdata(
			genMap, 5, string("Draw map (filename left upper width resolution)"))),
		std::pair<string, functdata*>(string("t"), new functdata(
			[](vector<string> inList){
				png::image<png::rgb_pixel> image(256,numMaps);
				cout<<"unsigned char cmap["<<numMaps<<"][256][3] {{"<<endl;
				for (short i = 0; i < numMaps; ++i) {
					auto cmap = genColorMap(i);
					cout<<"{{";
					for (short x = 0; x < 256; ++x) {
						image[i][x] = {cmap[x].red,cmap[x].green,cmap[x].blue};
						cout<<"{"<<+cmap[x].red<<','<<+cmap[x].green<<','<<+cmap[x].blue<<"},";
						if (x%64==63) {
							cout<<endl;
						}
					}
					cout<<"}},"<<endl;
				}
				cout<<"}};"<<endl;
				//image.write_stream<std::ostream>(cout);
				image.write(inList[1]);
			},
			1, string("Test colors (<output>.png)"))),
		std::pair<string, functdata*>(string("c"), new functdata(
			[](vector<string> inList){
				png::image<png::rgb_pixel> image(256,numMaps);
				for (short i = 0; i < numMaps; ++i) {
					auto cmap = genColorMap(i);
					for (short x = 0; x < 256; ++x) {
						image[i][x] = {cmap[x].red,cmap[x].green,cmap[x].blue};
					}
				}
				image.write(inList[1]);
			},
			1, string("Print colormaps (<output.png>)"))),
		std::pair<string, functdata*>(string("pc"), new functdata(
			[](vector<string> inList){
				for (short i = 0; i < numMaps; ++i) {
					cout<<":"<<mapNames[i]<<":"<<std::dec<<cmap[i].size()<<std::hex;
					for (short x = 0; x < cmap[i].size(); ++x) {
						if (x%8==0) {
							cout<<endl<<"\t";
						}
						cout<<std::setw(2)<<std::setfill('0')<<+cmap[i][x][0]
							<<std::setw(2)<<std::setfill('0')<<+cmap[i][x][1]
							<<std::setw(2)<<std::setfill('0')<<+cmap[i][x][2]
							<<" ";
					}
					cout<<endl;
				}
				cout<<std::dec;
			},
			0, string("Formats colormaps"))),
		}}));
	
	vector<string> scriptedCommands;
	
	bool display_promt = true, display_promt_s = false, cont_after = false;
	
	for (int i = 1; i < argc; i++) {
		//cout<<"["<<i<<"] "<<argv[i]<<endl;
		if (argv[i][0] == '-') {
			for (int j = 1; argv[i][j] != '\0'; ++j) {
				if (argv[i][j] == 'q') { //quiet (Don't display prompts)
					display_promt = false;
				} else if (argv[i][j] == 'h') { //help
					//from strings.h
					cout<<HelpStr;
					Continue = false;
				//verbose (print timing info)
				} else if (argv[i][j] == 't') {
					verbose = true;
				//verbose (Simulate interactive display for scripted commands)
				} else if (argv[i][j] == 'v') {
					display_promt_s = true;
				} else if (argv[i][j] == 'V') {
					cout<<"Version "<<vnum<<endl;
					Continue = false;
				} else if (argv[i][j] == 'c') { //continue after scripted commands
					cont_after = true;
				} else if (argv[i][j] == '-') { //continue after scripted commands
					continue;
				} else {
					cerr<<"Note: ignored option "<<argv[i][j]<<std::endl;
				}
			}
		} else { //interpret other strings as commands to be run
			scriptedCommands.emplace_back(argv[i]);
		}
	}
	
	Continue = Continue & !scriptedCommands.size();
	
	for (auto i : scriptedCommands) {
		if (display_promt_s && display_promt) cout<<"> "<<i<<endl;
		runCommand(commands, i);
	}
	
	//Since both are simple variables short-circuiting doesn't really help, and
		//bools are guaranteed to only be 1 or 0
	Continue = Continue | cont_after;
	
	//Don't end when cin fails, clear and move on
	while (Continue) {
		//Actual input loop, large-scope try because all exceptions are outside my code
		while (cin && Continue) {
			try {
				if (display_promt)
					cout<<"> ";
				getline(cin, in);
				if (in.empty()) {
					continue;
				}
				runCommand(commands, in);
			} catch (const std::runtime_error& err) {
				cout<<"Failed to read input. "<<err.what()<<endl;
			} catch (const std::exception& err) {
				cout<<"Unknown error: "<<err.what()<<endl;
				throw err;
			} catch (...) {
				cout<<"Unknown error"<<endl;
				throw;
			}
		}
		if (!cin.eof())
			cin.clear();
		else {
			cout<<endl;
			break;
		}
	}
	return 0;
}

void runCommand(input_trie commands, string in)
{
	if (in.empty()) {
		return;
	}
	
	//Split input on word boundaries
	vector<string> inList = tokenizeInput(in);
	//inList[0] = toLower(inList[0]);
	std::transform(inList[0].begin(), inList[0].end(), inList[0].begin(), ::tolower);
	
	int len = 0;
	functdata *found = commands.longestPrefix(toLower(inList[0]), &len);
	if (found) {
		if (inList.size() > (unsigned)found->argC) {
			(*found)(inList);
		} else {
			cout<<"Please specify all "<<found->argC<<" arguments for "<<inList[0]<<endl;
			cout<<"arguments recieved: [";
			for (auto i : inList) {
				cout<<i<<':';
			}
			cout<<']'<<endl;
		}
	} else {
		cout<<"Nothing found for "<<toLower(inList[0])<<endl;
	}
}
