#include "TripleGen.h"


// set for iszeroCircuit()
void twoPartyTripleGen(u32 numParties, u32 myIdx, u32 idx, u32 logNum, u32 numThreads, Socket &chl, std::string fileName)
{
    u32 numElements = 1ull << logNum;
    //std::cout << numElements << std::endl;
    std::string outFileName = "./offline/" + fileName + "_" + std::to_string(numElements) + "_P" + std::to_string(myIdx + 1) + std::to_string(idx + 1);

    std::ofstream outFile;
    outFile.open(outFileName, std::ios::binary | std::ios::out);
    if (!outFile.is_open()){
        std::cout << "Error opening file " << outFileName << std::endl;
        return;
    }

    u32 ssp = 40;
    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(numElements, ssp, 0, 3);
    u32 numBins = params.numBins();    
    u32 c_n2 = (numParties * (numParties -1))/2;
    u32 keyBitLength = ssp + oc::log2ceil(c_n2 * numBins);     
    
    volePSI::BetaCircuit cir = volePSI::isZeroCircuit(keyBitLength);
    // BetaCircuit cir = inverse_of_S_box_layer(numofboxes);
    // generate triples

    volePSI::Gmw gmw;
    gmw.init(numBins, cir, numThreads, myIdx > idx, sysRandomSeed());
    coproto::sync_wait(gmw.generateTriple(1 << 26, numThreads, chl));
    // write A, B, C, D
    outFile.write((char*)gmw.mA.data(), gmw.mA.size() * sizeof(block));
    outFile.write((char*)gmw.mB.data(), gmw.mB.size() * sizeof(block));
    outFile.write((char*)gmw.mC.data(), gmw.mC.size() * sizeof(block));
    outFile.write((char*)gmw.mD.data(), gmw.mD.size() * sizeof(block));

    outFile.close();
    coproto::sync_wait(chl.flush());


}

void tripleGenParty(u32 idx, u32 numParties, u32 logNum, u32 numThreads, double &commT, double &timeT){
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

    // generate triples
    std::string fileName = "triple";
    fileName = fileName + "_" + std::to_string(numParties);
    std::vector<std::thread> tripleGenThrds(numParties - 1);
    for (u32 i = 0; i < numParties; ++i){
        if (i < idx){
            tripleGenThrds[i] = std::thread([&, i]() {
                twoPartyTripleGen(numParties, idx, i, logNum, numThreads, chl[i], fileName);
            });
        } else if (i > idx){
            tripleGenThrds[i - 1] = std::thread([&, i]() {
                twoPartyTripleGen(numParties, idx, i, logNum, numThreads, chl[i - 1], fileName);
            });
        }
    }
    for (auto& thrd : tripleGenThrds) thrd.join();

    timer.setTimePoint("generate triples done");
    
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
        //std::cout << "P1 gmw triple cost = " << comm / 1024 / 1024 << " MB" << std::endl;
    //}
    

    // close channel
    for (u32 i = 0; i < chl.size(); ++i){
        coproto::sync_wait(chl[i].close());
    }

    if (idx == 0){
        //std::cout << timer << std::endl;
        //std::cout<< iter->second << "  " << std::fixed << std::setprecision(1) << std::setw(9) << time << std::endl;
        //std::cout << "P1 gmw triple cost = " << comm / 1024 / 1024 << " MB" << std::endl;
        //std::cout << " " << std::endl;

    }
}

