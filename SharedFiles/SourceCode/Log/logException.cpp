#include	"logException.hpp"



namespace	DawnEngine
{

	namespace	IO
	{

		/*
			UTF-16 template functions.
		*/

		//	The default constructor.
		BasicLogException<std::wstring>::BasicLogException() throw()	:	
			_message(L"")												{};	

		//	A constructor taking a message as a parameter.
		BasicLogException<std::wstring>::BasicLogException( const std::wstring& message ) throw()	:	
			_message(message)											{};	

		//	The destructor.
		BasicLogException<std::wstring>::~BasicLogException() throw()	{};


		//	The UTF-16 function required by the std::exception class.
		const char*	BasicLogException<std::wstring>::what() const throw()
		{ 
			std::string	return_value("");
			char*		buffer = NULL;
			int			size = 0;



			size = _message.length() + 1;
			buffer = new (std::nothrow) char[size];

			if ( buffer )
			{
				size_t	converted_chars = 0;



				memset(buffer,'\0',size);
				wcstombs_s(&converted_chars,buffer,size,_message.c_str(),_TRUNCATE);
				return_value = buffer;
				delete[] buffer;
			}


			return return_value.c_str();
		};

	}	/* IO */

}	/* DawnEngine */