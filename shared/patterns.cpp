#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <random>
#include <unordered_set>
#include <algorithm>
#include "common.h"
#include "patterns.h"

namespace fbcsa {

void Patterns::initialize() {
	this->initializePatterns();
}

void Patterns::initializePatterns() {
	unsigned int textLen, queriesFirstIndexArrayLen;
	unsigned char *text = readFileChar(this->textFileName, textLen, 0);
        if (textLen < this->m) {
                cout << "Error: text shorter than pattern length" << endl;
                exit(1);
        }
	unsigned int *queriesFirstIndexArray;
        stringstream ss;
	ss << "patterns-" << this->textFileName << "-" << this->m << "-" << this->queriesNum << "-" << getStringFromSelectedChars(this->selectedChars, ".") << ".dat";
	string s = ss.str();
	char *patternFileName = (char *)(s.c_str());

	if (!fileExists(patternFileName)) {
		cout << "Generating " << this->queriesNum << " patterns of length " << this->m << " from " << this->textFileName;
		if (this->selectedChars.size() != 0) {
			cout << ", alphabet (ordinal): {" << getStringFromSelectedChars(this->selectedChars, ", ") << "}";
		}
		cout << " ... " << flush;

		random_device rd;
		mt19937 gen(rd());
		uniform_int_distribution<unsigned int> dis(0, textLen - this->m);

		queriesFirstIndexArray = new unsigned int[this->queriesNum];

		unsigned int genVal;

		if (this->selectedChars.size() != 0) {
			for (unsigned long long i = 0; i < this->queriesNum; ++i) {
				genVal = dis(gen);
				queriesFirstIndexArray[i] = genVal;
				for (unsigned int j = 0; j < this->m; ++j) {
					bool inSigma = false;
					for (vector<unsigned char>::iterator it = this->selectedChars.begin(); it != this->selectedChars.end(); ++it) {
						if (text[j + genVal] == (*it)) {
							inSigma = true;
							break;
						}
					}
					if (!inSigma) {
						--i;
						break;
					}
				}
			}
		} else {
			for (unsigned int i = 0; i < this->queriesNum; ++i) queriesFirstIndexArray[i] = dis(gen);
		}
		cout << "Done" << endl;
		cout << "Saving patterns in " << patternFileName << " ... " << flush;
		FILE *outFile;
		outFile = fopen(patternFileName, "w");
		fwrite(queriesFirstIndexArray, (size_t)4, (size_t)(this->queriesNum), outFile);
		fclose(outFile);
		cout << "Done" << endl;
	} else {
		cout << "Loading patterns from " << patternFileName << " ... " << flush;
		queriesFirstIndexArray = readFileInt(patternFileName, queriesFirstIndexArrayLen, 0);
                cout << "Done" << endl;
	}
	this->patterns = new unsigned char *[this->queriesNum];
	for (unsigned int i = 0; i < queriesNum; ++i) {
		this->patterns[i] = new unsigned char[this->m + 1];
		this->patterns[i][this->m] = '\0';
		for (unsigned int j = 0; j < this->m; ++j) {
			this->patterns[i][j] = text[queriesFirstIndexArray[i] + j];
		}
	}

	delete[] queriesFirstIndexArray;
	delete[] text;
}

void Patterns::initializeSACounts() {
	stringstream ss;
	ss << "counts-" << this->textFileName << "-" << this->m << "-" << this->queriesNum << "-" << getStringFromSelectedChars(this->selectedChars, ".") << ".dat";
	string s = ss.str();
	char *countsFileName = (char *)(s.c_str());

	if (!fileExists(countsFileName)) {
		unsigned int textLen;
		unsigned char *text = readFileChar(this->textFileName, textLen, 0);

		unsigned int saLen;
		unsigned int *sa = getSA(text, textLen, saLen, 0, true);

		cout << "Getting counts from SA ... " << flush;
		this->counts = new unsigned int[this->queriesNum];
		for (unsigned int i = 0; i < this->queriesNum; ++i) {
			this->counts[i] = this->getSACount(sa, text, saLen, this->patterns[i], this->m);
		}
		delete[] text;
		delete[] sa;
		cout << "Done" << endl;
		cout << "Saving counts in " << countsFileName << " ... " << flush;
		FILE *outFile;
		outFile = fopen(countsFileName, "w");
		fwrite(this->counts, (size_t)4, (size_t)(this->queriesNum), outFile);
		fclose(outFile);
		cout << "Done" << endl;

	} else {
		cout << "Loading counts from " << countsFileName << " ... " << flush;
		unsigned int countsLen;
		this->counts = readFileInt(countsFileName, countsLen, 0);
		cout << "Done" << endl;
	}
}

void Patterns::initializeSALocates() {
	stringstream ss;
	ss << "locates-" << this->textFileName << "-" << this->m << "-" << this->queriesNum << "-" << getStringFromSelectedChars(this->selectedChars, ".") << ".dat";
	string s = ss.str();
	char *locatesFileName = (char *)(s.c_str());

	if (!fileExists(locatesFileName)) {
		unsigned int textLen;
		unsigned char *text = readFileChar(this->textFileName, textLen, 0);

		unsigned int saLen;
		unsigned int *sa = getSA(text, textLen, saLen, 0, true);
                
                unsigned long long counter = 0;

		cout << "Getting locates from SA ... " << flush;
		this->locates = new vector<unsigned int>[this->queriesNum];
		for (unsigned int i = 0; i < this->queriesNum; ++i) {
			this->getSALocate(sa, text, saLen, this->patterns[i], this->m, this->locates[i]);
                        counter += (this->locates[i].size() + 1);
		}
		delete[] text;
		delete[] sa;
		cout << "Done" << endl;
                
                if (counter * sizeof(unsigned int) > 1 * 1024 * 1024 * 1024) return;
                
		cout << "Saving locates in " << locatesFileName << " ... " << flush;
		FILE *outFile;
		outFile = fopen(locatesFileName, "w");
                unsigned int locateSize;
                for (unsigned int i = 0; i < this->queriesNum; ++i) {
                    locateSize = this->locates[i].size();
                    fwrite(&locateSize, (size_t)4, (size_t)1, outFile);
                    for(vector<unsigned int>::iterator it = this->locates[i].begin(); it != this->locates[i].end(); ++it) {
                        fwrite(&(*it), (size_t)4, (size_t)1, outFile);
                    }
                }
		fclose(outFile);
		cout << "Done" << endl;

	} else {
		cout << "Loading locates from " << locatesFileName << " ... " << flush;
                size_t result;
                unsigned int locateSize, locateValue;
                FILE *inFile;
		inFile = fopen(locatesFileName, "rb");
                this->locates = new vector<unsigned int>[this->queriesNum];
		for (unsigned int i = 0; i < this->queriesNum; ++i) {
                    result = fread(&locateSize, (size_t)4, (size_t)1, inFile);
                    if (result != 1) {
                            cout << "Error reading file " << locatesFileName << endl;
                            exit(1);
                    }
                    for (unsigned int j = 0; j < locateSize; ++j) {
                            result = fread(&locateValue, (size_t)4, (size_t)1, inFile);
                            if (result != 1) {
                                    cout << "Error reading file " << locatesFileName << endl;
                                    exit(1);
                            }
                            this->locates[i].push_back(locateValue);
                    }
                }
                fclose(inFile);
		cout << "Done" << endl;
	}
}

void Patterns::freeMemory() {
	for (unsigned int i = 0; i < this->queriesNum; ++i) {
		delete[] this->patterns[i];
	}
	delete[] this->patterns;
	if (this->counts != NULL) delete[] this->counts;
        if (this->locates != NULL) delete[] this->locates;
}


unsigned int Patterns::getSACount(unsigned int *sa, unsigned char *text, unsigned int saLen, unsigned char *pattern, int patternLength) {
	unsigned int beg = 0, end = 0;
	binarySearch(sa, text, 0, saLen, pattern, patternLength, beg, end);
	return end - beg;
}

void Patterns::getSALocate(unsigned int *sa, unsigned char *text, unsigned int saLen, unsigned char *pattern, int patternLength, vector<unsigned int>& res) {
	unsigned int beg = 0, end = 0;
	binarySearch(sa, text, 0, saLen, pattern, patternLength, beg, end);
        res.insert(res.end(), sa + beg, sa + end); 
}

unsigned char **Patterns::getPatterns() {
	return this->patterns;
}

unsigned int *Patterns::getSACounts() {
	if (this->counts == NULL) this->initializeSACounts();
	return this->counts;
}

vector<unsigned int> *Patterns::getSALocates() {
	if (this->locates == NULL) this->initializeSALocates();
	return this->locates;
}

void Patterns::setSelectedChars(vector<unsigned char> selectedChars) {
	this->selectedChars = selectedChars;
}

unsigned int Patterns::getErrorCountsNumber(unsigned int *countsToCheck) {
	if (this->counts == NULL) this->initializeSACounts();
	cout << "Checking counts consistency ... " << flush;
	unsigned int errorCountsNumber = 0;
	for (unsigned int i = 0; i < this->queriesNum; ++i) {
		if (countsToCheck[i] != this->counts[i]) ++errorCountsNumber;
	}
	cout << "Done" << endl;
	return errorCountsNumber;
}

unsigned int Patterns::getErrorLocatesNumber(vector<unsigned int> *locatesToCheck) {
	if (this->locates == NULL) this->initializeSALocates();
	cout << "Checking locates consistency ... " << flush;
	unsigned int errorLocatesNumber = 0;
	for (unsigned int i = 0; i < this->queriesNum; ++i) {
		if (locatesToCheck[i].size() != this->locates[i].size()) ++errorLocatesNumber;
                else {
                    unordered_set<unsigned int> set1(this->locates[i].begin(), this->locates[i].end());
                    unordered_set<unsigned int> set2(locatesToCheck[i].begin(), locatesToCheck[i].end());
                    if (set1 != set2) ++errorLocatesNumber;
                }
	}
	cout << "Done" << endl;
	return errorLocatesNumber;
}

}
