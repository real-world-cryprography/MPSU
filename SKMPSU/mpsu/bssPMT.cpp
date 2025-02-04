#include "bssPMT.h"


void bssPMTSend(u32 numParties, u32 numElements, block cuckooSeed, std::vector<std::vector<block>> &simHashTable, BitVector &out, Socket& chl, u32 numThreads, std::string triplePath, std::string volePath)
{  
    PRNG prng(sysRandomSeed());
    u32 numBins = simHashTable.size();

    block paxosSeed = prng.get();    
    coproto::sync_wait(chl.send(paxosSeed));
    Paxos<u32> mPaxos;
    mPaxos.init(numElements * 3, 3, ssp, PaxosParam::Binary, paxosSeed);

    u32 c_n2 = (numParties * (numParties -1))/2;
    u32 keyBitLength = ssp + oc::log2ceil(c_n2 * numBins);  
    u32 keyByteLength = oc::divCeil(keyBitLength, 8);
    // assert(keyBitLength < 65);
        

    std::ifstream voleFile;
    if (!volePath.empty()){
        voleFile.open(volePath, std::ios::binary | std::ios::in);
        if (!voleFile.is_open()){
            std::cout << "Error opening file " << volePath << ", using fake vole send" << std::endl;
        }
    }


    block mD;
    std::vector<block> mB(numBins, ZeroBlock);
    if (voleFile.is_open()){
        voleFile.read((char*)(&mD), sizeof(block));
        voleFile.read((char*)mB.data(), mB.size() * sizeof(block));        
    }
    
    // diffC received
    std::vector<block> diffC(numBins);
    coproto::sync_wait(chl.recv(diffC));

    // set for PRF(k, x||z)    
    oc::AES hasher;
    hasher.setKey(cuckooSeed);
    
    // get random labels
    mMatrix<u8> mLabel(numBins, keyByteLength);
    prng.get(mLabel.data(), mLabel.size());
    
    std::vector<block> vecK(numElements * 3);
    mMatrix<u8> values(numElements * 3, keyByteLength);
    u32 countV = 0;
    
    // simHashTable[i]: [BinSizes[i], vecK_0, ...]
    for (u32 i = 0; i < numBins; ++i)
    {
    	auto size = simHashTable[i].size();
    	for(u32 p = 0; p < size; ++p)
    	{
    	    auto hyj = simHashTable[i][p];
    	    vecK[countV] = hyj;
    	    
    	    hyj ^= diffC[i];
    	    auto tmp2 = mB[i] ^ (hyj.gf128Mul(mD));
    	    tmp2 = hasher.hashBlock(tmp2);
    	    memcpy(&values(countV, 0), &tmp2, keyByteLength);
    	    
    	    for(u64 ii = 0; ii < keyByteLength; ++ii)
    	    {
    	    	values(countV, ii) = values(countV, ii) ^ mLabel(i,ii); 
    	    }
    	    countV += 1;    	    	
    	}        
    }
    
    mMatrix<u8> okvs(mPaxos.size(), keyByteLength);
    mPaxos.setInput(vecK);
    mPaxos.encode<u8>(values, okvs);    
    // when using multi threads for okvs, there might be some problems    
    coproto::sync_wait(chl.send(okvs));
    
    // call gmw
    volePSI::BetaCircuit cir = volePSI::isZeroCircuit(keyBitLength);
    volePSI::Gmw cmp;
    cmp.init(mLabel.rows(), cir, numThreads, 1, prng.get());
    
    
    
    std::vector<block> a, b, c, d;
    u64 numTriples = cmp.mNumOts / 2;
    
    a.resize(numTriples / 128, ZeroBlock);
    b.resize(numTriples / 128, ZeroBlock);
    c.resize(numTriples / 128, ZeroBlock);
    d.resize(numTriples / 128, ZeroBlock);
    
    std::ifstream tripleFile;
    if (!triplePath.empty()){
        tripleFile.open(triplePath, std::ios::binary | std::ios::in);
        if (!tripleFile.is_open()){
            std::cout << "Error opening file " << triplePath << ", using fake triples send" << std::endl;
        }
    }


    if (tripleFile.is_open()){
        //tripleFile.seekg(0L, std::ios::beg);
        tripleFile.read((char*)a.data(), a.size() * sizeof(block));
        tripleFile.read((char*)b.data(), b.size() * sizeof(block));
        tripleFile.read((char*)c.data(), c.size() * sizeof(block));
        tripleFile.read((char*)d.data(), d.size() * sizeof(block));
    }    
    cmp.setTriples(a, b, c, d);    
    
    
    cmp.setInput(0, mLabel);
    coproto::sync_wait(cmp.run(chl));
    
    mMatrix<u8> mOut;
    mOut.resize(numBins, 1);
    cmp.getOutput(0, mOut);
    
    // get the final output
    out.resize(numBins);
    for (u32 i = 0; i < numBins; ++i){
        out[i] = mOut(i, 0) & 1;
    }    
    
   return;

}


void bssPMTRecv(u32 numParties, u32 numElements, block cuckooSeed, std::vector<block> &cuckooHashTable, BitVector &out, Socket &chl, u32 numThreads, std::string triplePath, std::string volePath)
{

    PRNG prng(sysRandomSeed());
    u32 numBins = cuckooHashTable.size();

    block paxosSeed;
    coproto::sync_wait(chl.recv(paxosSeed));

    Paxos<u32> mPaxos;
    mPaxos.init(numElements * 3, 3, ssp, PaxosParam::Binary, paxosSeed);
    
        
    u32 c_n2 = (numParties * (numParties -1))/2;
    u32 keyBitLength = ssp + oc::log2ceil(c_n2 * numBins);  
    u32 keyByteLength = oc::divCeil(keyBitLength, 8);
    // assert(keyBitLength < 65);
     
    // get mA mC of vole: a+b = c*d
    std::ifstream voleFile;
    if (!volePath.empty()){
        voleFile.open(volePath, std::ios::binary | std::ios::in);
        if (!voleFile.is_open()){
            std::cout << "Error opening file " << volePath << ", using fake vole recv" << std::endl;
        }
    }


    std::vector<block> mA(numBins, ZeroBlock);
    std::vector<block> mC(numBins, ZeroBlock);
    if (voleFile.is_open()){
        voleFile.read((char*)mA.data(), mA.size() * sizeof(block));
        voleFile.read((char*)mC.data(), mC.size() * sizeof(block));
    }
    
    
    // establish cuckoo hash table, compute diffC cuckooHashTable
    oc::AES hasher;
    //hasher.setKey(block(6788, 78978) ^ cuckooSeed);
    hasher.setKey(cuckooSeed);    
    
    std::vector<block> diffC(numBins); 
    mMatrix<u8> values(numBins, keyByteLength);
    
    for (u32 i = 0; i < numBins; ++i)
    {
    	diffC[i] = cuckooHashTable[i] ^ mC[i];                     
       // compute the difference between vecC and cuckooTable
                        
     }     
    
    coproto::sync_wait(chl.send(diffC));
    
    
    // compute the values = decode(k,okvs)
    mMatrix<u8> okvs(mPaxos.size(), keyByteLength);
    coproto::sync_wait(chl.recv(okvs));
    //mPaxos.decode<u8>(cuckooHashTable, values, okvs, numThreads);
    mPaxos.decode<u8>(cuckooHashTable, values, okvs);
    
    // compute mLabel = values ^ hash(mA)
    mMatrix<u8> mLabel(numBins, keyByteLength);
    
    for (u32 i = 0; i < numBins; ++i)
    {
    	auto prf = hasher.hashBlock(mA[i]);
    	memcpy(&mLabel(i,0), &prf, keyByteLength);
    	for(u32 ii = 0; ii < keyByteLength; ++ii)
    		mLabel(i, ii) ^= values(i,ii);          
    }
    
    
    volePSI::BetaCircuit cir = volePSI::isZeroCircuit(keyBitLength);
    volePSI::Gmw cmp;
    cmp.init(mLabel.rows(), cir, numThreads, 0, prng.get());
    cmp.implSetInput(0, mLabel, mLabel.cols());;
    
    std::vector<block> a, b, c, d;
    u64 numTriples = cmp.mNumOts / 2;
    a.resize(numTriples / 128, ZeroBlock);
    b.resize(numTriples / 128, ZeroBlock);
    c.resize(numTriples / 128, ZeroBlock);
    d.resize(numTriples / 128, ZeroBlock);

    std::ifstream tripleFile;
    if (!triplePath.empty()){
        tripleFile.open(triplePath, std::ios::binary | std::ios::in);
        if (!tripleFile.is_open()){
            std::cout << "Error opening file " << triplePath << ", using fake triples" << std::endl;
        }
    }


    if (tripleFile.is_open()){
        //tripleFile.seekg(0L, std::ios::beg);
        tripleFile.read((char*)a.data(), a.size() * sizeof(block));
        tripleFile.read((char*)b.data(), b.size() * sizeof(block));
        tripleFile.read((char*)c.data(), c.size() * sizeof(block));
        tripleFile.read((char*)d.data(), d.size() * sizeof(block));
    }
    cmp.setTriples(a, b, c, d);    
           
    
    coproto::sync_wait(cmp.run(chl));
    
    mMatrix<u8> mOut;
    mOut.resize(numBins, 1);
    cmp.getOutput(0, mOut);
    
    out.resize(numBins);
    for (u32 i = 0; i < numBins; ++i){
        out[i] = mOut(i, 0) & 1;
    }        
    
    
    
 return;  



}


























