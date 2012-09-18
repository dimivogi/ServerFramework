#include	<string>

#ifndef		_DAWN_ENGINE_ENCRYPTION_HPP_
	#define	_DAWN_ENGINE_ENCRYPTION_HPP_



	namespace	DawnEngine
	{

		namespace	Utility
		{

			//	Function responsible of encrypting the given string.
			std::string	encrypt( const std::string& input , const std::string& salt = "" , const unsigned int passes = 1 );
			//	Function responsible of decrypting the given string.
			std::string	decrypt( const std::string& input , const std::string& salt = "" , const unsigned int passes = 1 );

		}	/* Utilities */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_ENCRYPTION_HPP_ */