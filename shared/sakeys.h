#ifndef SHARED_SAKEYS_H_
#define SHARED_SAKEYS_H_

#include <cstdio>

using namespace std;

namespace fbcsa {

class SAKeys {
private:
	const char *textFileName;
	unsigned int queriesNum;
        unsigned int seqLen;
	unsigned int *saKeys;
	unsigned int *saValues = NULL;

	void initialize();
	void freeMemory();
	void initializeSAKeys();
	void initializeSAValues();
	
public:
	SAKeys(const char *textFileName, unsigned int queriesNum, unsigned int seqLen) {
		this->textFileName = textFileName;
		this->queriesNum = queriesNum;
                this->seqLen = seqLen;
		this->initialize();
	}
	~SAKeys() {
		this->freeMemory();
	}
	unsigned int *getSAKeys();
	unsigned int *getSAValues();
	unsigned int getErrorSAValuesNumber(unsigned int *valuesToCheck);
};

}

#endif /* SHARED_SAKEYS_H_ */
