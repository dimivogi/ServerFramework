#ifndef		DAWN_ENGINE_LOG_DEFINITIONS_HPP_
	#define	DAWN_ENGINE_LOG_DEFINITIONS_HPP_



	namespace	DawnEngine
	{
		
		namespace	IO
		{

			/*
				Enumeration holding all the possible types of log entries.
			*/
			enum	LogEntryType
			{
				LOG_ERROR = 0 , 
				LOG_WARNING , 
				LOG_MESSAGE
			};

		}	/* IO */

	}	/* DawnEngine */



#endif		/* DAWN_ENGINE_LOG_DEFINITIONS_HPP_ */