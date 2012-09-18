#include	"../globalDefinitions.hpp"
#include	<Windows.h>
#include	<process.h>
#include	"../Lock/slimReadWriterLock.hpp"

#ifndef		_DAWN_ENGINE_THREAD_HPP_
	#define	_DAWN_ENGINE_THREAD_HPP_



	namespace	DawnEngine
	{

		namespace	Parallel
		{

			/*
				Type definitions.
			*/
			typedef	unsigned int (*threadFunction)( void* );


			/*
				A class representing and handling a thread.
			*/
			class	Thread
			{
				private:

					//	A lock used to secure any changes in the various attributes of the class.
					mutable Concurrency::SlimReadWriterLock	_lock;
					//	A variable containing the id of the thread.
					unsigned int							_id;
					//	A handle to the created thread.
					HANDLE									_handle;
					//	A pointer to the function that contains the functionality of the thread.
					threadFunction							_function;
					//	A pointer to the parameters that the function providing the functionality of the thread, should have access to.
					void*									_parameter;
					//	 A variable that reflects the run state of the thread.
					volatile bool							_run;


					//	The static function used by the class to run the thread.
					static unsigned int						__stdcall _static_thread_function( void* parameter );


					//	The assignment operator. It does nothing. Exists to prevent bit-wise copying.
					Thread&									operator=( const Thread& thread );


				public:

					//	The default constructor of the class.
					explicit Thread( threadFunction function = NULL , void* parameter = NULL );
					//	 The copy constructor of the class. It creates a NEW thread entity.
					Thread( const Thread& thread );
					//	The destructor of the class.
					~Thread();


					//	A function responsible for setting the run variable for the thread.
					void									run( const bool value );
					//	A function responsible for setting the function that contains the functionality of the thread. 
					void									function( const threadFunction function );
					//	A function responsible for setting the parameter pointer that will be passed as an argument to the run function.
					void									parameter( void* parameter );

				
					//	A function returning the id of the thread.
					unsigned int							id() const;
					//	A function returning the value of the run variable.
					bool									run() const;
					//	A function returning the run function of the thread.
					threadFunction							function() const;
					//	A function returning the parameter of the thread.
					void*									parameter() const;


					//	A function responsible for creating and running the thread.
					void									create();
					//	A function responsible for closing the thread.
					DWORD									destroy();
			};



			/*
				Function definitions.
			*/


			//	The static function used by the class to run the thread.
			inline unsigned int	Thread::_static_thread_function( void* parameter )
			{
				unsigned int	return_value = 0;



				if ( parameter )
				{
					Thread*	thread = static_cast<Thread*>(parameter);



					if ( thread  &&  thread->function() )
						return_value = thread->function()(parameter);
				}


				return return_value;
			}

			//	A function responsible for setting the run variable for the thread.
			inline void	Thread::run( const bool value )
			{
				_lock.acquire();
				_run = value;
				_lock.release();
			};
		
			//	A function responsible for setting the function that contains the functionality of the thread. 
			inline void	Thread::function( const threadFunction function )
			{
				_lock.acquire();
				_function = function;
				_lock.release();
			};
		
			//	A function responsible for setting the parameter pointer that will be passed as an argument to the run function.
			inline void	Thread::parameter( void* parameter )
			{
				_lock.acquire();
				_parameter = parameter;
				_lock.release();
			};

		
			//	A function returning the id of the thread.
			inline unsigned int	Thread::id() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _id;
				_lock.release_shared();


				return return_value;
			};

			//	A function returning the value of the run variable.
			inline bool	Thread::run() const
			{
				bool	return_value = false;



				_lock.acquire_shared();
				return_value = _run;
				_lock.release_shared();


				return return_value;
			};
		
			//	A function returning the run function of the thread.
			inline threadFunction	Thread::function() const
			{
				threadFunction	return_value = NULL;



				_lock.acquire_shared();
				return_value = _function;
				_lock.release_shared();


				return return_value;
			};

			//	A function returning the parameter of the thread.
			inline void*	Thread::parameter() const
			{
				void*	return_value = NULL;



				_lock.acquire_shared();
				return_value = _parameter;
				_lock.release_shared();


				return return_value;
			};


			//	A function responsible for creating and running the thread.
			inline void	Thread::create()
			{
				_lock.acquire();

				if ( _handle == NULL )
					_handle = reinterpret_cast<HANDLE>(_beginthreadex(NULL,0,_static_thread_function,static_cast<void*>(this),0,&_id));

				_lock.release();
			};
		
			//	A function responsible for closing the thread.
			inline DWORD	Thread::destroy()
			{
				DWORD	return_value = 0;



			
				_lock.acquire_shared();

				if ( _handle != NULL )
				{
					_lock.release_shared();

					WaitForSingleObject(_handle,INFINITE);
					GetExitCodeThread(_handle,&return_value);
				
					_lock.acquire();
					CloseHandle(_handle);
					_handle = NULL;
					_lock.release();
				}
				else
					_lock.release_shared();

			


				return return_value;
			};

		}	/* Parallel */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_THREAD_HPP_ */