#include "ec_point.h"


ECPoint::ECPoint(){
    this->point_ptr = EC_POINT_new(group);
}

ECPoint::ECPoint(const ECPoint& other){
    this->point_ptr = EC_POINT_new(group);
    EC_POINT_copy(this->point_ptr, other.point_ptr);
}

ECPoint::ECPoint(const EC_POINT* &other){
    this->point_ptr = EC_POINT_new(group);
    EC_POINT_copy(this->point_ptr, other);
}

ECPoint::ECPoint(const BigInt& x, const BigInt& y){
    this->point_ptr = EC_POINT_new(group);
    int thread_num = omp_get_thread_num();
    EC_POINT_set_affine_coordinates_GFp(group, this->point_ptr, x.bn_ptr, y.bn_ptr, bn_ctx[thread_num]);
}

void ECPoint::ReInitialize(){
    if (this->point_ptr == nullptr){
        this->point_ptr = EC_POINT_new(group);
    }
}

ECPoint ECPoint::Mul(const BigInt& scalar) const {
    ECPoint result; 
    int thread_num = omp_get_thread_num();
    // use fix-point exp with precomputation
    if (EC_POINT_cmp(group, this->point_ptr, generator, bn_ctx[thread_num]) == 0){
        CRYPTO_CHECK(1 == EC_POINT_mul(group, result.point_ptr, scalar.bn_ptr, nullptr, nullptr, bn_ctx[thread_num]));
    }
    else{
        CRYPTO_CHECK(1 == EC_POINT_mul(group, result.point_ptr, nullptr, this->point_ptr, scalar.bn_ptr, bn_ctx[thread_num]));
    }
 
    return result;
}


ECPoint ECPoint::Add(const ECPoint& other) const {  

    ECPoint result; 
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == EC_POINT_add(group, result.point_ptr, this->point_ptr, other.point_ptr, bn_ctx[thread_num])); 
    return result; 
}


ECPoint ECPoint::Invert() const {
    // Create a copy of this.
    ECPoint result = (*this);  
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == EC_POINT_invert(group, result.point_ptr, bn_ctx[thread_num])); 
    return result; 
}


ECPoint ECPoint::Sub(const ECPoint& other) const { 
    ECPoint result = other.Invert(); 
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == EC_POINT_add(group, result.point_ptr, this->point_ptr, result.point_ptr, bn_ctx[thread_num]));
    return result; 
}


void ECPoint::Clone(const ECPoint& other) const {
    CRYPTO_CHECK(1 == EC_POINT_copy(this->point_ptr, other.point_ptr)); 
}


bool ECPoint::IsAtInfinity() const {
    return EC_POINT_is_at_infinity(group, this->point_ptr);
}

// Returns true if the given point is in the group.
bool ECPoint::IsOnCurve() const {
    int thread_num = omp_get_thread_num();
    return (1 == EC_POINT_is_on_curve(group, this->point_ptr, bn_ctx[thread_num]));
}

// Checks if the given point is valid. Returns false if the point is not in the group or if it is the point is at infinity.
bool ECPoint::IsValid() const{
    if (!this->IsOnCurve() || this->IsAtInfinity()){
        return false;
    }
    return true;
}

bool ECPoint::CompareTo(const ECPoint& other) const{
    int thread_num = omp_get_thread_num();
    return (0 == EC_POINT_cmp(group, this->point_ptr, other.point_ptr, bn_ctx[thread_num]));
}


void ECPoint::SetInfinity()
{
    CRYPTO_CHECK(1 == EC_POINT_set_to_infinity(group, this->point_ptr));
}

void ECPoint::Print() const
{ 
    int thread_num = omp_get_thread_num();
    char *ecp_str = EC_POINT_point2hex(group, this->point_ptr, POINT_CONVERSION_COMPRESSED, bn_ctx[thread_num]);
    std::cout << ecp_str << std::endl; 
    OPENSSL_free(ecp_str); 
}

// print an EC point with note
void ECPoint::Print(std::string note) const
{ 
    std::cout << note << " = "; 
    this->Print(); 
}

// convert an EC Point to byte string
std::string ECPoint::ToByteString() const
{
    std::string ecp_str(POINT_COMPRESSED_BYTE_LEN, '0'); 
    int thread_num = omp_get_thread_num();
    EC_POINT_point2oct(group, this->point_ptr, POINT_CONVERSION_COMPRESSED, 
                       reinterpret_cast<unsigned char *>(&ecp_str[0]), POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);
    return ecp_str; 
}

// convert an EC point to string
std::string ECPoint::ToHexString() const
{
    std::stringstream ss; 
    int thread_num = omp_get_thread_num();
    ss << EC_POINT_point2hex(group, this->point_ptr, POINT_CONVERSION_COMPRESSED, bn_ctx[thread_num]);
    return ss.str();  
}



// hash an ECPoint to uint64_4 number 
size_t ECPoint::ToUint64() const
{
    // standard method
    unsigned char buffer[POINT_COMPRESSED_BYTE_LEN];
    memset(buffer, 0, POINT_COMPRESSED_BYTE_LEN); 
    int thread_num = omp_get_thread_num();
    EC_POINT_point2oct(group, this->point_ptr, POINT_CONVERSION_COMPRESSED, buffer, 
                       POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);
    return MurmurHash64A(buffer, POINT_COMPRESSED_BYTE_LEN, fixed_salt64); 
}
/**
// adhoc lossy encoding for ECPoint based on AES
size_t ECPoint::FastToUint64() const 
{
    unsigned char buffer[POINT_COMPRESSED_BYTE_LEN];
    memset(buffer, 0, POINT_COMPRESSED_BYTE_LEN); 
    int thread_num = omp_get_thread_num();
    EC_POINT_point2oct(group, this->point_ptr, POINT_CONVERSION_COMPRESSED, buffer, 
                       POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);

    block data[2];
    data[0] = _mm_load_si128((block *)(buffer));
    data[1] = _mm_load_si128((block *)(buffer+16));

    AES::FastECBEnc(AES::fixed_enc_key, &data[0], 1);

    data[1] = _mm_xor_si128(data[1], data[0]);

    size_t hashvalue; 
    memcpy(&hashvalue, &data[1], 8); 

    return hashvalue; 
}
**/
std::ofstream &operator<<(std::ofstream &fout, const ECPoint &A)
{ 
    int thread_num = omp_get_thread_num();
	#ifdef ECPOINT_COMPRESSED
		unsigned char buffer[POINT_COMPRESSED_BYTE_LEN];
		EC_POINT_point2oct(group, A.point_ptr, POINT_CONVERSION_COMPRESSED, buffer, POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);
        fout.write(reinterpret_cast<char *>(buffer), POINT_COMPRESSED_BYTE_LEN); 
	#else
		unsigned char buffer[POINT_BYTE_LEN];
		EC_POINT_point2oct(group, A.point_ptr, POINT_CONVERSION_UNCOMPRESSED, buffer, POINT_BYTE_LEN, bn_ctx[thread_num]);
        fout.write(reinterpret_cast<char *>(buffer), POINT_BYTE_LEN); 
	#endif

    return fout;            
}
 
std::ifstream &operator>>(std::ifstream &fin, ECPoint &A)
{ 
    int thread_num = omp_get_thread_num();
    #ifdef ECPOINT_COMPRESSED
        unsigned char buffer[POINT_COMPRESSED_BYTE_LEN];
        fin.read(reinterpret_cast<char *>(buffer), POINT_COMPRESSED_BYTE_LEN); 
        EC_POINT_oct2point(group, A.point_ptr, buffer, POINT_COMPRESSED_BYTE_LEN, bn_ctx[thread_num]);
    #else
        unsigned char buffer[POINT_BYTE_LEN];
        fin.read(reinterpret_cast<char *>(buffer), POINT_BYTE_LEN); 
        EC_POINT_oct2point(group, A.point_ptr, buffer, POINT_BYTE_LEN, bn_ctx[thread_num]); 
    #endif
    return fin;            
}



/* 
 *  non-class functions
*/


std::ofstream &operator<<(std::ofstream &fout, const std::vector<ECPoint> &vec_A)
{ 
    for(auto i = 0; i < vec_A.size(); i++){
        fout << vec_A[i];
    }
    return fout;            
}
 
std::ifstream &operator>>(std::ifstream &fin, std::vector<ECPoint> &vec_A)
{ 
    for(auto i = 0; i < vec_A.size(); i++){
        fin >> vec_A[i];
    }
    return fin;            
}

// Creates an ECPoint object with the given x, y affine coordinates.
ECPoint CreateECPoint(const BigInt& x, const BigInt& y)
{
    ECPoint ecp_result(x, y);
    if (!ecp_result.IsValid()) {
        std::cerr << "ECGroup::CreateECPoint(x,y) - The point is not valid." << std::endl;
        exit(EXIT_FAILURE);
    }
    return ecp_result;
}

ECPoint GenRandomGenerator()
{
    BigInt bn_order(order); 
    ECPoint result = ECPoint(generator) * GenRandomBigIntBetween(bn_1, bn_order);
    return result; 
}

// Creates an ECPoint which is the identity.
ECPoint GetPointAtInfinity(){
    ECPoint result;
    CRYPTO_CHECK(1 == EC_POINT_set_to_infinity(group, result.point_ptr));
    return result;
}


bool IsSquare(const BigInt& q) {
    return q.ModExp(BigInt(curve_params_q), BigInt(curve_params_p)).IsOne();
}


// ecpoint vector operations

// mul exp operations
ECPoint ECPointVectorMul(const std::vector<ECPoint> &vec_A, std::vector<BigInt> &vec_a){
    if (vec_A.size()!=vec_a.size()){
        std::cerr << "vector size does not match" << std::endl; 
        exit(EXIT_FAILURE);
    }
    ECPoint result; 
    size_t LEN = vec_A.size(); 
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == EC_POINTs_mul(group, result.point_ptr, nullptr, LEN, 
                 (const EC_POINT**)vec_A.data(), (const BIGNUM**)vec_a.data(), bn_ctx[thread_num]));
    return result; 
}

// mul exp operations
ECPoint ECPointVectorMul(const std::vector<ECPoint> &vec_A, std::vector<BigInt> &vec_a, size_t start_index, size_t end_index){
    std::vector<ECPoint> subvec_A(vec_A.begin()+start_index, vec_A.begin()+end_index);
    std::vector<BigInt>  subvec_a(vec_a.begin()+start_index, vec_a.begin()+end_index);
    return ECPointVectorMul(subvec_A, subvec_a); 
}


/* g[i] = g[i]+h[i] */ 
std::vector<ECPoint> ECPointVectorAdd(std::vector<ECPoint> &vec_A, std::vector<ECPoint> &vec_B)
{
    if (vec_A.size()!= vec_B.size()) {
        std::cerr << "vector size does not match!" << std::endl;
        exit(EXIT_FAILURE); 
    }
    size_t LEN = vec_A.size();
    std::vector<ECPoint> vec_result(LEN); 

    #pragma omp parallel for num_threads(NUMBER_OF_THREADS)
    for (auto i = 0; i < vec_A.size(); i++) {
        vec_result[i] = vec_A[i] + vec_B[i]; 
    }
    return vec_result;
}



/* generate a vector of random EC points */  
std::vector<ECPoint> GenRandomECPointVector(size_t LEN)
{
    std::vector<ECPoint> vec_result(LEN); 
    #pragma omp parallel for num_threads(NUMBER_OF_THREADS)
    for(auto i = 0; i < LEN; i++){ 
        vec_result[i] = GenRandomGenerator(); 
    }
    return vec_result;
}

ECPoint GenRandomECPoint()
{
    return GenRandomGenerator(); 
}

// print an EC Point vector
void PrintECPointVector(const std::vector<ECPoint> &vec_A, std::string note)
{ 
    for (auto i = 0; i < vec_A.size(); i++)
    {
        std::cout << note << "[" << i << "]="; 
        vec_A[i].Print(); 
    }
}










