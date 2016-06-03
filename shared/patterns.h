#ifndef SHARED_PATTERNS_H_
#define SHARED_PATTERNS_H_

#include <vector>

using namespace std;

namespace fbcsa {

class Patterns {
private:
	const char *textFileName;
	unsigned int queriesNum;
	unsigned int m;
	vector<unsigned char> selectedChars;
	unsigned char **patterns = NULL;
	unsigned int *counts = NULL;
        vector<unsigned int> *locates = NULL;

	void freeMemory();
	void initializePatterns();
	void initializeSACounts();
        void initializeSALocates();
	unsigned int getSACount(unsigned int *sa, unsigned char *text, unsigned int saLen, unsigned char *pattern, int patternLength);
        void getSALocate(unsigned int *sa, unsigned char *text, unsigned int saLen, unsigned char *pattern, int patternLength, vector<unsigned int>& res);
	void setSelectedChars(vector<unsigned char> selectedChars);

public:
	Patterns(const char *textFileName, unsigned int queriesNum, unsigned int m, vector<unsigned char> selectedChars = {}) {
		this->textFileName = textFileName;
		this->queriesNum = queriesNum;
		this->m = m;
		this->setSelectedChars(selectedChars);
	}
	~Patterns() {
		this->freeMemory();
	}
	unsigned char **getPatterns();
	unsigned int *getSACounts();
        vector<unsigned int> *getSALocates();
	unsigned int getErrorCountsNumber(unsigned int *countsToCheck);
        unsigned int getErrorLocatesNumber(vector<unsigned int> *locatesToCheck);
};

}

#endif /* SHARED_PATTERNS_H_ */
