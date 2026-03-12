#ifndef LSFILEIO_H_
#define LSFILEIO_H_


#include <fstream>
#include <iostream>
#include <string>


// File modes
const int LSIO_BIN_APPEND	=	0;
const int LSIO_BIN_CLEAR	=	1;
const int LSIO_LINE_APPEND	=	2;
const int LSIO_LINE_CLEAR	=	3;
const int LSIO_LINE_READ	=	4;
const int LSIO_BIN_READ		=	5;

// Line buffer
const int LSIO_LINE_BUFFERSIZE	=	2048;


using namespace std;

//! File access
/*!
 * Read and write a file. Data is stored inside the class after a read, you need to call
 * getLine() and getByte() to access the data.
 *
 * readblock() and writeblock() work directly on your own buffer. call getBytesRead() for any error checking.
 *
 * Its best to use this inside a function, so the file is closed when it leaves scope.
 *
 */
class lsFileIO
{
public:
	lsFileIO();
	virtual ~lsFileIO();

	bool open( const string& file, int mode );
	bool readline();
	bool writeline( const string& line );
	bool readbyte();
	bool writebyte( const char& c, int mode );
	bool readblock( char* buff, int size );
	bool writeblock( char* buff, int size );

	string getLine() const
	{
		return line;
	}

	char getByte() const
	{
		return ch;
	}

	int getBytesRead() const
	{
		return bytesread;
	}

private:
	string filename;
	// lines
	string line;
	char buffer[LSIO_LINE_BUFFERSIZE];
	// bytes
	char ch;
	// blocks
	int bytesread;

	// files
	ifstream infile;
	ofstream outfile;
	int filemode;

};

#endif /* LSFILEIO_H_ */
