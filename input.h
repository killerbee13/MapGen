#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <vector>
#include <array>
#include <functional>
#include <algorithm>

//#include "FRC.h"

//Polymorphic input processing, yay!

//Function data
class functdata {
public:
  typedef std::function<void(std::vector<std::string>)> inputProcessor;
  functdata()
    : argC(0) {;}
  functdata(inputProcessor c, int a, std::string h)
    : call(c), argC(a), help(h) {;}
  void operator()(std::vector<std::string> in) {call(in);}
  inputProcessor call;
  int argC;
  std::string help;
};

//indexed trie for input lookup
//All data is ASCII (char & 127)
class input_indexed_trie {
public:
  //Search trie, also get match length
  functdata* longestPrefix(std::string key, int* len);
  //Find all (valid) terminal nodes downstream from key
  std::vector<std::string> allWithPrefix(std::string key);
  //Add key to index, adding extra rows to the end
  void insert(std::string key, functdata* data);
  //Efficient (sorted) insert
  void insert(std::vector<std::pair<std::string, functdata*>> data);
  //Clear data for key, leave extra paths
  void fast_remove(std::string key);//UNTESTED
  //Clear superfluous entries and possibly rebuild index
  void reduce();//NYI
  //Clear data for key, possibly rebuilding index
  void remove(std::string key) {fast_remove(key); reduce();}
  //Default constructor
  input_indexed_trie() : index(1) {;}
  //insert constructor
  input_indexed_trie(std::string key, functdata* data)
    : index(1) {insert(key,data);}
  //Vector constructor
  input_indexed_trie(std::vector<std::pair<std::string, functdata*>> data)
    : index(1) {insert(data);}
  
  //debug access, returns value-copy of internal index
  std::vector<std::array<std::pair<int, functdata*>, 128>> getIndex()
    {return index;}
private:
  //Get descendents of node
  std::vector<std::string> i_getNodes(unsigned int p, std::string prefix);
  
  constexpr static std::array<std::pair<int, functdata*>, 128> mapNull()
    {return std::array<std::pair<int, functdata*>, 128>();};
  std::vector<std::array<std::pair<int, functdata*>, 128>> index;
};

typedef input_indexed_trie input_trie;

input_trie initTrie();

std::vector<std::string> tokenizeInput(std::string in);

#endif //INPUT_H_INCLUDED