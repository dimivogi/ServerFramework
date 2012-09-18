#include	"socketInfo.hpp"



namespace	DawnEngine
{

	namespace	Network
	{

		//	The default constructor.
		SocketInfo::SocketInfo( Socket* socket , const unsigned int available_operations , const unsigned int pending_read_operations , const bool connected , const bool server )	:	
			_socket(socket) , 
			_pending_read_operations(pending_read_operations) , 
			_available_operations(available_operations) , 
			_connected(connected) , 
			_visible(true) , 
			_previous_state(connected) , 
			_server(server)			{ memset(&_info,'\0',sizeof(_info)); };

		//	A constructor taking a sockaddr_storage argument instead of a socket pointer.
		SocketInfo::SocketInfo( const sockaddr_storage& info , const unsigned int available_operations , const unsigned int pending_read_operations , const bool connected , const bool server )	:	
			_socket(NULL) , 
			_pending_read_operations(pending_read_operations) , 
			_available_operations(available_operations) , 
			_connected(connected) , 
			_visible(true) , 
			_previous_state(connected) , 
			_server(server)			{ memcpy(&_info,&info,sizeof(sockaddr_storage)); };

		//	The destructor.
		SocketInfo::~SocketInfo()	{};

	}	/* Network */

}	/* DawnEngine */