# FBCSA text indexes library

## What is it?
FBCSA (Fixed Block based Compact Suffix Array) \[[1](#references)\] is a compact text index, with space use of about 1.5n - 2.5n bytes (+1n bytes for the indexed text), depending on the indexed text characteristics and two construction-time parameters: block size and sampling step, which allows for relatively fast pattern search and access to an arbitrary T[i] symbol.

## Requirements
The FBCSA text indexes require:
- C++11 ready compiler such as g++ version 4.7 or higher
- a 64-bit UNIX operating system
- text size < 2GB

## Installation
To download and build the library use the following commands:
```
git clone https://github.com/mranisz/fbcsa.git
cd fbcsa
make
```

## Usage
To use the FBCSA library:
- include "fbcsa/fbcsa.hpp" to your project
- compile it with "-std=c++11 -O3 -mpopcnt" options and link it with libraries:
  - fbcsa/libfbcsa.a
  - fbcsa/libs/libaelf64.a
- use "fbcsa" and "shared" namespaces

## API
There are several functions you can call on each of the FBCSA text index:
- **build** the index using text file called textFileName:
```
void build(const char *textFileName);
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
unsigned long long getIndexSize();
```
- get the size in bytes of the text used to build the index:
```
unsigned int getTextSize();
```
- get the result of **extract** query:
```
unsigned int extract(unsigned int i);
```
- get the result of **count** query:
```
unsigned int count(unsigned char *pattern, unsigned int patternLen);
```
- get the result of **locate** query:
```
void locate(unsigned char *pattern, unsigned int patternLen, vector<unsigned int>& res);
```

## FBCSA\<unsigned int BS\>

Parameters:
- BS - block size
- ss - sampling step (default: ss = 5)

Limitations: 
- BS > 0 and ss > 0
- BS must be a multiple of 32

Constructors:
```
FBCSA<unsigned int BS>();
FBCSA<unsigned int BS>(unsigned int ss);
```

## FBCSAHash\<unsigned int BS, HTType HASHTYPE\>
FBCSAHash is FBCSA with hashed k-symbol prefixes of suffix array suffixes to speed up searches (k ≥ 2). This variant is particularly efficient in speed for short patterns (not much longer than k).

Parameters:
- BS - block size
- HASHTYPE:
  - HT_STANDARD - using 8 bytes for each hashed entry: 4 bytes for left boundary + 4 bytes for right boundary
  - HT_DENSE - using 6 bytes for each hashed entry: 4 bytes for left boundary + 2 bytes for right boundary
- ss - sampling step
- k - length of prefixes of suffixes from suffix array
- loadFactor - hash table load factor

Limitations:
- pattern length ≥ k (patterns shorter than k are handled by standard variant of FBCSA index)
- BS > 0 and ss > 0
- BS must be a multiple of 32
- k ≥ 2
- 0.0 < loadFactor < 1.0

Constructors:
```
FBCSAHash<unsigned int BS, HTType HASHTYPE>(unsigned int ss, unsigned int k, double loadFactor);
```

## FBCSALut2\<unsigned int BS\>
To speed up searches, FBCSA stores lookup table over all 2-symbol strings (LUT2), whose entries are the suffix intervals.

Parameters:
- BS - block size
- ss - sampling step (default: ss = 5)

Limitations: 
- pattern length ≥ 2 (patterns shorter than 2 are handled by standard variant of FBCSA index)
- BS > 0 and ss > 0
- BS must be a multiple of 32

Constructors:
```
FBCSALut2<unsigned int BS>();
FBCSALut2<unsigned int BS>(unsigned int ss);
```

## FBCSAHyb\<unsigned int BS\>
Hybrid of FBCSA and the plain SA. The first floor(log((n + 1) / 64)) + 1 binary search steps (searching for the left boundary) are performed using plain SA (for n < 63 we perform 1 step). Moreover, the right boundary is searched using doubling technique.

Parameters:
- BS - block size
- ss - sampling step (default: ss = 5)

Limitations: 
- BS > 0 and ss > 0
- BS must be a multiple of 32

Constructors:
```
FBCSAHyb<unsigned int BS>();
FBCSAHyb<unsigned int BS>(unsigned int ss);
```

## FBCSA usage example
```
#include <iostream>
#include <stdlib.h>
#include "fbcsa/shared/patterns.hpp"
#include "fbcsa/fbcsa.hpp"

using namespace std;
using namespace shared;
using namespace fbcsa;

int main(int argc, char *argv[]) {

	unsigned int queriesNum = 1000000;
	unsigned int patternLen = 20;
	FBCSA<32> *fbcsa = new FBCSA<32>();
	const char *textFileName = "english";
	const char *indexFileName = "english-fbcsa.idx";

	if (fileExists(indexFileName)) {
		fbcsa->load(indexFileName);
	} else {
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	double indexSize = (double)fbcsa->getIndexSize();
	cout << "Index size: " << indexSize << "B (" << (indexSize / (double)fbcsa->getTextSize()) << "n)" << endl << endl;

	Patterns32 *P = new Patterns32(textFileName, queriesNum, patternLen);
	unsigned char **patterns = P->getPatterns();

	for (unsigned int i = 0; i < queriesNum; ++i) {
		cout << "Pattern |" << patterns[i] << "| occurs " << fbcsa->count(patterns[i], patternLen) << " times." << endl;
	}

	delete fbcsa;
	delete P;
}
```
Using other FBCSA indexes is analogous.

## External resources used in FBCSA project
- Suffix array building by Yuta Mori (sais)
- A multi-platform library of highly optimized functions for C and C++ by Agner Fog (asmlib)
- A very fast hash function by Yann Collet (xxHash)

## References
1. Sz. Grabowski, M. Raniszewski. Two simple full-text indexes based on the suffix array. arXiv:1405.5919, 2016.

## Authors
- Szymon Grabowski
- [Marcin Raniszewski](https://github.com/mranisz)
