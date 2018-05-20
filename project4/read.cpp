// read input test file
// small class
// reads from a file and puts all things into a vector

// Includes --------------------------------------------------------------------

#include <iostream>
#include <functional>
#include <map>
#include <queue>
#include <string>
#include <set>
#include <sstream>
#include <vector>
#include <iterator>
#include <fstream>

#include <boost/algorithm/string/trim.hpp>

using namespace std;

class info {
    public:
        void readFileToVector(string filename){
            ifstream infile(filename);
            if (file.is_open()) {
                string line;
                while (getline(infile, line) {
                    istringstream iss(line);
                    string s;
                    if (!(iss >> s)) {
                        break;
                    }
                    // add string s to lines
                    lines.push_back(s);
                }
            }
            // print to test 
            for(size_t c = 0; c < lines.size() - 1; c++) {
	            cout << lines[c] << endl;
	        }
	        cout << lines[lines.size() - 1] << endl;
        }  
        
    private:
        vector<string> lines;
        
    
}
