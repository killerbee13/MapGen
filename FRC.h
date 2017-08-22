/*
 * Copyright (c) 2014, killerbee13
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */


#ifndef  SIMPLE_ALGORITHMS_H_INCLUDED
#define  SIMPLE_ALGORITHMS_H_INCLUDED

//Header containing frequently reused code that is not as easily done without helper functions

#include <string>
#include <sstream>
#include <cmath>
#include <complex>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <istream>
#include <cctype>
#include <utility>
#include <limits>

#include <typeinfo>
#include <exception>
#include <stdexcept>

//This is the only one that can throw an exception
template <typename T>
inline T fromStr(std::string val) {
  std::stringstream ss(val);
  T ret;
  if (!(ss>>ret).fail())
    return ret;
  else throw std::runtime_error("\""+val+"\" is not a "+typeid(T).name());
}

template <typename T>
inline std::string toStr(T val) {
  std::stringstream ss;
  ss<<val;
  return ss.str();
}

inline std::string reverseStr(std::string val) {
  std::reverse(val.begin(), val.end());
  return val;
}

inline std::string toLower(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
}

inline std::string toUpper(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
  return str;
}

template <typename T>
inline std::vector<T> listToVec(std::string list)
{
  std::vector<T> ret;
  std::stringstream ss(list);
  std::string tmp;
  while(!(ss>>tmp).fail()) {
    ret.push_back(fromStr<T>(tmp));
  }
  /*
  while(list.find_first_of(" ,") != std::string::npos) {
    std::string entry = list.substr(0, list.find_first_of(" ,"));
    ret.push_back(fromStr<T>(entry));
    list = list.substr(entry.size());
    if (list.find_first_of(" ,") == std::string::npos) {
      entry = list.substr(0, list.find_first_of(" ,"));
      ret.push_back(fromStr<T>(entry));
      list = list.substr(entry.size());
    }
  }*/
  return ret;
}

inline std::string repeat(std::string val, int count) {
  std::string tmp;
  for (int i = 0; i < count; ++i) {
    tmp += val;
  }
  return tmp;
}

inline std::string repeat(char val, int count) {
  std::string tmp;
  for (int i = 0; i < count; ++i) {
    tmp += val;
  }
  return tmp;
}

template<typename Number>
constexpr inline int digitsOf(Number val)
{
  return std::ceil(std::log10(val));
}

template<typename Number>
constexpr inline int digitsOf(Number val, int base)
{
  return std::ceil(std::log(val)/std::log(base));
}

template<typename ForwardIterator>
inline int digitsList(ForwardIterator first, ForwardIterator last)
{
  return digitsOf(*std::max_element(first, last));
}

template<typename ForwardIterator>
inline int digitsList(ForwardIterator first, ForwardIterator last, int base)
{
  return digitsOf(*std::max_element(first, last), base);
}

//Not working
template<typename ostream_type, typename ForwardIterator>
inline ostream_type& padList(ostream_type& os, ForwardIterator first, ForwardIterator last, char fill=' ', int base=10)
{
//os<<std::setw(std::ceil(std::log(*std::max_element(first, last))/std::log(base)))<<std::setfill(fill);
  
  //save state of os
  char fills = os.fill(); std::streamsize widths = os.width();
  
  os<<std::setw(digitsList(first, last, base))<<std::setfill(fill);
  
  for (ForwardIterator it = first; it != last; ++it) {
    os<<' '<<*it;
  }
  //restore state of os
  os<<std::setw(widths)<<std::setfill(fills);
  return os;
}

//Consume all non-spaces to first break, then eat that, too
inline std::istream& eatWord(std::istream& is)
{
  do {is.get();} while (!isspace(is.peek()));
  return is;
}

//Eat spaces, don't eat an extra
inline std::istream& eatSpace(std::istream& is)
{
  while (isspace(is.peek())) is.get();
  return is;
}

//Similar to std::pair
template <typename T>
class ordered_pair {
using pair=std::pair<T, T>;
public:
  ordered_pair(T a, T b)  { if (a < b) {first = a; second = b;}
                            else {first = b; second = a;} }
  T first, second;
  operator pair() {return std::pair<T, T>(first, second);}
  bool operator < (const ordered_pair& rhs) const
  {
    if (first < rhs.first) return 1;
    if (second < rhs.second) return 1;
  }
};

template <typename T>
ordered_pair<T> mkop(T a, T b) 
{
  return ordered_pair<T>(a, b);
}

template<typename T, typename F>
inline constexpr T quantizeRange(F min, F delta, F val)
{
	return static_cast<T>((val-min)*std::numeric_limits<T>::max()*delta);
}

//Memnonic aids

template <typename T>
using Ptr = T*;

template <typename T>
using PtrToConst = T const*;

template <typename T>
using ConstPtr = T* const;

template <typename T>
using ConstPtrToConst = T const* const;

#endif //SIMPLE_ALGORITHMS_H_INCLUDED
