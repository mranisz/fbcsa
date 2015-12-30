#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include "shared/common.h"
#include "shared/sakeys.h"
#include "shared/timer.h"
#include "fbcsa.h"

using namespace std;
using namespace fbcsa;

ChronoStopWatch timer;

void fbcsaExtract(string bs, string ss, const char *textFileName, unsigned int seqNum, unsigned int seqLen);

void getUsage(char **argv) {
	cout << "Syntax:" << endl;
	cout << "./" << argv[0] << " bs ss fileName seqNum seqLen" << endl;
        cout << "where:" << endl;
        cout << "bs - block size (must be a multiple of 32)" << endl;
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
        fbcsaExtract(string(argv[1]), string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
}

void fbcsaExtract(string bs, string ss, const char *textFileName, unsigned int seqNum, unsigned int seqLen) {
        unsigned char* text = NULL;
	unsigned int textLen;
	FBCSA *fbcsa;
        string indexFileNameString = "FBCSA-" + (string)textFileName + "-" +  bs + "-" + ss + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		fbcsa = new FBCSA();
		fbcsa->load(indexFileName);
	} else {
		fbcsa = new FBCSA(atoi(bs.c_str()), atoi(ss.c_str()));
		fbcsa->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		fbcsa->build(text, textLen);
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
	cout << "extract FBCSA-" << bs << "-" << ss << " " << textFileName << " seqLen=" << seqLen << " queries=" << seqNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << seqLen << " " << seqNum << " " << bs << " " << ss << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = SAK->getErrorSAValuesNumber(saValues);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	if (text != NULL) delete[] text;
	delete[] saValues;
	delete fbcsa;
	delete SAK;
        exit(0);
}