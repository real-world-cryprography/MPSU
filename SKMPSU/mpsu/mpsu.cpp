#include <libOTe/Base/BaseOT.h>
#include <libOTe/TwoChooseOne/SoftSpokenOT/SoftSpokenShOtExt.h>
#include <coproto/Socket/AsioSocket.h>
#include "../common/util.h"
#include "../shuffle/ShareCorrelationGen.h"
#include "../shuffle/MShuffle.h"
#include "mpsu.h"

using namespace oc;

// use the pre_generated ROT: mssROT
std::vector<block> MPSUParty(u32 idx, u32 numParties, u32 numElements, std::vector<block> &set, u32 numThreads)
{

    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(numElements, ssp, 0, 3);
    u32 numBins = params.numBins();
    
    MShuffleParty shuffleParty(idx, numParties, numBins * (numParties - 1));       
    shuffleParty.getShareCorrelation();

    Timer timer;
    timer.setTimePoint("start");

    // connect
    std::vector<Socket> chl;
    for (u32 i = 0; i < numParties; ++i){
        // PORT + senderId * 100 + receiverId
        if (i < idx){
            chl.emplace_back(coproto::asioConnect("localhost:" + std::to_string(PORT + idx * 100 + i), true));
            // std::cout << "P" << idx + 1 << " connects to P" << i + 1 << " as sender at port " << PORT + idx * 100 + i << std::endl;
        } else if (i > idx){
            chl.emplace_back(coproto::asioConnect("localhost:" + std::to_string(PORT + i * 100 + idx), false));
            // std::cout << "P" << idx + 1 << " connects to P" << i + 1 << " as receiver at port " << PORT + i * 100 + idx << std::endl;
        }
    }
    
    //timer.setTimePoint("connect done");
    //std::cout << "P" << idx + 1 << " connect done" << std::endl;
    
    PRNG prng(sysRandomSeed());
    oc::RandomOracle hash64(8); 
    block cuckooSeed = block(0x235677879795a931, 0x784915879d3e658a);   
    std::vector<block> share((numParties - 1) * numBins, ZeroBlock);
        
    // for mqssPMT 
    std::vector<BitVector> choice(numParties - 1);            
    std::vector<std::thread> mqssPMTThrds(numParties - 1); 
    std::vector<std::vector<block>> simHashTable(numBins);
    
    // for mqssROT
    std::vector<std::vector<block>> uVector;
    std::vector<u32> offsetVecS(numParties-1, 0); 
    std::vector<u32> offsetVecR(numParties-1, 0);
        
    // read all the P_idx_j and save in SendROT RecvROT BitVec
    std::vector<std::vector<std::array<block, 2>>> SendROT(numParties-1);
    std::vector<std::vector<block>> RecvROT(numParties-1);
    std::vector<BitVector> BitVec(numParties-1);
    u32 numBinsCeil = numBins;
    if(numBins % 128 != 0)
    {
    	numBinsCeil = (numBins/128 + 1) * 128;
    }
    
    for (u32 i = 0; i < numParties; ++i)
    {
        u32 sendNum, recvNum;
        std::string fileName = "./offline/rot_" + std::to_string(numParties) + "_" + std::to_string(numElements) + "_P";
        std::string FileNameS = fileName + std::to_string(idx+1) + std::to_string(i + 1) + "S";
        std::string FileNameR = fileName + std::to_string(idx+1) + std::to_string(i + 1) + "R";

    	std::ifstream rotFileS;
    	std::ifstream rotFileR;     
    	if(i != idx)
    	{
    	    rotFileS.open(FileNameS, std::ios::binary | std::ios::in);
    	    if (!rotFileS.is_open()){
    	        std::cout << "Error opening file " << FileNameS << ", s" << std::endl;
    	        return std::vector<block>();
    	    }    
    	    rotFileR.open(FileNameR, std::ios::binary | std::ios::in);
    	    if (!rotFileR.is_open()){
    	        std::cout << "Error opening file " << FileNameR << ", r" << std::endl;
    	        return std::vector<block>();
    	    }    	
    	}   
        if (i < idx){
            sendNum = (numParties - idx) * numBinsCeil;
            recvNum = (numParties - 1) * numBinsCeil; 
            
            SendROT[i].resize(sendNum);
            RecvROT[i].resize(recvNum);
            BitVec[i].resize(recvNum);
            
            rotFileS.read((char*)(SendROT[i].data()), sendNum * 2 * sizeof(block)); 
            rotFileS.close();
            
            rotFileR.read((char*)BitVec[i].data(), recvNum/8);  
            rotFileR.read((char*)(RecvROT[i].data()), recvNum * sizeof(block)); 
            rotFileR.close();
            
        } else if (i > idx){                    
            sendNum = (numParties - 1) * numBinsCeil; 
            recvNum = (numParties - i) * numBinsCeil; 
            SendROT[i-1].resize(sendNum);
            RecvROT[i-1].resize(recvNum);
            BitVec[i-1].resize(recvNum);            
            rotFileS.read((char*)SendROT[i-1].data(), sendNum * 2 * sizeof(block)); 
            rotFileS.close();
            
            //rotFileR.read((char*)BitVec[i-1].data(), BitVec[i-1].sizeBytes());  
            rotFileR.read((char*)BitVec[i-1].data(), recvNum/8);
            rotFileR.read((char*)RecvROT[i-1].data(), recvNum * sizeof(block));
            rotFileR.close();
        }           
    }
    //std::cout << "read done" << std::endl; 
        
    // all the parties establish simple hash table except pm
    if(idx != numParties - 1)
    {
    	volePSI::SimpleIndex sIdx;
    	sIdx.init(numBins, numElements, ssp, 3);
    	sIdx.insertItems(set, cuckooSeed);
    	
    	for (u64 i = 0; i < numBins; ++i)
    	{
    	    auto bin = sIdx.mBins[i];
    	    auto size = sIdx.mBinSizes[i];
    	    simHashTable[i].resize(size);
    	    
    	    for (u64 p = 0; p < size; ++p)
    	    {
    	    	auto j = bin[p].hashIdx();
    	    	auto b = bin[p].idx();
    	    	u64 halfK = set[b].mData[0];
    	    	simHashTable[i][p] = block(halfK, j);    	    	   	    
    	    }    	        	        	        	
    	}   
    }
    //timer.setTimePoint("simple hash done"); 
    
    if(idx == 0)
    {                               
        // mqssPMT with other parties
        for (u32 i = 1; i < numParties; ++i)
        {             
            std::string triplePath = "./offline/triple_" + std::to_string(numParties) + "_" + std::to_string(numElements) + "_P" + std::to_string(idx + 1) + std::to_string(i + 1);
            std::string volePath = "./offline/vole_" + std::to_string(numParties) + "_" + std::to_string(numElements) + "_P" + std::to_string(idx + 1) + std::to_string(i + 1);      
             
            mqssPMTThrds[i - 1] = std::thread([&, i, triplePath, volePath]() {  
                bssPMTSend(numParties, numElements, cuckooSeed, simHashTable, choice[i - 1], chl[i - 1], numThreads, triplePath, volePath);               
            });                               
        }
        for (auto& thrd : mqssPMTThrds) thrd.join();
        //timer.setTimePoint("mqssPMT done"); 
                 
        mssROT(SendROT, RecvROT, BitVec, offsetVecS, offsetVecR, idx, numParties, numBins, chl, prng, choice, uVector);     
        //timer.setTimePoint("mqssROT done");  

        
        // compute share by uVector
        for(u32 i = 0; i < numParties - 1; ++i)
        {
            for(u32 j = 0; j < numBins; ++j)	
            {
            	share[i*numBins + j] ^= uVector[i][j];            
            }                
        }
                
    	coproto::sync_wait(shuffleParty.run(chl, share));
    	//timer.setTimePoint("mshuffle done");         

        // receive shares from other parties
        std::vector<std::vector<block>> recvShares(numParties - 1);
        std::vector<std::thread> recvThrds(numParties - 1);
        for (u32 i = 0; i < numParties - 1; ++i){
            recvThrds[i] = std::thread([&, i]() {
                recvShares[i].resize(share.size());
                coproto::sync_wait(chl[i].recv(recvShares[i]));
            });
        }
        for (auto& thrd : recvThrds) thrd.join();
        //timer.setTimePoint("recv share done");

        // reconstruct
        for (size_t i = 0; i < numParties - 1; i++){
            for (size_t j = 0; j < share.size(); j++){
                share[j] ^= recvShares[i][j];
            }
        }
        //timer.setTimePoint("reconstruct done");

        // check MAC
        std::vector<block> setUnion(set);
        for (size_t i = 0; i < share.size(); i++){
            long long mac;
            hash64.Update(share[i].mData[0]);
            hash64.Final(mac);
            hash64.Reset();
            if (share[i].mData[1] == mac){
                // std::cout << "add element to union: " << share[i].mData[0] << std::endl;
                setUnion.emplace_back(share[i].mData[0]);                
            }
        }
        //timer.setTimePoint("check MAC and output done");
        timer.setTimePoint("end");

        double comm = 0;
        for (u32 i = 0; i < chl.size(); ++i){
            comm += chl[i].bytesSent() + chl[i].bytesReceived();
        }


        // close sockets
        for (u32 i = 0; i < chl.size(); ++i){
            coproto::sync_wait(chl[i].flush());
            coproto::sync_wait(chl[i].close());
        }

        std::cout << "P1 communication cost = " << std::fixed << std::setprecision(3) << comm / 1024 / 1024 << " MB" << std::endl; 
        //timer<< std::endl;   
        std::cout << "P" <<idx+1 <<": "<<  timer << std::endl;

        return setUnion;          
                               
    }
    else
    {
    
        // establish cuckoo hash table
        oc::CuckooIndex cuckoo;
        cuckoo.init(numElements, ssp, 0, 3);
        cuckoo.insert(set, cuckooSeed);
                
        oc::RandomOracle hash64(8);        
        std::vector<block> cuckooHashTable(numBins); //x||z              
        std::vector<block> hashSet(numBins);//compute hashset: h(x)||x
        for (u32 i = 0; i < numBins; ++i)
        {
            auto bin = cuckoo.mBins[i];

            if (bin.isEmpty() == false)
            {
                auto j = bin.hashIdx();
                auto b = bin.idx();

                u64 halfK = set[b].mData[0];                
                cuckooHashTable[i] = block(halfK, j);//compute x||z
                
                u64 hashx;
                hash64.Update(halfK);
                hash64.Final(hashx);
                hash64.Reset();                                
                hashSet[i] = block(hashx, halfK);  //h(x)||x                                                              
            }
            else
            {          	 
          	//cuckooHashTable[i] = block(0,0);          	
          	hashSet[i] = prng.get();           	              
            }                                
        }
        
        // add MAC to the share
        u32 offset = (idx - 1) * numBins;
        std::copy(hashSet.begin(), hashSet.end(), share.begin() + offset);

                       
        // mqssPMT with other parties       
        for (u32 i = 0; i < numParties; ++i){

            std::string triplePath = "./offline/triple_" + std::to_string(numParties) + "_" + std::to_string(numElements) + "_P" + std::to_string(idx + 1) + std::to_string(i + 1);
            std::string volePath = "./offline/vole_" + std::to_string(numParties) + "_" + std::to_string(numElements) + "_P" + std::to_string(idx + 1) + std::to_string(i + 1);
                                 
            if (i < idx){
                mqssPMTThrds[i] = std::thread([&, i, triplePath, volePath]() {                    
                    bssPMTRecv(numParties, numElements, cuckooSeed, cuckooHashTable, choice[i], chl[i], numThreads, triplePath, volePath);
                });
            } else if (i > idx){
                mqssPMTThrds[i - 1] = std::thread([&, i, triplePath, volePath]() {              
                    bssPMTSend(numParties, numElements, cuckooSeed, simHashTable, choice[i - 1], chl[i - 1], numThreads, triplePath, volePath);                    
                });
            }
        }
        for (auto& thrd : mqssPMTThrds) thrd.join();        

    	
    	mssROT(SendROT, RecvROT, BitVec, offsetVecS, offsetVecR, idx, numParties, numBins, chl, prng, choice, uVector);
    	
    	//std::cout << "mqssROT done" << std::endl;
    	
        // compute share by uVector
        for(u32 i = 0; i < numParties - 1; ++i)
        {
            for(u32 j = 0; j < numBins; ++j)	
            {
            	share[i*numBins + j] ^= uVector[i][j];            
            }                
        }   
        coproto::sync_wait(shuffleParty.run(chl, share));
        // timer.setTimePoint("mshuffle done"); 	         

        // reconstruct output
        coproto::sync_wait(chl[0].send(share));
        //timer.setTimePoint("end");
        if(idx == numParties - 1)
        {
        	double comm = 0;
        	for (u32 i = 0; i < chl.size(); ++i){
            	comm += chl[i].bytesSent() + chl[i].bytesReceived();
        	}

        	std::cout << "P" <<idx+1 <<" communication cost = " << std::fixed << std::setprecision(3) << comm / 1024 / 1024 << " MB" << std::endl;
        }
        else if(idx == 1){
        	timer.setTimePoint("end");
        	std::cout << "P" <<idx+1 <<": "<<  timer << std::endl;
        }


        // close sockets
        for (u32 i = 0; i < chl.size(); ++i){
            coproto::sync_wait(chl[i].flush());
            coproto::sync_wait(chl[i].close());
        }
        return std::vector<block>();    
    }

}
