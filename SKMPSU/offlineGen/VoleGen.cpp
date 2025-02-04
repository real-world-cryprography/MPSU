#include "VoleGen.h"

void voleGenSend(u32 myIdx, u32 idx, u32 logNum, u32 numThreads, Socket &chl, std::string fileName)
{

    u32 numElements = 1ull << logNum;
    //std::cout << numElements << std::endl;
    std::string outFileName = "./offline/" + fileName + "_" + std::to_string(numElements) + "_P" + std::to_string(myIdx + 1) + std::to_string(idx + 1);

    std::ofstream outFile;
    outFile.open(outFileName, std::ios::binary | std::ios::out);
    if (!outFile.is_open()){
        std::cout << "Vole error opening file " << outFileName << std::endl;
        return;
    }

    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(numElements, ssp, 0, 3);
    u32 numBins = params.numBins();
    
    PRNG prng(sysRandomSeed());
    // get vole : a + b  = c * d
    block mD = prng.get();
    oc::SilentVoleSender<block,block, oc::CoeffCtxGF128> mVoleSender;
    mVoleSender.mMalType = SilentSecType::SemiHonest;
    mVoleSender.configure(numBins, SilentBaseType::Base);
    AlignedUnVector<block> mB(numBins);
    coproto::sync_wait(mVoleSender.silentSend(mD, mB, prng, chl));
        
    outFile.write((char*)(&mD), sizeof(block));
    outFile.write((char*)mB.data(), mB.size() * sizeof(block));
    
    outFile.close();
    coproto::sync_wait(chl.flush());    
        
}

void voleGenRecv(u32 myIdx, u32 idx, u32 logNum, u32 numThreads, Socket &chl, std::string fileName)
{

    u32 numElements = 1ull << logNum;
    std::string outFileName = "./offline/" + fileName + "_" + std::to_string(numElements) + "_P" + std::to_string(myIdx + 1) + std::to_string(idx + 1);

    std::ofstream outFile;
    outFile.open(outFileName, std::ios::binary | std::ios::out);
    if (!outFile.is_open()){
        std::cout << "Vole error opening file " << outFileName << std::endl;
        return;
    }

    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(numElements, ssp, 0, 3);
    u32 numBins = params.numBins();
    
    PRNG prng(sysRandomSeed());
    // get mA mC of vole: a+b = c*d
    oc::SilentVoleReceiver<block, block, oc::CoeffCtxGF128> mVoleRecver;
    mVoleRecver.mMalType = SilentSecType::SemiHonest;
    mVoleRecver.configure(numBins, SilentBaseType::Base);
    AlignedUnVector<block> mA(numBins);
    AlignedUnVector<block> mC(numBins);
    coproto::sync_wait(mVoleRecver.silentReceive(mC, mA, prng, chl));

    outFile.write((char*)mA.data(), mA.size() * sizeof(block));
    outFile.write((char*)mC.data(), mC.size() * sizeof(block));
    outFile.close();
    coproto::sync_wait(chl.flush());    
        
}




void voleGenParty(u32 idx, u32 numParties, u32 logNum, u32 numThreads, double &commT, double &timeT){
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

    // generate vole
    std::string fileName = "vole";
    fileName = fileName + "_" + std::to_string(numParties);
    std::vector<std::thread> voleGenThrds(numParties - 1);
    for (u32 i = 0; i < numParties; ++i){
        if (i < idx){
            voleGenThrds[i] = std::thread([&, i]() {
                voleGenRecv(idx, i, logNum, numThreads, chl[i], fileName);
            });
        } else if (i > idx){
            voleGenThrds[i - 1] = std::thread([&, i]() {
                voleGenSend(idx, i, logNum, numThreads, chl[i - 1], fileName);
            });
        }
    }
    for (auto& thrd : voleGenThrds) thrd.join();

    timer.setTimePoint("generate vole done");
    
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
        //std::cout << "P1 vole triple cost = " << comm / 1024 / 1024 << " MB" << std::endl;
    //}
    

    // close channel
    for (u32 i = 0; i < chl.size(); ++i){
        coproto::sync_wait(chl[i].close());
    }

    if (idx == 0){
        //std::cout << timer << std::endl;
        //std::cout<< iter->second << "  " << std::fixed << std::setprecision(1) << std::setw(9) << time << std::endl;
        //std::cout << "P1 vole triple cost = " << comm / 1024 / 1024 << " MB" << std::endl;

    }
}

