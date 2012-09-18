#include	<sstream>
#include	<ctime>
#include	<iostream>
#include	"logManager.hpp"



namespace	DawnEngine
{

	namespace	IO
	{

		//	A pointer to the single instance of the class.
		LogManagerA*					LogManagerA::_instance = NULL;
		//	A static CriticalSection lock in order to make sure there are no concurency issues.
		Concurrency::CriticalSection	LogManagerA::_instance_lock;

		//	A pointer to the single instance of the class.
		LogManagerW*					LogManagerW::_instance = NULL;
		//	A static CriticalSection lock in order to make sure there are no concurency issues.
		Concurrency::CriticalSection	LogManagerW::_instance_lock;


		/*
			UTF-8 functions.
		*/

		//	The default constructor.
		LogManagerA::LogManagerA()	:	
			_lock() , 
			_log(0) , 
			_error_tag("Error:") , _warning_tag("Warning:") , _message_tag("Log:") , 
			_auto_dump_filename("") , _auto_dump_file() , 
			_auto_dump_file_mode(std::ofstream::out|std::ofstream::ate) , 
			_max_entries(10000) , 
			_auto_purge_threshold(100) , _auto_dump_threshold(100) , 
			_last_dump_position(0) , _auto_dump_counter(0) ,
			_echo_stream(&std::clog) , 
			_separator('\t') , _add_timestamp(true) , 
			_auto_purge(false) , _auto_dump(false) , _echo(true)							{};

		//	The destructor.
		LogManagerA::~LogManagerA()
		{
			if ( _auto_dump_file.is_open() )
				_auto_dump_file.close();
		};
	
	
		//	Function returning a formatted string representing a tm struct.
		std::string	LogManagerA::_format_time( const tm& time_struct )
		{
			std::stringstream	buffer;



			buffer	<< time_struct.tm_mday << '/' << (time_struct.tm_mon+1) << '/' << (time_struct.tm_year + 1900) << _separator ;

			if ( time_struct.tm_hour < 10 )
				buffer	<< 0;

			buffer	<< time_struct.tm_hour << ":";
			
			if ( time_struct.tm_min < 10 )
				buffer	<< 0;
		
			buffer	<< time_struct.tm_min << ":";
		
			if ( time_struct.tm_sec < 10 )
				buffer	<< 0;
		
			buffer << time_struct.tm_sec;


			return buffer.str();
		};

		//	Function returning the current time in a formatted string.
		std::string	LogManagerA::_generate_timestamp()
		{
			std::string			return_value("");
			tm					current_time;
			time_t				time_in_seconds;
			
		

			//	Get the current time in seconds
			time(&time_in_seconds);

			//	Convert the current time from seconds to a tm struct.
			if ( localtime_s(&current_time,&time_in_seconds) == 0 )
				return_value = _format_time(current_time);	// Format the resulting tm struct into a string.


			return return_value;
		};

		//	Function parsing an input string similarly to the printf() function.
		std::string	LogManagerA::_parse_input_string( const std::string& input , va_list arguments )
		{
			static char			deliminator = '%';
			std::stringstream	return_value;
			std::string			temp_message(input);
			size_t				i = 0;
			
			

			// While the whole input string hasn't been parsed.
			while( temp_message != "" )
			{
				//	Find the first occurence of the deliminator character.
				i = temp_message.find_first_of(deliminator,0);

				//	 If the deliminator character has been found.
				if ( i < temp_message.npos )
				{
					//	Add the message up until the deliminator character to the result.
					return_value << temp_message.substr(0,i);

					//	If the deliminator character was not the last character.
					if ( (i+1) < temp_message.size() )
					{
						char	character = temp_message[i+1];



						//	If the following character is the deliminator character, add the deliminator character to the result
						if ( character == deliminator )
							return_value << character;
						else if ( character == 'c' )	// Character input
							return_value << va_arg(arguments,char);
						else if ( character == 'i'  ||  character == 'd' )	// Integer input
							return_value << va_arg(arguments,int);
						else if ( character == 'u' )	// Unsigned integer input
							return_value << va_arg(arguments,unsigned int);
						else if ( character == 'f' )	//	Double input
							return_value << va_arg(arguments,double);
						else if ( character == 's' )	//	C-String input.
						{
							const char*	buffer = va_arg(arguments,const char*);



							if ( buffer )
								return_value << buffer;
						}

						//	Remove the characters up to the deliminator character and the following character from the message.
						temp_message = temp_message.substr(i+2);
					}
					else
						temp_message = "";	// The deliminator character was the last. Exit the while loop.
				}
				else
				{
					//	If the deliminator character has not been found, add the entire remaining message to the result.
					return_value << temp_message;
					temp_message = "";
				}
			}


			return return_value.str();
		};

		//	 Function adding a new entry to the log.
		void	LogManagerA::_log_entry( const LogEntryType& type , const std::string& message )
		{
			LogEntryA	entry(message,_generate_timestamp(),type);



			_lock.acquire(); 

			if ( message.size() > 0 )
			{
				_log.push_back(entry);
				++_auto_dump_counter;
			}

			//	 If the auto dump threshold has been reached
			if ( _auto_dump  &&  _auto_dump_counter >= _auto_dump_threshold )
			{
				//	Dump the log in the auto dump file.
				_dump_to_file(_auto_dump_file,true);
				//	Reset the auto dump counter.
				_auto_dump_counter = 0;

				//	If auto purge is enabled
				if ( _auto_purge )
				{
					//	Clear the log.
					_log.clear();
					//	Reset the last dump position counter.
					_last_dump_position = 0;
				}
			}

			if ( _auto_purge )
			{
				//	 If the auto purge threshold has been reached
				if ( _log.size() >= _auto_purge_threshold )
					purge_log();	// Clear the log.
			}
			else
			{
				if ( _log.size() > _max_entries )
				{
					_log.pop_front();

					if ( _last_dump_position > 0 )
						--_last_dump_position;
				}
			}

			if ( _echo  &&  _echo_stream != NULL )
				_print_entry(*_echo_stream,entry);

			_lock.release();
		};

		//	Function responsible for printing a entry to the specified stream.
		void	LogManagerA::_print_entry( std::ostream& stream , const LogEntryA& entry )
		{
			if ( _add_timestamp )
				stream << entry.timestamp() << _separator;

			switch ( entry.type() )
			{
				case	LOG_ERROR:		
										stream << _error_tag;
										break;
				case	LOG_WARNING:
										stream << _warning_tag;
										break;
				case	LOG_MESSAGE:	
										stream << _message_tag;
										break;
			}

			stream << _separator << entry.message() << std::endl;
		};

		//	Function dumping the contents of the log into the specified file stream.
		void	LogManagerA::_dump_to_file( std::ofstream& file , const bool update_position )
		{
			if ( file.is_open() )
			{
				unsigned int	offset = 0;



				if ( update_position )
					offset = _last_dump_position;

				for ( std::deque<LogEntryA>::iterator	log_iterator = _log.begin() + offset;  log_iterator != _log.end();  ++log_iterator )
				{
					_print_entry(file,*log_iterator);
					++offset;
				}

				if ( update_position )
					_last_dump_position = offset;
			}
		};


		//	Function responsible for setting the file that will be utilized in auto dump mode.
		void	LogManagerA::auto_dump_file( const std::string& filename )
		{
			if ( filename != "" )
			{
				_lock.acquire();
				_auto_dump_filename = filename;

				if ( _auto_dump_file.is_open() )
					_auto_dump_file.close();

				_auto_dump_file.open(_auto_dump_filename.c_str(),_auto_dump_file_mode);
				_lock.release();
			}
		};

		//	Function responsible for dumping the contents of the log into the specified file with open mode the mode given. Possible open modes are APPEND and TRUNCATE.
		void	LogManagerA::dump( const std::string& filename , const LogFileOpenMode& mode )
		{
			std::ofstream				file;
			std::ofstream::open_mode	file_mode = std::ofstream::out;



			if ( mode == APPEND )
				file_mode |= std::ofstream::app;
			else
				file_mode |= std::ofstream::trunc;

			file.open(filename.c_str(),file_mode);

			if ( file.is_open() )
			{
				_lock.acquire_shared();
				_dump_to_file(file,false);
				_lock.release_shared();
				file.close();
			}
		};

		//	Function responsible for purging the log.
		void	LogManagerA::purge_log()
		{
			_lock.acquire();

			//	 If auto dump is enabled
			if ( _auto_dump )
				_dump_to_file(_auto_dump_file,true);	//	Dump the log in the the auto dump file.

			//	Clear the log.
			_log.clear();
			//	 Reset the last dump position counter.
			_last_dump_position = 0;
			//	Reset the auto dump counter.
			_auto_dump_counter = 0;
			_lock.release();
		};


		//	Function returning the desired amount of entries, starting from the oldest entries.
		std::vector<LogEntryA>	LogManagerA::first_entries( const unsigned int amount , const unsigned int offset ) const
		{
			_lock.acquire_shared();
		

			std::vector<LogEntryA>					return_value(0);
			std::deque<LogEntryA>::const_iterator	log_iterator(_log.begin() + ( offset > _log.size()  ?  0 : offset ));



			if ( log_iterator != _log.end() )
			{
				unsigned int	i = 0;



				while ( log_iterator != _log.end()  &&  i < amount ) 
				{
					return_value.push_back(*log_iterator);
					++i;
					++log_iterator;
				}
			}

			_lock.release_shared();


			return return_value;
		};

		//	Function returning the desired amount of entries, starting from the latest entries. 
		std::vector<LogEntryA>	LogManagerA::last_entries( const unsigned int amount , const unsigned int offset ) const
		{
			_lock.acquire_shared();


			std::vector<LogEntryA>							return_value(0);
			std::deque<LogEntryA>::const_reverse_iterator	log_iterator(_log.rbegin() + ( offset > _log.size()  ?  0 : offset ));



			if ( log_iterator != _log.rend() )
			{
				unsigned int	i = 0;



				while ( log_iterator != _log.rend()  &&  i < amount ) 
				{
					return_value.push_back(*log_iterator);
					++i;
					++log_iterator;
				}
			}

			_lock.release_shared();


			return return_value;
		};



		/*
			UTF-16 functions.
		*/

		//	The default constructor.
		LogManagerW::LogManagerW()	:	
			_lock() , 
			_log(0) , 
			_auto_dump_file() , 
			_error_tag(L"Error:") , _warning_tag(L"Warning:") , _message_tag(L"Log:") , 
			_auto_dump_filename(L"") , _auto_dump_locale("") , 
			_auto_dump_file_mode(std::ofstream::out|std::ofstream::app) , 
			_max_entries(10000) , 
			_auto_purge_threshold(100) , _auto_dump_threshold(100) , 
			_last_dump_position(0) , _auto_dump_counter(0) ,
			_echo_stream(&std::wclog) , 
			_separator('\t') , _add_timestamp(true) , 
			_auto_purge(false) , _auto_dump(false) , _echo(true)							{};

		//	The destructor.
		LogManagerW::~LogManagerW()
		{
			if ( _auto_dump_file.is_open() )
				_auto_dump_file.close();
		};
	
	
		//	Function returning a formatted string representing a tm struct.
		std::wstring	LogManagerW::_format_time( const tm& time_struct )
		{
			std::wstringstream	buffer;



			buffer	<< time_struct.tm_mday << '/' << (time_struct.tm_mon+1) << '/' << (time_struct.tm_year + 1900) << _separator ;
		
			if ( time_struct.tm_hour < 10 )
				buffer	<< 0;

			buffer	<< time_struct.tm_hour << ":";

			if ( time_struct.tm_min < 10 )
				buffer	<< 0;

			buffer	<< time_struct.tm_min << ":";
		
			if ( time_struct.tm_sec < 10 )
				buffer	<< 0;
		
			buffer << time_struct.tm_sec;


			return buffer.str();
		};

		//	Function returning the current time in a formatted string.
		std::wstring	LogManagerW::_generate_timestamp()
		{
			std::wstring		return_value(L"");
			tm					current_time;
			time_t				time_in_seconds;
			
		

			//	Get the current time in seconds
			time(&time_in_seconds);

			//	Convert the current time from seconds to a tm struct.
			if ( localtime_s(&current_time,&time_in_seconds) == 0 )
				return_value = _format_time(current_time);	// Format the resulting tm struct into a string.


			return return_value;
		};

		//	Function parsing an input string similarly to the printf() function.
		std::wstring	LogManagerW::_parse_input_string( const std::wstring& input , va_list arguments )
		{
			static wchar_t		deliminator = '%';
			std::wstringstream	return_value;
			std::wstring		temp_message(input);
			size_t				i = 0;
			
			

			// While the whole input string hasn't been parsed.
			while( temp_message != L"" )
			{
				//	Find the first occurence of the deliminator character.
				i = temp_message.find_first_of(deliminator,0);

				//	 If the deliminator character has been found.
				if ( i < temp_message.npos )
				{
					//	Add the message up until the deliminator character to the result.
					return_value << temp_message.substr(0,i);

					//	If the deliminator character was not the last character.
					if ( (i+1) < temp_message.size() )
					{
						wchar_t	character = temp_message[i+1];



						//	If the following character is the deliminator character, add the deliminator character to the result
						if ( character == deliminator )
							return_value << character;
						else if ( character == 'c' )	// Character input
							return_value << va_arg(arguments,wchar_t);
						else if ( character == 'i'  ||  character == 'd' )	// Integer input
							return_value << va_arg(arguments,int);
						else if ( character == 'u' )	// Unsigned integer input
							return_value << va_arg(arguments,unsigned int);
						else if ( character == 'f' )	//	Double input
							return_value << va_arg(arguments,double);
						else if ( character == 's' )	//	C-String input.
						{
							const wchar_t*	buffer = va_arg(arguments,const wchar_t*);



							if ( buffer )
								return_value << buffer;
						}

						//	Remove the characters up to the deliminator character and the following character from the message.
						temp_message = temp_message.substr(i+2);
					}
					else
						temp_message = L"";	// The deliminator character was the last. Exit the while loop.
				}
				else
				{
					//	If the deliminator character has not been found, add the entire remaining message to the result.
					return_value << temp_message;
					temp_message = L"";
				}
			}


			return return_value.str();
		};

		//	 Function adding a new entry to the log.
		void	LogManagerW::_log_entry( const LogEntryType& type , const std::wstring& message )
		{
			LogEntryW	entry(message,_generate_timestamp(),type);



			_lock.acquire(); 

			if ( message.size() > 0 )
			{
				_log.push_back(entry);
				++_auto_dump_counter;
			}

			//	 If the auto dump threshold has been reached
			if ( _auto_dump  &&  _auto_dump_counter >= _auto_dump_threshold )
			{
				//	Dump the log in the auto dump file.
				_dump_to_file(_auto_dump_file,true);
				//	Reset the auto dump counter.
				_auto_dump_counter = 0;

				//	If auto purge is enabled
				if ( _auto_purge )
				{
					//	Clear the log.
					_log.clear();
					//	Reset the last dump position counter.
					_last_dump_position = 0;
				}
			}

			if ( _auto_purge )
			{
				//	 If the auto purge threshold has been reached
				if ( _log.size() >= _auto_purge_threshold )
					purge_log();	// Clear the log.
			}
			else
			{
				if ( _log.size() > _max_entries )
				{
					_log.pop_front();

					if ( _last_dump_position > 0 )
						--_last_dump_position;
				}
			}

			if ( _echo  &&  _echo_stream != NULL )
				_print_entry(*_echo_stream,entry);

			_lock.release();
		};

		//	Function responsible for printing a entry to the specified stream.
		void	LogManagerW::_print_entry( std::wostream& stream , const LogEntryW& entry )
		{
			if ( _add_timestamp )
				stream << entry.timestamp() << _separator;

			switch ( entry.type() )
			{
				case	LOG_ERROR:		
										stream << _error_tag;
										break;
				case	LOG_WARNING:
										stream << _warning_tag;
										break;
				case	LOG_MESSAGE:	
										stream << _message_tag;
										break;
			}

			stream << _separator << entry.message() << std::endl;
		};

		//	Function dumping the contents of the log into the specified file stream.
		void	LogManagerW::_dump_to_file( std::wofstream& file , const bool update_position )
		{
			if ( file.is_open() )
			{
				unsigned int	offset = 0;


				if ( update_position )
					offset = _last_dump_position;

				for ( std::deque<LogEntryW>::iterator	log_iterator = _log.begin() + offset;  log_iterator != _log.end();  ++log_iterator )
				{
					_print_entry(file,*log_iterator);
					++offset;
				}

				if ( update_position )
					_last_dump_position = offset;
			}
		};


		//	Function responsible for setting the file that will be utilized in auto dump mode.
		void	LogManagerW::auto_dump_file( const std::wstring& filename )
		{
			if ( filename != L"" )
			{
				_lock.acquire();
				_auto_dump_filename = filename;

				if ( _auto_dump_file.is_open() )
					_auto_dump_file.close();

				_auto_dump_file.imbue(std::locale(_auto_dump_locale));
				_auto_dump_file.open(_auto_dump_filename.c_str(),_auto_dump_file_mode);
				_lock.release();
			}
		};

		//	Function responsible for dumping the contents of the log into the specified file with open mode the mode given. Possible open modes are APPEND and TRUNCATE.
		void	LogManagerW::dump( const std::wstring& filename , const LogFileOpenMode& mode  , const std::string& language )
		{
			std::wofstream				file;
			std::wofstream::open_mode	file_mode = std::wofstream::out;



			if ( mode == APPEND )
				file_mode |= std::wofstream::app;
			else
				file_mode |= std::wofstream::trunc;

			file.imbue(std::locale(language));
			file.open(filename.c_str(),file_mode);

			if ( file.is_open() )
			{
				_lock.acquire_shared();
				_dump_to_file(file,false);
				_lock.release_shared();
				file.close();
			}
		};

		//	Function responsible for purging the log.
		void	LogManagerW::purge_log()
		{
			_lock.acquire();

			//	 If auto dump is enabled
			if ( _auto_dump )
				_dump_to_file(_auto_dump_file,true);	//	Dump the log in the the auto dump file.

			//	Clear the log.
			_log.clear();
			//	 Reset the last dump position counter.
			_last_dump_position = 0;
			//	Reset the auto dumo counter.
			_auto_dump_counter = 0;
			_lock.release();
		};


		//	Function returning the desired amount of entries, starting from the oldest entries.
		std::vector<LogEntryW>	LogManagerW::first_entries( const unsigned int amount , const unsigned int offset ) const
		{
			_lock.acquire_shared();
		

			std::vector<LogEntryW>					return_value(0);
			std::deque<LogEntryW>::const_iterator	log_iterator(_log.begin() + ( offset > _log.size()  ?  0 : offset ));
		


			if ( log_iterator != _log.end() )
			{
				unsigned int	i = 0;



				while ( log_iterator != _log.end()  &&  i < amount ) 
				{
					return_value.push_back(*log_iterator);
					++i;
					++log_iterator;
				}
			}

			_lock.release_shared();


			return return_value;
		};

		//	Function returning the desired amount of entries, starting from the latest entries. 
		std::vector<LogEntryW>	LogManagerW::last_entries( const unsigned int amount , const unsigned int offset ) const
		{
			_lock.acquire_shared();
		

			std::vector<LogEntryW>							return_value(0);
			std::deque<LogEntryW>::const_reverse_iterator	log_iterator(_log.rbegin() + ( offset > _log.size()  ?  0 : offset ));



			if ( log_iterator != _log.rend() )
			{
				unsigned int	i = 0;



				while ( log_iterator != _log.rend()  &&  i < amount ) 
				{
					return_value.push_back(*log_iterator);
					++i;
					++log_iterator;
				}
			}

			_lock.release_shared();


			return return_value;
		};

	}	/* IO */

}	/* DawnEngine */