#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>

using namespace std;

template<unsigned int N>
bool inc_and_rollover(char (&a)[N], int index)
{
    if(index < 0) return false;
    a[index] = (a[index] + 1) % 26;
    if(!a[index]) {
	return inc_and_rollover(a, index - 1);
    }
    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
	return 1;
    }
    unsigned int n = strtoul(argv[1], NULL, 10);
    vector<string> names;
    char buf[5] = {0};
    char sbuf[sizeof(buf)] = {0};
    unsigned int last = 0;
    for(unsigned int i = 0; i < n; ++i) {
	for(unsigned int j = 0; j <= last; ++j) {
	    sbuf[j] = buf[j] + 'a';
	}
	names.push_back(sbuf);
	if(!inc_and_rollover(buf, last)) {
	    ++last;
	}
    }
    ostringstream oss;
    oss << "module_" << n << "_exports.jsm";
    ofstream o(oss.str().c_str());
    o << "// " << oss.str() << endl << endl;
    o << "var EXPORTED_SYMBOLS = [";
    for(vector<string>::const_iterator itr = names.begin(); itr != names.end();
	++itr) {
	if (itr != names.begin() ) {
	    o << ", ";
	}
	o << "\"" << *itr << "\"";
    }
    o << "];" << endl << endl;
    for(vector<string>::const_iterator itr = names.begin(); itr != names.end();
	++itr) {
	o << "function " << *itr << "() {}" << endl << endl;
    }
    return 0;
}
