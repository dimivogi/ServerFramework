#include	"../globalDefinitions.hpp"
#include	"../Lock/slimReadWriterLock.hpp"

#ifndef		_DAWN_ENGINE_TIMER_HPP_
	#define	_DAWN_ENGINE_TIMER_HPP_



	namespace	DawnEngine
	{

		namespace	Utility
		{
			
			//	Variable holding the amount that we have to multiply in order to get the timer value in miliseconds.
			const double	TO_MILLISECONDS = 1000;



			/*
				Class responsible of keeping track of time. The functionality resembles a stopwatch.
			*/
			class	Timer
			{
				private:

					//	A lock used to handle concurrency issues.
					Concurrency::SlimReadWriterLock	_lock;
					//	A variable holding the counter frequency.
					unsigned long long				_frequency;
					//	A variable holding the creation time of the timer.
					unsigned long long				_start;
					//	A variable holding the current time of the timer.
					unsigned long long				_current;
					//	A variable holding the reverse frequency.
					double							_reverse_frequency;
					//	A variable holding whether the timer has been paused.
					bool							_paused;


					//	Function responsible of getting the current time.
					static unsigned long long		_get_current_time();


				public:

					//	The default constructor.
					Timer();
					//	The destructor.
					~Timer();


					//	Function responsible of pausing the timer.
					void							pause();
					//	Function responsible of resuming the timer.
					void							resume();
					//	Function responsible of resetting the timer.
					void							reset();


					//	Function returning the current time in seconds.
					double							seconds();
					//	Function returning the current time in milliseconds.
					double							milliseconds();
					//	Function returning the difference in time in secondssince the last call.
					double							difference_seconds();
					//	Function returning the difference in time in milliseconds since the last call.
					double							difference_milliseconds();
					//	Function returning the frequency of the timer.
					unsigned long long				frequency();
					//	Function returning the current time.
					unsigned long long				time();
					//	Function returning whether the timer is paused.
					bool							paused();
			};



			/*
				Function definitions.
			*/


			//	Function responsible of getting the current time.
			inline unsigned long long	Timer::_get_current_time()
			{
				LARGE_INTEGER		value;
				unsigned long long	return_value = 0;



				QueryPerformanceCounter(&value);
				return_value = value.QuadPart;


				return return_value;
			};


			//	Function responsible of pausing the timer.
			inline void	Timer::pause()
			{
				_lock.acquire();
				_paused = true;
				_lock.release();
			}

			//	Function responsible of resuming the timer.
			inline void	Timer::resume()
			{
				_lock.acquire();
				_paused = false;
				_lock.release();
			};

			//	Function responsible of resetting the timer.
			inline void	Timer::reset()
			{
				_lock.acquire();
				_current = _get_current_time();
				_start = _current;
				_lock.release();
			}



			//	Function returning the current time in seconds.
			inline double	Timer::seconds()
			{
				double	return_value = 0;



				_lock.acquire();
					
				if ( !_paused )
					_current = _get_current_time();

				return_value = static_cast<double>(_current-_start)*_reverse_frequency;
				_lock.release();


				return return_value;
			};

			//	Function returning the current time in milliseconds.
			inline double	Timer::milliseconds()
			{
				return seconds()*TO_MILLISECONDS;
			};

			//	Function returning the difference in time in seconds since the last call.
			inline double	Timer::difference_seconds()
			{
				double				return_value = 0;
				unsigned long long	current_time = 0;



				_lock.acquire();

				if ( !_paused ) 
					current_time = _get_current_time();
				else
					current_time = _current;

				return_value = static_cast<double>(current_time-_current)*_reverse_frequency;

				if ( !_paused )
					_current = current_time;

				_lock.release();


				return return_value;
			};

			//	Function returning the difference in time in milliseconds since the last call.
			inline double	Timer::difference_milliseconds()
			{
				return difference_seconds()*TO_MILLISECONDS;
			};

			//	Function returning the frequency of the timer.
			inline unsigned long long	Timer::frequency()
			{
				unsigned long long	return_value = 0;



				_lock.acquire_shared();
				return_value = _frequency;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the current time.
			inline unsigned long long	Timer::time()
			{
				unsigned long long	return_value = 0;



				_lock.acquire();

				if ( !_paused )
					_current = _get_current_time();

				return_value = _current;
				_lock.release();


				return return_value;
			}

			//	Function returning whether the timer is paused.
			inline bool	Timer::paused()
			{
				bool	return_value = false;



				_lock.acquire_shared();
				return_value = _paused;
				_lock.release_shared();


				return return_value;
			};

		}	/* Utilities */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_TIMER_HPP_ */

