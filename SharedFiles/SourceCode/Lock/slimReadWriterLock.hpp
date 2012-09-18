#include	"../globalDefinitions.hpp"
#include	<Windows.h>
#include	"conditionVariable.hpp"

#ifndef		_DAWN_ENGINE_SLIM_READ_WRITER_LOCK_HPP_
	#define	_DAWN_ENGINE_SLIM_READ_WRITER_LOCK_HPP_



	namespace	DawnEngine
	{

		namespace	Concurrency
		{

			/*
				A class representing and handling a SlimReadWriter lock.
			*/
			class	SlimReadWriterLock
			{
				private:

					//	The SlimReadWriter Lock structure.
					SRWLOCK					_lock;
					//	An internal SlimReadWriterLock used to manager the _shared_locks number.
					SRWLOCK					_shared_lock;
					//	A value representing the current number of shared locks.
					volatile unsigned int	_shared_locks;
					//	A value representing whether the lock has been locked.
					volatile bool			_locked;
					//	A value representing whether the lock has been exclusively locked.
					volatile bool			_exclusively_locked;


					//	A function responsible for initialising the locks.
					void					_initialise_locks();
					//	A function responsible for de-initialising the locks.
					void					_deinitialise_locks();
					//	A function responsible for incrementing the _shared_locks variable and setting the lock status.
					void					_increase_shared_locks();
					//	A function responsible for decrementing the _shared_locks variable and setting the lock status.
					void					_decrease_shared_locks();
					//	A function responsible for chaning the lock status of the lock.
					void					_set_lock_status( const bool locked , const bool exclusive = false );


					//	The assignment operator. It does nothing. Exists only to prevent bit-wise copying.
					SlimReadWriterLock&		operator=( const SlimReadWriterLock& lock );


				public:

					//	 The default constructor initialising the lock.
					SlimReadWriterLock();
					//	 The copy constructor initialising the lock. It creates a NEW lock.
					SlimReadWriterLock( const SlimReadWriterLock& lock );
					//	The destructor deinitialising the lock.
					~SlimReadWriterLock();


					//	A function responsible for acquiring the lock in exclusive mode.
					void					acquire();
					//	A function responsible for acquiring the lock in shared mode.
					void					acquire_shared();
					//	A function responsible for trying to acquire the lock in exclusive mode. Returns true for success and false for failure.
					bool					try_acquire();
					//	A function responsible for trying to acquire the lock in shared mode. Returns true for success and false for failure.
					bool					try_acquire_shared();
					//	A function responsible for releasing the lock in exclusive mode.
					void					release();
					//	A function responsible for releasing the lock in shared mode.
					void					release_shared();


					//	A function responsible for releasing a lock and causing the calling thread to sleep based on the given condition variable.
					bool					sleep( ConditionVariable& variable , const DWORD milliseconds = INFINITE );
			};



			/*
				Function definitions.
			*/


			//	A function responsible for initialising the locks.
			inline void	SlimReadWriterLock::_initialise_locks()
			{
				InitializeSRWLock(&_lock); 
				InitializeSRWLock(&_shared_lock); 
			};

			//	A function responsible for de-initialising the locks.
			inline void	SlimReadWriterLock::_deinitialise_locks()
			{
			};

			//	A function responsible for incrementing the _shared_locks variable and setting the lock status.
			inline void	SlimReadWriterLock::_increase_shared_locks()
			{
				AcquireSRWLockExclusive(&_shared_lock);
				++_shared_locks;
				ReleaseSRWLockExclusive(&_shared_lock);
			
				_set_lock_status(true);
			};

			//	A function responsible for decrementing the _shared_locks variable and setting the lock status.
			inline void	SlimReadWriterLock::_decrease_shared_locks()
			{
				AcquireSRWLockExclusive(&_shared_lock);
				--_shared_locks;

				if ( _shared_locks == 0 )
					_set_lock_status(false);

				ReleaseSRWLockExclusive(&_shared_lock);
			};

			//	A function responsible for chaning the lock status of the lock.
			inline void	SlimReadWriterLock::_set_lock_status( const bool locked , const bool exclusive )
			{
				_locked = locked;
			
				if ( exclusive )
					_exclusively_locked = locked;
			};


			//	A function responsible for acquiring the lock in exclusive mode.
			inline void	SlimReadWriterLock::acquire()
			{
				AcquireSRWLockExclusive(&_lock);
				_set_lock_status(true,true);
			};
		
			//	A function responsible for acquiring the lock in shared mode.
			inline void	SlimReadWriterLock::acquire_shared()
			{
				AcquireSRWLockShared(&_lock);
				_increase_shared_locks();
			};
		
			//	A function responsible for trying to acquire the lock in exclusive mode. Returns true for success and false for failure.
			inline bool	SlimReadWriterLock::try_acquire()
			{
				bool	return_value = false;



				if ( TryAcquireSRWLockExclusive(&_lock) > FALSE )
				{
					_set_lock_status(true,true);
					return_value = true;
				}
			

				return return_value;
			};
		
			//	A function responsible for trying to acquire the lock in shared mode. Returns true for success and false for failure.
			inline bool	SlimReadWriterLock::try_acquire_shared()
			{
				bool	return_value = false;



				if ( TryAcquireSRWLockShared(&_lock) > FALSE )
				{
					_increase_shared_locks();
					return_value = true;
				}


				return return_value;
			};
		
			//	A function responsible for releasing the lock in exclusive mode.
			inline void	SlimReadWriterLock::release()
			{
				_set_lock_status(false,false);
				ReleaseSRWLockExclusive(&_lock);
			};
		
			//	A function responsible for releasing the lock in shared mode.
			inline void	SlimReadWriterLock::release_shared()
			{
				_decrease_shared_locks();
				ReleaseSRWLockShared(&_lock);
			};


			//	A function responsible for releasing a lock and causing the calling thread to sleep based on the given condition variable.
			inline bool	SlimReadWriterLock::sleep( ConditionVariable& variable , const DWORD milliseconds )
			{
				return ( _locked  &&  variable.sleep(_lock,milliseconds,_exclusively_locked) );
			};

		}	/* Concurrency */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_SLIM_READ_WRITER_LOCK_HPP_ */