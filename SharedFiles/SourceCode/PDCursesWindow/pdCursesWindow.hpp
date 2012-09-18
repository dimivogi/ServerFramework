#include	"../globalDefinitions.hpp"
#include	<string>
#include	"../../Libraries/pdcurses/curses.h"

#ifndef		_DAWN_ENGINE_PDCURSES_WINDOW_HPP_
	#define	_DAWN_ENGINE_PDCURSES_WINDOW_HPP_



	namespace	DawnEngine
	{

		namespace	Console
		{

			/*
				Enumeration containing all the possible attributes for the window.
			*/
			enum	WindowAttribute
			{
				BOLD = A_BOLD 
			};

			/*
				Enumeration containing all the possible cursor types.
			*/
			enum	CursorType
			{
				INVISIBLE = 0 , 
				NORMAL = 1 , 
				EMPHASIZED = 2 
			};
		
		
			/*
				A class representing and handling a pdcurses window.
			*/
			class	PDCursesWindow
			{
				private:

					//	Variable containing the maximum amount of bytes that can be read when reading a string with undefined size.
					static const unsigned int	_buffer_size;
					//	The default instance of the class, assigned to the default PDCurses WINDOW stdscr.
					static PDCursesWindow*		_default_window;


					//	A variable containing a pointer to the PDCurses WINDOW associated with the instance of the class.
					WINDOW*						_window;
					//	A variable containing whether the instance has been initialised or not.
					bool						_initialised;


					//	A constructor for the class, taking a single WINDOW* pointer as an argument. It is used for the creation of the default window.
					PDCursesWindow( WINDOW* window );

				public:
				
					//	Function responsible of initialising the PDCurses library and creating the default window.
					static void					initialise();
					//	Function responsible of de-initialising the PDCurses library and destroying the default window.
					static void					deinitialise();
					//	Function responsible of returning a pointer to the default window.
					static PDCursesWindow*		get_default_window();
					//	Function responsible of resizing the terminal.
					static void					resize_terminal( const int width , const int height );
					//	Function responsible of altering a pre-set colour.
					static void					set_colour( const short colour , const short r , const short g , const short b );
					//	Function responsible of creating a colour pair for later use.
					static void					create_colour_pair( const short id , const short foreground , const short background );
					//	Function responsible of setting the type of the cursor of the terminal.
					static void					cursor_type( const CursorType& type );


					//	Function returning whether the terminal supports colours.
					static bool					has_colours();
					//	FUnction returning whether the terminal allows the change of colours.
					static bool					colours_changeable();


					//	The default constructor.
					PDCursesWindow();
					//	The destructor.
					~PDCursesWindow();


					//	Function responsible of creating the window.
					void						create();
					//	Function responsible of destroying the window.
					void						destroy();
					//	FUnction responsible of resizing the window.
					void						resize( const int width , const int height );

				
					//	Function responsible of enabling an attribute for the window.
					void						enable_attribute( const int attribute );
					//	Function responsible of disable an attribute for the window.
					void						disable_attribute( const int attribute );
					//	Function responsible of moving the cursor to the specified position.
					void						move_cursor( const int x , const int y );
					//	FUnction responsible of writing a character to the window.
					void						write( const char& character );
					//	Function responsible of writing a wide character to the window.
					void						write( const wchar_t& character );
					//	Function responsible of writing a string to the window.
					void						write( const std::string& string );
					//	Function responsible of writing a wide string to the window.
					void						write( const std::wstring& string );
					//	Function responsible of moving the cursor to the specified position and writing a character.
					void						move_and_write( const char& character , const int x , const int y );
					//	Function responsible of moving the cursor to the specified position and writing a wide character.
					void						move_and_write( const wchar_t& character , const int x , const int y );
					//	Function responsible of moving the cursor to the specified position and writing a string.
					void						move_and_write( const std::string& string , const int x , const int y );
					//	Function responsible of moving the cursor to the specified position and writing a wide string.
					void						move_and_write( const std::wstring& string , const int x , const int y );
					//	Function responsible of reading a character in an integer format.
					int							read();
					//	Function responsible of reading a string of the given size. For size = 0, a maximum number of _buffer_size characters is read.
					std::string					read_string( const unsigned int size = 0 ); 	
					//	Function responsible of reading a wide string of the given size. For size = 0, a maximum number of _buffer_size characters is read.
					std::wstring				read_wstring( const unsigned int size = 0 ); 	
					//	Function responsible of clearing the window.
					void						clear();
					//	Function responsible of refresing the window.
					void						refresh();


					//	Function returning the current position of the cursor.
					void						cursor( int& x , int& y );
					//	Function returning the current size of the window.
					void						size( int& x , int& y );
			};


		
			/*
				Function definitions.
			*/


			//	Function responsible of initialising the PDCurses library and creating the default window.
			inline void	PDCursesWindow::initialise()
			{
				if ( _default_window == NULL )
				{
					initscr();
					start_color();
					raw();
					cbreak();
					noecho();
					_default_window = new (std::nothrow) PDCursesWindow(stdscr);

					if ( _default_window != NULL )
						_default_window->create();
				}
			};

			//	Function responsible of de-initialising the PDCurses library and destroying the default window.
			inline void	PDCursesWindow::deinitialise()
			{
				if ( _default_window != NULL )
				{
					_default_window->destroy();
					delete _default_window;
					_default_window = NULL;
				}
			};

			//	Function responsible of returning a pointer to the default window.
			inline PDCursesWindow*	PDCursesWindow::get_default_window()		{ return _default_window; };
			//	Function responsible of resizing the terminal.
			inline void	PDCursesWindow::resize_terminal( const int width , const int height )
			{
				resize_term(abs(height),abs(width));
			};

			//	Function responsible of altering a pre-set colour.
			inline void	PDCursesWindow::set_colour( const short colour , const short r , const short g , const short b )
			{
				if ( has_colours()  &&  colours_changeable() )
					init_color(colour,r,g,b);
			};

			//	Function responsible of creating a colour pair for later use.
			inline void	PDCursesWindow::create_colour_pair( const short id , const short foreground , const short background )
			{
				if ( has_colours() )
					init_pair(id,foreground,background);
			};

			//	Function responsible of setting the type of the cursor of the terminal.
			inline void	PDCursesWindow::cursor_type( const CursorType& type )	{ curs_set(type); };


			//	Function returning whether the terminal supports colours.
			inline bool	PDCursesWindow::has_colours()							{ return ( has_colors() != 0  ?  true :false ); };
			//	FUnction returning whether the terminal allows the change of colours.
			inline bool	PDCursesWindow::colours_changeable()					{ return ( can_change_color() != 0  ?  true :false ); };
		

			//	Function responsible of creating the window.
			inline void	PDCursesWindow::create()
			{
				if ( !_initialised )
				{
					if ( _window == NULL )
						_window = newwin(0,0,0,0);

					if ( _window != NULL )
					{
						keypad(_window,true);
						wtimeout(_window,0);
						_initialised = true;
					}
				}
			};

			//	Function responsible of destroying the window.
			inline void	PDCursesWindow::destroy()
			{
				if ( _initialised )
				{
					endwin();

					if ( _window != stdscr )
					{
						delwin(_window);
						_window = NULL;
					}

					_initialised = false;
				}
			};

			//	FUnction responsible of resizing the window.
			inline void	PDCursesWindow::resize( const int width , const int height )
			{
				if ( _window != NULL )
					wresize(_window,height,width);
			};

		
			//	Function responsible of enabling an attribute for the window.
			inline void	PDCursesWindow::enable_attribute( const int attribute )
			{
				if ( _window != NULL )
					wattron(_window,attribute);
			};

			//	Function responsible of disable an attribute for the window.
			inline void	PDCursesWindow::disable_attribute( const int attribute )
			{
				if ( _window != NULL )
					wattroff(_window,attribute);
			};

			//	Function responsible of moving the cursor to the specified position.
			inline void	PDCursesWindow::move_cursor( const int x , const int y )
			{
				if ( _window != NULL )
					wmove(_window,y,x);
			};

			//	Function responsible of writing a character to the window.
			inline void	PDCursesWindow::write( const char& character )
			{
				if ( _window != NULL )
					waddch(_window,character);
			};

			//	Function responsible of writing a wide character to the window.
			inline void	PDCursesWindow::write( const wchar_t& character )
			{
				if ( _window != NULL )
					waddch(_window,character);
			};

			//	Function responsible of writing a string to the window.
			inline void	PDCursesWindow::write( const std::string& string )
			{
				if ( _window != NULL )
					waddstr(_window,string.c_str());
			};

			//	Function responsible of writing a wide string to the window.
			inline void	PDCursesWindow::write( const std::wstring& string )
			{
				if ( _window != NULL )
					waddwstr(_window,string.c_str());
			};

			//	Function responsible of moving the cursor to the specified position and writing a character.
			inline void	PDCursesWindow::move_and_write( const char& character , const int x , const int y )
			{
				if ( _window != NULL )
					mvwaddch(_window,y,x,character);
			};

			//	Function responsible of moving the cursor to the specified position and writing a wide character.
			inline void	PDCursesWindow::move_and_write( const wchar_t& character , const int x , const int y )
			{
				if ( _window != NULL )
					mvwaddch(_window,y,x,character);
			};
		
			//	Function responsible of moving the cursor to the specified position and writing a string.
			inline void	PDCursesWindow::move_and_write( const std::string& string , const int x , const int y )
			{
				if ( _window != NULL )
					mvwaddstr(_window,y,x,string.c_str());
			};

			//	Function responsible of moving the cursor to the specified position and writing a wide string.
			inline void	PDCursesWindow::move_and_write( const std::wstring& string , const int x , const int y )
			{
				if ( _window != NULL )
					mvwaddwstr(_window,y,x,string.c_str());
			};

			//	Function responsible of reading a character in an integer format.
			inline int	PDCursesWindow::read()
			{
				int	return_value = 0;

			
			
				if ( _window != NULL )
					return_value = wgetch(_window);


				return return_value;
			};

			//	Function responsible of clearing the window.
			inline void	PDCursesWindow::clear()
			{
				if ( _window != NULL )
					wclear(_window);
			};

			//	Function responsible of refresing the window.
			inline void	PDCursesWindow::refresh()
			{
				if ( _window != NULL )
					wrefresh(_window);
			};


			//	Function returning the current position of the cursor.
			inline void	PDCursesWindow::cursor( int& x , int& y )
			{ 
				if ( _window != NULL )
					getyx(_window,y,x);
			};

			//	Function returning the current size of the window.
			inline void	PDCursesWindow::size( int& x , int& y )
			{
				if ( _window != NULL )
					getmaxyx(_window,y,x);
			};

		}	/* Console */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_PDCURSES_WINDOW_HPP_ */