#ifndef PROCESS_H
#define PROCESS_H

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

using namespace std;

class Process {
	
	public:
		//Constructor
		Process(char* );
	
		//Functions
		void processConfig(char* filename);
		void print(); 
	
        // Variables
	    char* config_file; 
		string site_file;
		string search_file;
		int period_fetch; 
		int num_fetch; 
		int num_parse; 
		
};
#endif
