#include	<string>
#include	<exception>
#include	<cstdlib>
#include	"../globalDefinitions.hpp"
#include	"logDefinitions.hpp"

#ifndef		DAWN_ENGINE_LOG_EXCEPTION_HPP_
	#define	DAWN_ENGINE_LOG_EXCEPTION_HPP_



	namespace	DawnEngine
	{
		
		namespace	IO
		{

			/*
				A class representing an exception that can be logged by the LogManager.
			*/
			template< typename type >
			class	BasicLogException	:	public std::exception
			{
				private:

					//	The message of the exception.
					type		_message;


				public:

					//	The default constructor.
					BasicLogException() throw();
					//	A constructor taking a message as a parameter.
					BasicLogException( const type& message ) throw();
					//	The destructor.
					virtual ~BasicLogException() throw();


					//	A function responsible for changing the message of the exception.
					void		message( const type& message ) throw();


					//	A function returning the message of the exception.
					type		message() const throw();
					//	The function required by the std::exception class.
					const char*	what() const throw();
			};


			/*
				A class representing an UTF-16 exception that can be logged by the LogManager.
			*/
			template<>
			class	BasicLogException<std::wstring>	:	public std::exception
			{
				private:	

					//	The message of the exception.
					std::wstring		_message;


				public:

					//	The default constructor.
					BasicLogException() throw();
					//	A constructor taking a message as a parameter.
					BasicLogException( const std::wstring& message ) throw();
					//	The destructor.
					virtual ~BasicLogException() throw();


					//	A function responsible for changing the message of the exception.
					void			message( const std::wstring& message ) throw();


					//	A function returning the message of the exception.
					std::wstring	message() const throw();
					//	The function required by the std::exception class.
					const char*		what() const throw();
					//	The UTF-16 function required by the std::exception class (preferable over the what() function).
					const wchar_t*	whatW() const throw();
			};


			/*
				Type definitions.
			*/
			typedef	BasicLogException<std::string>	LogExceptionA;
			typedef	BasicLogException<std::wstring>	LogExceptionW;



			/*
				Function definitions.
			*/


			/*
				Generic template functions.
			*/

			//	The default constructor.
			template< typename type >
			BasicLogException<type>::BasicLogException() throw()	:	
				_message()																							{};

			//	A constructor taking a message as a parameter.
			template< typename type >
			BasicLogException<type>::BasicLogException( const type& message ) throw()	:	
				_message(message)																					{};
		
			//	The destructor.
			template< typename type >
			BasicLogException<type>::~BasicLogException() throw()													{};


			//	A function responsible for changing the message of the exception.
			template< typename type >
			inline void				BasicLogException<type>::message( const type& message )	throw()					{ _message = message; };


			//	A function returning the message of the exception.
			template< typename type >
			inline type				BasicLogException<type>::message() const throw()								{ return _message; };
		
			//	The function required by the std::exception class.
			template< typename type >
			inline const char*		BasicLogException<type>::what() const throw()									{ return std::string(_message).c_str(); };


			/*
				UTF-16 template functions.
			*/

			//	A function responsible for changing the message of the exception.
			inline void				BasicLogException<std::wstring>::message( const std::wstring& message ) throw()	{ _message = message; };


			//	A function returning the message of the exception.
			inline std::wstring		BasicLogException<std::wstring>::message() const throw()						{ return _message; };
			//	The UTF-16 function required by the std::exception class (preferable over the what() function).
			inline const wchar_t*	BasicLogException<std::wstring>::whatW() const throw()							{ return _message.c_str(); };

		}	/* IO */

	}	/* DawnEngine */



#endif		/* DAWN_ENGINE_LOG_EXCEPTION_HPP_ */