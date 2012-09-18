#include	"../globalDefinitions.hpp"
#include	<Windows.h>

#ifndef		_DAWN_ENGINE_CONDITION_VARIABLE_HPP_
	#define	_DAWN_ENGINE_CONDITION_VARIABLE_HPP_



	namespace	DawnEngine
	{

		namespace	Concurrency
		{

			/*
				A class representing and handling a condition variable.
			*/
			class	ConditionVariable
			{
				private:

					//	The condition variable structure.
					CONDITION_VARIABLE	_variable;


					//	 A function responsible for initialising the variable.
					void				_initialise();
					//	 A function responsible for de-initialising the variable.
					void				_deinitialise();


					//	 The assignment operator. It has no functionality. Exists only to prevent bit-wise copying.
					ConditionVariable	operator=( const ConditionVariable& variable );


				public:

					//	 A function responsible for returning the last error.
					static int			last_error();

					//	 The default constructor.
					ConditionVariable();
					//	 The copy-constructor. Since copy construction is disabled, it creates a NEW variable.
					ConditionVariable( const ConditionVariable& variable );
					//	The destructor.
					~ConditionVariable();


					//	A function responsible for releasing a critical section and putting the calling thread to sleep.
					bool				sleep( CRITICAL_SECTION& critical_section , const DWORD milliseconds = INFINITE );
					//	 A function responsible for releasing a SlimReadWriter lock and putting the calling thread to sleep.
					bool				sleep( SRWLOCK& slim_read_writer_lock , const DWORD milliseconds = INFINITE , bool shared = false );
					//	 A function responsible for waking a thread that has been put to sleep by this variable.
					void				wake();
					//	A function responsible for waking all the threads that have been put to sleep by this variable.
					void				wake_all();
			};



			/*
				Function definitions.
			*/


			//	 A function responsible for initialising the variable.
			inline void	ConditionVariable::_initialise()	{ InitializeConditionVariable(&_variable); };

			//	 A function responsible for de-initialising the variable.
			inline void	ConditionVariable::_deinitialise()	{ wake_all(); };


			//	 A function responsible for returning the last error.
			inline int	ConditionVariable::last_error()		{ return GetLastError(); };

		
			//	A function responsible for releasing a critical section and putting the calling thread to sleep.
			inline bool	ConditionVariable::sleep( CRITICAL_SECTION& critical_section , const DWORD milliseconds )
			{
				return ( SleepConditionVariableCS(&_variable,&critical_section,milliseconds) > FALSE  ?  true : false );
			};

			//	 A function responsible for releasing a SlimReadWriter lock and putting the calling thread to sleep.
			inline bool	ConditionVariable::sleep( SRWLOCK& slim_read_writer_lock , const DWORD milliseconds , bool shared )
			{
				return ( SleepConditionVariableSRW(&_variable,&slim_read_writer_lock,milliseconds, ( shared  ?  CONDITION_VARIABLE_LOCKMODE_SHARED : 0 ) ) > FALSE  ?  true : false );
			};

			//	 A function responsible for waking a thread that has been put to sleep by this variable.
			inline void	ConditionVariable::wake()			{ WakeConditionVariable(&_variable); };
			//	A function responsible for waking all the threads that have been put to sleep by this variable.
			inline void	ConditionVariable::wake_all()		{ WakeAllConditionVariable(&_variable); };

		}	/* Concurrency */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_CONDITION_VARIABLE_HPP_ */