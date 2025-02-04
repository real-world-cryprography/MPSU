#pragma once

#include "global.h"

inline size_t BN_BYTE_LEN;    // the byte length of bigint
inline size_t INT_BYTE_LEN; 
//inline size_t FIELD_BYTE_LEN;  // each scalar field element is 256 bit 

inline BN_CTX *bn_ctx[NUMBER_OF_THREADS]; // define ctx for ecc operations


void BN_Initialize();

void BN_Finalize();


// wrapper class for openssl BIGNUM

class BigInt{
public:
    BIGNUM* bn_ptr;
    
    // constructor functions
    BigInt();
    BigInt(const BigInt& other);
    BigInt(const BIGNUM *other);
    BigInt(size_t number);

    // destuctor function
    ~BigInt();

    // arithmetic operations 
    
    // Returns a BigInt whose value is (- *this). Causes a check failure if the operation fails.
    BigInt Negate() const;

    // Returns a BigInt whose value is (*this + other). Causes a check failure if the operation fails.
    BigInt Add(const BigInt& other) const;

    // Returns a BigInt whose value is (*this - other). Causes a check failure if the operation fails.
    BigInt Sub(const BigInt& other) const;

    // Returns a BigInt whose value is (*this * other). Causes a check failure if the operation fails.
    BigInt Mul(const BigInt& other) const;

    // Returns a BigInt whose value is (*this / other).
    // Causes a check failure if the remainder != 0 or if the operation fails.
    BigInt Div(const BigInt& other) const;

    // Returns a BigInt whose value is *this / val, rounding towards zero.
    // Causes a check failure if the remainder != 0 or if the operation fails.
    BigInt DivAndTruncate(const BigInt& other) const;

    // Returns a BigInt whose value is (*this ^ exponent).
    // Causes a check failure if the operation fails.
    BigInt Exp(const BigInt& exponent) const;

    BigInt Square() const;

    // Returns a BigInt whose value is (*this mod m).
    BigInt Mod(const BigInt& modulus) const;

    // Returns a BigInt whose value is (*this + other mod m).
    // Causes a check failure if the operation fails.
    BigInt ModAdd(const BigInt& other, const BigInt& modulus) const;

    // Returns a BigInt whose value is (*this - other mod m).
    // Causes a check failure if the operation fails.
    BigInt ModSub(const BigInt& other, const BigInt& modulus) const;

    // Returns a BigInt whose value is (*this * other mod m).
    // For efficiency, use Montgomery multiplication module if this is done multiple times with the same modulus.
    // Causes a check failure if the operation fails.
    BigInt ModMul(const BigInt& other, const BigInt& modulus) const;

    // Returns a BigInt whose value is (*this ^ exponent mod m).
    // Causes a check failure if the operation fails.
    BigInt ModExp(const BigInt& exponent, const BigInt& modulus) const;

    // Return a BigInt whose value is (*this ^ 2 mod m).
    // Causes a check failure if the operation fails.
    BigInt ModSquare(const BigInt& modulus) const;

    // Returns a BigInt whose value is (*this ^ -1 mod m).
    // Causes a check failure if the operation fails.
    BigInt ModInverse(const BigInt& modulus) const;

    // Returns r such that r^2 == *this mod p.
    // Causes a check failure if the operation fails.
    BigInt ModSquareRoot(const BigInt& modulus) const;

    // Computes -a mod m.
    // Causes a check failure if the operation fails.
    BigInt ModNegate(const BigInt& modulus) const;

    // Computes the greatest common divisor of *this and other.
    // Causes a check failure if the operation fails.
    BigInt GCD(const BigInt& other) const;

    
    // logic operations

    // Compares this BigInt with the specified BigInt.
    // Returns -1 if *this < other, 0 if *this == other and 1 if *this > other.
    int CompareTo(const BigInt& other) const;

    // Returns a BigInt whose value is (*this >> n).
    BigInt Lshift(int n) const; 

    // Returns a BigInt whose value is (*this << n).
    BigInt Rshift(int n) const;

    // operator overload

    inline BigInt& operator=(const BigInt& other) { BN_copy(this->bn_ptr, other.bn_ptr); return *this; }

    inline BigInt operator-() const { return this->Negate(); }

    inline BigInt operator+(const BigInt& other) const { return this->Add(other); }

    inline BigInt operator*(const BigInt& other) const { return this->Mul(other); }

    inline BigInt operator-(const BigInt& other) const { return this->Sub(other); }

    inline BigInt operator/(const BigInt& other) const { return this->Div(other); }

    inline BigInt& operator+=(const BigInt& other) { return *this = *this + other; }

    inline BigInt& operator*=(const BigInt& other) { return *this = *this * other; }

    inline BigInt& operator-=(const BigInt& other) { return *this = *this - other; }

    inline BigInt& operator/=(const BigInt& other) { return *this = *this / other; }

    inline bool operator==(const BigInt& other) const { return 0 == this->CompareTo(other); }

    inline bool operator!=(const BigInt& other) const { return !(*this == other); }

    inline bool operator<(const BigInt& other) const { return -1 == this->CompareTo(other); }

    inline bool operator>(const BigInt& other) const { return 1 == this->CompareTo(other); }

    inline bool operator<=(const BigInt& other) const { return this->CompareTo(other) <= 0; }

    inline bool operator>=(const BigInt& other) const { return this->CompareTo(other) >= 0; }

    inline BigInt operator%(const BigInt& modulus) const { return this->Mod(modulus); }

    inline BigInt operator>>(int n) { return this->Rshift(n); }

    inline BigInt operator<<(int n) { return this->Lshift(n); }

    inline BigInt& operator%=(const BigInt& other) { return *this = *this % other; }

    inline BigInt& operator>>=(int n) { return *this = *this >> n; }

    inline BigInt& operator<<=(int n) { return *this = *this << n; }

    // serialization and deserialization 

    void FromByteString(const std::string& str); 
    void FromByteString(const unsigned char* buffer, size_t LEN); 
  
    uint64_t ToUint64() const; 
    void ToByteString(unsigned char* buffer, size_t LEN) const;
    std::string ToByteString() const; 
    std::string ToHexString() const;

    void FromByteVector(const std::vector<uint8_t>& vec_A);
    std::vector<uint8_t> ToByteVector(size_t LEN) const; 

    friend std::ofstream &operator<<(std::ofstream &fout, const BigInt &A); 
    friend std::ifstream &operator>>(std::ifstream &fin, BigInt &A); 

    // attribute test routines

    inline int GetTheNthBit(size_t j) const;

    // returns 0 on error (if r is already shorter than n bits)
    // return value in that case should be the original value so there is no need to have error checking here.
    inline BigInt GetLastNBits(int n) const {
        BigInt result = *this;
        BN_mask_bits(result.bn_ptr, n);
        return result;
    }

    // returns the bit length of this BigInt.
    inline size_t GetBitLength() const { return BN_num_bits(this->bn_ptr); }
    inline size_t GetByteLength() const { return BN_num_bytes(this->bn_ptr); }

    inline bool IsBitSet(int n) const { return BN_is_bit_set(this->bn_ptr, n); }

    inline bool IsZero() const { return BN_is_zero(this->bn_ptr); }

    inline bool IsOne() const { return BN_is_one(this->bn_ptr); }

    inline bool IsNonNegative() const { 
        if (BN_is_negative(this->bn_ptr) == 1) return false; 
        else return true;
    }

    bool IsPrime(double prime_error_probability) const;

    bool IsSafePrime(double prime_error_probability) const;

    // print BigInt object, mode = {10, 16}
    void Print() const; 
    
    void Print(std::string note) const; 

    void PrintInDec(std::string note) const; 

    void PrintInDec() const; 
};

// global bigint objects
inline const BigInt bn_0(uint64_t(0)); 
inline const BigInt bn_1(uint64_t(1)); 
inline const BigInt bn_2(uint64_t(2)); 
inline const BigInt bn_3(uint64_t(3));  



//generates a cryptographically strong pseudo-random number rnd in the range 0 <= rnd < range.
BigInt GenRandomBigIntLessThan(const BIGNUM* max);

BigInt GenRandomBigIntLessThan(const BigInt& max);

// Generates a cryptographically strong pseudo-random in the range [start, end).
BigInt GenRandomBigIntBetween(const BigInt& start, const BigInt& end);

// Generates a cryptographically strong pseudo-random bytes of the specified length.
std::string GenRandomBytes(int num_bytes);

// Returns a BigNum that is relatively prime to the num and less than the num.
BigInt GenCoPrimeLessThan(const BigInt& num);

// Creates a safe prime BigNum with the given bit-length.
BigInt GenSafePrime(int prime_length);

// Creates a prime BigNum with the given bit-length.
// Note: In many cases, we need to use a safe prime for cryptographic security to hold. 
// In this case, we should use GenerateSafePrime.
BigInt GenPrime(int prime_length);



std::ofstream &operator<<(std::ofstream &fout, const BigInt& a);
 
std::ifstream &operator>>(std::ifstream &fin, BigInt &a);

std::ofstream &operator<<(std::ofstream &fout, const std::vector<BigInt>& vec_a);
 
std::ifstream &operator>>(std::ifstream &fin, std::vector<BigInt>& vec_a);

// print a Big Number vector
void PrintBigIntVector(std::vector<BigInt> &vec_a, std::string note);

/* a[i] = (a[i]+b[i]) mod modulus */
std::vector<BigInt> BigIntVectorModAdd(std::vector<BigInt> &vec_a, std::vector<BigInt> &vec_b, const BigInt &modulus);

/* a[i] = (a[i]-b[i]) mod modulus */ 
std::vector<BigInt> BigIntVectorModSub(std::vector<BigInt> &vec_a, std::vector<BigInt> &vec_b, const BigInt& modulus);
/* c[i] = a[i]*b[i] mod modulus */ 
std::vector<BigInt> BigIntVectorModProduct(std::vector<BigInt> &vec_a, std::vector<BigInt> &vec_b, const BigInt& modulus);

/* c[i] = a[i]*b[i] */ 
std::vector<BigInt> BigIntVectorProduct(std::vector<BigInt> &vec_a, std::vector<BigInt> &vec_b);


/* compute the inverse of a[i] */ 
std::vector<BigInt> BigIntVectorModInverse(std::vector<BigInt> &vec_a, const BigInt& modulus);


/* result[i] = c * a[i] */  
std::vector<BigInt> BigIntVectorModScalar(std::vector<BigInt> &vec_a, BigInt &c, const BigInt& modulus);

/* result[i] = c * a[i] */  
std::vector<BigInt> BigIntVectorScalar(std::vector<BigInt> &vec_a, BigInt &c);

/* result[i] = -result[i] */  
std::vector<BigInt> BigIntVectorModNegate(std::vector<BigInt> &vec_a, BigInt &modulus);


/* sum_i^n a[i]*b[i] */
BigInt BigIntVectorModInnerProduct(std::vector<BigInt> &vec_a, std::vector<BigInt> &vec_b, const BigInt& modulus);

/* sum_i^n a[i]*b[i] */
BigInt BigIntVectorInnerProduct(std::vector<BigInt> &vec_a, std::vector<BigInt> &vec_b, const BigInt& modulus);

/* generate a vector of random EC points */  
std::vector<BigInt> GenRandomBigIntVectorLessThan(size_t LEN, const BigInt &modulus);

