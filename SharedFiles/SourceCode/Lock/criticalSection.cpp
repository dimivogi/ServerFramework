#include	"criticalSection.hpp"



namespace	DawnEngine
{

	namespace	Concurrency
	{

		//	The assignment operator. It does nothing. Exists only to prevent bit-wise copying.
		CriticalSection&	CriticalSection::operator=( const CriticalSection& )	{ return *this; }


		//	The default constructor initialising the critical section.
		CriticalSection::CriticalSection()	:	
			_spin_count(0) ,
			_lock_count(0)															{ _initialise_critical_sections(); };
	
		//	The copy constructor initialising the critical section. It creates a NEW critical section.
		CriticalSection::CriticalSection( const CriticalSection& section )	:	
			_spin_count(section.spin_count()) , 
			_lock_count(0)															{ _initialise_critical_sections(); };

		//	The destructor deinitialising the critical section.
		CriticalSection::~CriticalSection()											{ _deinitialise_critical_sections(); };

	}	/* Concurrency */

}	/* DawnEngine */