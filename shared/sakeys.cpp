#include <iostream>
#include <sstream>
#include <fstream>
#include <random>
#include "common.h"
#include "sakeys.h"

namespace shared {

void SAKeys::initialize() {
	this->initializeSAKeys();
}

void SAKeys::initializeSAKeys() {
	unsigned int textLen, saKeysLen;
	unsigned char *text = readFileChar(this->textFileName, textLen, 0);
        if (textLen < this->seqLen) {
                cout << "Error: text shorter than sequence length" << endl;
                exit(1);
        }
        stringstream ss;
	ss << "sakeys-" << this->textFileName << "-" << this->queriesNum << "-" << this->seqLen << ".dat";
	string s = ss.str();
	char *sakeysFileName = (char *)(s.c_str());
        this->saKeys = new unsigned int[this->queriesNum];

	if (!fileExists(sakeysFileName)) {
		cout << "Generating " << this->queriesNum << " SA keys from " << this->textFileName << " (for sequence length = " << this->seqLen << ") ... " << flush;

		random_device rd;
		mt19937 gen(rd());
		uniform_int_distribution<unsigned int> dis(0, textLen - this->seqLen);

                for (unsigned int i = 0; i < this->queriesNum; ++i) this->saKeys[i] = dis(gen);
		cout << "Done" << endl;
		cout << "Saving SA keys in " << sakeysFileName << " ... " << flush;
		FILE *outFile;
		outFile = fopen(sakeysFileName, "w");
		fwrite(this->saKeys, (size_t)4, (size_t)(this->queriesNum), outFile);
		fclose(outFile);
		cout << "Done" << endl;
	} else {
		cout << "Loading SA keys from " << sakeysFileName << " ... " << flush;
		this->saKeys = readFileInt(sakeysFileName, saKeysLen, 0);
		cout << "Done" << endl;
	}

	delete[] text;
}

void SAKeys::initializeSAValues() {
	stringstream ss;
	ss << "savalues-" << this->textFileName << "-" << this->queriesNum  << "-" << this->seqLen << ".dat";
	string s = ss.str();
	char *saValuesFileName = (char *)(s.c_str());

	if (!fileExists(saValuesFileName)) {
		unsigned int textLen;
		unsigned char *text = readFileChar(this->textFileName, textLen, 0);
		unsigned int saLen;
		unsigned int *sa = getSA(this->textFileName, text, textLen, saLen, 0);
                
		cout << "Getting SA values (sequence length = " << this->seqLen << ") from SA ... " << flush;
		this->saValues = new unsigned int[this->queriesNum * this->seqLen];
		for (unsigned int i = 0; i < this->queriesNum; ++i) {
                    this->saValues[i * this->seqLen] = sa[this->saKeys[i]];
                    for (unsigned int j = 0; j < this->seqLen; ++j) this->saValues[i * this->seqLen + j] = sa[this->saKeys[i] + j];
                }
                delete[] text;
		delete[] sa;
		cout << "Done" << endl;
		cout << "Saving SA values in " << saValuesFileName << " ... " << flush;
		FILE *outFile;
		outFile = fopen(saValuesFileName, "w");
		fwrite(this->saValues, (size_t)4, (size_t)(this->queriesNum * this->seqLen), outFile);
		fclose(outFile);
		cout << "Done" << endl;

	} else {
		cout << "Loading SA values (sequence length = " << this->seqLen << ") from " << saValuesFileName << " ... " << flush;
		unsigned int saValuesLen;
		this->saValues = readFileInt(saValuesFileName, saValuesLen, 0);
		cout << "Done" << endl;
	}
}

void SAKeys::freeMemory() {
	delete[] this->saKeys;
	delete[] this->saValues;
}

unsigned int *SAKeys::getSAKeys() {
	return this->saKeys;
}

unsigned int *SAKeys::getSAValues() {
	if (this->saValues == NULL) this->initializeSAValues();
	return this->saValues;
}

unsigned int SAKeys::getErrorSAValuesNumber(unsigned int *valuesToCheck) {
	if (this->saValues == NULL) this->initializeSAValues();
	cout << "Checking SA values consistency ... " << flush;
	unsigned int errorSAValuesNumber = 0;
	for (unsigned int i = 0; i < this->queriesNum * this->seqLen; ++i) {
		if (valuesToCheck[i] != this->saValues[i]) ++errorSAValuesNumber;
	}
	cout << "Done" << endl;
	return errorSAValuesNumber;
}

}