// Process.cpp

#include <iostream>
#include <math.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <cctype>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <climits>
#include <stdlib.h>
#include "Process.h"

using namespace std;


//Base class 
Process::Process(char* configFile) {
	config_file = configFile;
	
}

void Process::processConfig(char* filename) {
    vector <string> data;    
    
    ifstream infile(filename);
    string line;
    while (getline(infile, line)) {
        stringstream ss(line);
        string temp = "";
        
        getline(ss, temp, '=');
        getline(ss, temp, '\n');
        
        // now temp is the value to the left of the equal sign
        cout << temp << endl;
        
        data.push_back(temp);
    }
    
    // store data in variables
    period_fetch = stoi(data[0]);
    num_fetch = stoi(data[1]);
    num_parse = stoi(data[2]);
    search_file = data[3];
    site_file = data[4];
    
}

//Print Function
void Process::print() {

	cout << "PERIOD_FETCH: " << period_fetch << endl;
	cout << "NUM_FETCH: " << num_fetch << endl;
	cout << "NUM_PARSE: " << num_parse << endl; 
	cout << "SEARCH_FILE: " << search_file << endl; 
	cout << "SITE_FILE: " << site_file << endl;
	cout << endl;
	
}

