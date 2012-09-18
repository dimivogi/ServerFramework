#include	"networkMessageFunctions.hpp"
#include	<algorithm>
#include	<sstream>



namespace	DawnEngine
{

	namespace	Utility
	{

		//	Function packing a string into the given network message.
		void	string_to_network_message( const std::string& value , const unsigned int offset , Network::Message& message )
		{
			if ( value.length() > 0 )
			{
				unsigned int	start = std::min(offset,Network::MESSAGE_FIELD_SIZE);
				unsigned long	length = std::min(value.length(),Network::MESSAGE_FIELD_SIZE-start);


				
				memset(message.field+start,'\0',length*sizeof(int));

				for ( unsigned int i = 0;  i < length;  ++i )
					message.field[start+i] = value[i];
			}
		};

		//	Function unpacking a string from the given message.
		std::string	message_to_string( const Network::Message& message , const unsigned int offset , const unsigned int string_size )
		{
			std::string		return_value("");
			unsigned int	start = std::min(offset,Network::MESSAGE_FIELD_SIZE);
			unsigned int	size = std::min(string_size,Network::MESSAGE_FIELD_SIZE-start);
			unsigned int	i = start;
			bool			done = false;



			while ( !done  &&  i < size )
			{
				if ( message.field[i] == 0 )
					done = true;
				else
				{
					return_value += static_cast<char>(message.field[i]);
					++i;
				}
			}
			

			return return_value;
		};

		//	Function unpacking a IP address as string from the given message.
		std::string	message_to_ip( const Network::Message& message , const unsigned int offset , const bool v6 )
		{
			std::stringstream	buffer;
			std::string			return_value("");
			unsigned int		size = std::min(Network::MESSAGE_FIELD_SIZE,static_cast<unsigned int>(( v6  ?  8 : 4 )));
			unsigned int		start = std::min(offset,Network::MESSAGE_FIELD_SIZE - size);
			unsigned int		final_element = start + size - 1;
			unsigned int		i = start;
			bool				done = false;



			while ( !done  &&  i <= final_element )
			{
				buffer << static_cast<unsigned int>(message.field[i]);

				if ( i < final_element )
					buffer << ( v6  ?  ':' : '.' );

				++i;
			}
			
			buffer >> return_value;


			return return_value;
		};

	}	/* Utility */

}	/* DawnEngine */