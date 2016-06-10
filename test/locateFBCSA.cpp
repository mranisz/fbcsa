#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <map>
#include "../shared/patterns.h"
#include "../shared/timer.h"
#include "../fbcsa.h"

using namespace std;
using namespace fbcsa;

ChronoStopWatch timer;

map<string, HT::HTType> hashTypesMap = {{"hash", HT::STANDARD}, {"hash-dense", HT::DENSE}};

void fbcsaStd(string bs, string ss, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fbcsaLut2(string bs, string ss, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fbcsaHash(string bs, string ss, string hTType, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);

void getUsage(char **argv) {
	cout << "Select index you want to test (locate):" << endl;
	cout << "FBCSA: " << argv[0] << " std bs ss fileName patternNum patternLen" << endl;
        cout << "FBCSA-LUT2: " << argv[0] << " lut2 bs ss fileName patternNum patternLen" << endl;
        cout << "FBCSA-hash: " << argv[0] << " std bs ss hash|hash-dense k loadFactor fileName patternNum patternLen" << endl;
        cout << "where:" << endl;
        cout << "bs - block size (must be a multiple of 32)" << endl;
	cout << "ss - sampling step" << endl;
	cout << "fileName - name of text file" << endl;
	cout << "patternNum - number of patterns (queries)" << endl;
	cout << "patternLen - pattern length" << endl;
	cout << "k - suffix length to be hashed (k > 0)" << endl;
	cout << "loadFactor - load factor of hash table (range: (0.0, 1.0))" << endl << endl;
}

int main(int argc, char *argv[]) {
	if (argc < 7) {
		getUsage(argv);
		exit(1);
	}
        if ((string)argv[1] == "std") {
                if (argc == 7) fbcsaStd(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
                else if (argc == 10 && hashTypesMap.find(string(argv[4])) != hashTypesMap.end()) fbcsaHash(string(argv[2]), string(argv[3]), string(argv[4]), string(argv[5]), string(argv[6]), argv[7], atoi(argv[8]), atoi(argv[9]));
	}
        else if ((string)argv[1] == "lut2") {
                if (argc == 7) fbcsaLut2(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	}
        getUsage(argv);
        exit(1);
}

void fbcsaStd(string bs, string ss, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSA *fbcsa;
        string indexFileNameString = "FBCSA-" + (string)textFileName + "-" +  bs + "-" + ss + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa = new FBCSA();
		fbcsa->load(indexFileName);
	} else {
		fbcsa = new FBCSA(atoi(bs.c_str()), atoi(ss.c_str()));
		fbcsa->setVerbose(true);
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/fbcsa/" + string(textFileName) + "_locate_FBCSA.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsa->getIndexSize() / (double)fbcsa->getTextSize();
	cout << "locate FBCSA-" << bs << "-" << ss << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << bs << " " << ss << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete fbcsa;
	delete P;
        exit(0);
}

void fbcsaLut2(string bs, string ss, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSALut2 *fbcsaLut2;
        string indexFileNameString = "FBCSALut2-" + (string)textFileName + "-" +  bs + "-" + ss + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsaLut2 = new FBCSALut2();
		fbcsaLut2->load(indexFileName);
	} else {
		fbcsaLut2 = new FBCSALut2(atoi(bs.c_str()), atoi(ss.c_str()));
		fbcsaLut2->setVerbose(true);
		fbcsaLut2->build(textFileName);
		fbcsaLut2->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsaLut2->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/fbcsa/" + string(textFileName) + "_locate_FBCSALut2.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsaLut2->getIndexSize() / (double)fbcsaLut2->getTextSize();
	cout << "locate FBCSALut2-" << bs << "-" << ss << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << bs << " " << ss << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete fbcsaLut2;
	delete P;
        exit(0);
}

void fbcsaHash(string bs, string ss, string hTType, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSA *fbcsa;
        string indexFileNameString = "FBCSA-" + hTType + "-" + (string)textFileName + "-" +  bs + "-" + ss + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa = new FBCSA();
		fbcsa->load(indexFileName);
	} else {
		fbcsa = new FBCSA(atoi(bs.c_str()), atoi(ss.c_str()), hashTypesMap[hTType], atoi(k.c_str()), atof(loadFactor.c_str()));
		fbcsa->setVerbose(true);
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/fbcsa/" + string(textFileName) + "_locate_FBCSA-" + hTType + ".txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsa->getIndexSize() / (double)fbcsa->getTextSize();
	cout << "locate FBCSA-" << hTType << "-" << bs << "-" << ss << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << bs << " " << ss << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete fbcsa;
	delete P;
        exit(0);
}
