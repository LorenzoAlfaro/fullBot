#pragma once
#include "FileIO.h"
#include <iostream>
#include <fstream>
#include <BWAPI.h>
class FileIO
{
public:
	static bool readStatsCoordinates(int StatsCoordinates[][2]);

	static void FileIO::WriteHeaders();
};

