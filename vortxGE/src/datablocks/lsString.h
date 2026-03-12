/**
 * @file lsString.h
 * @brief String utility functions
 * 
 * Provides string manipulation utilities including parsing,
 * conversion, and file path handling.
 * 
 * @author vortxGE
 * @date 2024
 */

#ifndef LSSTRING_H_
#define LSSTRING_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

/**
 * @class lsString
 * @brief String conversion and manipulation utilities
 * 
 * Provides static utility methods for:
 * - Converting strings to numeric types
 * - Splitting strings by delimiters
 * - File path manipulation
 * 
 * @note Most methods are static - class is not instantiated
 * 
 * @example
 * @code
 * // Split string by comma
 * vector<string> parts;
 * lsString::SplitString("a,b,c", ',', parts);
 * 
 * // Convert string to number
 * int i = lsString().toInt("42");
 * float f = lsString().toFloat("3.14");
 * 
 * // Get path and filename without extension
 * string path = lsString().ExtractPath("/data/images/texture.png");
 * string name = lsString().RemoveExtension("texture.png");
 * @endcode
 */
class lsString
{
public:
	/// Default constructor
	lsString();
	
	/// Destructor
	virtual ~lsString();

    //=========================================================================
    // String Parsing
    //=========================================================================
    
    /**
     * @brief Split string by delimiter
     * 
     * @param input String to split
     * @param delimiter Character to split on
     * @param results Vector to store results
     * @return Number of parts split
     */
    static int SplitString( const string& input, char delimiter, vector<string>& results );
    
    /**
     * @brief Convert string to integer
     * @param input String to convert
     * @return Integer value
     */
	int toInt( const string& input );
    
    /**
     * @brief Convert string to float
     * @param input String to convert
     * @return Float value
     */
	float toFloat( const string& input  );
    
    /**
     * @brief Convert string to double
     * @param input String to convert
     * @return Double value
     */
	double toDouble( const string& input  );

    //=========================================================================
    // File Path Utilities
    //=========================================================================
    
    /**
     * @brief Remove file extension
     * 
     * @param input Filename
     * @return Filename without extension
     */
    string RemoveExtension( const string& input );
    
    /**
     * @brief Extract directory path
     * 
     * @param input Full file path
     * @return Directory portion (excluding filename)
     */
    string ExtractPath( const string& input );

};

#endif /* LSSTRING_H_ */
