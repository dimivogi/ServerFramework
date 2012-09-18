#include	"slimReadWriterLock.hpp"



namespace	DawnEngine
{

	namespace	Concurrency
	{

		//	The assignment operator. It does nothing. Exists only to prevent bit-wise copying.
		SlimReadWriterLock&	SlimReadWriterLock::operator=( const SlimReadWriterLock& )	{ return *this; };


		//	 The default constructor initialising the lock.
		SlimReadWriterLock::SlimReadWriterLock()	:	
			_shared_locks(0) , 
			_locked(false) , 
			_exclusively_locked(false)													{ _initialise_locks(); };

		//	 The copy constructor initialising the lock. It creates a NEW lock.
		SlimReadWriterLock::SlimReadWriterLock( const SlimReadWriterLock& )	:	
			_shared_locks(0) , 
			_locked(false) , 
			_exclusively_locked(false)													{ _initialise_locks(); };

		//	The destructor deinitialising the lock.
		SlimReadWriterLock::~SlimReadWriterLock()										{ _deinitialise_locks(); };

	}	/* Concurrency */

}	/* DawnEngine */