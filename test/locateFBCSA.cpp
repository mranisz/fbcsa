#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <map>
#include "../shared/patterns.hpp"
#include "../shared/timer.hpp"
#include "../fbcsa.hpp"

using namespace std;
using namespace shared;
using namespace fbcsa;

ChronoStopWatch timer;

void fbcsa32(string ss, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fbcsa64(string ss, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fbcsaHyb32(string ss, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fbcsaHyb64(string ss, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fbcsa32Lut2(string ss, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fbcsa64Lut2(string ss, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fbcsa32Hash(string ss, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fbcsa64Hash(string ss, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fbcsa32HashDense(string ss, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fbcsa64HashDense(string ss, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);

void getUsage(char **argv) {
	cout << "Select index you want to test (locate):" << endl;
	cout << "FBCSA-32: " << argv[0] << " 32 ss fileName patternNum patternLen" << endl;
	cout << "FBCSA-64: " << argv[0] << " 64 ss fileName patternNum patternLen" << endl;
	cout << "FBCSAHyb-32: " << argv[0] << " 32-hyb ss fileName patternNum patternLen" << endl;
	cout << "FBCSAHyb-64: " << argv[0] << " 64-hyb ss fileName patternNum patternLen" << endl;
    cout << "FBCSA-32-LUT2: " << argv[0] << " 32-lut2 ss fileName patternNum patternLen" << endl;
	cout << "FBCSA-64-LUT2: " << argv[0] << " 64-lut2 ss fileName patternNum patternLen" << endl;
    cout << "FBCSA-32-hash: " << argv[0] << " 32-hash ss k loadFactor fileName patternNum patternLen" << endl;
	cout << "FBCSA-64-hash: " << argv[0] << " 64-hash ss k loadFactor fileName patternNum patternLen" << endl;
	cout << "FBCSA-32-hash-dense: " << argv[0] << " 32-hash-dense ss k loadFactor fileName patternNum patternLen" << endl;
	cout << "FBCSA-64-hash-dense: " << argv[0] << " 64-hash-dense ss k loadFactor fileName patternNum patternLen" << endl;
    cout << "where:" << endl;
	cout << "ss - sampling step" << endl;
	cout << "fileName - name of text file" << endl;
	cout << "patternNum - number of patterns (queries)" << endl;
	cout << "patternLen - pattern length" << endl;
	cout << "k - suffix length to be hashed (k > 0)" << endl;
	cout << "loadFactor - load factor of hash table (range: (0.0, 1.0))" << endl << endl;
}

int main(int argc, char *argv[]) {
	if (argc < 6) {
		getUsage(argv);
		exit(1);
	}
    if (string(argv[1]) == "32") fbcsa32(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	if (string(argv[1]) == "64") fbcsa64(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	if (string(argv[1]) == "32-hyb") fbcsaHyb32(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	if (string(argv[1]) == "64-hyb") fbcsaHyb64(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	if (string(argv[1]) == "32-lut2") fbcsa32Lut2(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	if (string(argv[1]) == "64-lut2") fbcsa64Lut2(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	if (string(argv[1]) == "32-hash") fbcsa32Hash(string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
	if (string(argv[1]) == "64-hash") fbcsa64Hash(string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
	if (string(argv[1]) == "32-hash-dense") fbcsa32HashDense(string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
	if (string(argv[1]) == "64-hash-dense") fbcsa64HashDense(string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
	getUsage(argv);
	exit(1);
}

void fbcsa32(string ss, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSA<32> *fbcsa = new FBCSA<32>(atoi(ss.c_str()));
    string indexFileNameString = "FBCSA-" + (string)textFileName + "-32-" + ss + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa->load(indexFileName);
	} else {
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_FBCSA.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsa->getIndexSize() / (double)fbcsa->getTextSize();
	cout << "locate FBCSA-32-" << ss << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 32 " << ss << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete fbcsa;
	delete P;
    exit(0);
}

void fbcsa64(string ss, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSA<64> *fbcsa = new FBCSA<64>(atoi(ss.c_str()));
    string indexFileNameString = "FBCSA-" + (string)textFileName + "-64-" + ss + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa->load(indexFileName);
	} else {
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_FBCSA.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsa->getIndexSize() / (double)fbcsa->getTextSize();
	cout << "locate FBCSA-64-" << ss << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 64 " << ss << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete fbcsa;
	delete P;
    exit(0);
}

void fbcsaHyb32(string ss, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSAHyb<32> *fbcsa = new FBCSAHyb<32>(atoi(ss.c_str()));
    string indexFileNameString = "FBCSAHyb-" + (string)textFileName + "-32-" + ss + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa->load(indexFileName);
	} else {
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_FBCSAHyb.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsa->getIndexSize() / (double)fbcsa->getTextSize();
	cout << "locate FBCSAHyb-32-" << ss << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 32 " << ss << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete fbcsa;
	delete P;
    exit(0);
}

void fbcsaHyb64(string ss, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSAHyb<64> *fbcsa = new FBCSAHyb<64>(atoi(ss.c_str()));
    string indexFileNameString = "FBCSAHyb-" + (string)textFileName + "-64-" + ss + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa->load(indexFileName);
	} else {
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_FBCSAHyb.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsa->getIndexSize() / (double)fbcsa->getTextSize();
	cout << "locate FBCSAHyb-64-" << ss << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 64 " << ss << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete fbcsa;
	delete P;
    exit(0);
}

void fbcsa32Lut2(string ss, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSALut2<32> *fbcsaLut2 = new FBCSALut2<32>(atoi(ss.c_str()));
    string indexFileNameString = "FBCSALut2-" + (string)textFileName + "-32-" + ss + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsaLut2->load(indexFileName);
	} else {
		fbcsaLut2->build(textFileName);
		fbcsaLut2->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsaLut2->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_FBCSALut2.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsaLut2->getIndexSize() / (double)fbcsaLut2->getTextSize();
	cout << "locate FBCSALut2-32-" << ss << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 32 " << ss << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete fbcsaLut2;
	delete P;
    exit(0);
}

void fbcsa64Lut2(string ss, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSALut2<64> *fbcsaLut2 = new FBCSALut2<64>(atoi(ss.c_str()));
    string indexFileNameString = "FBCSALut2-" + (string)textFileName + "-64-" + ss + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsaLut2->load(indexFileName);
	} else {
		fbcsaLut2->build(textFileName);
		fbcsaLut2->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsaLut2->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_FBCSALut2.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsaLut2->getIndexSize() / (double)fbcsaLut2->getTextSize();
	cout << "locate FBCSALut2-64-" << ss << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 64 " << ss << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete fbcsaLut2;
	delete P;
    exit(0);
}

void fbcsa32Hash(string ss, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSAHash<32, HT_STANDARD> *fbcsa = new FBCSAHash<32, HT_STANDARD>(atoi(ss.c_str()), atoi(k.c_str()), atof(loadFactor.c_str()));
    string indexFileNameString = "FBCSA-hash-" + (string)textFileName + "-32-" + ss + "-" + k + "-" + loadFactor + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa->load(indexFileName);
	} else {
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_FBCSA-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsa->getIndexSize() / (double)fbcsa->getTextSize();
	cout << "locate FBCSA-hash-32-" << ss << "-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 32 " << ss << " " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete fbcsa;
	delete P;
    exit(0);
}

void fbcsa64Hash(string ss, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSAHash<64, HT_STANDARD> *fbcsa = new FBCSAHash<64, HT_STANDARD>(atoi(ss.c_str()), atoi(k.c_str()), atof(loadFactor.c_str()));
    string indexFileNameString = "FBCSA-hash-" + (string)textFileName + "-64-" + ss + "-" + k + "-" + loadFactor + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa->load(indexFileName);
	} else {
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_FBCSA-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsa->getIndexSize() / (double)fbcsa->getTextSize();
	cout << "locate FBCSA-hash-64-" << ss << "-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 64 " << ss << " " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete fbcsa;
	delete P;
    exit(0);
}

void fbcsa32HashDense(string ss, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSAHash<32, HT_DENSE> *fbcsa = new FBCSAHash<32, HT_DENSE>(atoi(ss.c_str()), atoi(k.c_str()), atof(loadFactor.c_str()));
    string indexFileNameString = "FBCSA-hash-dense-" + (string)textFileName + "-32-" + ss + "-" + k + "-" + loadFactor + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa->load(indexFileName);
	} else {
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_FBCSA-hash-dense.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsa->getIndexSize() / (double)fbcsa->getTextSize();
	cout << "locate FBCSA-hash-dense-32-" << ss << "-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 32 " << ss << " " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete fbcsa;
	delete P;
    exit(0);
}

void fbcsa64HashDense(string ss, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FBCSAHash<64, HT_DENSE> *fbcsa = new FBCSAHash<64, HT_DENSE>(atoi(ss.c_str()), atoi(k.c_str()), atof(loadFactor.c_str()));
    string indexFileNameString = "FBCSA-hash-dense-" + (string)textFileName + "-64-" + ss + "-" + k + "-" + loadFactor + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa->load(indexFileName);
	} else {
		fbcsa->build(textFileName);
		fbcsa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		fbcsa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_FBCSA-hash-dense.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)fbcsa->getIndexSize() / (double)fbcsa->getTextSize();
	cout << "locate FBCSA-hash-dense-64-" << ss << "-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 64 " << ss << " " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete fbcsa;
	delete P;
    exit(0);
}