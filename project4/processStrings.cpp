// word count file

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cctype>
#include <string>
#include <map>
#include <algorithm>
using namespace std;

int main() {
	char file1[50];

	//Ask for name of text file
	cout << "Enter the name of the text file: " << endl;
	cin >> file1;
	
	// create vector
	vector <string> appears;
	
	ifstream infile1(file1);
	
	//read input, keeping track of each word and how often it appears
	string s;
	while (infile1 >> s) {
	    // filter out punctuation
	    for(size_t i = 0; i < s.length(); ++i) {
            if(ispunct(s[i])) {
                s.erase(i--, 1);
            }
        }
		
		// add to vector if not present
		if(find(appears.begin(), appears.end(), s) == appears.end()) {
            appears.push_back(s);
        }
	}
	
	//closing file
	infile1.close();
	
	return 0;
}

	
