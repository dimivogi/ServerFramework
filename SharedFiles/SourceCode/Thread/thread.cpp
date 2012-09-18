#include	"thread.hpp"



namespace	DawnEngine
{

	namespace	Parallel
	{
		//	The assignment operator. It does nothing. Exists to prevent bit-wise copying.
		Thread&	Thread::operator=( const Thread& )	{ return *this; };				


		//	The default constructor of the class.
		Thread::Thread( threadFunction function , void* parameter )	:	
			_lock() , 
			_id(0) , 
			_handle(NULL) , 
			_function(function) , 
			_parameter(parameter) , 
			_run(true)								{};

		//	 The copy constructor of the class. It creates a NEW thread entity.
		Thread::Thread( const Thread& thread )	:	
			_lock() ,
			_id(0) , 
			_handle(NULL) , 
			_function(thread.function()) , 
			_parameter(thread.parameter()) , 
			_run(true)								{};

		//	The destructor of the class.
		Thread::~Thread()
		{
			_lock.acquire();

			if ( _handle != NULL )
			{
				_lock.release();
				run(false);
				destroy();
			}
			else
				_lock.release();
		};

	}	/* Parallel */

}	/* DawnEngine */