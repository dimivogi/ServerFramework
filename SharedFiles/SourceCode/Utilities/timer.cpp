#include	"timer.hpp"



namespace	DawnEngine
{

	namespace	Utility
	{

		//	The default constructor.
		Timer::Timer()	:	
			_lock() , 
			_frequency(1) , 
			_start(0) , 
			_current(0) , 
			_reverse_frequency(1) , 
			_paused(false)
		{
			LARGE_INTEGER	value;



			value.QuadPart = 0;
			QueryPerformanceFrequency(&value);

			if ( value.QuadPart != 0 )
			{
				_frequency = static_cast<unsigned long long>(value.QuadPart);
				_reverse_frequency = 1.0 / static_cast<double>(_frequency);
			}

			value.QuadPart = 0;
			QueryPerformanceCounter(&value);
			_start = static_cast<unsigned long long>(value.QuadPart);
			_current = _start;
		};

		//	The destructor.
		Timer::~Timer()		{};

	};

}	/* DawnEngine */