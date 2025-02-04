#pragma once
#include <iostream>
#include <fstream>
#include "volePSI/GMW/Gmw.h"
#include "coproto/Socket/AsioSocket.h"


#include "cryptoTools/Common/CuckooIndex.h"
#include "../common/Defines.h"


void twoPartyTripleGen(u32 numParties, u32 myIdx, u32 idx, u32 logNum, u32 numThreads, Socket &chl, std::string fileName);

void tripleGenParty(u32 idx, u32 numParties, u32 logNum, u32 numThreads, double &commT, double &timeT);

