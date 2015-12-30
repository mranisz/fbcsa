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
	unsigned char **patterns;
	unsigned int *counts = NULL;

	void initialize();
	void freeMemory();
	void initializePatterns();
	void initializeSACounts();
	unsigned int getSACount(unsigned int *sa, unsigned char *text, unsigned int saLen, unsigned char *pattern, int patternLength);
	void setSelectedChars(vector<unsigned char> selectedChars);

public:
	Patterns(const char *textFileName, unsigned int queriesNum, unsigned int m, vector<unsigned char> selectedChars = {}) {
		this->textFileName = textFileName;
		this->queriesNum = queriesNum;
		this->m = m;
		this->setSelectedChars(selectedChars);
		this->initialize();
	}
	~Patterns() {
		this->freeMemory();
	}
	unsigned char **getPatterns();
	unsigned int *getSACounts();
	unsigned int getErrorCountsNumber(unsigned int *countsToCheck);
};

}

#endif /* SHARED_PATTERNS_H_ */
