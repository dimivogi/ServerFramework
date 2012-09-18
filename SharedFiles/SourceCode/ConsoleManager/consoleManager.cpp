#include	"consoleManager.hpp"
#include	"../Log/logManager.hpp"



namespace	DawnEngine
{

	namespace	Console
	{

		//	A lock for managing the creation and deletion of the instance in a thread safe manner.
		Concurrency::SlimReadWriterLock	ConsoleManagerA::_instance_lock;
		//	The single instance of the class.
		ConsoleManagerA*				ConsoleManagerA::_instance;
		//	A lock for managing the creation and deletion of the instance in a thread safe manner.
		Concurrency::SlimReadWriterLock	ConsoleManagerW::_instance_lock;
		//	The single instance of the class.
		ConsoleManagerW*				ConsoleManagerW::_instance;


		/*
			UTF-8 Console Manager.
		*/

		//	Function responsible of getting and parsing a single character from the console.
		void	ConsoleManagerA::_get_character()
		{
			_lock.acquire();

			if ( _window != NULL )
			{
				IO::LogManagerA*	log_manager = IO::LogManagerA::get();
				unsigned int	count = 0;
				int				ch = _window->read();
			



				if ( log_manager != NULL )
					count = log_manager->entry_count();

				if ( ch != ERR )
				{
					if ( ch >= KEY_MIN  &&  ch <= KEY_MAX )
					{
						if ( ch == KEY_UP )
						{
							if ( ( _offset + _normal_scroll + _entry_count ) <= count )
								_offset += _normal_scroll;
							else
								_offset += ( count - _offset - _entry_count );
						}
						else if ( ch == KEY_DOWN )
						{
							if ( _offset >= _normal_scroll )
								_offset -= _normal_scroll;
							else
								_offset = 0;
						}
						else if ( ch == KEY_NPAGE )
						{
							if ( _offset > _fast_scroll )
								_offset -= _fast_scroll;
							else
								_offset = 0;
						}
						else if ( ch == KEY_PPAGE )
						{
							if ( ( _offset + _fast_scroll + _entry_count ) <= count )
								_offset += _fast_scroll;
							else
								_offset += (count - _offset - _entry_count);
						}
						else if ( ch == KEY_HOME )
							_offset = 0;
						else if ( ch == KEY_END )
						{
							if ( _entry_count < count )
								_offset = count - _entry_count;
							else
								_offset = 0;
						}
						else if ( ch == KEY_BACKSPACE )
						{
							if ( _input.size() > 0 )
								_input.pop_back();
						}
					}
					else
					{
						if ( ch == '\b' )
						{
							if ( _input.size() > 0 )
								_input.pop_back();
						}
						else if ( ch != '\0'  &&  ch != '\n'  &&  ch != '\v'  &&  ch != '\r'  &&  ch != '\b' )
						{
							if ( _input.size() < ( _width - _input_prefix.size() - 1 ) )
								_input += static_cast<char>(ch);
						}
						else
							_read_line = true;
					}
				}
			}

			_lock.release();
		};

		//	Function responsible of printing the last _entry_count entries of the LogManager to the console.
		void	ConsoleManagerA::_print_entries()
		{
			_lock.acquire_shared();

			if ( _window != NULL )
			{
				IO::LogManagerA*	log_manager = IO::LogManagerA::get();
			
			

				if ( log_manager != NULL )
				{
					std::vector<IO::LogEntryA>	entries(log_manager->last_entries(_entry_count,_offset));
					bool						colours = _window->has_colours();
					int							x = 0;
					int							y = 0;



					for ( unsigned int i = 0;  i < entries.size();  ++i )
					{
						std::string		timestamp(entries[i].timestamp());
						std::string		message(entries[i].message());
						unsigned int	separator = timestamp.find_first_of(_log_separator);



						if ( separator != timestamp.npos )
							timestamp[separator] = _timestamp_separator;

						if ( _window->has_colours() )
							_window->enable_attribute(COLOR_PAIR(2));
	
						_window->move_and_write(timestamp,0,entries.size()-i-1);
						_window->write(_type_separator);

						if ( colours )
							_window->disable_attribute(COLOR_PAIR(2));
	
						_window->enable_attribute(BOLD);

						switch ( entries[i].type() )
						{
							case	DawnEngine::IO::LOG_ERROR:			
																	if ( colours )
																		_window->enable_attribute(COLOR_PAIR(3));

																	_window->write(_error_tag);
																	_window->write(_error_separator);

																	if ( colours )
																		_window->disable_attribute(COLOR_PAIR(3));

																	break;
							case	DawnEngine::IO::LOG_WARNING:	
																	if ( colours )
																		_window->enable_attribute(COLOR_PAIR(4));

																	_window->write(_warning_tag);
																	_window->write(_warning_separator);

																	if ( colours )
																		_window->disable_attribute(COLOR_PAIR(4));

																	break;
							default:							
																	if ( colours )
																		_window->enable_attribute(COLOR_PAIR(5));

																	_window->write(_message_tag);
																	_window->write(_message_separator);
														
																	if ( colours )
																		_window->disable_attribute(COLOR_PAIR(5));
														
																	break;
						}

						_window->disable_attribute(BOLD);

						if ( colours )
							_window->enable_attribute(COLOR_PAIR(1));

						_window->cursor(x,y);

						if ( (x+static_cast<int>(message.size())) > _width )
							message = message.substr(0,_width-x-1);

						_window->write(message);
						_window->cursor(x,y);
						_window->write(_line.substr(0,_width-x).c_str());
				
						if ( colours)
							_window->disable_attribute(COLOR_PAIR(1));
					}

					for ( unsigned int i = entries.size();  i < _entry_count;  ++i )
						_window->move_and_write(_line,0,i);
				}
			}

			_lock.release_shared();
		};

		//	Function responsible of printing the current input to the console.
		void	ConsoleManagerA::_print_input()
		{
			_lock.acquire();

			if ( _window != NULL )
			{
				int		x = 0;
				int		y = 0;
				bool	colours = _window->has_colours();



				_window->move_and_write(_input_prefix,0,_entry_count);

				if ( colours )
					_window->enable_attribute(COLOR_PAIR(1));

				if ( _read_line )
				{
					if ( _input_function != NULL )
						_input_function(_input);
				
					_input.clear();
					_read_line = false;
				}
				else
					_window->write(_input);

				_window->cursor(x,y);
				_window->write(_line.substr(0,_width-x));
				_window->move_cursor(_input_prefix.size()+_input.size(),_entry_count);

				if ( colours )
					_window->disable_attribute(COLOR_PAIR(1));
			}

			_lock.release();
		};


		//	The default constructor of the class.
		ConsoleManagerA::ConsoleManagerA()	:	
			_lock() , 
			_window(NULL) , 
			_input("") ,
			_input_prefix("> ") , 
			_line("") , 
			_error_tag("Error:") , _warning_tag("Warning:") , _message_tag("Log:") ,  
			_type_separator("  ") , _error_separator("    ") , _warning_separator("  ") , _message_separator("      ") , 
			_input_function(NULL) , 
			_entry_count(24) , 
			_offset(0) , 
			_normal_scroll(1) , 
			_fast_scroll(5) , 
			_width(0) , 
			_height(0) , 
			_timestamp_separator(' ') , 
			_log_separator('\t') , 
			_read_line(false)				{};

		//	The destructor of the class.
		ConsoleManagerA::~ConsoleManagerA()	{};


		//	Function responsible for creating and initialising a console window with the given width and height.
		void	ConsoleManagerA::create( const int width , const int height )
		{
			IO::LogManagerA*	log_manager = IO::LogManagerA::get();



			if ( log_manager != NULL )
			{
				log_manager->echo(false);
				log_manager->error_tag(_error_tag);
				log_manager->warning_tag(_warning_tag);
				log_manager->message_tag(_message_tag);
				log_manager->separator(_log_separator);
			}

			_lock.acquire();
			
			PDCursesWindow::initialise();
			_window = PDCursesWindow::get_default_window();

			if ( _window != NULL )
			{
				int	actual_width = abs(width);
				int	actual_height = abs(height);



				_window->cursor_type(EMPHASIZED);

				if ( actual_width > 0  &&  actual_height > 0 )
					_window->resize_terminal(actual_width,actual_height);
	
				_window->create();
				_window->size(_width,_height);
				_entry_count = static_cast<unsigned int>(abs(_height - 1));
				_line.resize(_width,' ');
				_window->set_colour(COLOR_RED,850,100,50);
				_window->set_colour(COLOR_GREEN,50,900,50);
				_window->set_colour(COLOR_BLUE,50,100,850);
				_window->create_colour_pair(1,COLOR_WHITE,COLOR_BLACK);
				_window->create_colour_pair(2,COLOR_GREEN|COLOR_BLUE,COLOR_BLACK);
				_window->create_colour_pair(3,COLOR_RED,COLOR_BLACK);
				_window->create_colour_pair(4,COLOR_RED|COLOR_GREEN,COLOR_BLACK);
				_window->create_colour_pair(5,COLOR_GREEN,COLOR_BLACK);
			}

			_lock.release();
		};



		/*
			UTF-16 Console Manager.
		*/

		//	Function responsible of getting and parsing a single character from the console.
		void	ConsoleManagerW::_get_character()
		{
			_lock.acquire();

			if ( _window != NULL )
			{
				IO::LogManagerW*	log_manager = IO::LogManagerW::get();
				unsigned int	count = 0;
				int				ch = _window->read();
			



				if ( log_manager != NULL )
					count = log_manager->entry_count();

				if ( ch != ERR )
				{
					if ( ch >= KEY_MIN  &&  ch <= KEY_MAX )
					{
						if ( ch == KEY_UP )
						{
							if ( ( _offset + _normal_scroll + _entry_count ) <= count )
								_offset += _normal_scroll;
							else
								_offset += ( count - _offset - _entry_count );
						}
						else if ( ch == KEY_DOWN )
						{
							if ( _offset >= _normal_scroll )
								_offset -= _normal_scroll;
							else
								_offset = 0;
						}
						else if ( ch == KEY_NPAGE )
						{
							if ( _offset > _fast_scroll )
								_offset -= _fast_scroll;
							else
								_offset = 0;
						}
						else if ( ch == KEY_PPAGE )
						{
							if ( ( _offset + _fast_scroll + _entry_count ) <= count )
								_offset += _fast_scroll;
							else
								_offset += (count - _offset - _entry_count);
						}
						else if ( ch == KEY_HOME )
							_offset = 0;
						else if ( ch == KEY_END )
						{
							if ( _entry_count < count )
								_offset = count - _entry_count;
							else
								_offset = 0;
						}
						else if ( ch == KEY_BACKSPACE )
						{
							if ( _input.size() > 0 )
								_input.pop_back();
						}
					}
					else
					{
						if ( ch == '\b' )
						{
							if ( _input.size() > 0 )
								_input.pop_back();
						}
						else if ( ch != '\0'  &&  ch != '\n'  &&  ch != '\v'  &&  ch != '\r'  &&  ch != '\b' )
						{
							if ( _input.size() < ( _width - _input_prefix.size() - 1 ) )
								_input += static_cast<wchar_t>(ch);
						}
						else
							_read_line = true;
					}
				}
			}

			_lock.release();
		};

		//	Function responsible of printing the last _entry_count entries of the LogManager to the console.
		void	ConsoleManagerW::_print_entries()
		{
			_lock.acquire_shared();

			if ( _window != NULL )
			{
				IO::LogManagerW*	log_manager = IO::LogManagerW::get();
			
			

				if ( log_manager != NULL )
				{
					std::vector<IO::LogEntryW>	entries(log_manager->last_entries(_entry_count,_offset));
					bool						colours = _window->has_colours();
					int							x = 0;
					int							y = 0;



					for ( unsigned int i = 0;  i < entries.size();  ++i )
					{
						std::wstring	timestamp(entries[i].timestamp());
						std::wstring	message(entries[i].message());
						unsigned int	separator = timestamp.find_first_of(_log_separator);



						if ( separator != timestamp.npos )
							timestamp[separator] = _timestamp_separator;

						if ( _window->has_colours() )
							_window->enable_attribute(COLOR_PAIR(2));
	
						_window->move_and_write(timestamp,0,entries.size()-i-1);
						_window->write(_type_separator);

						if ( colours )
							_window->disable_attribute(COLOR_PAIR(2));
	
						_window->enable_attribute(BOLD);

						switch ( entries[i].type() )
						{
							case	DawnEngine::IO::LOG_ERROR:			
																	if ( colours )
																		_window->enable_attribute(COLOR_PAIR(3));

																	_window->write(_error_tag);
																	_window->write(_error_separator);

																	if ( colours )
																		_window->disable_attribute(COLOR_PAIR(3));

																	break;
							case	DawnEngine::IO::LOG_WARNING:	
																	if ( colours )
																		_window->enable_attribute(COLOR_PAIR(4));

																	_window->write(_warning_tag);
																	_window->write(_warning_separator);

																	if ( colours )
																		_window->disable_attribute(COLOR_PAIR(4));

																	break;
							default:							
																	if ( colours )
																		_window->enable_attribute(COLOR_PAIR(5));

																	_window->write(_message_tag);
																	_window->write(_message_separator);
														
																	if ( colours )
																		_window->disable_attribute(COLOR_PAIR(5));
														
																	break;
						}

						_window->disable_attribute(BOLD);

						if ( colours )
							_window->enable_attribute(COLOR_PAIR(1));

						_window->cursor(x,y);

						if ( (x+static_cast<int>(message.size())) > _width )
							message = message.substr(0,_width-x-1);

						_window->write(message);
						_window->cursor(x,y);
						_window->write(_line.substr(0,_width-x).c_str());
				
						if ( colours)
							_window->disable_attribute(COLOR_PAIR(1));
					}

					for ( unsigned int i = entries.size();  i < _entry_count;  ++i )
						_window->move_and_write(_line,0,i);
				}
			}

			_lock.release_shared();
		};

		//	Function responsible of printing the current input to the console.
		void	ConsoleManagerW::_print_input()
		{
			_lock.acquire();

			if ( _window != NULL )
			{
				int		x = 0;
				int		y = 0;
				bool	colours = _window->has_colours();



				_window->move_and_write(_input_prefix,0,_entry_count);

				if ( colours )
					_window->enable_attribute(COLOR_PAIR(1));

				if ( _read_line )
				{
					if ( _input_function != NULL )
						_input_function(_input);
				
					_input.clear();
					_read_line = false;
				}
				else
					_window->write(_input);

				_window->cursor(x,y);
				_window->write(_line.substr(0,_width-x));
				_window->move_cursor(_input_prefix.size()+_input.size(),_entry_count);

				if ( colours )
					_window->disable_attribute(COLOR_PAIR(1));
			}

			_lock.release();
		};


		//	The default constructor of the class.
		ConsoleManagerW::ConsoleManagerW()	:	
			_lock() , 
			_window(NULL) , 
			_input(L"") ,
			_input_prefix(L"> ") , 
			_line(L"") , 
			_error_tag(L"Error:") , _warning_tag(L"Warning:") , _message_tag(L"Log:") ,  
			_type_separator(L"  ") , _error_separator(L"    ") , _warning_separator(L"  ") , _message_separator(L"      ") , 
			_input_function(NULL) , 
			_entry_count(24) , 
			_offset(0) , 
			_normal_scroll(1) , 
			_fast_scroll(5) , 
			_width(0) , 
			_height(0) , 
			_timestamp_separator(' ') , 
			_log_separator('\t') , 
			_read_line(false)				{};

		//	The destructor of the class.
		ConsoleManagerW::~ConsoleManagerW()	{};


		//	Function responsible for creating and initialising a console window with the given width and height.
		void	ConsoleManagerW::create( const int width , const int height )
		{
			IO::LogManagerW*	log_manager = IO::LogManagerW::get();



			if ( log_manager != NULL )
			{
				log_manager->echo(false);
				log_manager->error_tag(_error_tag);
				log_manager->warning_tag(_warning_tag);
				log_manager->message_tag(_message_tag);
				log_manager->separator(_log_separator);
			}

			_lock.acquire();
			
			PDCursesWindow::initialise();
			_window = PDCursesWindow::get_default_window();

			if ( _window != NULL )
			{
				int	actual_width = abs(width);
				int	actual_height = abs(height);



				_window->cursor_type(EMPHASIZED);

				if ( actual_width > 0  &&  actual_height > 0 )
					_window->resize_terminal(actual_width,actual_height);
	
				_window->create();
				_window->size(_width,_height);
				_entry_count = static_cast<unsigned int>(abs(_height - 1));
				_line.resize(_width,' ');
				_window->set_colour(COLOR_RED,850,100,50);
				_window->set_colour(COLOR_GREEN,50,900,50);
				_window->set_colour(COLOR_BLUE,50,100,850);
				_window->create_colour_pair(1,COLOR_WHITE,COLOR_BLACK);
				_window->create_colour_pair(2,COLOR_GREEN|COLOR_BLUE,COLOR_BLACK);
				_window->create_colour_pair(3,COLOR_RED,COLOR_BLACK);
				_window->create_colour_pair(4,COLOR_RED|COLOR_GREEN,COLOR_BLACK);
				_window->create_colour_pair(5,COLOR_GREEN,COLOR_BLACK);
			}

			_lock.release();
		};

	}	/* Console */

}	/* DawnEngine */
