#include "parse.h"

using namespace std;

void sequence(input::Sequences& sequences) {
	string seq = sequences.retrieve();
	while (seq != "finished") {
		cout << seq << endl;
	}
}
