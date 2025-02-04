#pragma once

#include <volePSI/GMW/Gmw.h>
#include <volePSI/Defines.h>
#include <volePSI/config.h>
#include <volePSI/Paxos.h>
#include <volePSI/SimpleIndex.h>

#include <cryptoTools/Crypto/PRNG.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Common/CuckooIndex.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Common/BitVector.h>

#include <libOTe/Vole/Silent/SilentVoleReceiver.h>
#include <libOTe/Vole/Silent/SilentVoleSender.h>

#include "../common/Defines.h"

#include <string> 
#include <fstream>

using namespace oc;

// batch secret-shared private membership test (batch ssPMT) for PK

void PKbssPMTSend(u32 numParties, u32 numElements, block cuckooSeed, std::vector<std::vector<block>> &simHashTable, BitVector &out, Socket& chl, u32 numThreads, std::string triplePath);
void PKbssPMTRecv(u32 numParties, u32 numElements, block cuckooSeed, std::vector<block> &cuckooHashTable, BitVector &out, Socket &chl, u32 numThreads, std::string triplePath);

