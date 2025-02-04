#include "pkmpsu.h"


void shuffleCT(std::vector<ElGamal::CT> &CT1, std::vector<ElGamal::CT> &CT2)
{
    u32 length = CT1.size();
    std::vector<u32> idxVec(length);
    for(u32 i = 0; i < length; ++i)
    {
       idxVec[i] = i;     
    }
    std::shuffle(idxVec.begin(), idxVec.end(), global_built_in_prg);
    CT2.resize(length);

    #pragma omp parallel for num_threads(NUMBER_OF_THREADS)
    for(u32 i = 0; i < length; ++i)
    {
       CT2[i] = CT1[idxVec[i]];     
    }
}


void sendECPoint(Socket &chl, ECPoint A)
{
    int thread_num = omp_get_thread_num();    
    std::vector<u8> buffer(POINT_COMPRESSED_BYTE_LEN);
    EC_POINT_point2oct(group, A.point_ptr, POINT_CONVERSION_COMPRESSED, buffer.data(), POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);
    coproto::sync_wait(chl.send(buffer));

}

void recvECPoint(Socket &chl, ECPoint &A)
{
    int thread_num = omp_get_thread_num();    
    std::vector<u8> buffer(POINT_COMPRESSED_BYTE_LEN);
    coproto::sync_wait(chl.recv(buffer));
    EC_POINT_oct2point(group, A.point_ptr, buffer.data(), POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);

}

void sendCTs(Socket &chl, std::vector<ElGamal::CT> &VecCT)
{

    u32 num = VecCT.size();
    std::vector<u8> buffer(POINT_COMPRESSED_BYTE_LEN * 2 * num);
    
    int thread_num = omp_get_thread_num();    
    for(u32 i = 0; i < num; ++i)
    {
    	EC_POINT_point2oct(group, VecCT[i].X.point_ptr, POINT_CONVERSION_COMPRESSED, buffer.data()+ 2*i*POINT_COMPRESSED_BYTE_LEN, POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);
    	EC_POINT_point2oct(group, VecCT[i].Y.point_ptr, POINT_CONVERSION_COMPRESSED, buffer.data()+ (2*i + 1)*POINT_COMPRESSED_BYTE_LEN, POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);
    	        
    }
    coproto::sync_wait(chl.send(buffer));

}

void recvCTs(Socket &chl, std::vector<ElGamal::CT> &VecCT)
{
    u32 num = VecCT.size();
    std::vector<u8> buffer(POINT_COMPRESSED_BYTE_LEN * 2 * num);
    coproto::sync_wait(chl.recv(buffer));
    
    int thread_num = omp_get_thread_num();
    
    for(u32 i = 0; i < num; ++i)
    {
    	EC_POINT_oct2point(group, VecCT[i].X.point_ptr, buffer.data() + 2*i*POINT_COMPRESSED_BYTE_LEN , POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]); 
    	EC_POINT_oct2point(group, VecCT[i].Y.point_ptr, buffer.data() + (2*i + 1)*POINT_COMPRESSED_BYTE_LEN , POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);    	        
    }    
}


void parDec(ElGamal::CT &ct, BigInt sk)
{
    ct.Y *= sk;
    ct.Y = ct.Y.Invert();
}


void CtToBlock5(ElGamal::CT ct, std::vector<block> &ctBlock)
{
    ctBlock.resize(5, ZeroBlock);
    int thread_num = omp_get_thread_num();
    EC_POINT_point2oct(group, ct.X.point_ptr, POINT_CONVERSION_COMPRESSED, (u8*)(ctBlock.data()), POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);
    EC_POINT_point2oct(group, ct.Y.point_ptr, POINT_CONVERSION_COMPRESSED, (u8*)(ctBlock.data()) + POINT_COMPRESSED_BYTE_LEN, POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);   
    
}

void Block5ToCt(ElGamal::CT &ct, std::vector<block> &ctBlock)
{
    assert(ctBlock.size() == 5);
    int thread_num = omp_get_thread_num();
    EC_POINT_oct2point(group, ct.X.point_ptr, (u8*)(ctBlock.data()), POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);
    EC_POINT_oct2point(group, ct.Y.point_ptr, (u8*)(ctBlock.data()) + POINT_COMPRESSED_BYTE_LEN, POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);    
}


void pkROTi(u32 idx, Socket &chl, const BitVector bitV, std::vector<ElGamal::CT> &ctVec1, PRNG& prng, ElGamal::PP &pp, ECPoint pk)
{

    u32 numElements = bitV.size();
    
    AlignedVector<block> rMgs(numElements);
    softRecv(numElements, bitV, chl, prng, rMgs);
    
    //get ryVec from ryVec 
    std::vector<block> ryVec(5 * numElements);
    for (u32 i = 0; i < numElements; i++){
    	PRNG prngR(rMgs[i]);
    	prngR.get(ryVec.data()+ i * 5, 5);        
    }
    

    // recv uo, u1
    std::vector<block> u0Vec(5 * numElements);
    std::vector<block> u1Vec(5 * numElements);
    coproto::sync_wait(chl.recv(u0Vec));
    coproto::sync_wait(chl.recv(u1Vec));        
       
    std::vector<ElGamal::CT> vyVec(numElements);
    
    #pragma omp parallel for num_threads(NUMBER_OF_THREADS)
    for(u32 i = 0; i < numElements; i++)
    {    
    	std::vector<block> ctblock(5);  
    	if(bitV[i])
    	{
    	    // ct = r ^ u1    	        	    
    	    for(u32 j = 0; j < 5; ++j)
    	    {
    	    	ctblock[j] = u1Vec[i*5 + j] ^ ryVec[i*5 + j];   	        	    
    	    }    	    	
    	}
    	else
    	{
    	    // ct = r ^ u0 	        	    
    	    for(u32 j = 0; j < 5; ++j)
    	    {
    	    	ctblock[j] = u0Vec[i*5 + j] ^ ryVec[i*5 + j];    	        	    
    	    }    	    	    	
    	}

    	Block5ToCt(vyVec[i], ctblock);
    	 	    	    	    
    }
    
    // rerand(pk, vy)
    if(idx == 0)
    {
    	ctVec1.resize(numElements);
    	#pragma omp parallel for num_threads(NUMBER_OF_THREADS)
    	for(u32 i = 0; i < numElements; i++)
    	{
    		ctVec1[i] = ElGamal::ReRand(pp, pk, vyVec[i]);
    	}    	    
    }
    else
    {
    	std::vector<ElGamal::CT> vyVec2(numElements);
    	#pragma omp parallel for num_threads(NUMBER_OF_THREADS)
    	for(u32 i = 0; i < numElements; i++)
    	{
    		vyVec2[i] = ElGamal::ReRand(pp, pk, vyVec[i]);
    	}    
    	sendCTs(chl, vyVec2);    
    }
   
}


void pkROTj(u32 pi, Socket &chl, const BitVector bitV, std::vector<ElGamal::CT> &cuckooCT, PRNG& prng, ElGamal::PP &pp, ECPoint pk)
{
    u32 numElements = bitV.size();

    // get r0,r1
    AlignedVector<std::array<block, 2>> sMgs(numElements);
    softSend(numElements, chl, prng, sMgs);

    
    std::vector<block> u0Vec(5 * numElements);
    std::vector<block> u1Vec(5 * numElements);
  
    // compute DUMB_CT = Enc(pk, pp.g)
    ElGamal::CT DUMB_CT = ElGamal::Enc(pp, pk, pp.g);
    std::vector<block> DUMB_BLOCK(5);
    CtToBlock5(DUMB_CT, DUMB_BLOCK);

    // compute u0,u1      
    #pragma omp parallel for num_threads(NUMBER_OF_THREADS)  
    for (u32 i = 0; i < numElements; i++){    	    
    	PRNG prng_u0(sMgs[i][0]);
    	prng_u0.get(u0Vec.data() + i*5, 5);    
    	
    	PRNG prng_u1(sMgs[i][1]);
    	prng_u1.get(u1Vec.data() + i*5, 5); 
    	
    	std::vector<block> ctblock(5, ZeroBlock);   
    	CtToBlock5(cuckooCT[i], ctblock);  	  
    	if(bitV[i])
    	{
    	    // u1 = r1 ^ ct, u0 = r0 ^ dumbct    	        	    
    	    for(u32 j = 0; j < 5; ++j)
    	    {
    	    	u1Vec[i*5 +j] ^= ctblock[j];
    	    	u0Vec[i*5 +j] ^= DUMB_BLOCK[j];    	        	    
    	    }    	    	
    	}
    	else
    	{
    	    // u0 = r0 ^ ct, u0 = r1 ^ dumbct    	        	    
    	    for(u32 j = 0; j < 5; ++j)
    	    {
    	    	u0Vec[i*5 +j] ^= ctblock[j];
    	    	u1Vec[i*5 +j] ^= DUMB_BLOCK[j];    	        	    
    	    }    	    	    	
    	}
    }
      
    coproto::sync_wait(chl.send(u0Vec));
    coproto::sync_wait(chl.send(u1Vec));
    
    // update cuckooCT
    if(pi != 0){   
       std::vector<ElGamal::CT> recvCT(numElements);
       recvCTs(chl, recvCT);
       #pragma omp parallel for num_threads(NUMBER_OF_THREADS)
       for (u32 i = 0; i < numElements; i++){
    	  cuckooCT[i] = ElGamal::ReRand(pp, pk, recvCT[i]);    
        }      
    }

}




std::vector<ECPoint> PKMPSUParty(u32 idx, u32 numParties, u32 numElements, std::vector<block> &set, std::vector<ECPoint> &set_EC, u32 numThreads)
{

    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(numElements, ssp, 0, 3);
    u32 numBins = params.numBins();
    
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
           
    // compute pk = PK_1 + PK_2 + ...
    ElGamal::PP pp = ElGamal::Setup();    
    ECPoint PK[numParties];
    BigInt sk; 
    std::tie(PK[idx], sk) = ElGamal::KeyGen(pp); 
    ECPoint pk;
        
    for (u32 i = 0; i < numParties; ++i)
    {
        if (i < idx)
        {
            recvECPoint(chl[i], PK[i]);
            sendECPoint(chl[i], PK[idx]);                
        } 
        else if (i > idx)
        {
            sendECPoint(chl[i-1], PK[idx]);        
            recvECPoint(chl[i-1], PK[i]);                        
        }            
    }
    pk = PK[0];    
    for(u32 i = 1; i < numParties; ++i)
    {
        pk += PK[i];        
    }
    //timer.setTimePoint("keyGen done");            

    // compute DUMB_CT
    ECPoint DUMB_ECPOINT = pp.g;
    ElGamal::CT DUMB_CT = ElGamal::Enc(pp, pk, DUMB_ECPOINT);
              
    PRNG prng(sysRandomSeed());
    block cuckooSeed = block(0x235677879795a931, 0x784915879d3e658a); 

    // all the parties establish simple hash table except pm    
    std::vector<std::vector<block>> simHashTable(numBins);   
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
        
    // for bssPMT 
    std::vector<BitVector> choice(numParties - 1);            
    std::vector<std::thread> bssPMTThrds(numParties - 1); 
            
    if(idx == 0)
    {     
        // bssPMT with other parties
        for (u32 i = 1; i < numParties; ++i)
        {             
            std::string triplePath = "./offline/triple_" + std::to_string(numParties) + "_" + std::to_string(numElements) + "_P" + std::to_string(idx + 1) + std::to_string(i + 1);                
            bssPMTThrds[i - 1] = std::thread([&, i, triplePath]() {      
                PKbssPMTSend(numParties, numElements, cuckooSeed, simHashTable, choice[i - 1], chl[i - 1], numThreads, triplePath);               
            });                               
        }
        for (auto& thrd : bssPMTThrds) thrd.join();
        //timer.setTimePoint("bssPMT done");        
        
        // ROT and messages rerandomization
        std::vector<ElGamal::CT> ctTotal(numBins * (numParties - 1));
        std::vector<std::vector<ElGamal::CT>> ctVec(numParties - 1);
        
        // std::vector<std::thread> ROTrrThrds(numParties - 1);  
        for (u32 i = 1; i < numParties; ++i){
            // ROTrrThrds[i - 1] = std::thread([&, i]() {   
            	pkROTi(idx, chl[i-1], choice[i - 1], ctVec[i-1], prng, pp, pk);            
            // });
        }
        // for (auto& thrd : ROTrrThrds) thrd.join(); 
        //timer.setTimePoint("ROT done"); 

        #pragma omp parallel for num_threads(NUMBER_OF_THREADS)
        for(u32 i = 0; i < numParties-1; ++i)
        {      
            for(u32 j = 0; j < numBins; ++j)
            {
            	ctTotal[i*numBins + j] = ctVec[i][j];           
            }
        }
        
        // shuffle the ctTotal
        std::vector<ElGamal::CT> ctTotal2(numBins * (numParties - 1));
        shuffleCT(ctTotal, ctTotal2);

        // p0 send ct to p1
        sendCTs(chl[0], ctTotal2);
        //timer.setTimePoint("shuffle and send done"); 
        
        // recv ct from pm
        std::vector<ElGamal::CT> ctRecv(numBins * (numParties - 1));
        recvCTs(chl[numParties-2], ctRecv);
        
        // output reconstruction
        std::vector<ECPoint> outputEC(set_EC);
        for(u32 i = 0; i < numBins * (numParties - 1); ++i)
        {
           ECPoint tmp;
           tmp = ElGamal::Dec(pp, sk, ctRecv[i]);
           if(tmp != DUMB_ECPOINT)
           {
           	outputEC.emplace_back(tmp);
           }               
        }   
        //timer.setTimePoint(" end");
        
        double comm = 0;
        for (u32 i = 0; i < chl.size(); ++i){
            comm += chl[i].bytesSent() + chl[i].bytesReceived();
        }
        // close sockets
        for (u32 i = 0; i < chl.size(); ++i){
            coproto::sync_wait(chl[i].flush());
            coproto::sync_wait(chl[i].close());
        }        
        timer.setTimePoint(" end");
        
	    std::cout << "P1 communication cost = " << std::fixed << std::setprecision(3) << comm / 1024 / 1024 << " MB" << std::endl;
        std::cout << timer<< std::endl;
        //auto iter = timer.mTimes.end();
        //--iter;
        //auto time = std::chrono::duration_cast<std::chrono::microseconds>(iter->first - timer.mTimes.front().first).count() / 1000.0;            
            	//std::cout<< " " << std::endl;
            	//std::cout<< "P1 online time cost = " << std::fixed << std::setprecision(3) << time/1000.0 << " s"  << std::endl;
            	//std::cout << "P1 online communication cost = "<< std::fixed << std::setprecision(3)  << comm / 1024 / 1024 << " MB" << std::endl;
            	//std::cout<< " " << std::endl;        
        return outputEC;        
            
    }
    else
    {    
        std::vector<ElGamal::CT> cuckooCT(numBins);        
        std::vector<block> cuckooHashTable(numBins); //x||z         
        
        // establish cuckoo hash table
        oc::CuckooIndex cuckoo;
        cuckoo.init(numElements, ssp, 0, 3);
        cuckoo.insert(set, cuckooSeed);
                              
        for (u32 i = 0; i < numBins; ++i)
        {
            auto bin = cuckoo.mBins[i];

            if (bin.isEmpty() == false)
            {
                auto j = bin.hashIdx();
                auto b = bin.idx();

                u64 halfK = set[b].mData[0];                
                cuckooHashTable[i] = block(halfK, j);//compute x||z                
                cuckooCT[i] = ElGamal::Enc(pp, pk, set_EC[b]);// compute C_j
            }                                                                                        
            else
            {          	         	   
          	cuckooCT[i] = DUMB_CT;       	              
            }                                
        } 
                        
        // bssPMT with other parties                      
        for (u32 i = 0; i < numParties; ++i){
            std::string triplePath = "./offline/triple_" + std::to_string(numParties) + "_" + std::to_string(numElements) + "_P" + std::to_string(idx + 1) + std::to_string(i + 1);                       
            
            if (i < idx){
                bssPMTThrds[i] = std::thread([&, i, triplePath]() {                    
                    PKbssPMTRecv(numParties, numElements, cuckooSeed, cuckooHashTable, choice[i], chl[i], numThreads, triplePath);
                });
            } else if (i > idx){
                bssPMTThrds[i - 1] = std::thread([&, i, triplePath]() {              
                    PKbssPMTSend(numParties, numElements, cuckooSeed, simHashTable, choice[i-1], chl[i-1], numThreads, triplePath);                    
                });
            }
        }
        for (auto& thrd : bssPMTThrds) thrd.join();        
        //timer.setTimePoint("bssPMT done"); 

        // ROT and messages rerandomization        
        //std::vector<std::thread> ROTrrThrds(numParties - 1); 
        for(u32 i = numParties -1; i > 0; --i){
            if (i < idx){
                //ROTrrThrds[i] = std::thread([&, i]() {                   
                    // there may be something wrong with the common cuckooCT
                    pkROTj(i, chl[i], choice[i], cuckooCT, prng, pp, pk);
                //});
            } else if (i > idx){
                //ROTrrThrds[i - 1] = std::thread([&, i]() {               
                    std::vector<ElGamal::CT> tmp;// unused
                    pkROTi(idx, chl[i-1], choice[i-1], tmp, prng, pp, pk);                
                //});
            }
        }
        pkROTj(0, chl[0], choice[0], cuckooCT, prng, pp, pk);
        //for (auto& thrd : ROTrrThrds) thrd.join();  
        
        // receive the CT from p_{idx-1} 
        std::vector<ElGamal::CT> ctTotal(numBins * (numParties - 1));        
        recvCTs(chl[idx-1], ctTotal);
        
        //corrected point 3
        // ct_j = ParDec(sk_j, ct_j-1)
        #pragma omp parallel for num_threads(NUMBER_OF_THREADS)
        for(u32 i = 0; i < numBins * (numParties - 1); ++i)
        {            
            ctTotal[i].Y = ctTotal[i].Y - ctTotal[i].X *sk;     
        }
        
        // compute pk_Aj = pk_0 + pk_{j+1} + ... + pk_m
        ECPoint pk_Aj = PK[0];
        for(u32 i = idx + 1; i < numParties; ++i)
        {
            pk_Aj += PK[i];
        }
        
        // rerand(pk_Aj, ct_j)
        #pragma omp parallel for num_threads(NUMBER_OF_THREADS)
        for(u32 i = 0; i < numBins * (numParties - 1); ++i)
        {            
            ctTotal[i] = ElGamal::ReRand(pp, pk_Aj, ctTotal[i]);      
        }    
        
        // shuffle the ct_j and send to p_{(j+1)%m}  
        std::vector<ElGamal::CT> ctTotal2(numBins * (numParties - 1));
        shuffleCT(ctTotal, ctTotal2);      
          
        // if j == m, send ct to p0; else send ct to p_j+1
        u32 next_party = (idx + 1) % numParties;
        if(next_party > idx){
            sendCTs(chl[next_party -1], ctTotal2); 
        }
        else{
            sendCTs(chl[next_party], ctTotal2); 
        }

        double comm = 0;
        for (u32 i = 0; i < chl.size(); ++i){
            	comm += chl[i].bytesSent() + chl[i].bytesReceived();
        }        
        
        // close sockets
        for (u32 i = 0; i < chl.size(); ++i){
            coproto::sync_wait(chl[i].flush());
            coproto::sync_wait(chl[i].close());
        }    
        if(idx == 1){    
        timer.setTimePoint(" end");
        
	    std::cout << "P" << idx+1 <<" communication cost = " << std::fixed << std::setprecision(3) << comm / 1024 / 1024 << " MB" << std::endl;
        std::cout << timer<< std::endl;    
        } 
        return std::vector<ECPoint>();
        
    
    }   

}

