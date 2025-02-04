// #pragma once
// // This file and the associated implementation has been placed in the public domain, waiving all copyright. No restrictions are placed on its use.


// #include <cryptoTools/Common/CLP.h>
// #include <iostream>
// #include <bitset>
// #include "Defines.h"

// // permute data according to pi
// void permute(std::vector<u32> &pi, std::vector<block> &data);

// void printPermutation(std::vector<u32> &pi);

// void blockToBitset(block &data, std::bitset<128> &out);

// void bitsetToBlock(std::bitset<128> &data, block &out);


#pragma once
// This file and the associated implementation has been placed in the public domain, waiving all copyright. No restrictions are placed on its use.


#include <cryptoTools/Common/CLP.h>
#include <iostream>
#include <bitset>
#include "Defines.h"

inline std::random_device rd2;
inline std::mt19937 global_built_in_prg2(rd2());

void permuteV2V(std::vector<u32> &pi_row, std::vector<u32> &pi_col, std::vector<block> &input, std::vector<block> &out);

void permuteV2VVec(std::vector<u32> &pi_row, std::vector<u32> &pi_col, std::vector<block> &input, std::vector<std::vector<block>> &out);

void permuteVVec(std::vector<u32> &pi_row, std::vector<u32> &pi_col, std::vector<std::vector<block>> &input, std::vector<std::vector<block>> &out);

void permuteInverse(std::vector<u32> &pi, std::vector<block> &data);

// permute data according to pi
void genPermutation(u32 size, std::vector<u32> &pi);

void permute64(std::vector<u32> &pi, std::vector<u64> &data);

void permute(std::vector<u32> &pi, std::vector<block> &data);

void printPermutation(std::vector<u32> &pi);

void blockToBitset(block &data, std::bitset<128> &out);

void bitsetToBlock(std::bitset<128> &data, block &out);