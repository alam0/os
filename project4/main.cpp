// main.cpp
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


int main(int argc, char *argv[]) {
    
    char filename[50];
    cout << "Enter the name of the config text file: " << endl;
	cin >> filename;
    
    // instantiate class
    Process proc(filename); 
    
    proc.processConfig(proc.config_file);
    
    proc.print();
    
    
    return 0;

}
