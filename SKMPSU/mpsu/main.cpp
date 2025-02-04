
/** @file
*****************************************************************************
This is an implementation of multi-party private set union (MPSU) based on batch ssPMT and mssROT.

References:
[DCZB-USENIX-2025]: Breaking Free: Efficient Multi-Party Private Set Union Without Non-Collusion Assumptions
Minglang Dong, Yu Chen, Cong Zhang, Yujie Bai
USENIX Security 2025,
<https://eprint.iacr.org/2024/1146>

*****************************************************************************
* @author developed by Yujie Bai (modified by Yu Chen)
*****************************************************************************/

#include <algorithm>
#include "../offlineGen/TripleGen.h"
#include "../offlineGen/VoleGen.h"
#include "../offlineGen/RotGen.h"
#include "../shuffle/ShareCorrelationGen.h"
#include "mpsu.h"


void SKMPSU_test(u32 idx, u32 numElements, u32 numParties, u32 numThreads){
    
    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(numElements, ssp, 0, 3);
    u32 numBins = params.numBins();    
    // generate share correlation
    ShareCorrelation sc(numParties, (numParties - 1) * numBins);  
    
    if (!sc.exist()){
        return;
    }
    std::vector<block> set(numElements);

    // generate set
    for (u32 i = 0; i < numElements; i++){
        set[i] = oc::toBlock(idx + i + 2);
    }

    if (idx == 0){
        std::vector<block> out;
        out = MPSUParty(idx, numParties, numElements, set, numThreads);
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
        MPSUParty(idx, numParties, numElements, set, numThreads);
    }
    sc.release();
    return ;
}




int main(int agrc, char** argv){
    
    CLP cmd;
    cmd.parse(agrc, argv);
    u32 nn = cmd.getOr("nn", 14);
    u32 n = cmd.getOr("n", 1ull << nn);
    u32 k = cmd.getOr("k", 3);
    u32 nt = cmd.getOr("nt", 1);
    u32 idx = cmd.getOr("r", -1);

    bool psuTest = cmd.isSet("psu");
    bool scGen = cmd.isSet("genSC");
    bool preGen = cmd.isSet("preGen");
    bool help = cmd.isSet("h");   

    if (help){
    	
        std::cout << "protocols" << std::endl;
        std::cout << "    -genSC:       generate share correlation" << std::endl;
        std::cout << "    -preGen:      generate vole, boolean triples and random OT" << std::endl;        
        std::cout << "    -psu:         multi-party private set union" << std::endl;
        std::cout << "parameters" << std::endl;
        std::cout << "    -n:           number of elements in each set, default 1024" << std::endl;
        std::cout << "    -nn:          logarithm of the number of elements in each set, default 10" << std::endl;
        std::cout << "    -k:           number of parties, default 3" << std::endl;
        std::cout << "    -nt:          number of threads, default 1" << std::endl;
        std::cout << "    -r:           index of party" << std::endl;
        return 0;
    }    

    if ((idx >= k || idx < 0) && !scGen){
        std::cout << "wrong idx of party, please use -h to print help information" << std::endl;
        return 0;
    }


    if (scGen){
        std::cout << "generate sc begin" << std::endl;
    	oc::CuckooParam params = oc::CuckooIndex<>::selectParams(n, ssp, 0, 3);
    	u32 numBins = params.numBins();         
        ShareCorrelation sc(k, (k - 1) * numBins);
        sc.generate();
        sc.writeToFile();
        sc.release();
        std::cout << "generate sc done" << std::endl;
        return 0;
    } else if (preGen){
        std::vector<double> comm(4);
        std::vector<double> timer(4);
        voleGenParty(idx, k, nn, nt, comm[0], timer[0]);
        rotGenParty(idx, k, nn, nt, comm[1], timer[1]);
        tripleGenParty(idx, k, nn, nt, comm[2], timer[2]);
        comm[3] = comm[0] + comm[1] + comm[2];
        timer[3] = timer[0] + timer[1] + timer[2];
        if(idx == 0)
        {
            std::cout<< " " << std::endl;
            std::cout<< "P1 offline preGen time cost = " << std::fixed << std::setprecision(3) << timer[3]/1000.0 << " s"  << std::endl;
            std::cout << "P1 offline preGen communication cost = "<< std::fixed << std::setprecision(3)  << comm[3] / 1024 / 1024 << " MB" << std::endl;
            std::cout<< " " << std::endl;           	
        
        }

    }        
    else if(psuTest){
        SKMPSU_test(idx, n, k, nt);
    }
    else {
        std::cout << "no protocol chosen, please use -h to print help information" << std::endl;
    }
    return 0;
}
