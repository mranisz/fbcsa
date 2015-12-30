#ifndef FBCSA_H_
#define FBCSA_H_

#include "shared/common.h"
#include "shared/hash.h"
#include <cstdio>

using namespace std;

namespace fbcsa {
    
/*FBCSA*/
    
class FBCSA : public Index {
protected:
	unsigned int *arr1;
        unsigned int *alignedArr1;
        unsigned int arr1Len;
	unsigned int *arr2;
        unsigned int *alignedArr2;
        unsigned int arr2Len;
        unsigned char *text;
        unsigned char *alignedText;
        unsigned int textLen;
	HT *ht = NULL;

        unsigned int bs;
        unsigned int ss;

        unsigned int (FBCSA::*getSAValue)(unsigned int i) = NULL;
	unsigned int (FBCSA::*countOperation)(unsigned char *, unsigned int) = NULL;
        void (FBCSA::*getBoundariesOperation)(unsigned char *, unsigned int &, unsigned int &) = NULL;

	void freeMemory();
	void initialize();
        void setBs(unsigned int bs);
        void setSs(unsigned int ss);
	void setFunctions();
        string getMostCommonCharsInBlock(unsigned char *block);
        unsigned int getArr2LenForBlock(unsigned char *bwtStart, unsigned int *saStart, unsigned int *saInv);
        void buildArraysForBlock(unsigned char *bwtStart, unsigned int *saStart, unsigned int *saInv, unsigned int &arr1Index, unsigned int &arr2Index);
        void buildIndex(unsigned int *sa, unsigned int saLen);
        unsigned int getSAValue_32(unsigned int i);
        unsigned int getSAValue_64(unsigned int i);
        unsigned int getSAValue_general(unsigned int i);
        void binarySearchAStrcmp(unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end);
        void binarySearchStrncmp(unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end);
        unsigned int count_std(unsigned char *pattern, unsigned int patternLength);
        unsigned int count_hash(unsigned char *pattern, unsigned int patternLength);
        void getStandardHTBoundaries(unsigned char *pattern, unsigned int &leftBoundary, unsigned int &rightBoundary);
        void getDenseHTBoundaries(unsigned char *pattern, unsigned int &leftBoundary, unsigned int &rightBoundary);
        void getBoundaries(unsigned char *pattern, unsigned int &leftBoundary, unsigned int &rightBoundary);
        
public:
	FBCSA() {
		this->initialize();
                this->setBs(32);
                this->setSs(5);
		this->setFunctions();
	}

	FBCSA(unsigned int bs, unsigned int ss) {
		this->initialize();
                this->setBs(bs);
                this->setSs(ss);
		this->setFunctions();
	}

	FBCSA(unsigned int bs, unsigned int ss, HT::HTType hTType, unsigned int k, double loadFactor) {
		this->initialize();
                this->setBs(bs);
                this->setSs(ss);
                this->ht = new HT(hTType, k, loadFactor);
		this->setFunctions();
	}

	~FBCSA() {
		this->freeMemory();
                if (this->ht != NULL) delete this->ht;
	}

	void build(unsigned char *text, unsigned int textLen);
	void save(const char *fileName);
	void load(const char *fileName);
	void free();
	unsigned int getIndexSize();
	unsigned int getTextSize();

	unsigned int count(unsigned char *pattern, unsigned int patternLen);
	unsigned int *locate(unsigned char *pattern, unsigned int patternLen);
        
        const static unsigned int encodedCharsNum;
};

/*FBCSALut2*/

class FBCSALut2 : public FBCSA {
private:
        alignas(128) unsigned int lut2[256][256][2];
        
        unsigned int (FBCSALut2::*countOperation)(unsigned char *, unsigned int) = NULL;
        
        void setFunctions();
        
public:
	FBCSALut2() {
		this->initialize();
                this->setBs(32);
                this->setSs(5);
		this->setFunctions();
	}

	FBCSALut2(unsigned int bs, unsigned int ss) {
		this->initialize();
                this->setBs(bs);
                this->setSs(ss);
		this->setFunctions();
	}
        
        void build(unsigned char *text, unsigned int textLen);
        void save(const char *fileName);
	void load(const char *fileName);
        unsigned int getIndexSize();
        
        unsigned int count(unsigned char *pattern, unsigned int patternLen);
	unsigned int *locate(unsigned char *pattern, unsigned int patternLen);
};

}

#endif /* FBCSA_H_ */
