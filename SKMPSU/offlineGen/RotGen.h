#pragma once
#include <string> 
#include <iostream>
#include <fstream>

#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Common/BitVector.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/IOService.h>

#include <libOTe/Base/BaseOT.h>
#include <libOTe/TwoChooseOne/SoftSpokenOT/SoftSpokenShOtExt.h>
#include <coproto/Socket/AsioSocket.h>
#include <volePSI/config.h>

#include <cryptoTools/Common/CuckooIndex.h>
#include "../common/Defines.h"

#include "../mpsu/mssROT.h"

using namespace oc;


void rot2PartyGen(u32 myIdx, u32 idx, u32 numParties, u32 numBins, u32 numThreads, Socket &chl, std::string fileName);

void rotGenParty(u32 idx, u32 numParties, u32 logNum, u32 numThreads, double &commT, double &timeT);


