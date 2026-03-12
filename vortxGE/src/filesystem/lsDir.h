/**
 * @file lsDir.h
 * @brief Directory utility functions
 * 
 * Provides directory and path utilities, primarily for
 * finding the executable's path.
 * 
 * @author vortxGE
 * @date 2024
 */

#ifndef LSDIR_H_
#define LSDIR_H_

#include <limits.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>


using namespace std;

/**
 * @class lsDir
 * @brief Directory utility class
 * 
 * Provides utilities for working with directories and paths.
 * Primarily used to find the application's executable path
 * for locating resources.
 * 
 * @note Platform-specific: Uses Linux/POSIX functions
 */
class lsDir
{
public:
	/// Default constructor
  lsDir();
  
  /// Destructor
  virtual ~lsDir();

  //=========================================================================
  // Path Operations
  //=========================================================================
  
  /**
   * @brief Get executable path
   * 
   * Returns the full path to the current executable.
   * Useful for locating resources relative to the application.
   * 
   * @return Full path to executable
   */
  string getexepath();

};

#endif
