#include	<string>
#include	"../globalDefinitions.hpp"
#include	"logDefinitions.hpp"
#include	"logException.hpp"

#ifndef		DAWN_ENGINE_LOG_ENTRY_HPP_
	#define	DAWN_ENGINE_LOG_ENTRY_HPP_



	namespace	DawnEngine
	{

		namespace	IO
		{

			/*
				A class representing an entry of the LogManager.
			*/
			template< typename data_type >
			class	BasicLogEntry
			{
				private:

					//	A variable that contains the timestamp of the log entry.
					data_type		_timestamp;
					//	A variable that contains the message of the log entry.
					data_type		_message;
					//	A variable that contains the type of the log entry.
					LogEntryType	_type;


				public:

					//	The default constructor.
					BasicLogEntry();
					//	A constructor taking a message, a timestamp and a type parameter.
					explicit BasicLogEntry( const data_type& message , const data_type& timestamp , const LogEntryType& type = LOG_MESSAGE );
					//	 A constructor taking an BasicLogException, a timestamp and a type parameter.
					explicit BasicLogEntry( const BasicLogException<data_type>& exception , const data_type& timestamp , const LogEntryType& type = LOG_ERROR );
					//	The copy constructor.
					BasicLogEntry( const BasicLogEntry<data_type>& entry );
					//	The destructor.
					virtual ~BasicLogEntry();


					//	A function responsible for changing the type of the log entry.
					void			type( const LogEntryType& type );
					//	A function responsible for changing the message of the log entry.
					void			message( const data_type& message );
					//	A function responsible for changing the timestamp of the log entry.
					void			timestamp( const data_type& timestamp );


					//	A function returning the type of the log entry.
					LogEntryType	type() const;
					//	 A function returning the message of the log entry.
					data_type		message() const;
					//	A function returning the timestamp of the log entry.
					data_type		timestamp() const;
			};


			/*
				Type Definitions.
			*/
			//	An UTF-8 log entry.
			typedef	BasicLogEntry<std::string>	LogEntryA;
			//	An UTF-16 log entry.
			typedef	BasicLogEntry<std::wstring>	LogEntryW;



			/*
				Function definitions.
			*/


			//	The default constructor.
			template< typename data_type >
			BasicLogEntry<data_type>::BasicLogEntry()	:	
				_message() , _timestamp() , _type(LOG_MESSAGE)										{};

			//	A constructor taking a message, a timestamp and a type parameter.
			template< typename data_type >
			BasicLogEntry<data_type>::BasicLogEntry( const data_type& message , const data_type& timestamp , const LogEntryType& type )	:	
				_message(message) , _timestamp(timestamp) , _type(type)								{};

			//	 A constructor taking an BasicLogException, a timestamp and a type parameter.
			template< typename data_type >
			BasicLogEntry<data_type>::BasicLogEntry( const BasicLogException<data_type>& exception , const data_type& timestamp , const LogEntryType& type )	:	
				_message(exception.message()) , _timestamp(timestamp) , _type(type)					{};

			//	The copy constructor.
			template< typename data_type >
			BasicLogEntry<data_type>::BasicLogEntry( const BasicLogEntry<data_type>& entry )	:	
				_message(entry.message()) , _timestamp(entry.timestamp()) , _type(entry.type())		{};

			//	The destructor.
			template< typename data_type >
			BasicLogEntry<data_type>::~BasicLogEntry()												{};


			//	A function responsible for changing the type of the log entry.
			template< typename data_type >
			inline void			BasicLogEntry<data_type>::type( const LogEntryType& type )			{ _type = type; };

			//	A function responsible for changing the message of the log entry.
			template< typename data_type >
			inline void			BasicLogEntry<data_type>::message( const data_type& message )		{ _message = message; };

			//	A function responsible for changing the timestamp of the log entry.
			template< typename data_type >
			inline void			BasicLogEntry<data_type>::timestamp( const data_type& timestamp )	{ _timestamp = timestamp; };


			//	A function returning the type of the log entry.
			template< typename data_type >
			inline LogEntryType	BasicLogEntry<data_type>::type() const								{ return _type; };

			//	 A function returning the message of the log entry.
			template< typename data_type >
			inline data_type	BasicLogEntry<data_type>::message() const							{ return _message; };

			//	A function returning the timestamp of the log entry.
			template< typename data_type >
			inline data_type	BasicLogEntry<data_type>::timestamp() const							{ return _timestamp; };

		}	/* IO */

	}	/* DawnEngine */



#endif		/* DAWN_ENGINE_LOG_ENTRY_HPP_ */