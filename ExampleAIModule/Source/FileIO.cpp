#include "FileIO.h"
#include <iostream>
#include <fstream>

using namespace std;

bool FileIO::readStatsCoordinates(int StatsCoordinates[][2])
{   //simple functions useful for readings the settings for displaying the stats
    string line;
    ifstream myfile("StatsCoordinates.txt");
    if (myfile.is_open())
    {
        int index = 0;
        while (getline(myfile, line))
        {
            istringstream iss(line);
            vector<string> results(istream_iterator<string>{iss},
                istream_iterator<string>());

            for (int i = 0; i < results.size() - 1; i++)
            {
                StatsCoordinates[index][i] = stoi(results[i]);
            }
            index += 1;
            //cout << line << '\n';
            //Broodwar->sendText(line.c_str());
        }
        myfile.close();
        return true;
    }
    else return false;        
}