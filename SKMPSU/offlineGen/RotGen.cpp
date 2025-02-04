#include "RotGen.h"



void rot2PartyGen(u32 myIdx, u32 idx, u32 numParties, u32 numBinsCeil, u32 numThreads, Socket &chl, std::string fileName)
{    
    assert(numBinsCeil % 128 == 0);
    std::string outFileNameS = fileName + "_P" + std::to_string(myIdx + 1) + std::to_string(idx + 1) + "S";
    std::string outFileNameR = fileName + "_P" + std::to_string(myIdx + 1) + std::to_string(idx + 1) + "R";

    std::ofstream outFileS;
    outFileS.open(outFileNameS, std::ios::binary | std::ios::out);
    if (!outFileS.is_open()){
        std::cout << "ROT error opening file " << outFileNameS << std::endl;
        return;
    }
    std::ofstream outFileR;
    outFileR.open(outFileNameR, std::ios::binary | std::ios::out);
    if (!outFileR.is_open()){
        std::cout << "ROT error opening file " << outFileNameR << std::endl;
        return;
    }
              
    PRNG prng(sysRandomSeed());
    u32 mySendNum, myRecvNum;
    // compute the needed ROT num
    // if myIdx < idx, myIdx need S: (numParties-1)*numBinsCeil, R: (numParties-idx)*numBinsCeil
    // if myIdx > idx, myIdx need S: (numParties-myIdx)*numBinsCeil, R: (numParties-1)*numBinsCeil
    
   
    if(myIdx < idx)
    {
    	mySendNum = (numParties - 1) * numBinsCeil;
    	myRecvNum = (numParties - idx) * numBinsCeil;   
    }
    else if(myIdx > idx)
    {
    	mySendNum = (numParties - myIdx) * numBinsCeil;
    	myRecvNum = (numParties - 1) * numBinsCeil;    	    
    }
               
    oc::AlignedVector<std::array<block, 2>> sMsgs(mySendNum);
    BitVector bitV(myRecvNum);
    bitV.randomize(prng);
    AlignedVector<block> rMsgs(myRecvNum);
    
        
    // set a constant order: if i < j, then [send, recv]; else [recv, send]
    if(myIdx < idx)
    {    	    	    	
    	// step1: perform ROT as sender, get sMsgs
    	softSend(mySendNum, chl, prng, sMsgs, numThreads);   	
    	    	
    	// step2: perform ROT as receiver, get rMsgs
    	softRecv(myRecvNum, bitV, chl, prng, rMsgs, numThreads);   
    }
    else if(myIdx > idx)
    {
      	
    	// step1: perform ROT as receiver, get bitV, rMsgs
    	softRecv(myRecvNum, bitV, chl, prng, rMsgs, numThreads);
  	      	    	    	
    	// step2: perform ROT as sender, get sMsgs
    	softSend(mySendNum, chl, prng, sMsgs, numThreads);   	   
    }
    
    // 
    
    // write bitV,rMsgs to outFileB
    outFileR.write((char*)bitV.data(), bitV.sizeBytes());       
    outFileR.write((char*)rMsgs.data(), myRecvNum * sizeof(block));
    outFileR.close();   
    
    // write sMsgs to outFileS
    outFileS.write((char*)sMsgs.data(), mySendNum * 2 * sizeof(block));
    outFileS.close();   

    coproto::sync_wait(chl.flush());

}



void rotGenParty(u32 idx, u32 numParties, u32 logNum, u32 numThreads, double &commT, double &timeT)
{

    Timer timer;
    timer.setTimePoint("begin");
    // connect
    std::vector<Socket> chl;
    for (u32 i = 0; i < numParties; ++i){
        // PORT + senderId * 100 + receiverId
        if (i < idx){
            chl.emplace_back(coproto::asioConnect("localhost:" + std::to_string(PORT + idx * 100 + i), true));
        } else if (i > idx){
            chl.emplace_back(coproto::asioConnect("localhost:" + std::to_string(PORT + i * 100 + idx), false));
        }
    }

    timer.setTimePoint("connect done");

    u32 numElements = 1ull << logNum;
    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(numElements, ssp, 0, 3);
    u32 numBins = params.numBins();
    u32 numBinsCeil = numBins;
    if(numBins % 128 != 0)
    {
    	numBinsCeil = (numBins/128 + 1) * 128;
    }
    
    // generate ROT
    std::string fileName = "./offline/rot_" + std::to_string(numParties) + "_" + std::to_string(numElements);
    
    std::vector<std::thread> rotGenThrds(numParties - 1);
    for (u32 i = 0; i < numParties; ++i){
        if (i < idx){
            rotGenThrds[i] = std::thread([&, i]() {
                rot2PartyGen(idx, i, numParties, numBinsCeil, numThreads, chl[i], fileName);
            });
        } else if (i > idx){
            rotGenThrds[i - 1] = std::thread([&, i]() {
                rot2PartyGen(idx, i, numParties, numBinsCeil, numThreads, chl[i-1], fileName);
            });
        }
    }
    for (auto& thrd : rotGenThrds) thrd.join();

    timer.setTimePoint("generate rot done");

    
    //if(idx == 0){

        double comm = 0;
        for (u32 i = 0; i < chl.size(); ++i){
            comm += chl[i].bytesSent() + chl[i].bytesReceived();
        }
        commT = comm;
        
        auto iter = timer.mTimes.end();
        --iter;
        auto time = std::chrono::duration_cast<std::chrono::microseconds>(iter->first - timer.mTimes.front().first).count() / 1000.0;
        timeT = time;
        //timeT = std::chrono::duration_cast<std::chrono::microseconds>(iter->first - timer.mTimes.front().first).count() / 1000.0;
        //std::cout << "P1 gmw triple cost = " << comm / 1024 / 1024 << " MB" << std::endl;
    //}
    

    // close channel
    for (u32 i = 0; i < chl.size(); ++i){
        coproto::sync_wait(chl[i].close());
    }

    // if (idx == 0){
        //std::cout << timer << std::endl;
        //std::cout<< iter->second << "  " << std::fixed << std::setprecision(1) << std::setw(9) << time << std::endl;
        //std::cout << "P1 rot cost = " << comm / 1024 / 1024 << " MB" << std::endl;

    // }

}
