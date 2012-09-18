#ifndef		_DAWN_ENGINE_NETWORK_MESSAGE_HPP_
	#define	_DAWN_ENGINE_NETWORK_MESSAGE_HPP_



	namespace	DawnEngine
	{

		namespace	Network
		{

			//	The number of integer fields in the field3 arrray of the message.
			const unsigned int	MESSAGE_FIELD_SIZE = 63;


			/*
				Struct representing a network message.
			*/
			struct	Message
			{
				int	message_code;
				int	field[MESSAGE_FIELD_SIZE];
			};

		}	/* Network */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_NETWORK_MESSAGE_HPP_ */