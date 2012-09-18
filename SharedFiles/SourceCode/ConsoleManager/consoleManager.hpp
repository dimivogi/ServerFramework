#include	"../globalDefinitions.hpp"
#include	"../Lock/slimReadWriterLock.hpp"
#include	"../Log/logManager.hpp"
#ifdef		MOUSE_MOVED
	#undef		MOUSE_MOVED
#endif		/* MOUSE_MOVED */
#include	"../PDCursesWindow/pdCursesWindow.hpp"

#ifndef		_DAWN_ENGINE_CONSOLE_MANAGER_HPP_
	#define	_DAWN_ENGINE_CONSOLE_MANAGER_HPP_



	namespace	DawnEngine
	{

		namespace	Console
		{

			/*
				Type definitions.
			*/

			typedef	void (*ConsoleInputParserA)( const std::string );
			typedef	void (*ConsoleInputParserW)( const std::wstring );


			/*
				Class responsible of outputting the contents of the wide character log to a PDCurses window.
			*/
			class	ConsoleManagerA
			{
				private:

					//	A lock for managing the creation and deletion of the instance in a thread safe manner.
					static Concurrency::SlimReadWriterLock	_instance_lock;
					//	The single instance of the class.
					static ConsoleManagerA*					_instance;


					//	A slim read writer lock for the class in order to avoid concurency issues.
					mutable Concurrency::SlimReadWriterLock	_lock;
					//	A pointer to a pdcurses window, which will be responsible for handling the console input and output.
					PDCursesWindow*							_window;
					//	A variable containing the current line input.
					std::string								_input;
					//	A variable containing the text that is displayed before the current line input.
					std::string								_input_prefix;
					//	A variable containing a string full of space characters with size equal to one line of the console.
					std::string								_line;
					//	A variable containing the string to be used for error messages.
					std::string								_error_tag;
					//	A variable containing the string to be used for warning messages.
					std::string								_warning_tag;
					//	A variable containing the string to be used for normal messages. 
					std::string								_message_tag;
					//	A variable containing the string to be used to separate the timestamp from the message type tag.
					std::string								_type_separator;
					//	A variable containing the string to be used to separate the error tag from the message text.
					std::string								_error_separator;
					//	A variable containing the string to be used to separate the warning tag from the message text.
					std::string								_warning_separator;
					//	A variable containing the string to be used to separate the normal message tag from the message text.
					std::string								_message_separator;
					//	A variable containing the pointer to the user-defined function for parsing any input.
					ConsoleInputParserA						_input_function;
					//	A variable containing the number of entries the console can hold.
					unsigned int							_entry_count;
					//	A variable containing the current scrolling offset.
					unsigned int							_offset;
					//	A variable containing the normal scroll step.
					unsigned int							_normal_scroll;
					//	A variable containing the fast scroll step.
					unsigned int							_fast_scroll;
					//	A variable containing the width of the console.
					int										_width;
					//	A variable containing the height of the console.
					int										_height;
					//	A variable containing the character to be used to separate the date from the time in a message timestamp.
					char									_timestamp_separator;
					//	A variable containing the character that is used by the LogManager to separate strings.
					char									_log_separator;
					//	A variable containing whether a line has been read from the console.
					bool									_read_line;

				
					//	Function responsible of getting and parsing a single character from the console.
					void									_get_character();
					//	Function responsible of printing the last _entry_count entries of the LogManager to the console.
					void									_print_entries();
					//	Function responsible of printing the current input to the console.
					void									_print_input();


					//	The default constructor of the class.
					ConsoleManagerA();
					//	The destructor of the class.
					~ConsoleManagerA();


				public:

					//	Function responsible of initialising the instance of the class.
					static bool								initialise();
					//	Function responsible of returning the instance of the class.
					static ConsoleManagerA*					get();
					//	Function responsible of de-initialising the instance of the class.
					static void								deinitialise();


					//	Function responsible of changing the input prefix text.
					void									input_prefix( const std::string& prefix );
					//	Function responsible of changing the error tag text.
					void									error_tag( const std::string& tag );
					//	Function responsible of changing the warning tag text.
					void									warning_tag( const std::string& tag );
					//	Function responsible of changing the message tag text.
					void									message_tag( const std::string& tag );
					//	Function responsible of changing the type separator text.
					void									type_separator( const std::string& separator );
					// Function responsible of changing the error tag separator text.
					void									error_separator( const std::string& separator );
					//	Function responsible of changing the warning tag separator text.
					void									warning_separator( const std::string& separator );
					//	Function responsible of changing the message tag separator text.
					void									message_separator( const std::string& separator );
					//	Function responsible of changing the input function.
					void									input_function( ConsoleInputParserA function );
					//	Function responsible of changing the normal scroll step.
					void									normal_scroll( const unsigned int amount );
					//	Function responsible of changing the fast scroll step.
					void									fast_scroll( const unsigned int amount );
					//	Function responsible of changing the type of the cursor of the console.
					void									cursor_type( const CursorType& type );
					//	Function responsible of changing the separating character of the timestamp.
					void									timestamp_separator( const char character );
					//	Function responsible of changing the separator used by the LogManager.
					void									log_sepator( const char character );


					//	Function returning the input prefix text.
					std::string								input_prefix() const;
					//	Function returning the error tag text.
					std::string								error_tag() const;
					//	Function returning the warning tag text.
					std::string								warning_tag() const;
					//	Function returning the normal message tag text.
					std::string								message_tag() const;
					//	Function returning the type separator text.
					std::string								type_separator() const;
					//	Function returning the error tag separator text.
					std::string								error_separator() const;
					//	Function returning the warning tag separator text.
					std::string								warning_separator() const;
					//	Function returning the normal message tag separator text.
					std::string								message_separator() const;
					//	Function returning the input function.
					ConsoleInputParserA						input_function() const;
					//	Function returning the number of entries the console can hold.
					unsigned int							entry_count() const;
					//	Function returning the normal scroll step.
					unsigned int							normal_scroll() const;
					//	Function returning the fast scroll step.
					unsigned int							fast_scroll() const;
					//	Function returning the timestamp separator character.
					char									timestamp_separator() const;
					//	Function returning the separator character used by the LogManager.
					char									log_separator() const;


					//	Function responsible for creating and initialising a console window with the given width and height.
					void									create( const int width , const int height );
					//	Function responsible for resizing the console window to the given width and height.
					void									resize( const int width , const int height );
					//	Function responsible for destroying a created console window.
					void									destroy();
					//	Function responsible for updating the console window.
					void									update();
			};


			/*
				Class responsible of outputting the contents of the wide character log to a PDCurses window.
			*/
			class	ConsoleManagerW
			{
				private:

					//	A lock for managing the creation and deletion of the instance in a thread safe manner.
					static Concurrency::SlimReadWriterLock	_instance_lock;
					//	The single instance of the class.
					static ConsoleManagerW*					_instance;


					//	A slim read writer lock for the class in order to avoid concurency issues.
					mutable Concurrency::SlimReadWriterLock	_lock;
					//	A pointer to a pdcurses window, which will be responsible for handling the console input and output.
					PDCursesWindow*							_window;
					//	A variable containing the current line input.
					std::wstring							_input;
					//	A variable containing the text that is displayed before the current line input.
					std::wstring							_input_prefix;
					//	A variable containing a string full of space characters with size equal to one line of the console.
					std::wstring							_line;
					//	A variable containing the string to be used for error messages.
					std::wstring							_error_tag;
					//	A variable containing the string to be used for warning messages.
					std::wstring							_warning_tag;
					//	A variable containing the string to be used for normal messages. 
					std::wstring							_message_tag;
					//	A variable containing the string to be used to separate the timestamp from the message type tag.
					std::wstring							_type_separator;
					//	A variable containing the string to be used to separate the error tag from the message text.
					std::wstring							_error_separator;
					//	A variable containing the string to be used to separate the warning tag from the message text.
					std::wstring							_warning_separator;
					//	A variable containing the string to be used to separate the normal message tag from the message text.
					std::wstring							_message_separator;
					//	A variable containing the pointer to the user-defined function for parsing any input.
					ConsoleInputParserW						_input_function;
					//	A variable containing the number of entries the console can hold.
					unsigned int							_entry_count;
					//	A variable containing the current scrolling offset.
					unsigned int							_offset;
					//	A variable containing the normal scroll step.
					unsigned int							_normal_scroll;
					//	A variable containing the fast scroll step.
					unsigned int							_fast_scroll;
					//	A variable containing the width of the console.
					int										_width;
					//	A variable containing the height of the console.
					int										_height;
					//	A variable containing the character to be used to separate the date from the time in a message timestamp.
					wchar_t									_timestamp_separator;
					//	A variable containing the character that is used by the LogManager to separate strings.
					wchar_t									_log_separator;
					//	A variable containing whether a line has been read from the console.
					bool									_read_line;

				
					//	Function responsible of getting and parsing a single character from the console.
					void									_get_character();
					//	Function responsible of printing the last _entry_count entries of the LogManager to the console.
					void									_print_entries();
					//	Function responsible of printing the current input to the console.
					void									_print_input();


					//	The default constructor of the class.
					ConsoleManagerW();
					//	The destructor of the class.
					~ConsoleManagerW();


				public:

					//	Function responsible of initialising the instance of the class.
					static bool								initialise();
					//	Function responsible of returning the instance of the class.
					static ConsoleManagerW*					get();
					//	Function responsible of de-initialising the instance of the class.
					static void								deinitialise();


					//	Function responsible of changing the input prefix text.
					void									input_prefix( const std::wstring& prefix );
					//	Function responsible of changing the error tag text.
					void									error_tag( const std::wstring& tag );
					//	Function responsible of changing the warning tag text.
					void									warning_tag( const std::wstring& tag );
					//	Function responsible of changing the message tag text.
					void									message_tag( const std::wstring& tag );
					//	Function responsible of changing the type separator text.
					void									type_separator( const std::wstring& separator );
					// Function responsible of changing the error tag separator text.
					void									error_separator( const std::wstring& separator );
					//	Function responsible of changing the warning tag separator text.
					void									warning_separator( const std::wstring& separator );
					//	Function responsible of changing the message tag separator text.
					void									message_separator( const std::wstring& separator );
					//	Function responsible of changing the input function.
					void									input_function( ConsoleInputParserW function );
					//	Function responsible of changing the normal scroll step.
					void									normal_scroll( const unsigned int amount );
					//	Function responsible of changing the fast scroll step.
					void									fast_scroll( const unsigned int amount );
					//	Function responsible of changing the type of the cursor of the console.
					void									cursor_type( const CursorType& type );
					//	Function responsible of changing the separating character of the timestamp.
					void									timestamp_separator( const wchar_t character );
					//	Function responsible of changing the separator used by the LogManager.
					void									log_sepator( const wchar_t character );


					//	Function returning the input prefix text.
					std::wstring							input_prefix() const;
					//	Function returning the error tag text.
					std::wstring							error_tag() const;
					//	Function returning the warning tag text.
					std::wstring							warning_tag() const;
					//	Function returning the normal message tag text.
					std::wstring							message_tag() const;
					//	Function returning the type separator text.
					std::wstring							type_separator() const;
					//	Function returning the error tag separator text.
					std::wstring							error_separator() const;
					//	Function returning the warning tag separator text.
					std::wstring							warning_separator() const;
					//	Function returning the normal message tag separator text.
					std::wstring							message_separator() const;
					//	Function returning the input function.
					ConsoleInputParserW						input_function() const;
					//	Function returning the number of entries the console can hold.
					unsigned int							entry_count() const;
					//	Function returning the normal scroll step.
					unsigned int							normal_scroll() const;
					//	Function returning the fast scroll step.
					unsigned int							fast_scroll() const;
					//	Function returning the timestamp separator character.
					wchar_t									timestamp_separator() const;
					//	Function returning the separator character used by the LogManager.
					wchar_t									log_separator() const;


					//	Function responsible for creating and initialising a console window with the given width and height.
					void									create( const int width , const int height );
					//	Function responsible for resizing the console window to the given width and height.
					void									resize( const int width , const int height );
					//	Function responsible for destroying a created console window.
					void									destroy();
					//	Function responsible for updating the console window.
					void									update();
			};



			/*
				Function definitions.
			*/

		
			/*
				UTF-8 Console Manager
			*/

			//	Function responsible of initialising the instance of the class.
			inline bool	ConsoleManagerA::initialise()
			{
				bool	return_value = true;



				_instance_lock.acquire();

				if ( _instance == NULL )
				{
					_instance = new (std::nothrow) ConsoleManagerA();

					if ( _instance == NULL )
						return_value = false;
				}

				_instance_lock.release();


				return return_value;
			};

			//	Function responsible of returning the instance of the class.
			inline ConsoleManagerA*	ConsoleManagerA::get()
			{
				ConsoleManagerA*	return_value = NULL;


				_instance_lock.acquire_shared();
				return_value = _instance;
				_instance_lock.release_shared();


				return return_value;
			};

			//	Function responsible of de-initialising the instance of the class.
			inline void	ConsoleManagerA::deinitialise()
			{
				_instance_lock.acquire();

				if ( _instance != NULL )
				{
					_instance->destroy();
					delete _instance;
					_instance = NULL;
				}

				_instance_lock.release();
			};


			//	Function responsible of changing the input prefix text.
			inline void	ConsoleManagerA::input_prefix( const std::string& prefix )
			{
				_lock.acquire();
				_input_prefix = prefix;
				_lock.release();
			};

			//	Function responsible of changing the error tag text.
			inline void	ConsoleManagerA::error_tag( const std::string& tag )
			{
				IO::LogManagerA*	log_manager = IO::LogManagerA::get();



				_lock.acquire();
				_error_tag = tag;

				if ( log_manager != NULL )
					log_manager->error_tag(tag);

				_lock.release();
			};

			//	Function responsible of changing the warning tag text.
			inline void	ConsoleManagerA::warning_tag( const std::string& tag )
			{
				IO::LogManagerA*	log_manager = IO::LogManagerA::get();



				_lock.acquire();
				_warning_tag = tag;

				if ( log_manager != NULL )
					log_manager->warning_tag(tag);

				_lock.release();
			};

			//	Function responsible of changing the message tag text.
			inline void	ConsoleManagerA::message_tag( const std::string& tag )
			{
				IO::LogManagerA*	log_manager = IO::LogManagerA::get();



				_lock.acquire();
				_message_tag = tag;

				if ( log_manager != NULL )
					log_manager->message_tag(tag);

				_lock.release();
			};

			//	Function responsible of changing the type separator text.
			inline void	ConsoleManagerA::type_separator( const std::string& separator )
			{
				_lock.acquire();
				_type_separator = separator;
				_lock.release();
			};

			// Function responsible of changing the error tag separator text.
			inline void	ConsoleManagerA::error_separator( const std::string& separator )
			{
				_lock.acquire();
				_error_separator = separator;
				_lock.release();
			};

			//	Function responsible of changing the warning tag separator text.
			inline void	ConsoleManagerA::warning_separator( const std::string& separator )
			{
				_lock.acquire();
				_warning_separator = separator;
				_lock.release();
			};

			//	Function responsible of changing the message tag separator text.
			inline void	ConsoleManagerA::message_separator( const std::string& separator )
			{
				_lock.acquire();
				_message_separator = separator;
				_lock.release();
			};

			//	Function responsible of changing the input function.
			inline void	ConsoleManagerA::input_function(  ConsoleInputParserA function )
			{
				_lock.acquire();
				_input_function = function;
				_lock.release();
			};

			//	Function responsible of changing the normal scroll step.
			inline void	ConsoleManagerA::normal_scroll( const unsigned int amount )
			{
				_lock.acquire();
				_normal_scroll = std::min(amount,_fast_scroll);
				_lock.release();
			};

			//	Function responsible of changing the fast scroll step.
			inline void	ConsoleManagerA::fast_scroll( const unsigned int amount )
			{
				_lock.acquire();
				_fast_scroll = std::max(amount,_normal_scroll);
				_lock.release();
			};

			//	Function responsible of changing the type of the cursor of the console.
			inline void ConsoleManagerA::cursor_type( const CursorType& type )
			{
				_lock.acquire();

				if ( _window != NULL )
					_window->cursor_type(type);

				_lock.release();
			};

			//	Function responsible of changing the separating character of the timestamp.
			inline void	ConsoleManagerA::timestamp_separator( const char character )
			{
				_lock.acquire();
				_timestamp_separator = character;
				_lock.release();
			};

			//	Function responsible of changing the separator used by the LogManager.
			inline void	ConsoleManagerA::log_sepator( const char character )
			{
				IO::LogManagerA*	log_manager = IO::LogManagerA::get();



				_lock.acquire();
				_log_separator = character;

				if ( log_manager != NULL )
					log_manager->separator(character);

				_lock.release();
			};

				
			//	Function returning the input prefix text.
			inline std::string		ConsoleManagerA::input_prefix() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _input_prefix;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the error tag text.
			inline std::string		ConsoleManagerA::error_tag() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _error_tag;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the warning tag text.
			inline std::string		ConsoleManagerA::warning_tag() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _warning_tag;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the normal message tag text.
			inline std::string		ConsoleManagerA::message_tag() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _message_tag;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the type separator text.
			inline std::string		ConsoleManagerA::type_separator() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _type_separator;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the error tag separator text.
			inline std::string		ConsoleManagerA::error_separator() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _error_separator;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the warning tag separator text.
			inline std::string		ConsoleManagerA::warning_separator() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _warning_separator;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the normal message tag separator text.
			inline std::string		ConsoleManagerA::message_separator() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _message_separator;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the input function.
			inline ConsoleInputParserA	ConsoleManagerA::input_function() const
			{
				ConsoleInputParserA	return_value = NULL;



				_lock.acquire_shared();
				return_value = _input_function;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the number of entries the console can hold.
			inline unsigned int	ConsoleManagerA::entry_count() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _entry_count;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the normal scroll step.
			inline unsigned int	ConsoleManagerA::normal_scroll() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _normal_scroll;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the fast scroll step.
			inline unsigned int	ConsoleManagerA::fast_scroll() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _fast_scroll;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the timestamp separator character.
			inline char	ConsoleManagerA::timestamp_separator() const
			{
				char	return_value = '\0';



				_lock.acquire_shared();
				return_value = _timestamp_separator;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the separator character used by the LogManager.
			inline char	ConsoleManagerA::log_separator() const
			{
				char	return_value = '\0';



				_lock.acquire_shared();
				return_value = _log_separator;
				_lock.release_shared();


				return return_value;
			};

		
			//	Function responsible for resizing the console window to the given width and height;
			inline void	ConsoleManagerA::resize( const int width , const int height )
			{
				_lock.acquire();

				if ( _window != NULL )
				{
					_window->resize_terminal(width,height);
					_window->size(_width,_height);
					_entry_count = static_cast<unsigned int>(abs(_height - 1));
					_line.resize(_width,' ');
				}

				_lock.release();
			};
		
			//	Function responsible for destroying a created console window.
			inline void	ConsoleManagerA::destroy()
			{
				_lock.acquire();

				if ( _window != NULL )
				{
					_window->destroy();
					_window->deinitialise();
					_window = NULL;
				}

				_lock.release();
			};

			//	Function responsible for updating the console window.
			inline 	void	ConsoleManagerA::update()
			{
				_get_character();
				_print_entries();
				_print_input();
				Sleep(34);
			};



			/*
				UTF-16 Console Manager
			*/

			//	Function responsible of initialising the instance of the class.
			inline bool	ConsoleManagerW::initialise()
			{
				bool	return_value = true;



				_instance_lock.acquire();

				if ( _instance == NULL )
				{
					_instance = new (std::nothrow) ConsoleManagerW();

					if ( _instance == NULL )
						return_value = false;
				}

				_instance_lock.release();


				return return_value;
			};

			//	Function responsible of returning the instance of the class.
			inline ConsoleManagerW*	ConsoleManagerW::get()
			{
				ConsoleManagerW*	return_value = NULL;


				_instance_lock.acquire_shared();
				return_value = _instance;
				_instance_lock.release_shared();


				return return_value;
			};

			//	Function responsible of de-initialising the instance of the class.
			inline void	ConsoleManagerW::deinitialise()
			{
				_instance_lock.acquire();

				if ( _instance != NULL )
				{
					_instance->destroy();
					delete _instance;
					_instance = NULL;
				}

				_instance_lock.release();
			};


			//	Function responsible of changing the input prefix text.
			inline void	ConsoleManagerW::input_prefix( const std::wstring& prefix )
			{
				_lock.acquire();
				_input_prefix = prefix;
				_lock.release();
			};

			//	Function responsible of changing the error tag text.
			inline void	ConsoleManagerW::error_tag( const std::wstring& tag )
			{
				IO::LogManagerW*	log_manager = IO::LogManagerW::get();



				_lock.acquire();
				_error_tag = tag;

				if ( log_manager != NULL )
					log_manager->error_tag(tag);

				_lock.release();
			};

			//	Function responsible of changing the warning tag text.
			inline void	ConsoleManagerW::warning_tag( const std::wstring& tag )
			{
				IO::LogManagerW*	log_manager = IO::LogManagerW::get();



				_lock.acquire();
				_warning_tag = tag;

				if ( log_manager != NULL )
					log_manager->warning_tag(tag);

				_lock.release();
			};

			//	Function responsible of changing the message tag text.
			inline void	ConsoleManagerW::message_tag( const std::wstring& tag )
			{
				IO::LogManagerW*	log_manager = IO::LogManagerW::get();



				_lock.acquire();
				_message_tag = tag;

				if ( log_manager != NULL )
					log_manager->message_tag(tag);

				_lock.release();
			};

			//	Function responsible of changing the type separator text.
			inline void	ConsoleManagerW::type_separator( const std::wstring& separator )
			{
				_lock.acquire();
				_type_separator = separator;
				_lock.release();
			};

			// Function responsible of changing the error tag separator text.
			inline void	ConsoleManagerW::error_separator( const std::wstring& separator )
			{
				_lock.acquire();
				_error_separator = separator;
				_lock.release();
			};

			//	Function responsible of changing the warning tag separator text.
			inline void	ConsoleManagerW::warning_separator( const std::wstring& separator )
			{
				_lock.acquire();
				_warning_separator = separator;
				_lock.release();
			};

			//	Function responsible of changing the message tag separator text.
			inline void	ConsoleManagerW::message_separator( const std::wstring& separator )
			{
				_lock.acquire();
				_message_separator = separator;
				_lock.release();
			};

			//	Function responsible of changing the input function.
			inline void	ConsoleManagerW::input_function(  ConsoleInputParserW function )
			{
				_lock.acquire();
				_input_function = function;
				_lock.release();
			};

			//	Function responsible of changing the normal scroll step.
			inline void	ConsoleManagerW::normal_scroll( const unsigned int amount )
			{
				_lock.acquire();
				_normal_scroll = std::min(amount,_fast_scroll);
				_lock.release();
			};

			//	Function responsible of changing the fast scroll step.
			inline void	ConsoleManagerW::fast_scroll( const unsigned int amount )
			{
				_lock.acquire();
				_fast_scroll = std::max(amount,_normal_scroll);
				_lock.release();
			};

			//	Function responsible of changing the type of the cursor of the console.
			inline void ConsoleManagerW::cursor_type( const CursorType& type )
			{
				_lock.acquire();

				if ( _window != NULL )
					_window->cursor_type(type);

				_lock.release();
			};

			//	Function responsible of changing the separating character of the timestamp.
			inline void	ConsoleManagerW::timestamp_separator( const wchar_t character )
			{
				_lock.acquire();
				_timestamp_separator = character;
				_lock.release();
			};

			//	Function responsible of changing the separator used by the LogManager.
			inline void	ConsoleManagerW::log_sepator( const wchar_t character )
			{
				IO::LogManagerW*	log_manager = IO::LogManagerW::get();



				_lock.acquire();
				_log_separator = character;

				if ( log_manager != NULL )
					log_manager->separator(character);

				_lock.release();
			};

				
			//	Function returning the input prefix text.
			inline std::wstring		ConsoleManagerW::input_prefix() const
			{
				std::wstring	return_value(L"");



				_lock.acquire_shared();
				return_value = _input_prefix;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the error tag text.
			inline std::wstring		ConsoleManagerW::error_tag() const
			{
				std::wstring	return_value(L"");



				_lock.acquire_shared();
				return_value = _error_tag;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the warning tag text.
			inline std::wstring		ConsoleManagerW::warning_tag() const
			{
				std::wstring	return_value(L"");



				_lock.acquire_shared();
				return_value = _warning_tag;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the normal message tag text.
			inline std::wstring		ConsoleManagerW::message_tag() const
			{
				std::wstring	return_value(L"");



				_lock.acquire_shared();
				return_value = _message_tag;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the type separator text.
			inline std::wstring		ConsoleManagerW::type_separator() const
			{
				std::wstring	return_value(L"");



				_lock.acquire_shared();
				return_value = _type_separator;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the error tag separator text.
			inline std::wstring		ConsoleManagerW::error_separator() const
			{
				std::wstring	return_value(L"");



				_lock.acquire_shared();
				return_value = _error_separator;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the warning tag separator text.
			inline std::wstring		ConsoleManagerW::warning_separator() const
			{
				std::wstring	return_value(L"");



				_lock.acquire_shared();
				return_value = _warning_separator;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the normal message tag separator text.
			inline std::wstring		ConsoleManagerW::message_separator() const
			{
				std::wstring	return_value(L"");



				_lock.acquire_shared();
				return_value = _message_separator;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the input function.
			inline ConsoleInputParserW	ConsoleManagerW::input_function() const
			{
				ConsoleInputParserW	return_value = NULL;



				_lock.acquire_shared();
				return_value = _input_function;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the number of entries the console can hold.
			inline unsigned int	ConsoleManagerW::entry_count() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _entry_count;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the normal scroll step.
			inline unsigned int	ConsoleManagerW::normal_scroll() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _normal_scroll;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the fast scroll step.
			inline unsigned int	ConsoleManagerW::fast_scroll() const
			{
				unsigned int	return_value = 0;



				_lock.acquire_shared();
				return_value = _fast_scroll;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the timestamp separator character.
			inline wchar_t	ConsoleManagerW::timestamp_separator() const
			{
				wchar_t	return_value = '\0';



				_lock.acquire_shared();
				return_value = _timestamp_separator;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning the separator character used by the LogManager.
			inline wchar_t	ConsoleManagerW::log_separator() const
			{
				wchar_t	return_value = '\0';



				_lock.acquire_shared();
				return_value = _log_separator;
				_lock.release_shared();


				return return_value;
			};

		
			//	Function responsible for resizing the console window to the given width and height;
			inline void	ConsoleManagerW::resize( const int width , const int height )
			{
				_lock.acquire();

				if ( _window != NULL )
				{
					_window->resize_terminal(width,height);
					_window->size(_width,_height);
					_entry_count = static_cast<unsigned int>(abs(_height - 1));
					_line.resize(_width,' ');
				}

				_lock.release();
			};
		
			//	Function responsible for destroying a created console window.
			inline void	ConsoleManagerW::destroy()
			{
				_lock.acquire();

				if ( _window != NULL )
				{
					_window->destroy();
					_window->deinitialise();
					_window = NULL;
				}

				_lock.release();
			};

			//	Function responsible for updating the console window.
			inline 	void	ConsoleManagerW::update()
			{
				_get_character();
				_print_entries();
				_print_input();
				Sleep(34);
			};

		}	/* Console */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_CONSOLE_MANAGER_HPP_ */