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
