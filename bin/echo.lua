--	Local variables used to improve performance.
local	default_operation = default_operate;
local	send_to_clients = SendToClients;



--	Function responsible of performing any server operations. It is performing cleanup of the Sockets table if necessary.
local function	operate( id )
	default_operation();
end

--	Function responsible of performing cleanup when a socket is closed.
local function	socket_closed( socket_id , server )
end
		
--	Function responsible of performing any relevant operation when the server shuts down.
local function	server_shutdown()
end

--	Function responding to all incoming messages. Simply echoes any received message to all clients.
local function	echo_message( socket , message )
	send_to_clients(message);
end



--	Registering the functions.
RegisterFunction(OPERATION,operate);
RegisterFunction(SOCKET_DISCONNECT,socket_closed);
RegisterFunction(SERVER_SHUTDOWN,server_shutdown);
RegisterFunction(DEFAULT_ACTION,echo_message);