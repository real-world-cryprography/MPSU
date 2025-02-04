
/** @file
*****************************************************************************
This is an implementation of multi-party private set union(MPSU) based on batch ssPMT and MKR-PKE.

References:
[DCZB-USENIX-2025]: Breaking Free: Efficient Multi-Party Private Set Union Without Non-Collusion Assumptions
Minglang Dong, Yu Chen, Cong Zhang, Yujie Bai
USENIX Security 2025,
<https://eprint.iacr.org/2024/1146>

*****************************************************************************
* @author developed by Yujie Bai (modified by Yu Chen)
*****************************************************************************/

#include "cryptoTools/Common/CLP.h"
#include <algorithm>
#include "pkmpsu.h"
#include "../offlineGen/TripleGen.h"

void PKMPSU_test(u32 idx, u32 numElements, u32 numParties, u32 numThreads){
    
    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(numElements, ssp, 0, 3);
    u32 numBins = params.numBins();    
    
    // establish a reflection: ECPoint<--->u64   
    std::vector<block> set(numElements);
    std::vector<ECPoint> set_EC(numElements);
    
    // generate set
    ElGamal::PP pp = ElGamal::Setup();
    for (u32 i = 0; i < numElements; i++)
    {
        u64 numI = idx + i;
        set[i] = oc::toBlock(numI);
        BigInt m(numI);
        set_EC[i] = pp.g * m;        
    }
    if (idx == 0){
        std::vector<ECPoint> out;
        out = PKMPSUParty(idx, numParties, numElements, set, set_EC, numThreads);

        u32 UNION_CARDINALITY = numElements + numParties - 1;
        if(UNION_CARDINALITY == out.size()){
            std::cout << "Success!  union size: " << out.size() << std::endl;
        }
        else
        {
            std::cout << "Failure!  ideal union size: " << UNION_CARDINALITY << std::endl;
            std::cout << "Failure!  real union size: " << out.size() << std::endl;
        }
    } else {
        PKMPSUParty(idx, numParties, numElements, set, set_EC, numThreads);
    }
   
}



int main(int agrc, char** argv){

    BN_Initialize();
    ECGroup_Initialize(); 
    
    CLP cmd;
    cmd.parse(agrc, argv);
    u32 nn = cmd.getOr("nn", 8);
    u32 n = cmd.getOr("n", 1ull << nn);
    u32 k = cmd.getOr("k", 3);
    u32 nt = cmd.getOr("nt", 1);
    u32 idx = cmd.getOr("r", -1);

    bool preGen = cmd.isSet("preGen");
    bool psuTest = cmd.isSet("psu");
    bool help = cmd.isSet("h");        

    if (help){
        std::cout << "protocols" << std::endl;
        std::cout << "    -preGen:      generate boolean triples" << std::endl;        
        std::cout << "    -psu:         multi-party private set union" << std::endl;
        std::cout << "parameters" << std::endl;
        std::cout << "    -n:           number of elements in each set, default 1024" << std::endl;
        std::cout << "    -nn:          logarithm of the number of elements in each set, default 10" << std::endl;
        std::cout << "    -k:           number of parties, default 3" << std::endl;
        std::cout << "    -nt:          number of threads, default 1" << std::endl;
        std::cout << "    -r:           index of party" << std::endl;
        return 0;
    }    

    if ((idx >= k || idx < 0)){
        std::cout << "wrong idx of party, please use -h to print help information" << std::endl;
        return 0;
    }

    if (preGen){
        tripleGenParty(idx, k, nn, nt);
    } else if (psuTest){
        PKMPSU_test(idx, n, k, nt);
    } else {
        std::cout << "no protocol chosen, please use -h to print help information" << std::endl;
    }
    
    BN_Finalize();
    ECGroup_Finalize();        
    return 0;
}










