#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include "../shared/patterns.hpp"
#include "../shared/timer.hpp"
#include "../fbcsa.hpp"

using namespace std;
using namespace shared;
using namespace fbcsa;

ChronoStopWatch timer;

void fbcsa32(string ss, const char *textFileName, unsigned int seqNum, unsigned int seqLen);
void fbcsa64(string ss, const char *textFileName, unsigned int seqNum, unsigned int seqLen);

void getUsage(char **argv) {
	cout << "Select index you want to test (extract):" << endl;
	cout << "FBCSA-32: " << argv[0] << " 32 ss fileName seqNum seqLen" << endl;
	cout << "FBCSA-64: " << argv[0] << " 64 ss fileName seqNum seqLen" << endl;
	cout << "ss - sampling step" << endl;
	cout << "fileName - name of text file" << endl;
	cout << "seqNum - number of SA keys sequences (queries)" << endl;
	cout << "seqLen - sequence length" << endl << endl;
}

int main(int argc, char *argv[]) {
	if (argc < 6) {
		getUsage(argv);
		exit(1);
	}
    if (string(argv[1]) == "32") fbcsa32(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	if (string(argv[1]) == "64") fbcsa64(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	getUsage(argv);
	exit(1);
}

void fbcsa32(string ss, const char *textFileName, unsigned int seqNum, unsigned int seqLen) {
	FBCSA<32> *fbcsa = new FBCSA<32>(atoi(ss.c_str()));
    string indexFileNameString = "FBCSA-" + (string)textFileName + "-32-" + ss + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa->load(indexFileName);
	} else {
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	SAKeys *SAK = new SAKeys(textFileName, seqNum, seqLen);
	unsigned int *saKeys = SAK->getSAKeys();
	unsigned int *saValues = new unsigned int[seqNum * seqLen];

	if (seqLen == 1) {
			timer.startTimer();
			for (unsigned int i = 0; i < seqNum; ++i) {
					saValues[i] = fbcsa->extract(saKeys[i]);
			}
			timer.stopTimer();
	} else {
			timer.startTimer();
			for (unsigned int i = 0; i < seqNum; ++i) {
					fbcsa->extractSeq(saKeys[i], seqLen, saValues + (i * seqLen));
			}
			timer.stopTimer();
	}

	string resultFileName = "results/fbcsa/" + string(textFileName) + "_extract_FBCSA.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsa->getIndexSize() / (double)fbcsa->getTextSize();
	cout << "extract FBCSA-32-" << ss << " " << textFileName << " seqLen=" << seqLen << " queries=" << seqNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << seqLen << " " << seqNum << " 32 " << ss << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = SAK->getErrorSAValuesNumber(saValues);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] saValues;
	delete fbcsa;
	delete SAK;
    exit(0);
}

void fbcsa64(string ss, const char *textFileName, unsigned int seqNum, unsigned int seqLen) {
	FBCSA<64> *fbcsa = new FBCSA<64>(atoi(ss.c_str()));
    string indexFileNameString = "FBCSA-" + (string)textFileName + "-64-" + ss + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa->load(indexFileName);
	} else {
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	SAKeys *SAK = new SAKeys(textFileName, seqNum, seqLen);
	unsigned int *saKeys = SAK->getSAKeys();
	unsigned int *saValues = new unsigned int[seqNum * seqLen];

	if (seqLen == 1) {
			timer.startTimer();
			for (unsigned int i = 0; i < seqNum; ++i) {
					saValues[i] = fbcsa->extract(saKeys[i]);
			}
			timer.stopTimer();
	} else {
			timer.startTimer();
			for (unsigned int i = 0; i < seqNum; ++i) {
					fbcsa->extractSeq(saKeys[i], seqLen, saValues + (i * seqLen));
			}
			timer.stopTimer();
	}

	string resultFileName = "results/fbcsa/" + string(textFileName) + "_extract_FBCSA.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsa->getIndexSize() / (double)fbcsa->getTextSize();
	cout << "extract FBCSA-64-" << ss << " " << textFileName << " seqLen=" << seqLen << " queries=" << seqNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << seqLen << " " << seqNum << " 64 " << ss << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = SAK->getErrorSAValuesNumber(saValues);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] saValues;
	delete fbcsa;
	delete SAK;
    exit(0);
}