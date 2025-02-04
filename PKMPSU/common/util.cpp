// #include "util.h"

// void permute(std::vector<u32> &pi, std::vector<block> &data){
//     std::vector<block> res(data.size());
//     for (size_t i = 0; i < pi.size(); ++i){
//         res[i] = data[pi[i]];
//     }
//     data.assign(res.begin(), res.end());
// }

// void printPermutation(std::vector<u32> &pi){
//     for (size_t i = 0; i < pi.size(); ++i){
//         std::cout << pi[i] << " ";
//     }
//     std::cout << std::endl;
// }

// void blockToBitset(block &data, std::bitset<128> &out){
//     out = 0;
//     out ^= data.get<u64>(1);
//     out = (out << 64) ^ std::bitset<128>(data.get<u64>(0));
// }

// void bitsetToBlock(std::bitset<128> &data, block &out){
//     out = oc::toBlock(((data >> 64) & std::bitset<128>(0xFFFFFFFFFFFFFFFF)).to_ullong(), 
//                        (data & std::bitset<128>(0xFFFFFFFFFFFFFFFF)).to_ullong());
// }


#include "util.h"

void permuteV2V(std::vector<u32> &pi_row, std::vector<u32> &pi_col, std::vector<block> &input, std::vector<block> &out){
    u32 row_size = pi_row.size();
    u32 col_size = pi_col.size();
    out.resize(row_size * col_size);
    for(auto i  = 0; i < row_size; ++i){
        for(auto j = 0; j < col_size; ++j){
            out[i * col_size + j] = input[pi_row[i] * col_size + pi_col[j]];
        }
    }
}

void permuteV2VVec(std::vector<u32> &pi_row, std::vector<u32> &pi_col, std::vector<block> &input, std::vector<std::vector<block>> &out){
    u32 row_size = pi_row.size();
    u32 col_size = pi_col.size();
    out.resize(row_size);
    for(auto i  = 0; i < row_size; ++i){
        out[i].resize(col_size);
        for(auto j = 0; j < col_size; ++j){
            out[i][j] = input[pi_row[i] * col_size + pi_col[j]];
        }
    }
}

void permuteVVec(std::vector<u32> &pi_row, std::vector<u32> &pi_col, std::vector<std::vector<block>> &input, std::vector<std::vector<block>> &out){
    u32 row_size = pi_row.size();
    u32 col_size = pi_col.size();
    out.resize(row_size);
    for(auto i  = 0; i < row_size; ++i){
        out[i].resize(col_size);
        for(auto j = 0; j < col_size; ++j){
            out[i][j] = input[pi_row[i]][pi_col[j]];
        }
    }
}

void permuteInverse(std::vector<u32> &pi, std::vector<block> &data){
    std::vector<block> res(data.size());
    for (size_t i = 0; i < pi.size(); ++i){
        res[pi[i]] = data[i];
    }
    data.assign(res.begin(), res.end());
}

void genPermutation(u32 size, std::vector<u32> &pi)
{
    pi.resize(size);
    for (size_t i = 0; i < pi.size(); ++i){
        pi[i] = i;
    }
    std::shuffle(pi.begin(), pi.end(), global_built_in_prg2);
    return;
}

void permute64(std::vector<u32> &pi, std::vector<u64> &data){
    std::vector<u64> res(data.size());
    for (size_t i = 0; i < pi.size(); ++i){
        res[i] = data[pi[i]];
    }
    data.assign(res.begin(), res.end());
}

void permute(std::vector<u32> &pi, std::vector<block> &data){
    std::vector<block> res(data.size());
    for (size_t i = 0; i < pi.size(); ++i){
        res[i] = data[pi[i]];
    }
    data.assign(res.begin(), res.end());
}

void printPermutation(std::vector<u32> &pi){
    for (size_t i = 0; i < pi.size(); ++i){
        std::cout << pi[i] << " ";
    }
    std::cout << std::endl;
}

void blockToBitset(block &data, std::bitset<128> &out){
    out = 0;
    out ^= data.get<u64>(1);
    out = (out << 64) ^ std::bitset<128>(data.get<u64>(0));
}

void bitsetToBlock(std::bitset<128> &data, block &out){
    out = oc::toBlock(((data >> 64) & std::bitset<128>(0xFFFFFFFFFFFFFFFF)).to_ullong(), 
                       (data & std::bitset<128>(0xFFFFFFFFFFFFFFFF)).to_ullong());
}