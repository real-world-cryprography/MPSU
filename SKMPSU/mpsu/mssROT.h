#pragma once

#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Common/BitVector.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/IOService.h>

#include <libOTe/Base/BaseOT.h>
#include <libOTe/TwoChooseOne/SoftSpokenOT/SoftSpokenShOtExt.h>
#include <coproto/Socket/AsioSocket.h>
#include <iostream>
#include <volePSI/config.h>
#include "../common/Defines.h"

using namespace oc;

// SoftSpoken random oblivious transfer
void softSend(u32 numElements, Socket &chl, PRNG& prng, AlignedVector<std::array<block, 2>> &sMsgs, u32 numThreads = 1);
void softRecv(u32 numElements, BitVector bitV, Socket &chl, PRNG& prng, AlignedVector<block> &rMsgs, u32 numThreads = 1);

inline u32 getIndex(u32 idx, u32 numParties, u32 numBins, u32 i, u32 j, u32 k){
    return (2*idx + i - 1)* i *numBins/2 + j*numBins+ k;
}

// secret-shared random oblivious transfer
void ssrotSend(std::vector<block> &sMsgs, u32 &offsetS, u32 numBins, u32 numBinsCeil, Socket &chl, const std::vector<std::array<block, 2>> &SendROT, const block* delta, u32 times);

void ssrotRecv(std::vector<block> &rMsgs, u32 &offsetR, u32 numBins, u32 numBinsCeil, Socket &chl, const std::vector<block> &RecvROT, const BitVector &RecvBit, const std::vector<BitVector> &bitVec, u32 times);

// multi-party secret-shared random oblivious transfer
void mssROT(const std::vector<std::vector<std::array<block, 2>>> &SendROT, const std::vector<std::vector<block>> &RecvROT, const std::vector<BitVector> &BitVec, std::vector<u32> &offsetVecS, std::vector<u32> &offsetVecR, u32 idx, u32 numParties, u32 numBins, std::vector<Socket> &chl, PRNG& prng, const std::vector<BitVector> &choice, std::vector<std::vector<block>> &uVec);







