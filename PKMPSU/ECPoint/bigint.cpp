#include "bigint.h"

void BN_Initialize(){
    for(auto i = 0; i < NUMBER_OF_THREADS; i++){
        bn_ctx[i] = BN_CTX_new();
        if (bn_ctx[i] == nullptr) std::cerr << "bn_ctx initialize fails" << std::endl;
    }
    //BN_BIT_LEN = BN_BYTE_LEN * 8; 
    INT_BYTE_LEN = sizeof(size_t); 
}

void BN_Finalize(){
    for(auto i = 0; i < NUMBER_OF_THREADS; i++){
        BN_CTX_free(bn_ctx[i]);
    }
} 

// Copies the given BigInt.
BigInt::BigInt(){ 
    this->bn_ptr = BN_new(); 
}

BigInt::BigInt(const BigInt& other){
    this->bn_ptr = BN_new();
    BN_copy(this->bn_ptr, other.bn_ptr);
}

BigInt::BigInt(const BIGNUM *other){
    this->bn_ptr = BN_new(); 
    BN_copy(this->bn_ptr, other); 
}

// Creates a new BigInt object from the number.
BigInt::BigInt(size_t number){
    this->bn_ptr = BN_new();
    CRYPTO_CHECK(BN_set_word(this->bn_ptr, number));
}

BigInt::~BigInt(){
    BN_free(this->bn_ptr); 
}


// Converts this BigInt to a uint64_t value. Returns an INVALID_ARGUMENT
uint64_t BigInt::ToUint64() const
{
    uint64_t result = BN_get_word(this -> bn_ptr);
    return result;
}

// Creates a new BigInt object from a bytes string.
void BigInt::FromByteString(const std::string& str) 
{ 
    BN_bin2bn(reinterpret_cast<const unsigned char*>(str.data()), str.size(), this->bn_ptr);
}

// Creates a new BigInt object from an unsigned char buffer.
void BigInt::FromByteString(const unsigned char* buffer, size_t LEN)
{ 
    BN_bin2bn(buffer, LEN, this->bn_ptr);
}

// an ad-hoc implementation for fixed length conversion: default LEN = BN_BYTE_LEN
void BigInt::ToByteString(unsigned char* buffer, size_t LEN) const
{
    BN_bn2bin(this->bn_ptr, buffer);
}  

std::string BigInt::ToByteString() const
{
    size_t LEN = this->GetByteLength();
    unsigned char buffer[LEN];
    memset(buffer, 0, LEN);  
    BN_bn2bin(this->bn_ptr, buffer);
    std::string result(reinterpret_cast<char *>(buffer), LEN); 
    return result;
}  

/* convert a Big number to string */
std::string BigInt::ToHexString() const
{
    std::stringstream ss; 
    ss << BN_bn2hex(this->bn_ptr);
    return ss.str();  
}

void BigInt::FromByteVector(const std::vector<uint8_t>& vec_A)
{ 
    BN_bin2bn(reinterpret_cast<const unsigned char *>(vec_A.data()), vec_A.size(), this->bn_ptr);         
}

std::vector<uint8_t> BigInt::ToByteVector(size_t LEN) const
{ 
    std::vector<uint8_t> vec_A(LEN, '0');
    BN_bn2binpad(this->bn_ptr, vec_A.data(), LEN);
    return vec_A;            
}



// Returns a BigInt whose value is (- *this).
// Causes a check failure if the operation fails.
BigInt BigInt::Negate() const {
    BigInt result = *this;
    BN_set_negative(result.bn_ptr, !BN_is_negative(result.bn_ptr));
    return result;
}

// Returns a BigInt whose value is (*this + val).
// Causes a check failure if the operation fails.
BigInt BigInt::Add(const BigInt& other) const {
    BigInt result;
    CRYPTO_CHECK(1 == BN_add(result.bn_ptr, this->bn_ptr, other.bn_ptr));
    return result;
}

// Returns a BigInt whose value is (*this - val).
// Causes a check failure if the operation fails.
BigInt BigInt::Sub(const BigInt& other) const {
    BigInt result;
    CRYPTO_CHECK(1 == BN_sub(result.bn_ptr, this->bn_ptr, other.bn_ptr));
    return result;
}

// Returns a BigInt whose value is (*this * val).
// Causes a check failure if the operation fails.
BigInt BigInt::Mul(const BigInt& other) const {
    BigInt result;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == BN_mul(result.bn_ptr, this->bn_ptr, other.bn_ptr, bn_ctx[thread_num]));
    return result;
}

// Returns a BigInt whose value is (*this / val).
// Causes a check failure if the remainder != 0 or if the operation fails.
BigInt BigInt::Div(const BigInt& other) const {
    BigInt result;
    BigInt remainder;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == BN_div(result.bn_ptr, remainder.bn_ptr, this->bn_ptr, other.bn_ptr, bn_ctx[thread_num]));
    if (BN_is_zero(remainder.bn_ptr)){
        std::cerr << "Use DivAndTruncate() instead of Div() if you want truncated division." << std::endl;  
    } 
    return result;
}

// Returns a BigInt whose value is *this / val, rounding towards zero.
// Causes a check failure if the remainder != 0 or if the operation fails.
BigInt BigInt::DivAndTruncate(const BigInt& other) const {
    BigInt result;
    BigInt remainder;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == BN_div(result.bn_ptr, remainder.bn_ptr, this->bn_ptr, other.bn_ptr, bn_ctx[thread_num]));
    return result;
}

// Compares this BigInt with the specified BigInt.
// Returns -1 if *this < val, 0 if *this == val and 1 if *this > val.
int BigInt::CompareTo(const BigInt& other) const {
    return BN_cmp(this->bn_ptr, other.bn_ptr);
}

// Returns a BigInt whose value is (*this ^ exponent).
// Causes a check failure if the operation fails.
BigInt BigInt::Exp(const BigInt& exponent) const{
    BigInt result;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == BN_exp(result.bn_ptr, this->bn_ptr, exponent.bn_ptr, bn_ctx[thread_num]));
    return result;
}

// return square
BigInt BigInt::Square() const{
    BigInt result;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == BN_sqr(result.bn_ptr, this->bn_ptr, bn_ctx[thread_num]));
    return result;
}

// Returns a BigInt whose value is (*this mod m).
BigInt BigInt::Mod(const BigInt& modulus) const {
    BigInt result;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == BN_nnmod(result.bn_ptr, this->bn_ptr, modulus.bn_ptr, bn_ctx[thread_num]));
    return result;
}

// Returns a BigInt whose value is (*this + val mod m).
// Causes a check failure if the operation fails.
BigInt BigInt::ModAdd(const BigInt& other, const BigInt& modulus) const {
    BigInt result;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == BN_mod_add(result.bn_ptr, this->bn_ptr, other.bn_ptr, modulus.bn_ptr, bn_ctx[thread_num]));
    return result;
}

// Returns a BigInt whose value is (*this - val mod m).
// Causes a check failure if the operation fails.
BigInt BigInt::ModSub(const BigInt& other, const BigInt& modulus) const {
    BigInt result;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == BN_mod_sub(result.bn_ptr, this->bn_ptr, other.bn_ptr, modulus.bn_ptr, bn_ctx[thread_num]));
    return result;
}

// Returns a BigInt whose value is (*this * val mod m).
BigInt BigInt::ModMul(const BigInt& other, const BigInt& modulus) const {
    BigInt result;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == BN_mod_mul(result.bn_ptr, this->bn_ptr, other.bn_ptr, modulus.bn_ptr, bn_ctx[thread_num]));
    return result;
}

// Returns a BigInt whose value is (*this ^ exponent mod m).
// Causes a check failure if the operation fails.
BigInt BigInt::ModExp(const BigInt& exponent, const BigInt& modulus) const {
    if (exponent.IsNonNegative() == false){
        std::cerr << "Cannot use a negative exponent in BigInt ModExp." << std::endl; 
    } 
    BigInt result;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == BN_mod_exp(result.bn_ptr, this->bn_ptr, exponent.bn_ptr, modulus.bn_ptr, bn_ctx[thread_num]));
    return result;
}

// Return a BigInt whose value is (*this^2 mod m).
// Causes a check failure if the operation fails.
BigInt BigInt::ModSquare(const BigInt& modulus) const {
    BigInt result;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == BN_mod_sqr(result.bn_ptr, this->bn_ptr, modulus.bn_ptr, bn_ctx[thread_num]));
    return result;
}

// Returns a BigInt whose value is (*this ^ -1 mod m).
// Causes a check failure if the operation fails.
BigInt BigInt::ModInverse(const BigInt& modulus) const {
    BigInt result;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(nullptr != BN_mod_inverse(result.bn_ptr, this->bn_ptr, modulus.bn_ptr, bn_ctx[thread_num]));
    return result;
}

// Returns r such that r^2 == *this mod p.
// Causes a check failure if the operation fails.
BigInt BigInt::ModSquareRoot(const BigInt& modulus) const {
    BigInt result;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(nullptr != BN_mod_sqrt(result.bn_ptr, bn_ptr, modulus.bn_ptr, bn_ctx[thread_num]));
    return result;
}

// Computes -a mod m.
// Causes a check failure if the operation fails.
BigInt BigInt::ModNegate(const BigInt& modulus) const {
    if (IsZero()) {
        return *this;
    }
    return modulus - Mod(modulus);
}

// Returns a BigInt whose value is (*this >> n).
BigInt BigInt::Lshift(int n) const {
    BigInt result;
    CRYPTO_CHECK(1 == BN_lshift(result.bn_ptr, this->bn_ptr, n));
    return result;
}

// Returns a BigInt whose value is (*this << n).
// Causes a check failure if the operation fails.
BigInt BigInt::Rshift(int n) const {
    BigInt result;
    CRYPTO_CHECK(1 == BN_rshift(result.bn_ptr, this->bn_ptr, n));
    return result;
}

// Computes the greatest common divisor of *this and val.
// Causes a check failure if the operation fails.
BigInt BigInt::GCD(const BigInt& other) const {
    BigInt result;
    int thread_num = omp_get_thread_num();
    CRYPTO_CHECK(1 == BN_gcd(result.bn_ptr, this->bn_ptr, other.bn_ptr, bn_ctx[thread_num]));
    return result;
}


// Returns False if the number is composite
// True if it is prime with an error probability of 1e-40, which gives at least 128 bit security.
bool BigInt::IsPrime(double prime_error_probability) const {
    int rounds = static_cast<int>(ceil(-log(prime_error_probability) / log(4)));
    int thread_num = omp_get_thread_num();
    return (1 == BN_is_prime_ex(this->bn_ptr, rounds, bn_ctx[thread_num], nullptr));
}

bool BigInt::IsSafePrime(double prime_error_probability = 1e-40) const {
    return IsPrime(prime_error_probability) && ((*this - bn_1) / bn_2).IsPrime(prime_error_probability);
}


void BigInt::Print() const
{
    char *bn_str;
    bn_str = BN_bn2hex(this->bn_ptr);  
    // switch(mode){
    //     case 16: bn_str = BN_bn2hex(this->bn_ptr); break; 
    //     case 10: bn_str = BN_bn2dec(this->bn_ptr); break;
    // }
    std::cout << bn_str << std::endl;
    OPENSSL_free(bn_str);
}

void BigInt::Print(std::string note) const
{
    std::cout << note << " = "; 
    this->Print();
}

void BigInt::PrintInDec(std::string note) const
{
    std::cout << note << " = "; 
    char *bn_str;
    bn_str = BN_bn2dec(this->bn_ptr);  

    std::cout << bn_str << std::endl;
    OPENSSL_free(bn_str);
}

void BigInt::PrintInDec() const
{ 
    char *bn_str;
    bn_str = BN_bn2dec(this->bn_ptr);  

    std::cout << bn_str;
    OPENSSL_free(bn_str);
}

/* compute the jth bit of a big integer i (count from little endian to big endian) */
int BigInt::GetTheNthBit(size_t j) const
{
    BigInt a = *this;  
    a = a >> j;
    a = a.GetLastNBits(1);  

    int result; 
    if (a.IsOne()) return 1; 
    else return 0; 
}












//generates a cryptographically strong pseudo-random number rnd in the range 0 <= rnd < range.
BigInt GenRandomBigIntLessThan(const BIGNUM* max) {
    BigInt result;
    CRYPTO_CHECK(1 == BN_rand_range(result.bn_ptr, max));
    // BN_rand_range(result.bn_ptr, max);
    // BN_priv_rand_range(result.bn_ptr, max.bn_ptr);
    return result;
}

BigInt GenRandomBigIntLessThan(const BigInt& max) {
    BigInt result;
    CRYPTO_CHECK(1 == BN_rand_range(result.bn_ptr, max.bn_ptr));
    // BN_priv_rand_range(result.bn_ptr, max.bn_ptr);
    return result;
}

// Generates a cryptographically strong pseudo-random in the range [start, end).
BigInt GenRandomBigIntBetween(const BigInt& start, const BigInt& end) {
    if (start > end) {
        std::cerr << "provided range is invalid" << std::endl; 
    }
    return GenRandomBigIntLessThan(end - start) + start;
}

// Generates a cryptographically strong pseudo-random bytes of the specified length.
std::string GenRandomBytes(int num_bytes) {
    if (num_bytes < 0){
        std::cerr << "num_bytes must be nonnegative, provided value was" << num_bytes << "."<<std::endl;
    } 
    std::unique_ptr<unsigned char[]> bytes(new unsigned char[num_bytes]);
    CRYPTO_CHECK(1 == RAND_bytes(bytes.get(), num_bytes));
    return std::string(reinterpret_cast<char*>(bytes.get()), num_bytes);
}

// Returns a BigNum that is relatively prime to the num and less than the num.
BigInt GenCoPrimeLessThan(const BigInt& num) {
    BigInt rand_num = GenRandomBigIntLessThan(num);
    while (rand_num.GCD(num) > bn_1) {
        rand_num = GenRandomBigIntLessThan(num);
    }
    return rand_num;
}

// Creates a safe prime BigNum with the given bit-length.
BigInt GenSafePrime(int prime_length) {
    BigInt result;
    CRYPTO_CHECK(1 == BN_generate_prime_ex(result.bn_ptr, prime_length, 1, nullptr, nullptr, nullptr));
    return result;
}

// Creates a prime BigNum with the given bit-length.
// Note: In many cases, we need to use a safe prime for cryptographic security to hold. 
// In this case, we should use GenerateSafePrime.
BigInt GenPrime(int prime_length) {
    BigInt result;
    CRYPTO_CHECK(1 == BN_generate_prime_ex(result.bn_ptr, prime_length, 0, nullptr, nullptr, nullptr));
    return result;
}



std::ofstream &operator<<(std::ofstream &fout, const BigInt& a)
{ 
    unsigned char buffer[BN_BYTE_LEN];
    BN_bn2binpad(a.bn_ptr, buffer, BN_BYTE_LEN);
    fout.write(reinterpret_cast<char *>(buffer), BN_BYTE_LEN);   // write to output file
    return fout;            
}
 
std::ifstream &operator>>(std::ifstream &fin, BigInt &a)
{ 
    char buffer[BN_BYTE_LEN];
    fin.read(buffer, BN_BYTE_LEN);
    BN_bin2bn(reinterpret_cast<unsigned char *>(buffer), BN_BYTE_LEN, a.bn_ptr); // read from input file
    return fin;            
}

std::ofstream &operator<<(std::ofstream &fout, const std::vector<BigInt>& vec_a)
{ 
    for(auto i = 0; i < vec_a.size(); i++){
        fout << vec_a[i]; 
    }
    return fout;            
}
 
std::ifstream &operator>>(std::ifstream &fin, std::vector<BigInt>& vec_a)
{ 
    for(auto i = 0; i < vec_a.size(); i++){
        fin >> vec_a[i]; 
    }
    return fin;             
}

// print a Big Number vector
void PrintBigIntVector(std::vector<BigInt> &vec_a, std::string note)
{
    for (auto i = 0; i < vec_a.size(); i++)
    {
        std::cout << note <<"[" << i << "]="; 
        vec_a[i].Print(); 
    }
}

/* a[i] = (a[i]+b[i]) mod modulus */
std::vector<BigInt> BigIntVectorModAdd(std::vector<BigInt> &vec_a, std::vector<BigInt> &vec_b, const BigInt &modulus)
{
    if (vec_a.size() != vec_b.size()) {
        std::cerr << "vector size does not match!" << std::endl;
        exit(EXIT_FAILURE); 
    }
    size_t LEN = vec_a.size(); 
    std::vector<BigInt> vec_result(LEN);
    
    for (auto i = 0; i < vec_a.size(); i++) {
        vec_result[i] = (vec_a[i] + vec_b[i]) % modulus;  
    }
    return vec_result; 
}

/* a[i] = (a[i]-b[i]) mod modulus */ 
std::vector<BigInt> BigIntVectorModSub(std::vector<BigInt> &vec_a, std::vector<BigInt> &vec_b, const BigInt& modulus)
{
    if (vec_a.size() != vec_b.size()) {
        std::cout << "vector size does not match!" << std::endl;
        exit(EXIT_FAILURE); 
    }
    size_t LEN = vec_a.size(); 
    std::vector<BigInt> vec_result(LEN);

    for (auto i = 0; i < LEN; i++) {
        vec_result[i] = (vec_a[i] - vec_b[i]) % modulus;
    } 
    return vec_result; 
}

/* c[i] = a[i]*b[i] mod modulus */ 
std::vector<BigInt> BigIntVectorModProduct(std::vector<BigInt> &vec_a, std::vector<BigInt> &vec_b, const BigInt& modulus)
{
    if (vec_a.size() != vec_b.size()) {
        std::cerr << "vector size does not match!" << std::endl;
        exit(EXIT_FAILURE); 
    }
    size_t LEN = vec_a.size(); 
    std::vector<BigInt> vec_result(LEN);

    for (auto i = 0; i < vec_a.size(); i++) {
        vec_result[i] = (vec_a[i] * vec_b[i]) % modulus; // product = (vec_a[i]*vec_b[i]) mod modulus
    }
    return vec_result; 
}


/* c[i] = a[i]*b[i] */ 
std::vector<BigInt> BigIntVectorProduct(std::vector<BigInt> &vec_a, std::vector<BigInt> &vec_b)
{
    if (vec_a.size() != vec_b.size()) {
        std::cerr << "vector size does not match!" << std::endl;
        exit(EXIT_FAILURE); 
    }
    size_t LEN = vec_a.size(); 

    std::vector<BigInt> vec_result(LEN);
    for (auto i = 0; i < vec_a.size(); i++) {
        vec_result[i] = vec_a[i] * vec_b[i]; 
    }
    return vec_result; 
}


/* compute the inverse of a[i] */ 
std::vector<BigInt> BigIntVectorModInverse(std::vector<BigInt> &vec_a, const BigInt& modulus)
{
    size_t LEN = vec_a.size(); 
    std::vector<BigInt> vec_result(LEN);

    for (auto i = 0; i < vec_a.size(); i++) {
        vec_result[i] = vec_a[i].ModInverse(modulus); 

    }
    return vec_result;
}


/* result[i] = c * a[i] */  
std::vector<BigInt> BigIntVectorModScalar(std::vector<BigInt> &vec_a, BigInt &c, const BigInt& modulus)
{
    size_t LEN = vec_a.size();
    std::vector<BigInt> vec_result(LEN);

    for (auto i = 0; i < LEN; i++) {
        vec_result[i] = (vec_a[i] * c) % modulus;
    } 
    return vec_result; 
}

/* result[i] = c * a[i] */  
std::vector<BigInt> BigIntVectorScalar(std::vector<BigInt> &vec_a, BigInt &c)
{
    size_t LEN = vec_a.size();
    std::vector<BigInt> vec_result(LEN); 

    for (auto i = 0; i < vec_a.size(); i++) {
        vec_result[i] = vec_a[i] * c;
    } 
    return vec_result;
}

/* result[i] = -result[i] */  
std::vector<BigInt> BigIntVectorModNegate(std::vector<BigInt> &vec_a, BigInt &modulus)
{
    size_t LEN = vec_a.size();
    std::vector<BigInt> vec_result(LEN); 

    for (auto i = 0; i < vec_result.size(); i++) {
        vec_result[i] = vec_a[i].ModNegate(modulus);
    }
    return vec_result; 
}


/* sum_i^n a[i]*b[i] */
BigInt BigIntVectorModInnerProduct(std::vector<BigInt> &vec_a, std::vector<BigInt> &vec_b, const BigInt& modulus)
{
    BigInt result = bn_0; 

    if (vec_a.size() != vec_b.size()){
        std::cerr << "vector size does not match!" << std::endl;
        exit(EXIT_FAILURE); 
    } 

    for (auto i = 0; i < vec_a.size(); i++) {
        result += vec_a[i] * vec_b[i]; // product = (vec_a[i]*vec_b[i]) mod modulus
    }
    result = result % modulus;
    return result; 
}

/* sum_i^n a[i]*b[i] */
BigInt BigIntVectorInnerProduct(std::vector<BigInt> &vec_a, std::vector<BigInt> &vec_b, const BigInt& modulus)
{
    BigInt result = bn_0; 

    if (vec_a.size() != vec_b.size()){
        std::cerr << "vector size does not match!" << std::endl;
        exit(EXIT_FAILURE); 
    } 

    for (auto i = 0; i < vec_a.size(); i++) {
        result += vec_a[i] * vec_b[i]; 
    }
    return result % modulus; 
}


/* generate a vector of random EC points */  
std::vector<BigInt> GenRandomBigIntVectorLessThan(size_t LEN, const BigInt &modulus)
{
    std::vector<BigInt> vec_result(LEN);
    
    #pragma omp parallel for num_threads(NUMBER_OF_THREADS)
    for(auto i = 0; i < LEN; i++){ 
        vec_result[i] = GenRandomBigIntLessThan(modulus); 
    }
    return vec_result; 
}