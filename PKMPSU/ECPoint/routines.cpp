#include "routines.h"

std::string ToHexString(std::string byte_str)
{
    std::string hex_str;
    std::stringstream ss;

    for (const auto &item : byte_str) {
        ss << std::hex << int(item);
    }
    hex_str = ss.str();

    // format to uppercase
    for (auto & c: hex_str) c = toupper(c);
    return hex_str;
}

// A simple trick to decide if x = 2^n for n > 0 and x > 0
bool IsPowerOfTwo(size_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

// 0 <= r_i < MAX
std::vector<int64_t> GenRandomNaturalIntegerVectorLessThan(size_t LEN, int64_t MAX)
{
    std::vector<int64_t> vec_result(LEN); 
    srand(time(0));
    for(auto i = 0; i < LEN; i++)
    {
        vec_result[i] = rand() % MAX;
    }
    return vec_result; 
}

// -MAX < r_i < MAX 
std::vector<int64_t> GenRandomIntegerVectorAbsoluteLessThan(size_t LEN, int64_t MAX)
{
    std::vector<int64_t> vec_result = GenRandomNaturalIntegerVectorLessThan(LEN, 2*MAX); 
    for(auto i = 0; i < LEN; i++)
    {
        vec_result[i] = vec_result[i] - MAX;
    }
    return vec_result; 
}