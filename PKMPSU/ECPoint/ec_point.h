/*
** Modified from the following two projects
** 1. https://github.com/emp-toolkit/
** 2. https://github.com/google/private-join-and-compute
*/
// #ifndef KUNLUN_EC_POINT_HPP_
// #define KUNLUN_EC_POINT_HPP_
#pragma once

#include "ec_group.h"
//#include "aes.hpp"
#include "murmurhash2.h"
#include "routines.h"

class BigInt;

// C++ Wrapper class for openssl EC_POINT.
class ECPoint {
public:
    EC_POINT* point_ptr; 
    
    // constructor functions
    
    ECPoint(); 
    ECPoint(const ECPoint& other);
    ECPoint(const EC_POINT* &other);
    
    // Creates an ECPoint object with given x, y affine coordinates.
    ECPoint(const BigInt& x, const BigInt& y);

    /* 
    ** Re-initialization function
    ** this function is somewhat dirty, only used as a ad-hoc bypass to initialize 
    ** global ECPoint object before group is created
    */

    void ReInitialize(); 
    // Returns an ECPoint that is a copy of this.
    void Clone(const ECPoint& other) const;

    void SetInfinity(); 

    // EC point group operations
    
    // Returns an ECPoint whose value is (this * scalar).
    ECPoint Mul(const BigInt& scalar) const;

    // Returns an ECPoint whose value is (this + other).
    ECPoint Add(const ECPoint& other) const;

    // Returns an ECPoint whose value is (- this), the additive inverse of this.
    ECPoint Invert() const;

    // Returns an ECPoint whose value is (this - other).
    ECPoint Sub(const ECPoint& other) const; 


    // attribute check operations

    // Returns "true" if the value of this ECPoint is the point-at-infinity.
    // (The point-at-infinity is the additive unit in the EC group).
    bool IsOnCurve() const; 
    bool IsValid() const;
    bool IsAtInfinity() const;  

    // Returns true if this equals point, false otherwise.
    bool CompareTo(const ECPoint& point) const;


    inline ECPoint& operator=(const ECPoint& other) { EC_POINT_copy(this->point_ptr, other.point_ptr); return *this; }

    inline bool operator==(const ECPoint& other) const{ return this->CompareTo(other); }

    inline bool operator!=(const ECPoint& other) const{ return !this->CompareTo(other);}

    inline ECPoint operator-() const { return this->Invert(); }

    inline ECPoint operator+(const ECPoint& other) const { return this->Add(other); }

    inline ECPoint operator*(const BigInt& scalar) const { return this->Mul(scalar); }

    inline ECPoint operator-(const ECPoint& other) const { return this->Sub(other); }

    inline ECPoint& operator+=(const ECPoint& other) { return *this = *this + other; }

    inline ECPoint& operator*=(const BigInt& scalar) { return *this = *this * scalar; }

    inline ECPoint& operator-=(const ECPoint& other) { return *this = *this - other; }

    void Print() const;

    void Print(std::string note) const;  

    std::string ToByteString() const;

    std::string ToHexString() const;

    size_t ToUint64() const; 
    size_t FastToUint64() const; 

    friend std::ofstream &operator<<(std::ofstream &fout, const ECPoint &A); 
 
    friend std::ifstream &operator>>(std::ifstream &fin, ECPoint &A);
};
  

std::ofstream &operator<<(std::ofstream &fout, const ECPoint &A);
 
std::ifstream &operator>>(std::ifstream &fin, ECPoint &A);



/* 
 *  non-class functions
*/


std::ofstream &operator<<(std::ofstream &fout, const std::vector<ECPoint> &vec_A);
 
std::ifstream &operator>>(std::ifstream &fin, std::vector<ECPoint> &vec_A);

// Creates an ECPoint object with the given x, y affine coordinates.
ECPoint CreateECPoint(const BigInt& x, const BigInt& y);

ECPoint GenRandomGenerator();

// Creates an ECPoint which is the identity.
ECPoint GetPointAtInfinity();


bool IsSquare(const BigInt& q);

// ecpoint vector operations

// mul exp operations
ECPoint ECPointVectorMul(const std::vector<ECPoint> &vec_A, std::vector<BigInt> &vec_a);

// mul exp operations
ECPoint ECPointVectorMul(const std::vector<ECPoint> &vec_A, std::vector<BigInt> &vec_a, size_t start_index, size_t end_index);


/* g[i] = g[i]+h[i] */ 
std::vector<ECPoint> ECPointVectorAdd(std::vector<ECPoint> &vec_A, std::vector<ECPoint> &vec_B);


/* vec_result[i] = vec_A[i] * a */ 
inline std::vector<ECPoint> ECPointVectorScalar(std::vector<ECPoint> &vec_A, BigInt &a)
{
    size_t LEN = vec_A.size();
    std::vector<ECPoint> vec_result(LEN);  

    #pragma omp parallel for num_threads(NUMBER_OF_THREADS)
    for (auto i = 0; i < LEN; i++) {
        vec_result[i] = vec_A[i] * a;  
    }
    return vec_result;  
}



/* result[i] = A[i]*a[i] */ 
inline std::vector<ECPoint> ECPointVectorProduct(const std::vector<ECPoint> &vec_A, std::vector<BigInt> &vec_a)
{
    if (vec_A.size() != vec_a.size()) {
        std::cerr << "vector size does not match!" << std::endl;
        exit(EXIT_FAILURE); 
    } 

    size_t LEN = vec_A.size(); 
    std::vector<ECPoint> vec_result(LEN);
    
    #pragma omp parallel for num_threads(NUMBER_OF_THREADS)
    for (auto i = 0; i < LEN; i++) {
        vec_result[i] = vec_A[i] * vec_a[i];  
    } 
    return vec_result;  
}


/* generate a vector of random EC points */  
std::vector<ECPoint> GenRandomECPointVector(size_t LEN);

ECPoint GenRandomECPoint();

// print an EC Point vector
void PrintECPointVector(const std::vector<ECPoint> &vec_A, std::string note);




// #endif  // KUNLUN_EC_POINT_HPP_


