#include	<string>
#include	"../globalDefinitions.hpp"
#include	"../Server/networkMessage.hpp"

#ifndef		_DAWN_ENGINE_NETWORK_MESSAGE_FUNCTIONS_HPP_
	#endif	_DAWN_ENGINE_NETWORK_MESSAGE_FUNCTIONS_HPP_



	namespace	DawnEngine
	{

		namespace	Utility
		{

			//	Function packing a string into the given network message.
			void		string_to_network_message( const std::string& string , const unsigned int offset , Network::Message& message );
			//	Function unpacking a string from the given message.
			std::string	message_to_string( const Network::Message& message, const unsigned int offset = 0 , const unsigned int string_size = Network::MESSAGE_FIELD_SIZE );
			//	Function unpacking a IP address as string from the given message.
			std::string	message_to_ip( const Network::Message& message , const unsigned int offset , const bool v6 );


		}	/* Utility */

	}	/* DawnEngine */