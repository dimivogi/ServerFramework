#include	<string>
#include	<fstream>
#include	<locale>
#include	<vector>
#include	<deque>
#include	<cstdarg>
#include	"../globalDefinitions.hpp"
#include	"logDefinitions.hpp"
#include	"logEntry.hpp"
#include	"logException.hpp"
#include	"../Lock/criticalSection.hpp"
#include	"../Lock/slimReadWriterLock.hpp"

#ifndef		DAWN_ENGINE_LOG_MANAGER_HPP_
	#define	DAWN_ENGINE_LOG_MANAGER_HPP_



	namespace	DawnEngine
	{

		namespace	IO
		{

			/*
				Enumeration holding the possible file open modes.
			*/
			enum	LogFileOpenMode
			{
				APPEND = 0 , 
				TRUNCATE
			};


			/*
				A singleton class responsible for handling an UTF-8 application log.
			*/
			class	LogManagerA
			{
				private:

					//	A pointer to the single instance of the class.
					static	LogManagerA*					_instance;
					//	A static CriticalSection lock in order to make sure there are no concurency issues.
					static Concurrency::CriticalSection		_instance_lock;


					//	A SlimReadWriterLock in order to handle concurency.
					mutable Concurrency::SlimReadWriterLock	_lock;
					//	The log of entries.
					std::deque<LogEntryA>					_log;
					//	The tag that will be associated with the LOG_ERROR entries.
					std::string								_error_tag;
					//	The tag that will be associated with the LOG_WARNING entries.
					std::string								_warning_tag;
					//	The tag that will be associated with the LOG_MESSAGE entries.
					std::string								_message_tag;
					//	The name of the file to dump entries into when auto dump is enabled.
					std::string								_auto_dump_filename;
					//	The file stream used when auto dump is enabled.
					std::ofstream							_auto_dump_file;
					//	The open mode of the file stream used when auto dump is enabled.
					std::ofstream::open_mode				_auto_dump_file_mode;
					//	The number of entries before the log manager starts to replace previous entries.
					unsigned int							_max_entries;
					//	The entries of the log will be purged when this number of entries is reached.
					unsigned int							_auto_purge_threshold;
					//	The entries of the log will be dumped in the specified file when this number of entries is reached.
					unsigned int							_auto_dump_threshold;
					//	The position of the last entry to be dumped in the auto dump file.
					unsigned int							_last_dump_position;
					//	The number of new entries since the last auto dump.
					unsigned int							_auto_dump_counter;
					//	The stream used for echoing.
					std::ostream*							_echo_stream;
					//	 The separating character that will be utilized when dumping or printing the log entries.
					char									_separator;
					//	Whether or not the timestamp of the log entries will be displayed.
					bool									_add_timestamp;
					//	Whether auto-purge mode is enabled or not.
					bool									_auto_purge;
					//	Whether auto-dump mode is enabled or not.
					bool									_auto_dump;
					//	Whether echo mode is enabled or not.
					bool									_echo;


					//	The default constructor.
					LogManagerA();
					//	The destructor.
					~LogManagerA();


					//	Function responsible for returning a formatted string representing a tm struct.
					std::string								_format_time( const tm& time_struct );
					//	Function responsible for returning the current time in a formatted string.
					std::string								_generate_timestamp();
					//	Function responsible for parsing an input string similarly to the printf() function.
					std::string								_parse_input_string( const std::string& input , va_list arguments );
					//	 Function responsible for adding a new entry to the log.
					void									_log_entry( const LogEntryType& type , const std::string& message );
					//	Function responsible for printing a entry to the specified stream.
					void									_print_entry( std::ostream& stream , const LogEntryA& entry );
					//	Function responsible for dumping the contents of the log into the specified file stream.
					void									_dump_to_file( std::ofstream& file , const bool update_position );


				public:

					//	Static function allocating the required memory for the single instance.
					static bool								initialise();
					//	Static function deallocating the memory allocated by the initialise() function.
					static void								deinitialise();
					//	Static function returning a pointed to the single instance of the class.
					static LogManagerA*						get();


					//	Function responsible for changing the error tag of the log entries.
					void									error_tag( const std::string& tag );
					//	Function responsible for changing the warning tag of the log entries.
					void									warning_tag( const std::string& tag );
					//	Function responsible for changing the message tag of the log entries.
					void									message_tag( const std::string& tag );
					//	Function responsible for changing the separating character of the log entries.
					void									separator( const char character );
					//	Function responsible for enabling and disabling the timestamp of the log entries.
					void									add_timestamp( const bool value );
					//	Function responsible for setting the number of entries before the log manager starts replacing entries.
					void									max_entries( const unsigned int value );
					//	Function responsible for setting the amount of entries before an auto purge occurs.
					void									auto_purge_threshold( const unsigned int threshold );
					//	Function responsible for enabling and disabling the auto purge mode.
					void									auto_purge( const bool value );
					//	Function responsible for setting the open mode of the auto dump file. Possible values either APPEND or TRUNCATE.
					void									auto_dump_mode( const LogFileOpenMode& mode );
					//	Function responsible for setting the amount of entries before an auto dump occurs.
					void									auto_dump_threshold( const unsigned int threshold );
					//	Function responsible for setting the file that will be utilized in auto dump mode.
					void									auto_dump_file( const std::string& filename );
					//	Function responsible for enabling and disabling the auto dump mode.
					void									auto_dump( const bool value );
					//	Function responsible for changing the echo stream.
					void									echo_stream( std::ostream& stream );
					//	Function responsible for echoing the log entries to the echo stream.
					void									echo( const bool value );


					//	Function responsible for logging an error with parameters similar to the printf() function.
					void									log_error( const char* message , ... );
					//	Function responsible for logging a warning with parameters similar to the printf() function.
					void									log_warning( const char* message , ... );
					//	Function responsible for logging a message with parameters similar to the printf() function.
					void									log_message( const char* message , ... );
					//	Function responsible for logging an error with contents the given string.
					void									log_error( const std::string& message );
					//	Function responsible for logging a warning with contents the given string.
					void									log_warning( const std::string& message );
					//	Function responsible for logging a message with contents the given string.
					void									log_message( const std::string& message );
					//	Function responsible for logging an error with contents the message of the give exception.
					void									log_error( const LogExceptionA& exception );
					//	Function responsible for logging a warning with contents the message of the give exception.
					void									log_warning( const LogExceptionA& exception );
					//	Function responsible for logging a message with contents the message of the give exception.
					void									log_message( const LogExceptionA& exception );
					//	Function responsible for dumping the contents of the log into the specified file with open mode the mode given. Possible open modes are APPEND and TRUNCATE.
					void									dump( const std::string& filename , const LogFileOpenMode& mode = APPEND );
					//	Function responsible for purging the log.
					void									purge_log();


					//	Function returning the error tag.
					std::string								error_tag() const;
					//	Function returning the warning tag.
					std::string								warning_tag() const;
					//	Function returning the message tag.
					std::string								message_tag() const;
					//	Function returning the separating character.
					char									separator() const;
					//	Function returning whether the timestamp is enabled or not.
					bool									add_timestamp() const;
					//	Function returning the number of entries before the log manager starts replacing previous entries;
					unsigned int							max_entries() const;
					//	Function returning the number of log entries before a purge occurs in auto purge mode.
					unsigned int							auto_purge_threshold() const;
					//	Function returning whether the auto purge mode is enabled or not.
					bool									auto_purge() const;
					//	Function returning the open mode for the auto dump file.
					LogFileOpenMode							auto_dump_mode() const;
					//	Function returning the number of log entries before a dump occurs in auto dump mode.
					unsigned int							auto_dump_threshold() const;
					//	Function returning the name of the file that is used in auto dump mode.
					std::string								auto_dump_file() const;
					//	Function returning whether the auto dump mode is enabled or not.
					bool									auto_dump() const;
					//	Function returning whether the echo mode is enabled or not.
					bool									echo() const;


					//	Function returning the amount of entries currently in the log.
					unsigned int							entry_count() const;
					//	Function returning the entry at the specified index.
					LogEntryA								entry( const unsigned int index ) const;
					//	Function returning the desired amount of entries, starting from the oldest entries.
					std::vector<LogEntryA>					first_entries( const unsigned int amount , const unsigned int offset = 0 ) const;
					//	Function returning the desired amount of entries, starting from the latest entries. 
					std::vector<LogEntryA>					last_entries( const unsigned int amount , const unsigned int offset = 0 ) const;
			};

			/*
				A singleton class responsible for handling an UTF-16 application log.
			*/
			class	LogManagerW
			{
				private:

					//	A pointer to the single instance of the class.
					static	LogManagerW*					_instance;
					//	A static CriticalSection lock in order to make sure there are no concurency issues.
					static Concurrency::CriticalSection		_instance_lock;


					//	A SlimReadWriterLock in order to handle concurency.
					mutable Concurrency::SlimReadWriterLock	_lock;
					//	The log of entries.
					std::deque<LogEntryW>					_log;
					//	The file stream used when auto dump is enabled.
					std::wofstream							_auto_dump_file;
					//	The tag that will be associated with the LOG_ERROR entries.
					std::wstring							_error_tag;
					//	The tag that will be associated with the LOG_WARNING entries.
					std::wstring							_warning_tag;
					//	The tag that will be associated with the LOG_MESSAGE entries.
					std::wstring							_message_tag;
					//	The name of the file to dump entries into when auto dump is enabled.
					std::wstring							_auto_dump_filename;
					//	The locale of the auto-dump file.
					std::string								_auto_dump_locale;
					//	The open mode of the file stream used when auto dump is enabled.
					std::wofstream::open_mode				_auto_dump_file_mode;
					//	The number of entries before the log manager starts to replace previous entries.
					unsigned int							_max_entries;
					//	The entries of the log will be purged when this number of entries is reached.
					unsigned int							_auto_purge_threshold;
					//	The entries of the log will be dumped in the specified file when this number of entries is reached.
					unsigned int							_auto_dump_threshold;
					//	The position of the last entry to be dumped in the auto dump file.
					unsigned int							_last_dump_position;
					//	The number of new entries since the last auto dump.
					unsigned int							_auto_dump_counter;
					//	The stream used for echoing.
					std::wostream*							_echo_stream;
					//	 The separating character that will be utilized when dumping or printing the log entries.
					wchar_t									_separator;
					//	Whether or not the timestamp of the log entries will be displayed.
					bool									_add_timestamp;
					//	Whether auto-purge mode is enabled or not.
					bool									_auto_purge;
					//	Whether auto-dump mode is enabled or not.
					bool									_auto_dump;
					//	Whether echo mode is enabled or not.
					bool									_echo;


					//	The default constructor.
					LogManagerW();
					//	The destructor.
					~LogManagerW();


					//	Function responsible for returning a formatted string representing a tm struct.
					std::wstring							_format_time( const tm& time_struct );
					//	Function responsible for returning the current time in a formatted string.
					std::wstring							_generate_timestamp();
					//	Function responsible for parsing an input string similarly to the printf() function.
					std::wstring							_parse_input_string( const std::wstring& input , va_list arguments );
					//	 Function responsible for adding a new entry to the log.
					void									_log_entry( const LogEntryType& type , const std::wstring& message );
					//	Function responsible for printing a entry to the specified stream.
					void									_print_entry( std::wostream& stream , const LogEntryW& entry );
					//	Function responsible for dumping the contents of the log into the specified file stream.
					void									_dump_to_file( std::wofstream& file , const bool update_position );


				public:

					//	Static function allocating the required memory for the single instance.
					static bool								initialise();
					//	Static function deallocating the memory allocated by the initialise() function.
					static void								deinitialise();
					//	Static function returning a pointed to the single instance of the class.
					static LogManagerW*						get();


					//	Function responsible for changing the error tag of the log entries.
					void									error_tag( const std::wstring& tag );
					//	Function responsible for changing the warning tag of the log entries.
					void									warning_tag( const std::wstring& tag );
					//	Function responsible for changing the message tag of the log entries.
					void									message_tag( const std::wstring& tag );
					//	Function responsible for changing the separating character of the log entries.
					void									separator( const wchar_t character );
					//	Function responsible for enabling and disabling the timestamp of the log entries.
					void									add_timestamp( const bool value );
					//	Function responsible for setting the number of entries before the log manager starts replacing entries.
					void									max_entries( const unsigned int value );
					//	Function responsible for setting the amount of entries before an auto purge occurs.
					void									auto_purge_threshold( const unsigned int threshold );
					//	Function responsible for enabling and disabling the auto purge mode.
					void									auto_purge( const bool value );
					//	Function responsible for setting the open mode of the auto dump file. Possible values either APPEND or TRUNCATE.
					void									auto_dump_mode( const LogFileOpenMode& mode );
					//	Function responsible for setting the amount of entries before an auto dump occurs.
					void									auto_dump_threshold( const unsigned int threshold );
					//	Function responsible for setting the file that will be utilized in auto dump mode.
					void									auto_dump_file( const std::wstring& filename );
					//	Function responsible for imbuing the auto dump file with the specified locale.
					void									auto_dump_file_locale( const std::string& language );
					//	Function responsible for enabling and disabling the auto dump mode.
					void									auto_dump( const bool value );
					//	Function responsible for changing the echo stream.
					void									echo_stream( std::wostream& stream );
					//	Function responsible for echoing the log entries to the echo stream.
					void									echo( const bool value );


					//	Function responsible for logging an error with parameters similar to the printf() function.
					void									log_error( const wchar_t* message , ... );
					//	Function responsible for logging a warning with parameters similar to the printf() function.
					void									log_warning( const wchar_t* message , ... );
					//	Function responsible for logging a message with parameters similar to the printf() function.
					void									log_message( const wchar_t* message , ... );
					//	Function responsible for logging an error with contents the given string.
					void									log_error( const std::wstring& message );
					//	Function responsible for logging a warning with contents the given string.
					void									log_warning( const std::wstring& message );
					//	Function responsible for logging a message with contents the given string.
					void									log_message( const std::wstring& message );
					//	Function responsible for logging an error with contents the message of the give exception.
					void									log_error( const LogExceptionW& exception );
					//	Function responsible for logging a warning with contents the message of the give exception.
					void									log_warning( const LogExceptionW& exception );
					//	Function responsible for logging a message with contents the message of the give exception.
					void									log_message( const LogExceptionW& exception );
					//	Function responsible for dumping the contents of the log into the specified file with open mode the mode given. Possible open modes are APPEND and TRUNCATE.
					void									dump( const std::wstring& filename , const LogFileOpenMode& mode = APPEND , const std::string& locale = "" );
					//	Function responsible for purging the log.
					void									purge_log();


					//	Function returning the error tag.
					std::wstring							error_tag() const;
					//	Function returning the warning tag.
					std::wstring							warning_tag() const;
					//	Function returning the message tag.
					std::wstring							message_tag() const;
					//	Function returning the separating character.
					wchar_t									separator() const;
					//	Function returning whether the timestamp is enabled or not.
					bool									add_timestamp() const;
					//	Function returning the number of entries before the log manager starts replacing previous entries;
					unsigned int							max_entries() const;
					//	Function returning the number of log entries before a purge occurs in auto purge mode.
					unsigned int							auto_purge_threshold() const;
					//	Function returning whether the auto purge mode is enabled or not.
					bool									auto_purge() const;
					//	Function returning the open mode for the auto dump file.
					LogFileOpenMode							auto_dump_mode() const;
					//	Function returning the number of log entries before a dump occurs in auto dump mode.
					unsigned int							auto_dump_threshold() const;
					//	Function returning the auto dump file locale.
					std::string								auto_dump_file_locale() const;
					//	Function returning the name of the file that is used in auto dump mode.
					std::wstring							auto_dump_file() const;
					//	Function returning whether the auto dump mode is enabled or not.
					bool									auto_dump() const;
					//	Function returning whether the echo mode is enabled or not.
					bool									echo() const;

				
					//	Function returning the amount of entries currently in the log.
					unsigned int							entry_count() const;
					//	Function returning the entry at the specified index.
					LogEntryW								entry( const unsigned int index ) const;
					//	Function returning the desired amount of entries, starting from the oldest entries.
					std::vector<LogEntryW>					first_entries( const unsigned int amount  , const unsigned int offset = 0 ) const;
					//	Function returning the desired amount of entries, starting from the latest entries. 
					std::vector<LogEntryW>					last_entries( const unsigned int amount  , const unsigned int offset = 0 ) const;
			};

		
			/*
				Function definitions.
			*/


			/*
				UTF-8 functions.
			*/

			//	Static function allocating the required memory for the single instance.
			inline bool	LogManagerA::initialise()
			{
				bool	return_value = true;



				_instance_lock.enter();
			
				if ( _instance == NULL )
				{
					_instance = new (std::nothrow) LogManagerA();

					if ( _instance )
						return_value = true;
				}
				else
					return_value = true;
			
				_instance_lock.leave();


				return return_value;
			};

			//	Static function deallocating the memory allocated by the initialise() function.
			inline void	LogManagerA::deinitialise()
			{
				_instance_lock.enter();

				if ( _instance )
				{
					delete _instance;
					_instance = NULL;
				}
			
				_instance_lock.leave();
			};

			//	Static function returning a pointed to the single instance of the class.
			inline LogManagerA*		LogManagerA::get()													{ return _instance; };


			//	Function responsible for changing the error tag of the log entries.
			inline void	LogManagerA::error_tag( const std::string& tag )
			{ 
				_lock.acquire();
				_error_tag = tag;
				_lock.release();
			};

			//	Function responsible for changing the warning tag of the log entries.
			inline void	LogManagerA::warning_tag( const std::string& tag )
			{ 
				_lock.acquire();
				_warning_tag = tag;
				_lock.release();
			};

			//	Function responsible for changing the message tag of the log entries.
			inline void	LogManagerA::message_tag( const std::string& tag )
			{
				_lock.acquire();
				_message_tag = tag;
				_lock.release();
			};

			//	Function responsible for changing the separating character of the log entries.
			inline void	LogManagerA::separator( const char character )
			{
				_lock.acquire();
				_separator = character;
				_lock.release();
			};

			//	Function responsible for enabling and disabling the timestamp of the log entries.
			inline void	LogManagerA::add_timestamp( const bool value )
			{
				_lock.acquire();
				_add_timestamp = value;
				_lock.release();
			};

			//	Function responsible for setting the number of entries before the log manager starts replacing entries.
			inline void	LogManagerA::max_entries( const unsigned int value )
			{
				_lock.acquire();
				_max_entries = value;
				_lock.release();
			};

			//	Function responsible for setting the amount of entries before an auto purge occurs.
			inline void	LogManagerA::auto_purge_threshold( const unsigned int threshold )
			{
				_lock.acquire();
				_auto_purge_threshold = threshold;
				_lock.release();
			};

			//	Function responsible for enabling and disabling the auto purge mode.
			inline void	LogManagerA::auto_purge( const bool value )
			{ 
				_lock.acquire();
				_auto_purge = value;
				_lock.release();
			};

			//	Function responsible for setting the open mode of the auto dump file. Possible values either APPEND or TRUNCATE.
			inline void	LogManagerA::auto_dump_mode( const LogFileOpenMode& mode )
			{
				_lock.acquire();
				_auto_dump_file_mode = std::ofstream::out;

				if ( mode == APPEND )
					_auto_dump_file_mode |= std::ofstream::ate;
				else
					_auto_dump_file_mode |= std::ofstream::trunc;
			
				_lock.release();

				auto_dump_file(_auto_dump_filename);
			};

			//	Function responsible for setting the amount of entries before an auto dump occurs.
			inline void	LogManagerA::auto_dump_threshold( const unsigned int threshold )
			{
				_lock.acquire();
				_auto_dump_threshold = threshold;
				_lock.release();
			};

			//	Function responsible for enabling and disabling the auto dump mode.
			inline void	LogManagerA::auto_dump( const bool value )
			{
				_lock.acquire();
				_auto_dump = value;
				_lock.release();
			};

			//	Function responsible for changing the echo stream.
			inline void	LogManagerA::echo_stream( std::ostream& stream )
			{
				_lock.acquire();
				_echo_stream = &stream;
				_lock.release();
			};

			//	Function responsible for echoing the log entries to the echo stream.
			inline void	LogManagerA::echo( const bool value )
			{
				_lock.acquire();
				_echo = value;
				_lock.release();
			};


			//	Function responsible for logging an error with parameters similar to the printf() function.
			inline void	LogManagerA::log_error( const char* message , ... )
			{ 
				va_list	args;
			
			

				va_start(args,message);
				_log_entry(LOG_ERROR,_parse_input_string(message,args));
				va_end(args);
			};

			//	Function responsible for logging a warning with parameters similar to the printf() function.
			inline void	LogManagerA::log_warning( const char* message , ... )
			{
				va_list	args;
			
			

				va_start(args,message);
				_log_entry(LOG_WARNING,_parse_input_string(message,args));
				va_end(args);
			};

			//	Function responsible for logging a message with parameters similar to the printf() function.
			inline void	LogManagerA::log_message( const char* message , ... )
			{
				va_list	args;
			
			

				va_start(args,message);
				_log_entry(LOG_MESSAGE,_parse_input_string(message,args));
				va_end(args);
			};

			//	Function responsible for logging an error with contents the given string.
			inline void	LogManagerA::log_error( const std::string& message )				{ _log_entry(LOG_ERROR,message); };
			//	Function responsible for logging a warning with contents the given string.
			inline void	LogManagerA::log_warning( const std::string& message )				{ _log_entry(LOG_WARNING,message); };
			//	Function responsible for logging a message with contents the given string.
			inline void	LogManagerA::log_message( const std::string& message )				{ _log_entry(LOG_MESSAGE,message); };
			//	Function responsible for logging an error with contents the message of the give exception.
			inline void	LogManagerA::log_error( const LogExceptionA& exception )			{ _log_entry(LOG_ERROR,exception.message()); };
			//	Function responsible for logging a warning with contents the message of the give exception.
			inline void	LogManagerA::log_warning( const LogExceptionA& exception )			{ _log_entry(LOG_WARNING,exception.message()); };
			//	Function responsible for logging a message with contents the message of the give exception.
			inline void	LogManagerA::log_message( const LogExceptionA& exception )			{ _log_entry(LOG_MESSAGE,exception.message()); };


			//	Function returning the error tag.
			inline std::string		LogManagerA::error_tag() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _error_tag;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the warning tag.
			inline std::string		LogManagerA::warning_tag() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _warning_tag;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the message tag.
			inline std::string		LogManagerA::message_tag() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _message_tag;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the separating character.
			inline char	LogManagerA::separator() const
			{
				char	return_value = '\0';



				_lock.acquire_shared();
				return_value = _separator;
				_lock.release_shared();


				return return_value;
			};
		
			//	Function returning whether the timestamp is enabled or not.
			inline bool	LogManagerA::add_timestamp() const
			{
				bool	return_value = false;



				_lock.acquire_shared();
				return_value = _add_timestamp;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the number of entries before the log manager starts replacing previous entries;
			inline unsigned int	LogManagerA::max_entries() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _max_entries;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the number of log entries before a purge occurs in auto purge mode.
			inline unsigned int	LogManagerA::auto_purge_threshold() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _auto_purge_threshold;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning whether the auto purge mode is enabled or not.
			inline bool	LogManagerA::auto_purge() const
			{
				bool	return_value = false;



				_lock.acquire_shared();
				return_value = _auto_purge;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the open mode for the auto dump file.
			inline LogFileOpenMode	LogManagerA::auto_dump_mode() const
			{
				LogFileOpenMode	return_value = APPEND;


			
				_lock.acquire_shared();

				if ( _auto_dump_file_mode == (std::ofstream::out|std::ofstream::trunc) )
					return_value = TRUNCATE;

				_lock.release_shared();


				return return_value;
			};

			//	Function returning the number of log entries before a dump occurs in auto dump mode.
			inline unsigned int	LogManagerA::auto_dump_threshold() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _auto_dump_threshold;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the name of the file that is used in auto dump mode.
			inline std::string	LogManagerA::auto_dump_file() const	
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _auto_dump_filename;
				_lock.release_shared();


				return return_value;
			};
		
			//	Function returning whether the auto dump mode is enabled or not.
			inline bool	LogManagerA::auto_dump() const
			{
				bool	return_value = false;



				_lock.acquire_shared();
				return_value = _auto_dump;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning whether the echo mode is enabled or not.
			inline bool	LogManagerA::echo() const
			{
				bool	return_value = false;



				_lock.acquire_shared();
				return_value = _echo;
				_lock.release_shared();


				return return_value;
			};


			//	Function returning the amount of entries currently in the log.
			inline unsigned int	LogManagerA::entry_count() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _log.size();
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the entry at the specified index.
			inline LogEntryA	LogManagerA::entry( const unsigned int index ) const
			{
				LogEntryA	return_value("","",LOG_MESSAGE);



				_lock.acquire_shared();

				if ( index < _log.size() )
					return _log[index]; 
			
				_lock.release_shared();


				return return_value;
			};



			/*
				UTF-16 functions.
			*/

			//	Static function allocating the required memory for the single instance.
			inline bool	LogManagerW::initialise()
			{
				bool	return_value = true;



				_instance_lock.enter();
			
				if ( _instance == NULL )
				{
					_instance = new (std::nothrow) LogManagerW();

					if ( _instance )
						return_value = true;
				}
				else
					return_value = true;
			
				_instance_lock.leave();


				return return_value;
			};

			//	Static function deallocating the memory allocated by the initialise() function.
			inline void	LogManagerW::deinitialise()
			{
				_instance_lock.enter();

				if ( _instance )
				{
					delete _instance;
					_instance = NULL;
				}
			
				_instance_lock.leave();
			};

			//	Static function returning a pointed to the single instance of the class.
			inline LogManagerW*		LogManagerW::get()													{ return _instance; };


			//	Function responsible for changing the error tag of the log entries.
			inline void	LogManagerW::error_tag( const std::wstring& tag )
			{ 
				_lock.acquire();
				_error_tag = tag;
				_lock.release();
			};

			//	Function responsible for changing the warning tag of the log entries.
			inline void	LogManagerW::warning_tag( const std::wstring& tag )
			{ 
				_lock.acquire();
				_warning_tag = tag;
				_lock.release();
			};

			//	Function responsible for changing the message tag of the log entries.
			inline void	LogManagerW::message_tag( const std::wstring& tag )
			{
				_lock.acquire();
				_message_tag = tag;
				_lock.release();
			};

			//	Function responsible for changing the separating character of the log entries.
			inline void	LogManagerW::separator( const wchar_t character )
			{
				_lock.acquire();
				_separator = character;
				_lock.release();
			};

			//	Function responsible for enabling and disabling the timestamp of the log entries.
			inline void	LogManagerW::add_timestamp( const bool value )
			{
				_lock.acquire();
				_add_timestamp = value;
				_lock.release();
			};

			//	Function responsible for setting the number of entries before the log manager starts replacing entries.
			inline void	LogManagerW::max_entries( const unsigned int value )
			{
				_lock.acquire();
				_max_entries = value;
				_lock.release();
			};

			//	Function responsible for setting the amount of entries before an auto purge occurs.
			inline void	LogManagerW::auto_purge_threshold( const unsigned int threshold )
			{
				_lock.acquire();
				_auto_purge_threshold = threshold;
				_lock.release();
			};

			//	Function responsible for enabling and disabling the auto purge mode.
			inline void	LogManagerW::auto_purge( const bool value )
			{ 
				_lock.acquire();
				_auto_purge = value;
				_lock.release();
			};

			//	Function responsible for setting the open mode of the auto dump file. Possible values either APPEND or TRUNCATE.
			inline void	LogManagerW::auto_dump_mode( const LogFileOpenMode& mode )
			{
				_lock.acquire();
				_auto_dump_file_mode = std::wofstream::out;

				if ( mode == APPEND )
					_auto_dump_file_mode |= std::wofstream::ate;
				else
					_auto_dump_file_mode |= std::wofstream::trunc;
			
				_lock.release();

				auto_dump_file(_auto_dump_filename);
			};

			//	Function responsible for setting the amount of entries before an auto dump occurs.
			inline void	LogManagerW::auto_dump_threshold( const unsigned int threshold )
			{
				_lock.acquire();
				_auto_dump_threshold = threshold;
				_lock.release();
			};

			//	Function responsible for imbuing the auto dump file with the specified locale.
			inline void	LogManagerW::auto_dump_file_locale( const std::string& language )
			{
				_lock.acquire();
				_auto_dump_locale = language;
				_lock.release();

				auto_dump_file(_auto_dump_filename);
			};

			//	Function responsible for enabling and disabling the auto dump mode.
			inline void	LogManagerW::auto_dump( const bool value )
			{
				_lock.acquire();
				_auto_dump = value;
				_lock.release();
			};

			//	Function responsible for changing the echo stream.
			inline void	LogManagerW::echo_stream( std::wostream& stream )
			{
				_lock.acquire();
				_echo_stream = &stream;
				_lock.release();
			};

			//	Function responsible for echoing the log entries to the echo stream.
			inline void	LogManagerW::echo( const bool value )
			{
				_lock.acquire();
				_echo = value;
				_lock.release();
			};


			//	Function responsible for logging an error with parameters similar to the printf() function.
			inline void	LogManagerW::log_error( const wchar_t* message , ... )
			{ 
				va_list	args;
			
			

				va_start(args,message);
				_log_entry(LOG_ERROR,_parse_input_string(message,args));
				va_end(args);
			};

			//	Function responsible for logging a warning with parameters similar to the printf() function.
			inline void	LogManagerW::log_warning( const wchar_t* message , ... )
			{
				va_list	args;
			
			

				va_start(args,message);
				_log_entry(LOG_WARNING,_parse_input_string(message,args));
				va_end(args);
			};

			//	Function responsible for logging a message with parameters similar to the printf() function.
			inline void	LogManagerW::log_message( const wchar_t* message , ... )
			{
				va_list	args;
			
			

				va_start(args,message);
				_log_entry(LOG_MESSAGE,_parse_input_string(message,args));
				va_end(args);
			};

			//	Function responsible for logging an error with contents the given string.
			inline void	LogManagerW::log_error( const std::wstring& message )				{ _log_entry(LOG_ERROR,message); };
			//	Function responsible for logging a warning with contents the given string.
			inline void	LogManagerW::log_warning( const std::wstring& message )				{ _log_entry(LOG_WARNING,message); };
			//	Function responsible for logging a message with contents the given string.
			inline void	LogManagerW::log_message( const std::wstring& message )				{ _log_entry(LOG_MESSAGE,message); };
			//	Function responsible for logging an error with contents the message of the give exception.
			inline void	LogManagerW::log_error( const LogExceptionW& exception )			{ _log_entry(LOG_ERROR,exception.message()); };
			//	Function responsible for logging a warning with contents the message of the give exception.
			inline void	LogManagerW::log_warning( const LogExceptionW& exception )			{ _log_entry(LOG_WARNING,exception.message()); };
			//	Function responsible for logging a message with contents the message of the give exception.
			inline void	LogManagerW::log_message( const LogExceptionW& exception )			{ _log_entry(LOG_MESSAGE,exception.message()); };


			//	Function returning the error tag.
			inline std::wstring	LogManagerW::error_tag() const
			{
				std::wstring	return_value(L"");



				_lock.acquire_shared();
				return_value = _error_tag;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the warning tag.
			inline std::wstring	LogManagerW::warning_tag() const
			{
				std::wstring	return_value(L"");



				_lock.acquire_shared();
				return_value = _warning_tag;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the message tag.
			inline std::wstring	LogManagerW::message_tag() const
			{
				std::wstring	return_value(L"");



				_lock.acquire_shared();
				return_value = _message_tag;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the separating character.
			inline wchar_t	LogManagerW::separator() const
			{
				wchar_t	return_value = '\0';



				_lock.acquire_shared();
				return_value = _separator;
				_lock.release_shared();


				return return_value;
			};
		
			//	Function returning whether the timestamp is enabled or not.
			inline bool	LogManagerW::add_timestamp() const
			{
				bool	return_value = false;



				_lock.acquire_shared();
				return_value = _add_timestamp;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the number of entries before the log manager starts replacing previous entries;
			inline unsigned int	LogManagerW::max_entries() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _max_entries;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the number of log entries before a purge occurs in auto purge mode.
			inline unsigned int	LogManagerW::auto_purge_threshold() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _auto_purge_threshold;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning whether the auto purge mode is enabled or not.
			inline bool	LogManagerW::auto_purge() const
			{
				bool	return_value = false;



				_lock.acquire_shared();
				return_value = _auto_purge;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the open mode for the auto dump file.
			inline LogFileOpenMode	LogManagerW::auto_dump_mode() const
			{
				LogFileOpenMode	return_value = APPEND;


			
				_lock.acquire_shared();

				if ( _auto_dump_file_mode == (std::ofstream::out|std::ofstream::trunc) )
					return_value = TRUNCATE;

				_lock.release_shared();


				return return_value;
			};

			//	Function returning the number of log entries before a dump occurs in auto dump mode.
			inline unsigned int	LogManagerW::auto_dump_threshold() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _auto_dump_threshold;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the auto dump file locale.
			inline std::string	LogManagerW::auto_dump_file_locale() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _auto_dump_locale;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the name of the file that is used in auto dump mode.
			inline std::wstring	LogManagerW::auto_dump_file() const	
			{
				std::wstring	return_value(L"");



				_lock.acquire_shared();
				return_value = _auto_dump_filename;
				_lock.release_shared();


				return return_value;
			};
		
			//	Function returning whether the auto dump mode is enabled or not.
			inline bool	LogManagerW::auto_dump() const
			{
				bool	return_value = false;



				_lock.acquire_shared();
				return_value = _auto_dump;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning whether the echo mode is enabled or not.
			inline bool	LogManagerW::echo() const
			{
				bool	return_value = false;



				_lock.acquire_shared();
				return_value = _echo;
				_lock.release_shared();


				return return_value;
			};


			//	Function returning the amount of entries currently in the log.
			inline unsigned int	LogManagerW::entry_count() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _log.size();
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the entry at the specified index.
			inline LogEntryW	LogManagerW::entry( const unsigned int index ) const
			{
				LogEntryW	return_value(L"",L"",LOG_MESSAGE);



				_lock.acquire_shared();

				if ( index < _log.size() )
					return _log[index]; 
			
				_lock.release_shared();


				return return_value;
			};
		
		}	/* IO */

	}	/* DawnEngine */



#endif		/* DAWN_ENGINE_LOG_MANAGER_HPP_ */