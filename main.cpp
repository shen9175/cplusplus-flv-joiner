
#include"stdafx.h"

int main() {
	fstream out;
	
	vector<string> videolist(6);
	for (int i = 0; i < 6; ++i) {
		//errno = 0;
		string name = "c:\\123\\" + to_string(i) + ".flv";
		cout << "file name is " << name << endl;
		out.open(name.c_str(), ifstream::in | ifstream::binary);
		if (!out.fail()) {
			stringstream strm;
			strm << out.rdbuf();
			videolist[i] = strm.str();
			out.close();
		} else {
			const char * errorStr = strerror(errno);
			cout << "open file failed: " << errorStr << endl;
			out.close();
			return -1;
		}
	}
	string whole;
	concat_flv(videolist, whole);
	out.open("full.flv", ofstream::out | ofstream::binary);
	if (!out.fail()) {
		out << whole;
		out.close();
	} else {
		const char * errorStr = strerror(errno);
		cout << "create file failed!" << errorStr << endl;
		out.close();
	}
	return 0;
}