#include	"pdCursesWindow.hpp"



namespace	DawnEngine
{

	namespace	Console
	{

		//	Variable containing the maximum amount of bytes that can be read when reading a string with undefined size.
		const unsigned int	PDCursesWindow::_buffer_size = 1024;
		//	The default instance of the class, assigned to the default PDCurses WINDOW stdscr.
		PDCursesWindow*		PDCursesWindow::_default_window = NULL;


		//	A constructor for the class, taking a single WINDOW* pointer as an argument. It is used for the creation of the default window.
		PDCursesWindow::PDCursesWindow( WINDOW* window )	:	
			_window(window) , 
			_initialised(false)				{};

		//	The default constructor.
		PDCursesWindow::PDCursesWindow()	:	
			_window(NULL) , 
			_initialised(false)				{};

		//	The destructor.
		PDCursesWindow::~PDCursesWindow()	{};


		//	Function responsible of reading a string of the given size. For size = 0, a maximum number of _buffer_size characters is read.
		std::string	PDCursesWindow::read_string( const unsigned int size )
		{
			std::string	return_value("");
			

			
			if ( _window != NULL )
			{
				char*			buffer = NULL;
				unsigned int	buffer_size = 0;


				
				if ( size == 0 )
				{
					buffer = new (std::nothrow) char[_buffer_size+1];
					buffer_size = _buffer_size;
				}
				else
				{
					buffer = new (std::nothrow) char[size+1];
					buffer_size = size;
				}

				if ( buffer != NULL )
				{
					memset(buffer,'\0',buffer_size+1);
					wgetnstr(_window,buffer,buffer_size);
					return_value = buffer;
					delete[] buffer;
				}
			}
			
			
			return return_value;
		};

		//	Function responsible of reading a wide string of the given size. For size = 0, a maximum number of _buffer_size characters is read.
		std::wstring	PDCursesWindow::read_wstring( const unsigned int size )
		{
			std::wstring	return_value(L"");
			
			
			
			if ( _window != NULL )
			{
				wint_t*			buffer = NULL;
				unsigned int	buffer_size = 0;


				
				if ( size == 0 )
				{
					buffer = new (std::nothrow) wint_t[_buffer_size+1];
					buffer_size = _buffer_size;
				}
				else
				{
					buffer = new (std::nothrow) wint_t[size+1];
					buffer_size = size;
				}

				if ( buffer != NULL )
				{
					memset(buffer,'\0',buffer_size+1);
					wgetn_wstr(_window,buffer,buffer_size);

					for ( unsigned int i = 0;  i < size;  ++i )
						return_value += static_cast<wchar_t>(buffer[i]);
					
					delete[] buffer;
				}
			}


			return return_value;
		};

	}	/* Console */

}	/* DawnEngine */