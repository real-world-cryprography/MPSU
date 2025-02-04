#pragma once

#include "../ECPoint/bigint.h"
#include "../ECPoint/elgamal.h"
#include "rot.h"
#include "pkpmt.h"
#include "../offlineGen/TripleGen.h"
#include "cryptoTools/Common/CLP.h"
#include <algorithm>

using namespace oc;

void shuffleCT(std::vector<ElGamal::CT> &CT1, std::vector<ElGamal::CT> &CT2);



void sendECPoint(Socket &chl, ECPoint A);


void recvECPoint(Socket &chl, ECPoint &A);


void sendCTs(Socket &chl, std::vector<ElGamal::CT> &VecCT);


void recvCTs(Socket &chl, std::vector<ElGamal::CT> &VecCT);


void parDec(ElGamal::CT &ct, BigInt sk);



void CtToBlock5(ElGamal::CT ct, std::vector<block> &ctBlock);


void Block5ToCt(ElGamal::CT &ct, std::vector<block> &ctBlock);



void pkROTi(u32 idx, Socket &chl, const BitVector bitV, std::vector<ElGamal::CT> &ctVec1, PRNG& prng,  ElGamal::PP &pp, ECPoint pk);



void pkROTj(u32 pi, Socket &chl, const BitVector bitV, std::vector<ElGamal::CT> &cuckooCT, PRNG& prng, ElGamal::PP &pp, ECPoint pk);


// PKMPSU
std::vector<ECPoint> PKMPSUParty(u32 idx, u32 numParties, u32 numElements, std::vector<block> &set, std::vector<ECPoint> &set_EC, u32 numThreads = 1);



