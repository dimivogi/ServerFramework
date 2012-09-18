--	Local variables used to improve performance.
local	table_insert = table.insert;
local	table_unpack = table.unpack;
local	table_concat = table.concat;
local	string_char = string.char;
local	string_find = string.find;
local	math_min = math.min;
local	math_max = math.max;
local	sockets = Sockets;
local	create_server_socket = CreateServerSocket;
local	send_to_servers = SendToServers;
local	table_find = find_in_table;
local	token_generation = generate_token;
local	encrypt = Encrypt;
local	default_operation = default_operate;
local	shard_manager_server_type = SHARD_MANAGER_SERVER_TYPE;
local	chat_server_type = CHAT_SERVER_TYPE;
--	The needed databases connections.
--	Format: { <ip> , <port> , <username> , <password> , <schema> }
--	Connection as reader to the game databases database.
local	databases = ConnectToDatabase("120.0.0.1",3306,"reader","readerpass","GameDatabases");



if ( databases ~= nil ) then 

	--	The number of passes for the encrypt function.
	local	pass_count = 100;
	--	The id of the server.
	local	id = login_server_id;
	-- The prepared statement that is used to get the databases for a given server.
	local	get_databases = databases:CreateStatement("SELECT IP,PORT,USERNAME,PASSWORD,DATABASE_NAME FROM Registry WHERE SERVER_ID = ? AND USERNAME = ?;");
	local	login_database = databases:Query(get_databases,"susss","us",id,"writer");
	local	server_database = databases:Query(get_databases,"susss","us",shard_list_server_id,"writer");

	
	
	if ( login_database ~= nil  and  server_database ~= nil ) then
	
		--	Connection as reader to the account database.
		local	account = ConnectToDatabase(login_database[1][1],login_database[1][2],login_database[1][3],login_database[1][4],login_database[1][5]);
		--	Connection as writer to the servers database.
		local	servers = ConnectToDatabase(server_database[1][1],server_database[1][2],server_database[1][3],server_database[1][4],server_database[1][5]);
		--	The prepared statement that is used to get account information from the database.
		local	select_info = account:CreateStatement("SELECT ID , MMORPG FROM Registry WHERE USERNAME = ? AND PASSWORD = ?;");
		--	The prepared statement that is used to get the server information from the server database.
		local	get_server_info = servers:CreateStatement("SELECT IP,PORT,TCP FROM Registry WHERE TYPE = ? OR TYPE = ?");
		--	The prepared statement that is used to set the token of the server upon startup.
		local	set_server_token = servers:CreateStatement("UPDATE Registry SET TOKEN = ? WHERE ID = ?;");
		--	The prepared statement that is used to change the session token.
		local	set_token = account:CreateStatement("UPDATE Registry SET MMORPG = ? WHERE ID = ?;");		
		--	Database query holding information regarding the server list.
		local	server_list = servers:Query(get_server_info,"suu","uu",shard_manager_server_type,chat_server_type);
		--	Tabel holding whether an authentication message has been sent to server sockets.
		local	server_status = {
									{} ,	-- An array holding the id of the server sockets , 
									{} , 	-- An array holding whether the server socket has been authenticated , 
									{} , 	-- An array holding whether a authentication request has been sent to the server socket.
								};
		--	The token that is used to authenticate the server by the other servers.
		local	session_token = {};
		--	The message that is sent when an incoming message is not supported by the server.
		local	message_not_supported = { [0] = MESSAGE_NOT_SUPPORTED };
		--	The message that is sent when authentication fails.
		local	message_authentication_failure = { [0] = AUTHENTICATION_FAILURE };
		--	The message that is sent to any servers that the server is connected to in order to authenticate.
		local	authenticate_message = {
											[0] = REQUEST_CONNECTION , 
											id 
										};
		
		
		
		if ( server_list ~= nil ) then 
			
			for i = 1,#server_list do
			
				if ( server_list[i][1] ~= nil  and  server_list[i][1] ~= "" ) then 
				
					--	Server socket creation.
					--	Format: { <ip address either IPv4 or IPv6> , <port number> , <if it's TCP or not> , <if it's IPv6 or not> }
					local	socket = create_server_socket(server_list[i][1],server_list[i][2],( server_list[i][3] ~= 0 ),( string_find(server_list[i][1],":") ~= nil ));
				
				
				
					if ( socket ~= nil ) then
					
						table_insert(server_status[1],socket:GetID());
						table_insert(server_status[2],false);
						table_insert(server_status[3],false);
						
					end
				end
				
			end
			
			server_list = nil;
			
		end


		--	Function responsible of performing any server operations. It is performing cleanup of the Sockets table if necessary.
		local function	operate( id )

			local	done = false;
			local	counter = 1;
			
			
			
			default_operation();
			
			while ( not done  and  counter <= #server_status[1] ) do
			
				if ( not server_status[2][counter]  and  not server_status[3][counter] ) then
				
					if ( sockets[server_status[1][counter]]:IsConnected() ) then
						
						sockets[server_status[1][counter]]:Send(authenticate_message);
						server_status[3][counter] = true;
						done = true;
						
					else
						counter = counter + 1;
					end
					
				else
					counter = counter + 1;
				end
			
			end

		end
		
		--	Function responsible of performing cleanup when a socket is closed.
		local function	socket_closed( socket_id , server )

			if ( server ) then
			
				local	index = table_find(server_status[1],socket_id);
				
				
				
				if ( index > 0 ) then
				
					server_status[2][index] = false;
					server_status[3][index] = false;
				
				end
				
			end
			
		end
		
		--	Function responsible of performing any relevant operation when the server shuts down.
		local function	server_shutdown()
		end
			
		
		--	Function responsible of handling any SERVER_AUTHENTICATION_SUCCESS messages.
		local function	server_authenticated( socket , message )
		
			if ( #server_status[1] > 0 ) then
			
				if ( socket:IsServer()  and  socket:IsConnected() ) then
				
					local	index = table_find(server_status[1],socket_id);
					
					
					
					if ( index > 0 ) then 
						server_status[2][index] = true;
					end
					
				end
			
			end
		
		end
		
		--	Function responsible of handling any SERVER_AUTHENTICATION_FAILURE messages.
		local function	server_not_authenticated( socket , message )
		
			if ( #server_status[1] > 0 ) then
			
				if ( socket:IsServer() ) then
			
					local	index = table_find(server_status[1],socket_id);
					
					
					
					if ( index > 0 ) then 
						server_status[3][index] = false;
					end
				
				end
				
			end
		
		end

		--	Function responding to any incoming messages other than SERVER_AUTHENTICATION_SUCCESS , SERVER_AUTHENTICATION_FAILURE , AUTHENTICATION_REQUEST.
		local function	default_reaction( socket , message )

			if ( socket:IsConnected() ) then
				
				socket:Send(message_not_supported);
				
				if ( not socket:IsServer() ) then
					socket:Disconnect();
				end
			
			end

		end

		--	Function responding to an AUTHENTICATION_REQUEST message, by authenticating the user.
		local function	parse_message( socket , message )

			if ( socket:IsConnected()  and  not socket:IsServer() ) then
				
				local	username_chars = {};
				local	password_chars = {};
				local	result = nil;
				local	username = nil;
				local	password = nil;
				local	change = false;
				local	done = false;
				local	counter = 1;
				
				
				
				while( counter <= #message  and not done ) do
					
					if ( message[counter] ~= 0 ) then
						
						local	character = string_char(math_max(0,math_min(message[counter],255)));
						
						
						
						if ( change == true ) then 
							password_chars[#password_chars + 1] = character;
						else
							username_chars[#username_chars + 1] = character;
						end
						
					else
						
						if ( change == true ) then
							done = true;
						else
							change = true;
						end
						
					end
				
					counter = counter + 1;
				end
			
				username = table_concat(username_chars);
				password = table_concat(password_chars);
				username = encrypt(username,username,pass_count);
				password = encrypt(password,password,pass_count);
				result = account:Query(select_info,"us","ss",username,password);
				
				if ( result ~= nil ) then
				
					local	reply_message = {
												[0] = AUTHENTICATION_SUCCESS , 
												result[1][1] , 
												0
											};
												
												
								
					if ( result[1][2] == nil  or  result[1][2] == "" ) then
						
						local	token =	token_generation(61);
												
												
							
						print("Authenticated user: "..result[1][1]);
						
						for i = 1,#token do
							table_insert(reply_message,token[i]);
						end
						
						account:Update(set_token,"su",string_char(table_unpack(token)),result[1][1]);
						socket:Send(reply_message);
						
					else
						
						reply_message[0] = DISCONNECT_USER;
						send_to_servers(reply_message);
						socket:Send(message_authentication_failure);
						account:Update(set_token,"su",nil,result[1][1]);
						
					end
				
				else
					socket:Send(message_authentication_failure);
				end
				
				socket:Disconnect();
			
			end

		end


		--	Create server token.
		session_token = generate_token(62);
		
		for i = 1,#session_token do
			table_insert(authenticate_message,session_token[i]);
		end
		
		servers:Update(set_server_token,"su",string_char(table_unpack(session_token)),id);
		
		
		--	Registering the functions.
		RegisterFunction(OPERATION,operate);
		RegisterFunction(SOCKET_DISCONNECT,socket_closed);
		RegisterFunction(SERVER_SHUTDOWN,server_shutdown);
		RegisterFunction(SERVER_AUTHENTICATION_SUCCESS,server_authenticated);
		RegisterFunction(SERVER_AUTHENTICATION_FAILURE,server_not_authenticated);
		RegisterFunction(DEFAULT_ACTION,default_reaction);
		RegisterFunction(AUTHENTICATION_REQUEST,parse_message);
		
	end
	
end