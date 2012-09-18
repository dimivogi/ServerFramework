#include	<sstream>
#include	"Server/server.hpp"
#include	"ConsoleManager/consoleManager.hpp"



BOOL WINAPI	console_proc( DWORD )
{
	DawnEngine::Network::Server*	server = DawnEngine::Network::Server::get();



	if ( server != NULL )
	{
		server->close();
		Sleep(1000);
	}


	return TRUE;
};

void	parse_input( const std::string input )
{
	DawnEngine::IO::LogManagerA*	log_manager_a = DawnEngine::IO::LogManagerA::get();
	DawnEngine::IO::LogManagerW*	log_manager_w = DawnEngine::IO::LogManagerW::get();
	DawnEngine::Network::Server*	server = DawnEngine::Network::Server::get();


	
	if ( input == "quit" )
	{
		if ( log_manager_a )
			log_manager_a->log_message("Quitting.");

		if ( log_manager_w )
			log_manager_w->log_message(L"Quitting.");

		if ( server != NULL )
			server->close();
	}
	else if ( input == "restart" )
	{
		if ( log_manager_a )
			log_manager_a->log_message("Restarting.");

		if ( log_manager_w )
			log_manager_w->log_message(L"Restarting.");

		if ( server != NULL )
		{
			server->close();
			server->start();
		}
	}
	else if ( input == "purge_log" )
	{
		if ( log_manager_a )
		{
			log_manager_a->purge_log();
			log_manager_a->log_message("Log purged.");
		}

		if ( log_manager_w )
		{
			log_manager_w->purge_log();
			log_manager_w->log_message(L"Log purged");
		}
	}
	else if ( input.compare(0,std::min(input.size(),static_cast<size_t>(8)),"dump_log") == 0 )
	{
		if ( log_manager_a )
		{
			std::stringstream	parse;
			std::string			filename("");
			bool				truncate = false;


			parse << input.substr(8);
			parse >> filename >> (std::boolalpha) >> truncate >> (std::noboolalpha);

		
			if ( filename != "" )
			{
				if ( truncate )
					log_manager_a->dump(filename,DawnEngine::IO::TRUNCATE);
				else
					log_manager_a->dump(filename,DawnEngine::IO::APPEND);
			}

			log_manager_a->log_message("Log dumped to %s.",filename.c_str());
		}
	}
	else if ( input.compare(0,std::min(input.size(),static_cast<size_t>(3)),"run") == 0 )
	{
		if ( server != NULL )
			server->parse_string(input.substr(3));
	}
};

int	main( int argc , char** argv )
{
	int	return_value = 0;



	argc; argv;
	#ifdef	_DEBUG
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif	/* _DEBUG */

	SetConsoleCtrlHandler(&console_proc,TRUE);


	if ( DawnEngine::IO::LogManagerA::initialise() )
	{
		if ( DawnEngine::Network::Server::initialise() )
		{
			if ( DawnEngine::Console::ConsoleManagerA::initialise() )
			{
				DawnEngine::IO::LogManagerA*			log_manager = DawnEngine::IO::LogManagerA::get();
				DawnEngine::Console::ConsoleManagerA*	console = DawnEngine::Console::ConsoleManagerA::get();
				DawnEngine::Network::Server*			server = DawnEngine::Network::Server::get();



				if ( log_manager != NULL  &&  console != NULL  &&  server != NULL )
				{
					std::string						server_config_file("LoginServer.config");
					std::string						log_file("LoginServer.log");
					DawnEngine::IO::LogFileOpenMode	dump_mode = DawnEngine::IO::TRUNCATE;



					console->input_function(parse_input);
					console->create(150,50);
					server->config_file(server_config_file);
					
					try
					{
						server->start();
				
						while ( server->run() )
							console->update();
					}
					catch ( std::exception& e )
					{
						if ( log_manager != NULL )
							log_manager->log_error(DawnEngine::IO::LogExceptionA(e.what()));
					}

					server->close();
					console->destroy();
					log_manager->dump(log_file,dump_mode);
				}
				
				DawnEngine::Console::ConsoleManagerA::deinitialise();
			}

			DawnEngine::Network::Server::deinitialise();
		}

		DawnEngine::IO::LogManagerA::deinitialise();
	}


	return	return_value;
};