# FBCSA text indexes library

##What is it?
The FBCSA text indexes are ...

The current version handles only the count query (i.e., returns the number of occurrences of the given pattern).

##Requirements
The FBCSA text indexes require:
- C++11 ready compiler such as g++ version 4.7 or higher
- a 64-bit operating system
- text size is limited to 4GB

##Installation
To download and build the library use the following commands:
```
git clone https://github.com/mranisz/fbcsa.git
cd fbcsa
make
```

##Usage
To use the FBCSA library:
- include "fbcsa/fbcsa.h" to your project
- compile it with "-std=c++11 -O3 -mpopcnt" options and link it with libraries:
  - fbcsa/libfbcsa.a
  - fbcsa/libs/libaelf64.a (linux) or fbcsa/libs/libacof64.lib (windows)
- use "fbcsa" namespace

##API
There are several functions you can call on each of the FBCSA text index:
- **build** the index using the text:
```
void build(unsigned char* text, unsigned int textLen);
```
- **save** the index to file called fileName:
```
void save(const char *fileName);
```
- **load** the index from file called fileName:
```
void load(const char *fileName);
```
- **free** memory occupied by index:
```
void free();
```
- get the **index size** in bytes (size in memory):
```
unsigned int getIndexSize();
```
- get the size in bytes of the text used to build the index:
```
unsigned int getTextSize();
```
- get the result of **count** query:
```
unsigned int count(unsigned char *pattern, unsigned int patternLen);
```
- set **verbose** mode:
```
void setVerbose(bool verbose);
```

##FBCSA

Parameters:
- bs - block size, must be a multiple of 32 (default: bs = 32)
- ss - sampling step (default: ss = 5)

Constructors:
```
FBCSA();
FBCSA(unsigned int bs, unsigned int ss);
```

##FBCSA-hash
SamSAMi1-hash is SamSAMi1 with hashed k-symbol prefixes of suffixes from sampled suffix array to speed up searches (k ≥ 2). This variant is particularly efficient in speed for short patterns (not much longer than max(q, k + q - p)).

Parameters:
- bs - block size, must be a multiple of 32 (default: bs = 32)
- ss - sampling step (default: ss = 5)
- hash type:
      - HT::STANDARD - using 8 bytes for each hashed entry: 4 bytes for left boundary + 4 bytes for right boundary
      - HT::DENSE - using 6 bytes for each hashed entry: 4 bytes for left boundary + 2 bytes for right boundary
- k - length of prefixes of suffixes from suffix array (k ≥ 2)
- loadFactor - hash table load factor (0.0 < loadFactor < 1.0)

Limitations: 
- pattern length >= k (patterns shorter than k are handled by standard variant of FBCSA index)

Constructors:
```
FBCSA(unsigned int bs, unsigned int ss, HT::HTType hTType, unsigned int k, double loadFactor);
```

##FBCSA-LUT2
To speed up searches, FBCSA stores lookup table over all 2-symbol strings (LUT2), whose entries are the suffix intervals.

Parameters:
- bs - block size, must be a multiple of 32 (default: bs = 32)
- ss - sampling step (default: ss = 5)

Limitations: 
- pattern length >= 2 (patterns shorter than 2 are handled by standard variant of FBCSA index)

Constructors:
```
FBCSALut2();
FBCSALut2(unsigned int bs, unsigned int ss);
```

##FBCSA usage example
```
#include <iostream>
#include <stdlib.h>
#include "fbcsa/shared/common.h"
#include "fbcsa/shared/patterns.h"
#include "fbcsa/fbcsa.h"

using namespace std;
using namespace fbcsa;

int main(int argc, char *argv[]) {

	unsigned int queriesNum = 1000000;
	unsigned int patternLen = 20;
	unsigned char* text = NULL;
	unsigned int textLen;
	FBCSA *fbcsa;
	const char *textFileName = "english";
	const char *indexFileName = "english-fbcsa.idx";

	if (fileExists(indexFileName)) {
		fbcsa = new FBCSA();
		fbcsa->load(indexFileName);
	} else {
		fbcsa = new FBCSA();
		fbcsa->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		fbcsa->build(text, textLen);
		fbcsa->save(indexFileName);
	}

	double indexSize = (double)fbcsa->getIndexSize();
	cout << "Index size: " << indexSize << "B (" << (indexSize / (double)fbcsa->getTextSize()) << "n)" << endl << endl;

	Patterns *P = new Patterns(textFileName, queriesNum, patternLen);
	unsigned char **patterns = P->getPatterns();

	for (unsigned int i = 0; i < queriesNum; ++i) {
		cout << "Pattern |" << patterns[i] << "| occurs " << fbcsa->count(patterns[i], patternLen) << " times." << endl;
	}

	if (text != NULL) delete[] text;
	delete fbcsa;
	delete P;
}
```
Using other FBCSA indexes is analogous.

##External resources used in FBCSA project
- Suffix array building by Yuta Mori (sais)
- A multi-platform library of highly optimized functions for C and C++ by Agner Fog (asmlib)
- A very fast hash function by Yann Collet (xxHash)

##Authors
- Szymon Grabowski
- [Marcin Raniszewski](https://github.com/mranisz)
