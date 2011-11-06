#pragma once

#include <cstdio>

bool readFileToString(const char* filename, std::string& result)
{
	FILE* file = fopen(filename, "r");
	if (file == NULL)
		return false;
	fseek(file, 0, SEEK_END);
	fpos_t size;
	fgetpos(file, &size);
	fseek(file, 0, SEEK_SET);
	int filesize = size;
	result.resize(filesize+1);
	fread(&result[0], sizeof(char), filesize, file);
	fclose(file);
	return true;
}
