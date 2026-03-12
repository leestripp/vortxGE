/**
 * @file lsFileIO.h
 * @brief File input/output operations
 * 
 * Provides file reading and writing functionality for both
 * text and binary files.
 * 
 * @author vortxGE
 * @date 2024
 */

#ifndef LSFILEIO_H_
#define LSFILEIO_H_


#include <fstream>
#include <iostream>
#include <string>


// File modes
const int LSIO_BIN_APPEND	=	0;	///< Binary: append to file
const int LSIO_BIN_CLEAR	=	1;	///< Binary: clear/create new file
const int LSIO_LINE_APPEND	=	2;	///< Text: append line
const int LSIO_LINE_CLEAR	=	3;	///< Text: clear/create new file
const int LSIO_LINE_READ	=	4;	///< Text: read lines
const int LSIO_BIN_READ		=	5;	///< Binary: read bytes

// Line buffer
const int LSIO_LINE_BUFFERSIZE	=	2048;	///< Default line buffer size


using namespace std;

/**
 * @class lsFileIO
 * @brief File I/O wrapper
 * 
 * Provides unified file access for reading and writing:
 * - Text files (line by line)
 * - Binary files (byte/block)
 * 
 * Data is stored internally after reading - use getLine() or getByte()
 * to access the data. For larger blocks, use readblock()/writeblock().
 * 
 * @note RAII-style: file is closed when object goes out of scope
 * 
 * @example
 * @code
 * // Read text file
 * lsFileIO file;
 * if (file.open("data.txt", LSIO_LINE_READ)) {
 *     while (file.readline()) {
 *         cout << file.getLine() << endl;
 *     }
 * }
 * 
 * // Write binary file
 * lsFileIO out;
 * if (out.open("data.bin", LSIO_BIN_CLEAR)) {
 *     out.writeblock(buffer, size);
 * }
 * @endcode
 */
class lsFileIO
{
public:
	/// Default constructor
	lsFileIO();
	
	/// Destructor - closes file if open
	virtual ~lsFileIO();

    //=========================================================================
    // File Operations
    //=========================================================================
	
    /**
     * @brief Open file
     * 
     * @param file File path
     * @param mode Open mode (LSIO_* constant)
     * @return true if opened successfully
     */
	bool open( const string& file, int mode );
	
    /**
     * @brief Read next line
     * 
     * @return true if line read successfully
     */
	bool readline();
	
    /**
     * @brief Write line
     * 
     * @param line Text to write
     * @return true if written successfully
     */
	bool writeline( const string& line );
	
    /**
     * @brief Read next byte
     * @return true if byte read
     */
	bool readbyte();
	
    /**
     * @brief Write byte
     * 
     * @param c Byte to write
     * @param mode Write mode
     * @return true if written
     */
	bool writebyte( const char& c, int mode );
	
    /**
     * @brief Read block of data
     * 
     * @param buff Buffer to store data
     * @param size Number of bytes to read
     * @return true if read successfully
     */
	bool readblock( char* buff, int size );
	
    /**
     * @brief Write block of data
     * 
     * @param buff Data to write
     * @param size Number of bytes to write
     * @return true if written
     */
	bool writeblock( char* buff, int size );

    //=========================================================================
    // Accessors
    //=========================================================================
	
    /**
     * @brief Get last read line
     * @return Line string
     */
	string getLine() const
	{
		return line;
	}

	/**
	 * @brief Get last read byte
	 * @return Byte value
	 */
	char getByte() const
	{
		return ch;
	}

	/**
	 * @brief Get bytes read in last operation
	 * @return Number of bytes
	 */
	int getBytesRead() const
	{
		return bytesread;
	}

private:
	/// Current filename
	string filename;
	
	// Line reading
	string line;	///< Current line buffer
	char buffer[LSIO_LINE_BUFFERSIZE];	///< Line buffer
	
	// Byte reading
	char ch;	///< Current byte
	
	// Block reading
	int bytesread;	///< Bytes read in last operation

	// File handles
	ifstream infile;	///< Input file stream
	ofstream outfile;	///< Output file stream
	int filemode;	///< Current file mode

};

#endif /* LSFILEIO_H_ */
