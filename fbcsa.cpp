#include <iostream>
#include <stdlib.h>
#include <math.h>
#include "libs/asmlib.h"
#include "fbcsa.h"

using namespace std;

namespace fbcsa {

/*FBCSA*/
    
const unsigned int FBCSA::encodedCharsNum = 4;

void FBCSA::setBs(unsigned int bs) {
	if (bs == 0 || bs % 32 != 0) {
		cout << "Error: not valid bs value (bs parameter must be multiple of 32)" << endl;
		exit(1);
	}
	this->bs = bs;
}

void FBCSA::setSs(unsigned int ss) {
	if (ss == 0) {
		cout << "Error: not valid ss value" << endl;
		exit(1);
	}
	this->ss = ss;
}

void FBCSA::setFunctions() {
        switch (this->bs) {
        case 32:
               this->getSAValue = &FBCSA::getSAValue_32;
               this->getSAValuesSeq = &FBCSA::getSAValuesSeq_32;
               break;
        case 64:
               this->getSAValue = &FBCSA::getSAValue_64;
               this->getSAValuesSeq = &FBCSA::getSAValuesSeq_64;
               break;
        default:
               this->getSAValue = &FBCSA::getSAValue_general;
               this->getSAValuesSeq = &FBCSA::getSAValuesSeq_general;
               break; 
        }
	if (this->ht != NULL) {
                this->countOperation = &FBCSA::count_hash;
                switch(this->ht->type) {
                case HT::STANDARD:
                    this->getBoundariesOperation = &FBCSA::getStandardHTBoundaries;
                    break;
                case HT::DENSE:
                    this->getBoundariesOperation = &FBCSA::getDenseHTBoundaries;
                    break;
                }
	} else {
                this->countOperation = &FBCSA::count_std;
	}
}

void FBCSA::free() {
	this->freeMemory();
	this->initialize();
}

void FBCSA::initialize() {
	this->arr1 = NULL;
	this->alignedArr1 = NULL;
	this->arr1Len = 0;
        this->arr2 = NULL;
	this->alignedArr2 = NULL;
	this->arr2Len = 0;
        this->text = NULL;
	this->alignedText = NULL;
        this->textLen = 0;
}

void FBCSA::freeMemory() {
	if (this->arr1 != NULL) delete[] this->arr1;
        if (this->arr2 != NULL) delete[] this->arr2;
        if (this->text != NULL) delete[] this->text;
	if (this->ht != NULL) this->ht->free();
}

string FBCSA::getMostCommonCharsInBlock(unsigned char *block) {
	unsigned int freq[256] = { 0 };
	for (unsigned int i = 0; i < this->bs; ++i) if (block[i] != '\0') freq[block[i]]++;
	string mostCommonCharsInBlock = "";
	unsigned char selectedChar = 0;
	for (unsigned int i = 0; i < FBCSA::encodedCharsNum - 1; ++i) {
		unsigned int maxFreq = 0;
		for (unsigned int j = 0; j < 256; ++j) {
			if (freq[j] > maxFreq) {
				maxFreq = freq[j];
				selectedChar = j;
			}
		}
		if (maxFreq == 0) break;
		freq[selectedChar] = 0;
		mostCommonCharsInBlock += selectedChar;
	}
	return mostCommonCharsInBlock;
}

unsigned int FBCSA::getArr2LenForBlock(unsigned char *bwtStart, unsigned int *saStart, unsigned int *saInv) {
        unsigned int arr2Length = FBCSA::encodedCharsNum - 1;
        string mostCommonCharsInBlock = this->getMostCommonCharsInBlock(bwtStart);
	
        for (unsigned int j = 0; j < (this->bs / 32); ++j) {
		for (unsigned int i = 0; i < 32; ++i) {
			unsigned int g = j * 32 + i;
			if (mostCommonCharsInBlock.find(bwtStart[g]) == string::npos || (mostCommonCharsInBlock.find(bwtStart[g]) != string::npos && (saStart[g] % this->ss) == 0)) ++arr2Length;
		}
	}
        return arr2Length;
}

void FBCSA::buildArraysForBlock(unsigned char *bwtStart, unsigned int *saStart, unsigned int *saInv, unsigned int &arr1Index, unsigned int &arr2Index) {
	string mostCommonCharsInBlock = this->getMostCommonCharsInBlock(bwtStart);
	unsigned int *resultArr1Vals = new unsigned int[this->bs / 16 + this->bs / 32 + 1];
	unsigned int startArr2Len = arr2Index;
	for (unsigned int i = 0; i < (this->bs / 16); ++i) {
		resultArr1Vals[i] = 0;
		for (unsigned int j = 0; j < 16; ++j) {
			if (mostCommonCharsInBlock.find(bwtStart[16 * i + j]) == string::npos) resultArr1Vals[i] = resultArr1Vals[i] * FBCSA::encodedCharsNum + (FBCSA::encodedCharsNum - 1);
			else resultArr1Vals[i] = resultArr1Vals[i] * FBCSA::encodedCharsNum + mostCommonCharsInBlock.find(bwtStart[16 * i + j]);
		}
	}

	for (unsigned int i = 0; i < mostCommonCharsInBlock.size(); ++i) {
		for (unsigned int j = 0; j < this->bs; ++j) {
			if (bwtStart[j] == (unsigned char)mostCommonCharsInBlock[i]) {
				this->alignedArr2[arr2Index++] = saInv[saStart[j] - 1];
				break;
			}
		}
	}
	for (unsigned int i = mostCommonCharsInBlock.size(); i < FBCSA::encodedCharsNum - 1; ++i) this->alignedArr2[arr2Index++] = 0;
	
        for (unsigned int j = 0; j < (this->bs / 32); ++j) {
		resultArr1Vals[(this->bs / 16) + j] = 0;
		for (unsigned int i = 0; i < 32; ++i) {
			unsigned int g = j * 32 + i;
			if (mostCommonCharsInBlock.find(bwtStart[g]) == string::npos || (mostCommonCharsInBlock.find(bwtStart[g]) != string::npos && (saStart[g] % this->ss) == 0)) {
				resultArr1Vals[(this->bs / 16) + j] = resultArr1Vals[(this->bs / 16) + j] * 2 + 1;
				this->alignedArr2[arr2Index++] = saStart[g];
			} else resultArr1Vals[(this->bs / 16) + j] = resultArr1Vals[(this->bs / 16) + j] * 2;
		}
	}
        
	resultArr1Vals[(this->bs / 16) + (this->bs / 32)] = startArr2Len;
	for (unsigned int i = 0; i < (this->bs / 16) + (this->bs / 32) + 1; ++i) this->alignedArr1[arr1Index++] = resultArr1Vals[i];
	delete[] resultArr1Vals;
}

void FBCSA::buildIndex(unsigned int *sa, unsigned int saLen) {
        unsigned int bwtLen;
        unsigned int addLen = this->bs - ((this->textLen + 1) % this->bs);
        unsigned char *bwt = getBWT(this->alignedText, this->textLen, sa, saLen, bwtLen, addLen, this->verbose);
        
        for (unsigned int i = saLen; i < saLen + addLen; ++i) sa[i] = 0;
	for (unsigned int i = bwtLen; i < bwtLen + addLen; ++i) bwt[i] = bwt[i - 1];
        
        if (this->verbose) cout << "Building inverse SA ... " << flush;
        unsigned int *saInv = new unsigned int[saLen];
	for (unsigned int i = 0; i < saLen; i++) saInv[i] = 0;
	for (unsigned int i = 0; i < saLen; i++) saInv[sa[i]] = i;
        if (this->verbose) cout << "Done" << endl;
        
        if (this->verbose) cout << "Building FBCSA index ... " << flush;
        this->arr1Len = (bwtLen / this->bs + 1) * (this->bs / 16 + this->bs / 32 + 1);
        this->arr1 = new unsigned int[this->arr1Len + 32];
        this->alignedArr1 = this->arr1;
        while ((unsigned long long)this->alignedArr1 % 128) ++this->alignedArr1;
        this->arr2Len = 0;
        for (unsigned int i = 0; i < (bwtLen - this->bs + 1); i += this->bs) this->arr2Len += this->getArr2LenForBlock(bwt + i, sa + i, saInv);
	if (bwtLen % this->bs > 0) this->arr2Len += this->getArr2LenForBlock(bwt + bwtLen - (bwtLen % this->bs), sa + bwtLen - (bwtLen % this->bs), saInv);
	this->arr2 = new unsigned int[this->arr2Len + 32];
        this->alignedArr2 = this->arr2;
        while ((unsigned long long)this->alignedArr2 % 128) ++this->alignedArr2;
        
        unsigned int arr1Index = 0;
        unsigned int arr2Index = 0;
	for (unsigned int i = 0; i < (bwtLen - this->bs + 1); i += this->bs) this->buildArraysForBlock(bwt + i, sa + i, saInv, arr1Index, arr2Index);
	if (bwtLen % this->bs > 0) this->buildArraysForBlock(bwt + bwtLen - (bwtLen % this->bs), sa + bwtLen - (bwtLen % this->bs), saInv, arr1Index, arr2Index);
        if (this->verbose) cout << "Done" << endl;
        
        delete[] saInv;
        delete[] bwt;
}

void FBCSA::build(unsigned char* text, unsigned int textLen) {
	checkNullChar(text, textLen);
	this->free();
        if (this->verbose) cout << "Loading text ... " << flush;
	this->textLen = textLen;
        this->text = new unsigned char [this->textLen + 128 + 1];
        this->alignedText = this->text;
        while ((unsigned long long)this->alignedText % 128) ++this->alignedText;
        for (unsigned int i = 0; i < this->textLen; ++i) this->alignedText[i] = text[i];
        this->alignedText[this->textLen] = '\0';
        if (this->verbose) cout << "Done" << endl;
        
        unsigned int saLen;
        unsigned int *sa = getSA(this->alignedText, this->textLen, saLen, this->bs - ((this->textLen + 1) % this->bs), this->verbose);
        
        this->buildIndex(sa, saLen);
        
        if (this->ht != NULL) {
                if (this->verbose) cout << "Building hash table ... " << flush;
		this->ht->build(this->alignedText, this->textLen, sa, saLen);
		if (this->verbose) cout << "Done" << endl;
	}
        
	delete[] sa;
}

unsigned int FBCSA::getIndexSize() {
	unsigned int size = sizeof(this->bs) + sizeof(this->ss) + sizeof(this->arr1Len) + sizeof(this->arr2Len) + sizeof(this->textLen) + sizeof(FBCSA::encodedCharsNum) + sizeof(this->ht);
        if (this->arr1Len > 0) size += (this->arr1Len + 32) * sizeof(unsigned int);
        if (this->arr2Len > 0) size += (this->arr2Len + 32) * sizeof(unsigned int);
        if (this->textLen > 0) size += (this->textLen + 128 + 1) * sizeof(unsigned char);
        if (this->ht != NULL) size += this->ht->getHTSize();
	return size;
}

unsigned int FBCSA::getTextSize() {
	return this->textLen * sizeof(unsigned char);
}

unsigned int FBCSA::getSAValue_32(unsigned int i) {
	unsigned int m = (i / 32) * 4;
	unsigned int p = i % 32;
	unsigned int b = this->alignedArr1[m + 2];
	unsigned int c = this->alignedArr1[m + 3];
	unsigned int r = 0, w = 0;

	if (((b >> (32 - p - 1)) & 1) == 1) {
		if (p > 0) r = __builtin_popcountll(b >> (32 - p));
		return this->alignedArr2[c + 3 + r];
	}
	else {
		if (p > 15) {
			int t = p - 16;
			unsigned int a = (this->alignedArr1[m + 1] >> (32 - 2 * t - 2)) & 3;
			switch (a) {
			case 2:
				w = ~(this->alignedArr1[m] ^ 0xAAAAAAAA);
				break;
			case 1:
				w = ~(this->alignedArr1[m] ^ 0x55555555);
				break;
			default:
				w = ~(this->alignedArr1[m] ^ 0x00000000);
			}
			r = __builtin_popcountll((w & 0x55555555) & ((w & 0xAAAAAAAA) >> 1));
			if (t > 0) {
				switch (a) {
				case 2:
					w = ~(this->alignedArr1[m + 1] ^ 0xAAAAAAAA);
					break;
				case 1:
					w = ~(this->alignedArr1[m + 1] ^ 0x55555555);
					break;
				default:
					w = ~(this->alignedArr1[m + 1] ^ 0x00000000);
				}
				w >>= (32 - 2 * t);
				r += __builtin_popcountll((w & 0x55555555) & ((w & 0xAAAAAAAA) >> 1));
			}
			return this->getSAValue_32(this->alignedArr2[c + a] + r) + 1;
		}
		else {
			unsigned int a = (this->alignedArr1[m] >> (32 - 2 * p - 2)) & 3;
			if (p > 0) {
				switch (a) {
				case 2:
					w = ~(this->alignedArr1[m] ^ 0xAAAAAAAA);
					break;
				case 1:
					w = ~(this->alignedArr1[m] ^ 0x55555555);
					break;
				default:
					w = ~(this->alignedArr1[m] ^ 0x00000000);
				}
				w >>= (32 - 2 * p);
				r = __builtin_popcountll((w & 0x55555555) & ((w & 0xAAAAAAAA) >> 1));
			}
			return this->getSAValue_32(this->alignedArr2[c + a] + r) + 1;
		}
	}
}

unsigned int FBCSA::getSAValue_64(unsigned int i) {
	unsigned int m = (i / 64) * 7;
	unsigned int p = i % 64;
	unsigned long long b = (((unsigned long long)this->alignedArr1[m + 4]) << 32) + this->alignedArr1[m + 5];
	unsigned int c = this->alignedArr1[m + 6];
	unsigned int r = 0;
	unsigned long long arr1Long1, arr1Long2, w = 0;

	if (((b >> (64 - p - 1)) & 1) == 1) {
		if (p > 0) r = __builtin_popcountll(b >> (64 - p));
		return this->alignedArr2[c + 3 + r];
	}
	else {
		arr1Long1 = (((unsigned long long)this->alignedArr1[m]) << 32) + this->alignedArr1[m + 1];
		if (p > 31) {
			int t = p - 32;
			arr1Long2 = (((unsigned long long)this->alignedArr1[m + 2]) << 32) + this->alignedArr1[m + 3];
			unsigned int a = (arr1Long2 >> (64 - 2 * t - 2)) & 3;
			switch (a) {
			case 2:
				w = ~(arr1Long1 ^ 0xAAAAAAAAAAAAAAAA);
				break;
			case 1:
				w = ~(arr1Long1 ^ 0x5555555555555555);
				break;
			default:
				w = ~(arr1Long1 ^ 0x0000000000000000);
			}
			r = __builtin_popcountll((w & 0x5555555555555555) & ((w & 0xAAAAAAAAAAAAAAAA) >> 1));
			if (t > 0) {
				switch (a) {
				case 2:
					w = ~(arr1Long2 ^ 0xAAAAAAAAAAAAAAAA);
					break;
				case 1:
					w = ~(arr1Long2 ^ 0x5555555555555555);
					break;
				default:
					w = ~(arr1Long2 ^ 0x0000000000000000);
				}
				w >>= (64 - 2 * t);
				r += __builtin_popcountll((w & 0x5555555555555555) & ((w & 0xAAAAAAAAAAAAAAAA) >> 1));
			}
			return this->getSAValue_64(this->alignedArr2[c + a] + r) + 1;
		}
		else {
			unsigned int a = (arr1Long1 >> (64 - 2 * p - 2)) & 3;
			if (p > 0) {
				switch (a) {
				case 2:
					w = ~(arr1Long1 ^ 0xAAAAAAAAAAAAAAAA);
					break;
				case 1:
					w = ~(arr1Long1 ^ 0x5555555555555555);
					break;
				default:
					w = ~(arr1Long1 ^ 0x0000000000000000);
				}
				w >>= (64 - 2 * p);
				r = __builtin_popcountll((w & 0x5555555555555555) & ((w & 0xAAAAAAAAAAAAAAAA) >> 1));
			}
			return this->getSAValue_64(this->alignedArr2[c + a] + r) + 1;
		}
	}
}

unsigned int FBCSA::getSAValue_general(unsigned int i) {
	unsigned int m = (i / this->bs) * ((this->bs / 16) + (this->bs / 32) + 1);
	unsigned int p = i % this->bs;
	unsigned int d0 = p / 32;
	unsigned int p0 = p % 32;
	unsigned int b = this->alignedArr1[m + (this->bs / 16) + d0];
	unsigned int c = this->alignedArr1[m + (this->bs / 16) + (this->bs / 32)];
	unsigned int r = 0, w = 0;

	if (((b >> (32 - p0 - 1)) & 1) == 1) {
		for (unsigned int i = 0; i < d0; ++i) {
			r += __builtin_popcountll(this->alignedArr1[m + (this->bs / 16) + i]);
		}
		if (p0 > 0) r += __builtin_popcountll(b >> (32 - p0));
		return this->alignedArr2[c + 3 + r];
	}
	else {
		int d1 = p / 16;
		int d2 = d1 * 16;
		unsigned int a = (this->alignedArr1[m + d1] >> (32 - 2 * (p - d2) - 2)) & 3;
		for (int i = 0; i < d1; ++i) {
			switch (a) {
			case 2:
				w = ~(this->alignedArr1[m + i] ^ 0xAAAAAAAA);
				break;
			case 1:
				w = ~(this->alignedArr1[m + i] ^ 0x55555555);
				break;
			default:
				w = ~(this->alignedArr1[m + i] ^ 0x00000000);
			}
			r += __builtin_popcountll((w & 0x55555555) & ((w & 0xAAAAAAAA) >> 1));
		}
		if (p - d2 > 0) {
			switch (a) {
			case 2:
				w = ~(this->alignedArr1[m + d1] ^ 0xAAAAAAAA);
				break;
			case 1:
				w = ~(this->alignedArr1[m + d1] ^ 0x55555555);
				break;
			default:
				w = ~(this->alignedArr1[m + d1] ^ 0x00000000);
			}
			w >>= (32 - 2 * (p - d2));
			r += __builtin_popcountll((w & 0x55555555) & ((w & 0xAAAAAAAA) >> 1));
		}
		return this->getSAValue_general(this->alignedArr2[c + a] + r) + 1;
	}
}

void FBCSA::getSAValuesSeq_32(unsigned int i, unsigned int seqLen, unsigned int *saValues) {
	unsigned int startBlock = (i / 32) * 4;
	unsigned int endBlock = ((i + seqLen) / 32) * 4;
	unsigned int p = i % 32;
	unsigned int b, c, w = 0, counter = 0;

	for (unsigned int i = startBlock; i <= endBlock; i = i + 4) {
		long long offSetInd = -1;
		b = this->alignedArr1[i + 2];
		c = this->alignedArr1[i + 3];
		unsigned int r = 0;
		for (unsigned int p0 = p; p0 < 32; ++p0) {
			if (((b >> (32 - p0 - 1)) & 1) == 1) {
				if (offSetInd == -1) {
					if (p0 > 0) r = __builtin_popcountll(b >> (32 - p0));
					offSetInd = c + 3 + r;
					r = 0;
				}
				else {
					++offSetInd;
				}
				saValues[counter++] = this->alignedArr2[offSetInd];
			}
			else {
				if (p0 > 15) {
					unsigned int t = p0 - 16;
					unsigned int a = (this->alignedArr1[i + 1] >> (32 - 2 * t - 2)) & 3;
					switch (a) {
					case 2:
						w = ~(this->alignedArr1[i] ^ 0xAAAAAAAA);
						break;
					case 1:
						w = ~(this->alignedArr1[i] ^ 0x55555555);
						break;
					default:
						w = ~(this->alignedArr1[i] ^ 0x00000000);
					}
					r = __builtin_popcountll((w & 0x55555555) & ((w & 0xAAAAAAAA) >> 1));
					if (t > 0) {
						switch (a) {
						case 2:
							w = ~(this->alignedArr1[i + 1] ^ 0xAAAAAAAA);
							break;
						case 1:
							w = ~(this->alignedArr1[i + 1] ^ 0x55555555);
							break;
						default:
							w = ~(this->alignedArr1[i + 1] ^ 0x00000000);
						}
						w >>= (32 - 2 * t);
						r += __builtin_popcountll((w & 0x55555555) & ((w & 0xAAAAAAAA) >> 1));
					}
					saValues[counter++] = this->getSAValue_32(this->alignedArr2[c + a] + r) + 1;
					r = 0;
				}
				else {
					unsigned int a = (this->alignedArr1[i] >> (32 - 2 * p0 - 2)) & 3;
					if (p0 > 0) {
						switch (a) {
						case 2:
							w = ~(this->alignedArr1[i] ^ 0xAAAAAAAA);
							break;
						case 1:
							w = ~(this->alignedArr1[i] ^ 0x55555555);
							break;
						default:
							w = ~(this->alignedArr1[i] ^ 0x00000000);
						}
						w >>= (32 - 2 * p0);
						r = __builtin_popcountll((w & 0x55555555) & ((w & 0xAAAAAAAA) >> 1));
					}
					saValues[counter++] = this->getSAValue_32(this->alignedArr2[c + a] + r) + 1;
					r = 0;
				}
			}
			if (counter == seqLen) break;
		}
		if (counter == seqLen) break;
		p = 0;
	}
}

void FBCSA::getSAValuesSeq_64(unsigned int i, unsigned int seqLen, unsigned int *saValues) {
        unsigned int startBlock = (i / 64) * 7;
	unsigned int endBlock = ((i + seqLen) / 64) * 7;
	unsigned int p = i % 64;
	unsigned int c, counter = 0;
	unsigned long long arr1Long1, arr1Long2, w = 0;

	for (unsigned int i = startBlock; i <= endBlock; i = i + 7) {
		c = this->alignedArr1[i + 6];
		int r = 0;
		unsigned long long b = (((unsigned long long)this->alignedArr1[i + 4]) << 32) + this->alignedArr1[i + 5];
		for (unsigned int p0 = p; p0 < 64; ++p0) {
			if (((b >> (64 - p0 - 1)) & 1) == 1) {
				if (p0 > 0) r = __builtin_popcountll(b >> (64 - p0));
				saValues[counter++] = this->alignedArr2[c + 3 + r];
			}
			else {
				arr1Long1 = (((unsigned long long)this->alignedArr1[i]) << 32) + this->alignedArr1[i + 1];
				if (p0 > 31) {
					int t = p0 - 32;
					arr1Long2 = (((unsigned long long)this->alignedArr1[i + 2]) << 32) + this->alignedArr1[i + 3];
					unsigned int a = (arr1Long2 >> (64 - 2 * t - 2)) & 3;
					switch (a) {
					case 2:
						w = ~(arr1Long1 ^ 0xAAAAAAAAAAAAAAAA);
						break;
					case 1:
						w = ~(arr1Long1 ^ 0x5555555555555555);
						break;
					default:
						w = ~(arr1Long1 ^ 0x0000000000000000);
					}
					r = __builtin_popcountll((w & 0x5555555555555555) & ((w & 0xAAAAAAAAAAAAAAAA) >> 1));
					if (t > 0) {
						switch (a) {
						case 2:
							w = ~(arr1Long2 ^ 0xAAAAAAAAAAAAAAAA);
							break;
						case 1:
							w = ~(arr1Long2 ^ 0x5555555555555555);
							break;
						default:
							w = ~(arr1Long2 ^ 0x0000000000000000);
						}
						w >>= (64 - 2 * t);
						r += __builtin_popcountll((w & 0x5555555555555555) & ((w & 0xAAAAAAAAAAAAAAAA) >> 1));
					}
					saValues[counter++] = this->getSAValue_64(this->alignedArr2[c + a] + r) + 1;
				}
				else {
					unsigned int a = (arr1Long1 >> (64 - 2 * p0 - 2)) & 3;
					if (p0 > 0) {
						switch (a) {
						case 2:
							w = ~(arr1Long1 ^ 0xAAAAAAAAAAAAAAAA);
							break;
						case 1:
							w = ~(arr1Long1 ^ 0x5555555555555555);
							break;
						default:
							w = ~(arr1Long1 ^ 0x0000000000000000);
						}
						w >>= (64 - 2 * p0);
						r = __builtin_popcountll((w & 0x5555555555555555) & ((w & 0xAAAAAAAAAAAAAAAA) >> 1));
					}
					saValues[counter++] = this->getSAValue_64(this->alignedArr2[c + a] + r) + 1;
				}
				r = 0;
			}
			if (counter == seqLen) break;
		}
		if (counter == seqLen) break;
		p = 0;
	}
}

void FBCSA::getSAValuesSeq_general(unsigned int i, unsigned int seqLen, unsigned int *saValues) {
        unsigned int bT = (this->bs / 16) + (this->bs / 32) + 1;
	unsigned int startBlock = (i / this->bs) * bT;
	unsigned int endBlock = ((i + seqLen) / this->bs) * bT;
	unsigned int p = i % this->bs;
	unsigned int b, c, w = 0, counter = 0;

	for (unsigned int i = startBlock; i <= endBlock; i = i + bT) {
		long long offSetInd = -1;
		c = this->alignedArr1[i + (this->bs / 16) + (this->bs / 32)];
		int r = 0;
		for (unsigned int p0 = p; p0 < this->bs; ++p0) {
			unsigned int d0 = p0 / 32;
			unsigned int p1 = p0 % 32;
			b = this->alignedArr1[i + (this->bs / 16) + d0];
			if (((b >> (32 - p1 - 1)) & 1) == 1) {
				if (offSetInd == -1) {
					for (unsigned int k = 0; k < d0; ++k) {
						r += __builtin_popcountll(this->alignedArr1[i + (this->bs / 16) + k]);
					}
					if (p1 > 0) r += __builtin_popcountll(b >> (32 - p1));
					offSetInd = c + 3 + r;
					r = 0;
				}
				else {
					++offSetInd;
				}
				saValues[counter++] = this->alignedArr2[offSetInd];
			}
			else {
				unsigned int d1 = p0 / 16;
				unsigned int d2 = d1 * 16;
				unsigned int a = (this->alignedArr1[i + d1] >> (32 - 2 * (p0 - d2) - 2)) & 3;
				for (unsigned int k = 0; k < d1; ++k) {
					switch (a) {
					case 2:
						w = ~(this->alignedArr1[i + k] ^ 0xAAAAAAAA);
						break;
					case 1:
						w = ~(this->alignedArr1[i + k] ^ 0x55555555);
						break;
					default:
						w = ~(this->alignedArr1[i + k] ^ 0x00000000);
					}
					r += __builtin_popcountll((w & 0x55555555) & ((w & 0xAAAAAAAA) >> 1));
				}
				if (p0 - d2 > 0) {
					switch (a) {
					case 2:
						w = ~(this->alignedArr1[i + d1] ^ 0xAAAAAAAA);
						break;
					case 1:
						w = ~(this->alignedArr1[i + d1] ^ 0x55555555);
						break;
					default:
						w = ~(this->alignedArr1[i + d1] ^ 0x00000000);
					}
					w >>= (32 - 2 * (p0 - d2));
					r += __builtin_popcountll((w & 0x55555555) & ((w & 0xAAAAAAAA) >> 1));
				}
				saValues[counter++] = this->getSAValue_general(this->alignedArr2[c + a] + r) + 1;
				r = 0;
			}
			if (counter == seqLen) break;
		}
		if (counter == seqLen) break;
		p = 0;
	}
}

void FBCSA::binarySearchAStrcmp(unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end) {
	unsigned int l = lStart;
	unsigned int r = rStart;
	unsigned int mid;
	while (l < r) {
		mid = (l + r) / 2;
		if (A_strcmp((const char*)pattern, (const char*)(this->alignedText + (this->*getSAValue)(mid))) > 0) {
			l = mid + 1;
		}
		else {
			r = mid;
		}
	}
	beg = l;
	r = rStart;
	++pattern[patternLength - 1];
	while (l < r) {
		mid = (l + r) / 2;
		if (A_strcmp((const char*)pattern, (const char*)(this->alignedText + (this->*getSAValue)(mid))) <= 0) {
			r = mid;
		}
		else {
			l = mid + 1;
		}
	}
	--pattern[patternLength - 1];
	end = r;
}

void FBCSA::binarySearchStrncmp(unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end) {
	unsigned int l = lStart;
	unsigned int r = rStart;
	unsigned int mid;
	while (l < r) {
		mid = (l + r) / 2;
		if (strncmp((const char*)pattern, (const char*)(this->alignedText + (this->*getSAValue)(mid)), patternLength) > 0) {
			l = mid + 1;
		}
		else {
			r = mid;
		}
	}
	beg = l;
	r = rStart;
	while (l < r) {
		mid = (l + r) / 2;
		if (strncmp((const char*)pattern, (const char*)(this->alignedText + (this->*getSAValue)(mid)), patternLength) < 0) {
			r = mid;
		}
		else {
			l = mid + 1;
		}
	}
	end = r;
}

unsigned int FBCSA::count_std(unsigned char *pattern, unsigned int patternLength) {
	unsigned int beg, end;
	if (pattern[patternLength - 1] == 255) this->binarySearchStrncmp(0, this->textLen + 1, pattern, patternLength, beg, end);
	else this->binarySearchAStrcmp(0, this->textLen + 1, pattern, patternLength, beg, end);
	return end - beg;
}

unsigned int FBCSA::count_hash(unsigned char *pattern, unsigned int patternLength) {
        if (patternLength < this->ht->k) return this->count_std(pattern, patternLength);
        unsigned int leftBoundary, rightBoundary, beg, end;
        this->getBoundaries(pattern, leftBoundary, rightBoundary);
        if (pattern[patternLength - 1] == 255) this->binarySearchStrncmp(leftBoundary, rightBoundary, pattern, patternLength, beg, end);
        else this->binarySearchAStrcmp(leftBoundary, rightBoundary, pattern, patternLength, beg, end);
        return end - beg;
}

unsigned int FBCSA::count(unsigned char *pattern, unsigned int patternLen) {
	return (this->*countOperation)(pattern, patternLen);
}

unsigned int *FBCSA::locate(unsigned char *pattern, unsigned int patternLen) {
	return 0;
}

unsigned int FBCSA::extract(unsigned int i) {
        return (this->*getSAValue)(i);
}

void FBCSA::extractSeq(unsigned int i, unsigned int seqLen, unsigned int *saValues) {
        return (this->*getSAValuesSeq)(i, seqLen, saValues);
}

void FBCSA::save(const char *fileName) {
	if (this->verbose) cout << "Saving index in " << fileName << " ... " << flush;
	bool nullPointer = false;
	bool notNullPointer = true;
	FILE *outFile;
	outFile = fopen(fileName, "w");
	fwrite(&this->verbose, (size_t)sizeof(bool), (size_t)1, outFile);
        fwrite(&this->bs, (size_t)sizeof(unsigned int), (size_t)1, outFile);
        fwrite(&this->ss, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(&this->arr1Len, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->arr1Len > 0) fwrite(this->alignedArr1, (size_t)sizeof(unsigned int), (size_t)this->arr1Len, outFile);
        fwrite(&this->arr2Len, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->arr2Len > 0) fwrite(this->alignedArr2, (size_t)sizeof(unsigned int), (size_t)this->arr2Len, outFile);
        fwrite(&this->textLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->textLen > 0) fwrite(this->alignedText, (size_t)sizeof(unsigned char), (size_t)this->textLen, outFile);
        if (this->ht == NULL) fwrite(&nullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
        else {
                fwrite(&notNullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
                this->ht->save(outFile);
        }
	fclose(outFile);
	if (this->verbose) cout << "Done" << endl;
}

void FBCSA::load(const char *fileName) {
	this->free();
        if (this->ht != NULL) {
                delete this->ht;
                this->ht = NULL;
        }
	bool isNotNullPointer;
	FILE *inFile;
	inFile = fopen(fileName, "rb");
	size_t result;
	result = fread(&this->verbose, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->verbose) cout << "Loading index from " << fileName << " ... " << flush;
        result = fread(&this->bs, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
        result = fread(&this->ss, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->arr1Len, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->arr1Len > 0) {
		this->arr1 = new unsigned int[this->arr1Len + 32];
		this->alignedArr1 = this->arr1;
		while ((unsigned long long)this->alignedArr1 % 128) ++this->alignedArr1;
		result = fread(this->alignedArr1, (size_t)sizeof(unsigned int), (size_t)this->arr1Len, inFile);
		if (result != this->arr1Len) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
        result = fread(&this->arr2Len, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->arr2Len > 0) {
		this->arr2 = new unsigned int[this->arr2Len + 32];
		this->alignedArr2 = this->arr2;
		while ((unsigned long long)this->alignedArr2 % 128) ++this->alignedArr2;
		result = fread(this->alignedArr2, (size_t)sizeof(unsigned int), (size_t)this->arr2Len, inFile);
		if (result != this->arr2Len) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
        result = fread(&this->textLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->textLen > 0) {
		this->text = new unsigned char[this->textLen + 128 + 1];
		this->alignedText = this->text;
		while ((unsigned long long)this->alignedText % 128) ++this->alignedText;
		result = fread(this->alignedText, (size_t)sizeof(unsigned char), (size_t)this->textLen, inFile);
                this->alignedText[this->textLen] = '\0';
		if (result != this->textLen) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
	result = fread(&isNotNullPointer, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (isNotNullPointer) {
                this->ht = new HT();
                this->ht->load(inFile);
	}
	fclose(inFile);
	this->setFunctions();
	if (this->verbose) cout << "Done" << endl;
}

void FBCSA::getStandardHTBoundaries(unsigned char *pattern, unsigned int &leftBoundary, unsigned int &rightBoundary) {
	unsigned int leftBoundaryLUT2 = this->ht->lut2[pattern[0]][pattern[1]][0];
	unsigned int rightBoundaryLUT2 = this->ht->lut2[pattern[0]][pattern[1]][1];
	if (leftBoundaryLUT2 < rightBoundaryLUT2) {
		unsigned long long hash = this->ht->getHashValue(pattern, this->ht->k) % this->ht->bucketsNum;
		while (true) {
			leftBoundary = this->ht->alignedBoundariesHT[2 * hash];
			if (leftBoundary >= leftBoundaryLUT2 && leftBoundary < rightBoundaryLUT2 && strncmp((const char *)pattern + 2, (const char *)(this->alignedText + (this->*getSAValue)(leftBoundary) + 2), this->ht->prefixLength) == 0) {
				rightBoundary = this->ht->alignedBoundariesHT[2 * hash + 1];
				break;
			}
			if (leftBoundary == HT::emptyValueHT) {
				leftBoundary = 0;
				rightBoundary = 0;
				return;
			}
			++hash;
			if (hash == this->ht->bucketsNum) {
				hash = 0;
			}
		}
	} else {
		leftBoundary = 0;
		rightBoundary = 0;
	}
}

void FBCSA::getDenseHTBoundaries(unsigned char *pattern, unsigned int &leftBoundary, unsigned int &rightBoundary) {
	unsigned int step;
        unsigned int leftBoundaryLUT2 = this->ht->lut2[pattern[0]][pattern[1]][0];
	unsigned int rightBoundaryLUT2 = this->ht->lut2[pattern[0]][pattern[1]][1];
	if (leftBoundaryLUT2 < rightBoundaryLUT2) {
		unsigned long long hash = this->ht->getHashValue(pattern, this->ht->k) % this->ht->bucketsNum;
		while (true) {
			leftBoundary = this->ht->alignedBoundariesHT[hash];
			if (leftBoundary >= leftBoundaryLUT2 && leftBoundary < rightBoundaryLUT2 && strncmp((const char *)pattern + 2, (const char *)(this->alignedText + (this->*getSAValue)(leftBoundary) + 2), this->ht->prefixLength) == 0) {
				step = (unsigned int)ceil(((double)rightBoundaryLUT2 + 1 - leftBoundaryLUT2) / 65536);
                                if ((hash & 1) == 0) rightBoundary = (this->ht->alignedDenseBoundariesHT[hash / 2] >> 16) * step + leftBoundaryLUT2;
                                else rightBoundary = (this->ht->alignedDenseBoundariesHT[hash / 2] & 0xFFFF) * step + leftBoundaryLUT2;
				break;
			}
			if (leftBoundary == HT::emptyValueHT) {
				leftBoundary = 0;
				rightBoundary = 0;
				return;
			}
			++hash;
			if (hash == this->ht->bucketsNum) {
				hash = 0;
			}
		}
	} else {
		leftBoundary = 0;
		rightBoundary = 0;
	}
}

void FBCSA::getBoundaries(unsigned char *pattern, unsigned int &leftBoundary, unsigned int &rightBoundary) {
    return (this->*getBoundariesOperation)(pattern, leftBoundary, rightBoundary);
}

/*FBCSALut2*/

void FBCSALut2::setFunctions() {
        switch (this->bs) {
        case 32:
               this->getSAValue = &this->getSAValue_32;
               this->getSAValuesSeq = &this->getSAValuesSeq_32;
               break;
        case 64:
               this->getSAValue = &this->getSAValue_64;
               this->getSAValuesSeq = &this->getSAValuesSeq_64;
               break;
        default:
               this->getSAValue = &this->getSAValue_general;
               this->getSAValuesSeq = &this->getSAValuesSeq_general;
               break; 
        }
}

void FBCSALut2::build(unsigned char* text, unsigned int textLen) {
	checkNullChar(text, textLen);
	this->free();
        if (this->verbose) cout << "Loading text ... " << flush;
	this->textLen = textLen;
        this->text = new unsigned char [this->textLen + 128 + 1];
        this->alignedText = this->text;
        while ((unsigned long long)this->alignedText % 128) ++this->alignedText;
        for (unsigned int i = 0; i < this->textLen; ++i) this->alignedText[i] = text[i];
        this->alignedText[this->textLen] = '\0';
        if (this->verbose) cout << "Done" << endl;
        
        unsigned int saLen;
        unsigned int *sa = getSA(this->alignedText, this->textLen, saLen, this->bs - ((this->textLen + 1) % this->bs), this->verbose);
        
        this->buildIndex(sa, saLen);
        
        fillLUT2(this->lut2, this->alignedText, sa, saLen);
        
	delete[] sa;
}

unsigned int FBCSALut2::getIndexSize() {
	unsigned int size = FBCSA::getIndexSize();
        size += 256 * 256 * 2 * sizeof(unsigned int);
        return size;
}

void FBCSALut2::save(const char *fileName) {
	if (this->verbose) cout << "Saving index in " << fileName << " ... " << flush;
	FILE *outFile;
	outFile = fopen(fileName, "w");
	fwrite(&this->verbose, (size_t)sizeof(bool), (size_t)1, outFile);
        fwrite(&this->bs, (size_t)sizeof(unsigned int), (size_t)1, outFile);
        fwrite(&this->ss, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(&this->arr1Len, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->arr1Len > 0) fwrite(this->alignedArr1, (size_t)sizeof(unsigned int), (size_t)this->arr1Len, outFile);
        fwrite(&this->arr2Len, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->arr2Len > 0) fwrite(this->alignedArr2, (size_t)sizeof(unsigned int), (size_t)this->arr2Len, outFile);
        fwrite(&this->textLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->textLen > 0) fwrite(this->alignedText, (size_t)sizeof(unsigned char), (size_t)this->textLen, outFile);
        fwrite(&this->lut2, (size_t)sizeof(unsigned int), (size_t)(256 * 256 * 2), outFile);
	fclose(outFile);
	if (this->verbose) cout << "Done" << endl;
}

void FBCSALut2::load(const char *fileName) {
	this->free();
	FILE *inFile;
	inFile = fopen(fileName, "rb");
	size_t result;
	result = fread(&this->verbose, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->verbose) cout << "Loading index from " << fileName << " ... " << flush;
        result = fread(&this->bs, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
        result = fread(&this->ss, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->arr1Len, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->arr1Len > 0) {
		this->arr1 = new unsigned int[this->arr1Len + 32];
		this->alignedArr1 = this->arr1;
		while ((unsigned long long)this->alignedArr1 % 128) ++this->alignedArr1;
		result = fread(this->alignedArr1, (size_t)sizeof(unsigned int), (size_t)this->arr1Len, inFile);
		if (result != this->arr1Len) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
        result = fread(&this->arr2Len, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->arr2Len > 0) {
		this->arr2 = new unsigned int[this->arr2Len + 32];
		this->alignedArr2 = this->arr2;
		while ((unsigned long long)this->alignedArr2 % 128) ++this->alignedArr2;
		result = fread(this->alignedArr2, (size_t)sizeof(unsigned int), (size_t)this->arr2Len, inFile);
		if (result != this->arr2Len) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
        result = fread(&this->textLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->textLen > 0) {
		this->text = new unsigned char[this->textLen + 128 + 1];
		this->alignedText = this->text;
		while ((unsigned long long)this->alignedText % 128) ++this->alignedText;
		result = fread(this->alignedText, (size_t)sizeof(unsigned char), (size_t)this->textLen, inFile);
                this->alignedText[this->textLen] = '\0';
		if (result != this->textLen) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
        result = fread(this->lut2, (size_t)sizeof(unsigned int), (size_t)(256 * 256 * 2), inFile);
	if (result != (256 * 256 * 2)) {
		cout << "Error loading index" << endl;
		exit(1);
	}
	fclose(inFile);
	this->setFunctions();
	if (this->verbose) cout << "Done" << endl;
}

unsigned int FBCSALut2::count(unsigned char *pattern, unsigned int patternLen) {
	if (patternLen < 2) return this->count_std(pattern, patternLen);
        unsigned int leftBoundaryLUT2 = this->lut2[pattern[0]][pattern[1]][0];
	unsigned int rightBoundaryLUT2 = this->lut2[pattern[0]][pattern[1]][1];
	if (leftBoundaryLUT2 < rightBoundaryLUT2) {
                unsigned int beg, end;
		if (pattern[patternLen - 1] == 255) this->binarySearchStrncmp(leftBoundaryLUT2, rightBoundaryLUT2, pattern, patternLen, beg, end);
                else this->binarySearchAStrcmp(leftBoundaryLUT2, rightBoundaryLUT2, pattern, patternLen, beg, end);
                return end - beg;
	} else return 0;
}

unsigned int *FBCSALut2::locate(unsigned char *pattern, unsigned int patternLen) {
	return 0;
}

}