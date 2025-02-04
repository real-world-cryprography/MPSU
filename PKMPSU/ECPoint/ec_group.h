#pragma once

#include "global.h"
//#include "../utility/print.hpp"
#include "bigint.h"

// enable pre-computation for fixed point multiplication
#define PRECOMPUTE_ENABLE

/* 
** enable point compression
** will save bandwidth by half at the cost of expensive decompression 
*/

inline int curve_id = NID_X9_62_prime256v1;  
#define ECPOINT_COMPRESSED
//#define ENABLE_X25519_ACCELERATION

inline EC_GROUP *group;
const inline EC_POINT *generator; 

inline BIGNUM *order;
inline BIGNUM *cofactor;  // The cofactor of this group
inline BIGNUM *curve_params_p; 
inline BIGNUM *curve_params_a; 
inline BIGNUM *curve_params_b;
inline BIGNUM *curve_params_q; // q = (p-1)/2

inline size_t POINT_BYTE_LEN; // the byte length of ec point
inline size_t POINT_COMPRESSED_BYTE_LEN; // the byte length of ec point in compressed form

inline BN_CTX *ec_ctx; // define ctx for ecc operations


void ECGroup_Initialize();

void ECGroup_Finalize();

// #endif //_CRYPTO_EC_GROUP_HPP_







