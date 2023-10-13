/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef vortxGE_lsData_h
#define vortxGE_lsData_h

#include <iostream>
#include <string>

using namespace std;

class lsData
{
public:
    lsData();
    virtual ~lsData();
    
    // Memory
    unsigned int getMem_used() const
    {
        return mem_used;
    }
    
    void setMem_used( unsigned int val )
    {
        mem_used = val;
    }
    
    void addMem_used( unsigned int val )
	{
		mem_used += val;
	}
	
    
private:
    unsigned int mem_used;
    
};


#endif
