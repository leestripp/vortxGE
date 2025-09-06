#include "lsString.h"

lsString::lsString()
{
	
}

lsString::~lsString()
{
}

int lsString::SplitString( const string& input, char delimiter, vector<string>& results )
{
    stringstream ss;
    ss.str(input);
    string item;

    while( getline(ss, item, delimiter) )
    {
        results.push_back( item );
    }

    return results.size();
}

int lsString::toInt( const string& input )
{
	int result = 0;
	
	istringstream ss( input );
	ss >> result;
	
	return( result );
}


float lsString::toFloat( const string& input )
{
	float result = 0;
	
	stringstream ss;
	ss << input;
	ss >> result;
	
	return( result );
}

double lsString::toDouble( const string& input  )
{
	double result = 0;
	
	stringstream ss;
	ss << input;
	ss >> result;
	
	return( result );
}


string lsString::RemoveExtension(const string &input)
{
    // get location
    size_t lastindex = input.find_last_of( "." );
    string rawname = input.substr( 0, lastindex );

    return rawname;
}

string lsString::ExtractPath(const string &input)
{
    size_t lastindex = input.find_last_of( "/" );
    if( lastindex == string::npos )
    {
        return "";
    }
    string rawpath = input.substr( 0, lastindex );

    return rawpath;
}


