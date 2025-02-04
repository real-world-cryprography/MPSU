#include "mssROT.h"



void softSend(u32 numElements, Socket &chl, PRNG& prng, AlignedVector<std::array<block, 2>> &sMsgs, u32 numThreads)
{
    SoftSpokenShOtSender<> sender;
    sender.init(fieldBits, true);
    const size_t numBaseOTs = sender.baseOtCount();
    PRNG prngOT(prng.get<block>());
    AlignedVector<block> baseMsg;
    // choice bits for baseOT
    BitVector baseChoice;

    // OTE's sender is base OT's receiver
    DefaultBaseOT base;
    baseMsg.resize(numBaseOTs);
    // randomize the base OT's choice bits
    baseChoice.resize(numBaseOTs);
    baseChoice.randomize(prngOT);
    // perform the base ot, call sync_wait to block until they have completed.
    coproto::sync_wait(base.receive(baseChoice, baseMsg, prngOT, chl));

    sender.setBaseOts(baseMsg, baseChoice);
    // perform random ots

    sMsgs.resize(numElements);
    coproto::sync_wait(sender.send(sMsgs, prngOT, chl));

}

void softRecv(u32 numElements, BitVector bitV, Socket &chl, PRNG& prng, AlignedVector<block> &rMsgs, u32 numThreads)
{

    SoftSpokenShOtReceiver<> receiver;
    receiver.init(fieldBits, true);
    const size_t numBaseOTs = receiver.baseOtCount();
    AlignedVector<std::array<block, 2>> baseMsg(numBaseOTs);
    PRNG prngOT(prng.get<block>());

    // OTE's receiver is base OT's sender
    DefaultBaseOT base;
    // perform the base ot, call sync_wait to block until they have completed.
    coproto::sync_wait(base.send(baseMsg, prngOT, chl));

    receiver.setBaseOts(baseMsg);

    rMsgs.resize(numElements);
    coproto::sync_wait(receiver.receive(bitV, rMsgs, prngOT, chl));

}







//-------------------------------------------new mssROT---------------------------------------------------------------    

void ssrotSend(std::vector<block> &sMsgs, u32 &offsetS, u32 numBins, u32 numBinsCeil, Socket &chl, const std::vector<std::array<block, 2>> &SendROT, const block* delta, u32 times)
{    
    std::vector<std::array<block, 2>> tMsgs(numBins * times);
    u32 offset = offsetS * numBinsCeil;    
    memcpy(tMsgs.data(), SendROT.data() + offset, numBins * 2 * sizeof(block) * times);
    BitVector bitRecv(numBins * times);
    coproto::sync_wait(chl.recv(bitRecv));       

    std::vector<block> uMsgs(numBins * times);
    for (u32 i = 0; i < numBins * times; ++i)
    {
    	uMsgs[i] = delta[i] ^ tMsgs[i][0] ^ tMsgs[i][1];  	    
    }
    coproto::sync_wait(chl.send(uMsgs));
    offsetS += times;
    
    for (u32 i = 0; i < numBins * times; ++i)
    {
    	sMsgs[i % numBins] ^= tMsgs[i][0];    	    
    }
    for (u32 i = 0; i < numBins * times; ++i)
    {
        if(bitRecv[i])
    	{
    	    sMsgs[i % numBins] ^= delta[i] ^ uMsgs[i];
    	}   	    
    }
}

void ssrotRecv(std::vector<block> &rMsgs, u32 &offsetR, u32 numBins, u32 numBinsCeil, Socket &chl, const std::vector<block> &RecvROT, const BitVector &RecvBit, const BitVector &bitVec, u32 times)
{   std::vector<block> tMsgs(numBins * times);
    u32 offset = offsetR * numBinsCeil; 
    memcpy(tMsgs.data(), RecvROT.data() + offset, numBins * sizeof(block) * times);
    BitVector bitSend(numBins * times);
    u32 byte_offset = offset / 8; // u8 
    memcpy(bitSend.data(), RecvBit.data() + byte_offset, bitSend.sizeBytes());
	for (u32 i = 0; i < times; ++i){
        for(u32 j = 0; j < numBins; ++j)
		bitSend[i * numBins + j] ^= bitVec[i * numBins + j];
	}
    coproto::sync_wait(chl.send(bitSend));    
    offsetR += times;

    std::vector<block> uMsgs(numBins * times);
    coproto::sync_wait(chl.recv(uMsgs));

    for (u32 i = 0; i < numBins * times; ++i)
    {
    	rMsgs[i % numBins] ^= tMsgs[i];    	    
    }      
    for (u32 i = 0; i < times; ++i){
        for(u32 j = 0; j < numBins; ++j){
            if(bitVec[i * numBins + j]){
                rMsgs[j] ^= uMsgs[i * numBins + j];
    	    }  
        } 	    
    }
}

// updated prtocol
void mssROT(const std::vector<std::vector<std::array<block, 2>>> &SendROT, const std::vector<std::vector<block>> &RecvROT, const std::vector<BitVector> &BitVec, std::vector<u32> &offsetVecS, std::vector<u32> &offsetVecR, u32 idx, u32 numParties, u32 numBins, std::vector<Socket> &chl, PRNG& prng, const std::vector<BitVector> &choice, std::vector<std::vector<block>> &uVec){

   u32 numBinsCeil = numBins;
    if(numBins % 128 != 0)
    {
    	numBinsCeil = (numBins/128 + 1) * 128;
    }

	uVec.resize(numParties - 1);
	for(u32 i = 0; i < numParties - 1; ++i){
        uVec[i].resize(numBins, ZeroBlock);
	}

    std::vector<std::vector<std::vector<block>>> uMat;
	uMat.resize(numParties - 1);
	for(u32 i = 0; i < numParties - 1; ++i){
        uMat[i].resize(numParties - 1);
        for(u32 j = 0; j < numParties - 1; ++j){
			uMat[i][j].resize(numBins, ZeroBlock);
		}
	}

    if(idx == 0){

    std::vector<std::thread> mssROTThrds(numParties - 1);
    for(u32 i = 1; i < numParties; ++i) 
    {
        mssROTThrds[i-1] = std::thread([&, i]() {             	    	            	                	                                   
            for(u32 j = i; j < numParties; ++j){
                ssrotRecv(uMat[j-1][i-1], offsetVecR[i-1], numBins, numBinsCeil, chl[i-1], RecvROT[i-1], BitVec[i-1], choice[j-1], 1);
            }  	              	                	            	            	
        });              	            	            	                    	             
    }
    for (auto& thrd : mssROTThrds) thrd.join();

    }else{
    
    std::vector<block> delta((idx + numParties - 1)*(numParties - idx) * numBins/2); 
    for(u32 i = 0; i < numParties - idx; ++i){
        for(u32 j = 0; j < idx + i; ++j){
            prng.get(&delta[getIndex(idx, numParties, numBins, i, j, 0)], numBins);
        }
    }

    for(u32 i = 0; i < numParties - 1; ++i){
        if(i < idx){
            for (u32 j = 0; j < numBins; ++j){
                if(choice[i][j]){
                    uVec[idx-1][j] ^= delta[getIndex(idx, numParties, numBins, 0,i,j)];
                }
            }
        }
        else{
            for (u32 j = 0; j < numBins; ++j){
                if(choice[i][j]){
                    uVec[i][j] ^= delta[getIndex(idx, numParties, numBins, i+1-idx, idx,j)];
                }
            }
        }
    }

    std::vector<std::thread> mssROTThrds1(numParties - 1);
    for(u32 i = 0; i < numParties; ++i) 
    {
        if(i < idx)
        {
            mssROTThrds1[i] = std::thread([&, i]() {  
                for(u32 j = idx; j < numParties; ++j){
                    ssrotSend(uMat[j-1][i], offsetVecS[i], numBins, numBinsCeil, chl[i], SendROT[i], &delta[getIndex(idx, numParties, numBins,j-idx,i,0)], 1);
                }                         	            	
            });                    
        }
        else if(i > idx)
        {           	
            mssROTThrds1[i-1] = std::thread([&, i]() {             	    	            	                	                                   
                for(u32 j = i; j < numParties; ++j){
                    ssrotRecv(uMat[j-1][i-1], offsetVecR[i-1], numBins, numBinsCeil, chl[i-1], RecvROT[i-1], BitVec[i-1], choice[j-1], 1);
                }  	              	                	            	            	
            });
        }                	            	            	                    	             
    }
    for (auto& thrd : mssROTThrds1) thrd.join();

    std::vector<std::thread> mssROTThrds2(numParties - 2);
    for(u32 i = 1; i < numParties; ++i) 
    {
        if(i < idx)
        {
            mssROTThrds2[i-1] = std::thread([&, i]() {
				BitVector choices(idx * numBins);
				for(u32 j = 0; j < idx; ++j){
					for(u32 k = 0; k < numBins; ++k){
						choices[j * numBins + k] = choice[j][k];
					}
				}
                ssrotRecv(uMat[idx-1][i], offsetVecR[i], numBins, numBinsCeil, chl[i], RecvROT[i], BitVec[i], choices, idx);  
                for(u32 j = idx + 1; j < numParties; ++j){
                    ssrotRecv(uMat[j-1][i], offsetVecR[i], numBins, numBinsCeil, chl[i], RecvROT[i], BitVec[i], choice[j-1], 1);
                }        	    	            	                	                  	              	                	            	            	
            });                    
        }
        else if(i > idx)
        {           	
            mssROTThrds2[i-2] = std::thread([&, i]() {           	    	            	                	                  
            	ssrotSend(uMat[i-1][i-1], offsetVecS[i-1], numBins, numBinsCeil, chl[i-1], SendROT[i-1], &delta[getIndex(idx, numParties, numBins,i-idx,0,0)], i);
                for(u32 j = i + 1; j < numParties; ++j){
                    ssrotSend(uMat[j-1][i-1], offsetVecS[i-1], numBins, numBinsCeil, chl[i-1], SendROT[i-1], &delta[getIndex(idx, numParties, numBins,j-idx,i,0)], 1);
                }	              	                	            	            	
            });
        }                	            	            	                    	             
    }
    for (auto& thrd : mssROTThrds2) thrd.join();

    }

	for(u32 i = 0; i < numParties - 1; ++i){
        for(u32 j = 0; j < numParties - 1; ++j){
            for(u32 k = 0; k < numBins; ++k){
				uVec[i][k] ^= uMat[i][j][k];
			}
			
		}
	}
}

