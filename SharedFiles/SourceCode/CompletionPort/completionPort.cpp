#include	"completionPort.hpp"
#include	"../Socket/socket.hpp"



namespace	DawnEngine
{

	namespace	IO
	{
		//	The defaults constructor.
		CompletionPort::CompletionPort( const unsigned int threads )	:	
			_lock() , 
			_port(CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,threads)) , 
			_max_threads(threads)											{};
	
		//	A constructor with delayed initialisation. Argument is ignored.
		CompletionPort::CompletionPort(  const bool )	:	
			_lock() , 
			_port(NULL) , 
			_max_threads(0)													{};

		//	The copy constructor.
		CompletionPort::CompletionPort( const CompletionPort& port )	:	
			_lock() , 
			_port(CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,port.max_threads())) , 
			_max_threads(port.max_threads())								{};
	
		//	The destructor.
		CompletionPort::~CompletionPort()
		{
			deinitialise();
		};


		//	The assignment operator. It does NOT create a new port. It exists to disable copying by assignment.
		CompletionPort&	CompletionPort::operator=( const CompletionPort& )	{ return *this; };


		//	Function responsible of associating a socket with the completion port.
		bool	CompletionPort::assosiate( Network::Socket& socket , ULONG_PTR key )
		{
			bool	return_value = true;



			if ( initialised()  &&  socket.initialised() )
			{
				if ( _associate_handle(reinterpret_cast<HANDLE>(socket.socket_ref()),key) )
				{
					socket.associated(true);
					return_value = true;
				}
			}


			return return_value;
		}

	}	/* IO */

}	/* DawnEngine */