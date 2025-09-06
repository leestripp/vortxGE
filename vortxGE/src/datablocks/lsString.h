#ifndef LSSTRING_H_
#define LSSTRING_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

//! String conversion
/*!
 * Split strings, convert to int and float etc...
 *
 */
class lsString
{
public:
	lsString();
	virtual ~lsString();

    static int SplitString( const string& input, char delimiter, vector<string>& results );
	int toInt( const string& input );
	float toFloat( const string& input  );
	double toDouble( const string& input  );

    // File utils
    string RemoveExtension( const string& input );
    string ExtractPath( const string& input );

};

#endif /* LSSTRING_H_ */
