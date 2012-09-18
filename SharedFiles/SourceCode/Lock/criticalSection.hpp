#include	"../globalDefinitions.hpp"
#include	<Windows.h>
#include	"conditionVariable.hpp"

#ifndef		_DAWN_ENGINE_CRITICAL_SECTION_HPP_
	#define	_DAWN_ENGINE_CRITICAL_SECTION_HPP_



	namespace	DawnEngine
	{

		namespace	Concurrency
		{

			/*
				A class representing and managing a critical section structure.
			*/
			class	CriticalSection
			{
				private:

					//	The CRITICAL_SECTION structure.
					CRITICAL_SECTION			_critical_section;
					//	A critical section managin the spin count structure.
					mutable CRITICAL_SECTION	_manager;
					//	The spin count for the critical section.
					DWORD						_spin_count;
					//	A counter tracking how many time the enter() function has been called since the last leave() function call
					unsigned int				_lock_count;



					//	A functionr responsible for initialising the critical sections.
					void						_initialise_critical_sections();
					//	A function responsible for deinitialising the critical sections.
					void						_deinitialise_critical_sections();
					//	A function responsible for incrementing the lock counter.	
					void						_increase_lock_count();
					//	A function responsible for decrementing the lock counter.
					void						_decrease_lock_count();


					//	The assignment operator. It does nothing. Exists only to prevent bit-wise copying.
					CriticalSection&			operator=( const CriticalSection& section );


				public:

					//	The default constructor initialising the critical section.
					CriticalSection();
					//	The copy constructor initialising the critical section. It creates a NEW critical section.
					CriticalSection( const CriticalSection& section );
					//	The destructor deinitialising the critical section.
					~CriticalSection();


					//	A function responsible for setting the spin count for the critical section.
					void						spin_count( const DWORD count );
					//	A function responsible for getting the spin count of the critical section.
					DWORD						spin_count() const;
					//	A function responsible for returning the lock count.
					unsigned int				lock_count() const;


					//	A function responsible for entering the critical section.
					void						enter();
					//	A function responsible for trying to enter the critical section. Returns true for success and false for failure.
					bool						try_enter();
					//	A function responsible for leaving the critical section. 
					void						leave();


					//	A function responsible for releasing the critical section and putting the calling thread to sleep.
					bool						sleep( ConditionVariable& variable , const DWORD millisecods = INFINITE );

			};



			/*
				Function definitions.
			*/


			//	A functionr responsible for initialising the critical sections.
			inline void		CriticalSection::_initialise_critical_sections()
			{
				InitializeCriticalSection(&_critical_section); 
				InitializeCriticalSection(&_manager);
			};
		
			//	A function responsible for deinitialising the critical sections.
			inline void		CriticalSection::_deinitialise_critical_sections()
			{
				DeleteCriticalSection(&_critical_section);
				DeleteCriticalSection(&_manager);
			};

			//	A function responsible for incrementing the lock counter.	
			inline void		CriticalSection::_increase_lock_count()
			{
				EnterCriticalSection(&_manager);
				++_lock_count;
				LeaveCriticalSection(&_manager);
			};

			//	A function responsible for decrementing the lock counter.
			inline void		CriticalSection::_decrease_lock_count()
			{
				EnterCriticalSection(&_manager);
				--_lock_count;
				LeaveCriticalSection(&_manager);
			};


			//	A function responsible for setting the spin count for the critical section.
			inline void	CriticalSection::spin_count( const DWORD count )
			{
				EnterCriticalSection(&_manager);
				SetCriticalSectionSpinCount(&_critical_section,count);
				_spin_count = count;
				LeaveCriticalSection(&_manager);
			};

			//	A function responsible for getting the spin count of the critical section.
			inline DWORD	CriticalSection::spin_count() const
			{ 
				DWORD	return_value = 0;



				EnterCriticalSection(&_manager);
				return_value = _spin_count;
				LeaveCriticalSection(&_manager);

			
				return return_value;
			};

			//	A function responsible for returning the lock count.
			inline unsigned int	CriticalSection::lock_count() const
			{
				unsigned int	return_value = 0;



				EnterCriticalSection(&_manager);
				return_value = _lock_count;
				LeaveCriticalSection(&_manager);


				return return_value;
			};


			//	A function responsible for entering the critical section.
			inline void	CriticalSection::enter()
			{
				EnterCriticalSection(&_critical_section);
				_increase_lock_count();
			};

			//	A function responsible for trying to enter the critical section. Returns true for success and false for failure.
			inline bool	CriticalSection::try_enter()
			{
				bool	return_value = false;



				if ( TryEnterCriticalSection(&_critical_section) > FALSE )
				{
					_increase_lock_count();
					return_value = true;
				}


				return return_value;
			};

			//	A function responsible for leaving the critical section. 
			inline void	CriticalSection::leave()
			{
				_decrease_lock_count();
				LeaveCriticalSection(&_critical_section);
			};


			//	A function responsible for releasing the critical section and putting the calling thread to sleep.
			inline bool	CriticalSection::sleep( ConditionVariable& variable , const DWORD millisecods )
			{
				return ( lock_count() == 1  &&  variable.sleep(_critical_section,millisecods) );
			};

		}	/* Concurrency */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_CRITICAL_SECTION_HPP_ */