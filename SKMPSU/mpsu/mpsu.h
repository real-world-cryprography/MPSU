#pragma once


#include <iostream>
#include <volePSI/config.h>
#include "../common/Defines.h"
#include "bssPMT.h"
#include "mssROT.h"
#include <volePSI/Paxos.h>

using namespace oc;

std::vector<block> MPSUParty(u32 idx, u32 numParties, u32 numElements, std::vector<block> &set, u32 numThreads = 1);




