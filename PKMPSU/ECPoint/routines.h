/****************************************************************************
this hpp implements some routine algorithms
*****************************************************************************
* @author     This file is part of Kunlun, developed by Yu Chen
* @copyright  MIT license (see LICENSE file)
*****************************************************************************/
// #ifndef KUNLUN_UTILITY_ROUTINES_HPP_
// #define KUNLUN_UTILITY_ROUTINES_HPP_

#include "global.h"

// check the existence of a given file
inline bool FileExist(const std::string& filename)
{
    bool existing_flag; 
    std::ifstream fin; 
    fin.open(filename);
    if(!fin)  existing_flag = false;    
    else existing_flag = true;
    return existing_flag; 
}


std::string ToHexString(std::string byte_str);

// A simple trick to decide if x = 2^n for n > 0 and x > 0
bool IsPowerOfTwo(size_t x);

// 0 <= r_i < MAX
std::vector<int64_t> GenRandomNaturalIntegerVectorLessThan(size_t LEN, int64_t MAX);

// -MAX < r_i < MAX 
std::vector<int64_t> GenRandomIntegerVectorAbsoluteLessThan(size_t LEN, int64_t MAX);

// #endif

