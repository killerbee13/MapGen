#include "input.h"

#include <exception>

//Search trie, also get match length
functdata* input_indexed_trie::longestPrefix(std::string key, int* len)
{
	if (key.empty())
		return nullptr;
	unsigned int p = 0, i = 0;
	functdata *r = nullptr;
	*len = 0;
	for (; i != key.length() - 1; ++i) {
		//std::cout<<"key["<<i<<"]:"<<key[i]<<";p:"<<p;
		//Limit to ASCII, NUL becomes DEL
		(key[i] &= 127) ? false : key[i] = 127;
		//Keep track of last good return value
		if (index[p][key[i]].second) {
			r = index[p][key[i]].second;
			(*len) = i+1;
		}
		if (index[p][key[i]].first == 0) {
			//std::cout<<";no child"<<std::endl;
			return r;
		}
		p = index[p][key[i]].first;
		//std::cout<<";new p:"<<p<<std::endl;
	}
	//std::cout<<"key["<<i<<"]:"<<key[i]<<";p:"<<p<<std::endl;
	if (index[p][key[i]].second) {
		r = index[p][key[i]].second;
		(*len) = i+1;
	}
	return r;
}

//Add key to index, adding extra rows to the end
void input_indexed_trie::insert(std::string key, functdata* data)
{
	if (key.empty())
		return;
	unsigned int p = 0, i = 0;
	for (; i != key.length() - 1; ++i) {
		//Limit to ASCII, NUL becomes DEL
		(key[i] &= 127) ? false : key[i] = 127;
		if (index[p][key[i]].first == 0) {
			index[p][key[i]].first = index.size();
			index.push_back(mapNull());
		}
		p = index[p][key[i]].first;
	}
	index[p][key[i]].second = data;
}

//Efficient (sorted) insert
void input_indexed_trie::insert(std::vector<std::pair<std::string, functdata*>> data)
{
	//Sort input by strings
	std::sort(data.begin(), data.end(), 
		[](std::pair<std::string, functdata*> a, std::pair<std::string, functdata*> b)
		{return a.first>b.first;});
	for (auto i : data) {
		insert(i.first, i.second);
	}
}

//Clear data for key, leave extra paths
void input_indexed_trie::fast_remove(std::string key)
{
	if (key.empty())
		return;
	unsigned int p = 0, i = 0;
	for (; i != key.length() - 1; ++i) {
		(key[i] &= 127) ? false : key[i] = 127;
		if (index[p][key[i]].first == 0) {
			return; //key is not in trie
		}
		p = index[p][key[i]].first;
	}
	//key (or a string with prefix key) is in trie
	delete (index[p][key[i]].second);
	index[p][key[i]].second = nullptr;
}

//NYI
//Clear superfluous entries and possibly rebuild index
void input_indexed_trie::reduce()
{
	//no-op
	return;
}

//Find all (valid) terminal nodes downstream from key
std::vector<std::string> input_indexed_trie::allWithPrefix(std::string key)
{
	if (key.empty())
		return std::vector<std::string>();
	unsigned int p = 0, i = 0;
	for (; i != key.length() - 1; ++i) {
		(key[i] &= 127) ? false : key[i] = 127;
		if (index[p][key[i]].first == 0) {
			return std::vector<std::string>(); //key is not in trie
		}
		p = index[p][key[i]].first;
	}
	std::vector<std::string> tmp;
	if (index[p][key[i]].second) 
		tmp.push_back(key);
	if (index[p][key[i]].first == 0) {
		return tmp;
	}
	p = index[p][key[i]].first;
	auto tmp2 = i_getNodes(p, key);
	tmp.insert(tmp.end(), tmp2.begin(), tmp2.end());
	return tmp;
}

//Get descendents of node
std::vector<std::string> input_indexed_trie::i_getNodes(unsigned int p, std::string prefix)
{
	//Depth-first (recursive) search
	std::vector<std::string> retArray;
	for (unsigned char j = 0; j != 128; j++) {
		if (index[p][j].second) { //Terminal node, add to output
			retArray.push_back(prefix + static_cast<char>(j));
		}
		if (index[p][j].first) { //Node has children, recurse
			auto tmp = i_getNodes(index[p][j].first, prefix + static_cast<char>(j));
			retArray.insert(retArray.end(), tmp.begin(), tmp.end());
		}
	}
	return retArray;
}

//FSM for input
std::vector<std::string> tokenizeInput(std::string in)
{
	size_t word = 0;
	std::vector<std::string> ret(1);
	int_fast16_t state = 1;
	bool inSpace = false;
	int_fast16_t states[10][5] = {
	// N, ', ", \, S,
		{1, 2, 3, 4, 0}, //Space
		{1, 2, 3, 4, 0}, //Normal
		{2, 1, 2, 5, 2,}, //'-quote
		{3, 3, 1, 6, 3,}, //"-quote
		{1, 1, 1, 1, 1,}, //Escape (normal)
		{2, 2, 2, 2, 2,}, //Escape ('-quote)
		{3, 3, 3, 3, 3,}, //Escape ("-quote)
	};
	
	//Iterate characters and add to vector
	for (auto i : in) {
		//If we're at the end of a space-group then increase size of vector
		if (inSpace && (i != ' ')) {
			word++;
			ret.emplace_back("");
			inSpace = false;
		}
		switch (i) {
		case '\'':
			//if in double-quote or any escape
			if (state == 3 || state > 3) {
				ret[word] += i;
			}
			state = states[state][1];
			break;
		case '"':
			//if in single-quote or any escape
			if (state == 2 || state > 3) {
				ret[word] += i;
			}
			state = states[state][2];
			break;
		case '\\':
			//If in an escape
			if (state > 3) {
				ret[word] += i;
			}
			state = states[state][3];
			break;
		case ' ':
			//If in any kind of quote or escape
			if (state > 1) {
				ret[word] += i;
			}
			state = states[state][4];
			break;
		default:
			//Always add normal characters
			ret[word] += i;
			state = states[state][0];
			break;
		}
		if (!state) {
			inSpace = true;
		}
	}
	return ret;
}
