#ifndef		_GAME_MESSAGE_CODES_HPP_
	#define	_GAME_MESSAGE_CODES_HPP_



	namespace	Game
	{
		/*
	
			Server Message codes.
	
		*/


		//	Fixed-size strings are always considered to start from index 3 and after. Meaning in a message taking 2 arguments with one of them
		//	being an fixed-size string, the string would start at position 3.


		/*

			Server communication codes.
	
		*/

		//	Message not supported code.
		const unsigned int	MESSAGE_NOT_SUPPORTED			= 0x00000;
		//	Server authentication success.
		const unsigned int	SERVER_AUTHENTICATION_SUCCESS	= 0x00001;
		//	Server authentication failure.
		const unsigned int	SERVER_AUTHENTICATION_FAILURE	= 0x00002;
		//	Server requests connection to another server. 1 parameter: 1 fixed-size string containing the server token.
		const unsigned int	REQUEST_CONNECTION				= 0x00003;


		/*
	
			Login Server codes.
	
		*/

		//	Authentication succeeded. 2 parameters: 2 unsigned integers for the exit value.	
		const unsigned int	AUTHENTICATION_SUCCESS			= 0x10000;
		//	Authentication failed. 2 parameters: 2 unsigned integers for the exit value.
		const unsigned int	AUTHENTICATION_FAILURE			= 0x10001;
		//	Authentication request. 3 Parameters: 2 fixed-size strings for username and password and
		//	1 unsigned integer for the authentication code.
		const unsigned int	AUTHENTICATION_REQUEST			= 0x10002;
		//	User disconnect order. Sent to the shard servers and from there to the client to prevent
		//	double login.
		const unsigned int	DISCONNECT_USER					= 0x10003;


		/*

			Server codes.
	
		*/

		//	Server list request. 3 parameter:  1 unsigned integer representing the account id , 1 fixed-size string representing the session token.
		const unsigned int	SERVER_LIST_REQUEST				= 0x20000;
		//	Server list size. 1 parameter: 1 unsigned integer for the size of the list.
		const unsigned int	SERVER_LIST_SIZE				= 0x20001;
		//	Server information. 
		//	11 or 15 parameters: 1 unsigned int representing if it's IPV6 or not , 1 unsigend int representing whether it's a TCP connection or not , 
		//	4 or 8 unsigned integers representing the ip address ( 8 spots reserved ), 1 unsigned int for the port , 1 unsigned int for the type of the server , 
		//	1 unsigned int for the logged in players on the server , 1 unsigned int for the number of characters on the server , 1 fixed size string for the name of the server.
		const unsigned int	SERVER_INFO						= 0x20002;
		//	Request the time of the server. No parameters.
		const unsigned int	REQUEST_TIME					= 0x20003;
		//	The time of the server. 3 parameters: 3 unsigned integers representing the time H:M:S.
		const unsigned int	SERVER_TIME						= 0x20004;


		/*

			User codes.
	
		*/

		//	User action succeeded. 3 parameters: 1 unsigned integer for the action code , 2 unsigned integers for the exit value.	
		const unsigned int	USER_SUCCESS					= 0x30000;
		//	User action failed. 3 parameters: 1 unsigned integer for the action code , 2 unsigned integers for the exit value.
		const unsigned int	USER_FAILURE					= 0x30001;
		//	User connecting to the server. 3 parameter: 2 unsigned integers representing the id of the user , 1 fixed-size string representing the session token.
		const unsigned int	USER_CONNECT					= 0x30002;
		//	User disconecting from the server. No parameters. (For shard manager 1 parameter set to greater than zero if the user is switching servers)
		const unsigned int	USER_DISCONNECT					= 0x30003;
		//	User coming online. 2 parameter: 2 unsigned integers representing the id of the user.
		const unsigned int	USER_ONLINE						= 0x30004;
		//	User going offline. 2 parameter: 2 unsigned integers representing the id of the player.
		const unsigned int	USER_OFFLINE					= 0x30005;
		//	User position in server queue. 1 parameter: 1 unsigned integer representing the position in the queue. If 0 then the user has exited the queue and is "active".
		const unsigned int	USER_QUEUE_POSITION				= 0x30006;


		/*

			Chat codes.
	
		*/

		//	Chat operation succeeded. 2 parameters: 1 unsigned integer representing the if of the message that succeeded, 1 unsigned integer with the exit code.
		const unsigned int	CHAT_SUCCESS					= 0x40000;
		//	Chat operation failed. 2 parameters: 1 unsigned int representing the id of the message that failed, 1 unsigned integer with the exit code.
		const unsigned int	CHAT_FAILURE					= 0x40001;
		//	Chat message. 2 parameters: 1 unsigned integer representing the id of the channel, 1 fixed-size string containing the message.
		const unsigned int	CHAT_MESSAGE					= 0x40002;
		//	User requests channel join. 3 parameters: 1 fixed-size string representing the name of the channel.
		const unsigned int	CHAT_USER_REQUEST_JOIN_CHANNEL	= 0x40003;
		//	User requests channel exit. 3 parameters: 1 unsigned integer representing the id of the channel.
		const unsigned int	CHAT_USER_REQUEST_LEAVE_CHANNEL	= 0x40004;
		//	User joined channel. 3 parameters: 1 unsigned integer representing the id of the channel, 2 unsigned integers representing the id of the player;
		const unsigned int	CHAT_USER_JOINED_CHANNEL		= 0x40005;
		//	User left channel. 3 parameters: 1 unsigned integer representing the id of the channel, 2 unsigned integers representing the id of the player.
		const unsigned int	CHAT_USER_LEFT_CHANNEL			= 0x40006;


		/*

			Character codes.
	
		*/

		//	Character action success. 3 parameter: 1 unsigned int with the action code , 2 unsigned int for the exit value.
		const unsigned int	CHARACTER_SUCCESS				= 0x50000;
		//	Character action failure. 3 parameter: 1 unsigned int with the action code , 2 unsigned int for the exit value.
		const unsigned int	CHARACTER_FAILURE				= 0x50001;
		//	Character list request. No parameters.
		const unsigned int	CHARACTER_LIST_REQUEST			= 0x50002;
		//	Character list size. 1 parameter: 1 unsigned integer for the list size.
		const unsigned int	CHARACTER_LIST_SIZE				= 0x50003;
		//	Character creation. 2 parameters:  1 unsigned integer for character type, 1 fixed-size string for the name.
		const unsigned int	CHARACTER_CREATE				= 0x50004;
		//	Character delection. 1 parameters: 1 unsigned integer for the character id.
		const unsigned int	CHARACTER_DELETE				= 0x50005;
		//	Request character name. 1 parameter: 2 unsigned integers representing the character id.
		const unsigned int	CHARACTER_GET_NAME				= 0x50006;
		//	Request character information. 1 parameter: 2 unsigned integers representing the character id.
		const unsigned int	CHARACTER_GET_INFO				= 0x50007;
		//	Character name. 4 parameters: 2 unsigned integers representing the character id, 1 unsigned integer containing the character type, 1 fixed-size string containing the name.
		const unsigned int	CHARACTER_NAME					= 0x50008;
		//	Character information. 4 parameters: 2 unsigned integers representing the character id , 2 unsigned integers for 2 type values.
		const unsigned int	CHARACTER_INFO					= 0x50009;
		//	Login with the selected character. 1 parameters: 1 unsigned integer representing the character id.
		const unsigned int	CHARACTER_LOGIN					= 0x5000A;
		//	Logout the selected character. No parameters.
		const unsigned int	CHARACTER_LOGOUT				= 0x5000B;
		//	Move character to another location. 1 parameter: 1 unsigned integer for the location id .
		const unsigned int	CHARACTER_TRANSPORT				= 0x5000C;
		//	Change the server that ownes the character. 10 parameters: 2 unsigned integers for the character id, 1 unsigned integer for the operation , 
		//	1 unsigned integer whether should read from the database , 6 unsigned integers for the position.
		const unsigned int	CHARACTER_OWNERSHIP				= 0x5000D;
		//	Get the position of the character. 1 argument plus 10 arguments per player: 1 unsigned integer with the timestamp , (per_player:) 2 unsigned integers for the character id, 1 unsigned integer whether we player should be removed , 
		//	1 unsigned integer with the zone id, 6 integers for the position of the character.
		const unsigned int	CHARACTER_POSITION				= 0x5000E;
		//	Move the character. 8 arguments:  1 unsigned integer with the timestamp , 1 unsigend integer with the zone id , 6 integers for the movement of the character.
		const unsigned int	CHARACTER_MOVEMENT				= 0x5000F;


		/*

			World codes.
	
		*/

		//	World server should take this region. 3 parameters: 1 unsigned integer for the width, 1 unsigned integer for the depth,
		//	1 unsigned integer for the x offset , 1 unsigned integer for the z offset , 1 unsigned integer for the player viewing distance.
		const unsigned int	WORLD_REGION					= 0x60000;


		/*

			Instance codes.
	
		*/

		//	Instance action succeeded. 3 parameters: 1 unsigned integer for the action code , 2 unsigned integers for the exit value.
		const unsigned int	INSTANCE_SUCCESS				= 0x70000;
		//	Instance action failed. 2 parameters: 1 unsigned integer for the action code , 2 unsigned integers for the exit value.
		const unsigned int	INSTANCE_FAILLURE				= 0x70001;
		//	Create instance. 1 parameter: 1 unsigned integer for the id of the type of instance to create.
		const unsigned int	INSTANCE_CREATE					= 0x70002;
		//	Destroy instance. 1 paramete: 1 unsigned integer for the id of the instance.
		const unsigned int	INSTANCE_DESTROY				= 0x70003;
		//	Request number of running instances. No parameters.
		const unsigned int	INSTANCE_COUNT_REQUEST			= 0x70004;
		//	Number of running instancnes. 1 parameter: 1 unsigned integer containing the number of instances.
		const unsigned int	INSTANCE_COUNT					= 0x70005;

	}	/* Game */



#endif		/* _GAME_MESSAGE_CODES_HPP_ */