// #ifndef ELGAMAL_HPP_
// #define ELGAMAL_HPP_
#pragma once
#include "ec_point.h"

/*
 * x25519 NIKE protocol implies PKE, but x25519 is not full-fledged
 * the addition on EC curve is not well-defined
 * and thus does not obey distributive law w.r.t. exponentiation
 * therefore, it does not support re-randomization and homomorphic operations 
 * when you need re-randomizable PKE, you should disable X25519_ACCELERATION
*/

namespace ElGamal{
 
//using Serialization::operator<<; 
//using Serialization::operator>>; 

//#ifndef ENABLE_X25519_ACCELERATION
// define the structure of PP
struct PP
{ 
    ECPoint g; // random generator 
};

// define the structure of ciphertext
struct CT
{
    ECPoint X; // X = g^r 
    ECPoint Y; // Y = pk^r + M  
};


// compare two ciphertexts
bool operator==(const CT& ct_left, const CT& ct_right);

void PrintPP(const PP &pp);

void PrintCT(const CT &ct);

// core algorithms

/* Setup algorithm */ 
PP Setup();

/* KeyGen algorithm */ 
std::tuple<ECPoint, BigInt> KeyGen(const PP &pp);


/* Encryption algorithm: compute CT = Enc(pk, m; r) */ 
CT Enc(const PP &pp, const ECPoint &pk, const ECPoint &m);

/* Encryption algorithm: compute CT = Enc(pk, m; r): with explicit randomness */ 
CT Enc(const PP &pp, const ECPoint &pk, const ECPoint &m, const BigInt &r);

/* 
** re-rand ciphertext CT  
** run by anyone
*/ 
CT ReRand(const PP &pp, const ECPoint &pk, const CT &ct);

// decrypt 
ECPoint Dec(const PP &pp, const BigInt &sk, const CT &ct);


std::vector<unsigned char> CTtoByteArray(ElGamal::CT &ct);
 
ElGamal::CT ByteArraytoCT(std::vector<unsigned char> &buffer);



}




// # endif




