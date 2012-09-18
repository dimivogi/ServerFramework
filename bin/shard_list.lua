--	Local variables used to improve performance.
local	table_insert = table.insert;
local	table_remove = table.remove;
local	table_pack = table.pack;
local	table_unpack = table.unpack;
local	string_byte = string.byte;
local	string_char = string.char;
local	string_find = string.find;
local	string_gmatch = string.gmatch;
local	string_len = string.len;
local	connect_to_database = ConnectToDatabase;
local	token_check = check_token;
local	default_operation = default_operate;
local	table_find = find_in_table;
local	shard_server_type = SHARD_MANAGER_SERVER_TYPE;
local	login_id = login_server_id;
--	The needed databases connections.
--	Format: { <ip> , <port> , <username> , <password> , <schema> }
--	Connection as reader to the game databases database.
local	databases = connect_to_database("120.0.0.1",3306,"reader","readerpass","GameDatabases");



if ( databases ~= nil ) then

	--	The id of the server.
	local	id = shard_list_server_id;
	-- The prepared statement that is used to get the databases for a given server.
	local	get_databases = databases:CreateStatement("SELECT IP,PORT,USERNAME,PASSWORD,DATABASE_NAME FROM Registry WHERE SERVER_ID = ? AND USERNAME = ?;");
	local	login_database = databases:Query(get_databases,"susss","us",login_id,"reader");
	local	server_database = databases:Query(get_databases,"susss","us",id,"writer");
	
	
	
	if ( login_database ~= nil  and  server_database ~= nil ) then
	
		--	The server list.
			server_list = {
									{} ,	-- An array holding the id of the sockets that the servers are connected on.
									{} ,	-- An array holding the database connections to the server databases.
									{} , 	-- An array holding the id of the prepared statements that are used to query each servers database.
									{} 		-- An array holding the messages that contain each server's information.
								};
		--	Connection as reader to the account database.
		local	account = connect_to_database(login_database[1][1],login_database[1][2],login_database[1][3],login_database[1][4],login_database[1][5]);
		--	Connection as writer to the servers database.
		local	servers = connect_to_database(server_database[1][1],server_database[1][2],server_database[1][3],server_database[1][4],server_database[1][5]);
		--	The prepared statement that is used to get the session token from the database.
		local	read_token = account:CreateStatement("SELECT MMORPG FROM Registry WHERE ID = ?;");
		--	The prepared statement that is used to get the information of a server from the database.
		local	read_server_info = servers:CreateStatement("SELECT TYPE,IP,PORT,TCP,NAME,TOKEN FROM Registry WHERE ID = ?;");
		--	The prepared statement that is used to set the token of the server upon startup.
		local	set_server_token = servers:CreateStatement("UPDATE Registry SET TOKEN = ? WHERE ID = ?;");
		--	The token that is used to authenticate the server by the other servers.
		local	session_token = {};
		--	The message that is sent when an incoming message is not supported by the server.
		local	message_not_supported = { [0] = MESSAGE_NOT_SUPPORTED };
		--	The message that is sent when server authentication fails.
		local	message_server_authentication_failure = { [0] = SERVER_AUTHENTICATION_FAILURE };
		--	The message that is sent when server authentication succeeds.
		local	message_server_authentication_success = { [0] = SERVER_AUTHENTICATION_SUCCESS };
		--	The message that is sent when authentication fails.
		local	message_user_failure = { [0] = USER_FAILURE , SERVER_LIST_REQUEST };
		--	The message that is sent when a request for the server list size is parses.
		local	message_server_list_size = { [0] = SERVER_LIST_SIZE , 0 };

		

		--	Function responsible of finding a server in the socket list.
		local function	find_server( socket_id )
			return table_find(server_list[1],socket_id);
		end
		
		--	Function responsible of removing a server from the socket list.
		local function	remove_server( server_index )
		
			if ( server_index > 0 ) then
			
				table_remove(server_list[1],server_index);
				table_remove(server_list[2],server_index);
				table_remove(server_list[3],server_index);
				table_remove(server_list[4],server_index);
				
			end
			
		end
		
		--	Function responsible of creating the message that contains the information of the server.
		local function	create_server_message( ip , port , tcp , server_type , name )
		
			local	message = { 
								[0] = SERVER_INFO , 
								0 , 
								tcp ,
								0 , 
								0 , 
								0 , 
								0 , 
								0 , 
								0 , 
								0 , 
								0 , 
								port , 
								server_type , 
								0 , 
								0
							};
			local	name = table_pack(string_byte(name,1,string_len(name)));
			local	offset = 2;
			local	counter = 1;
			
			
			
			if ( string_find(ip,":")  ~= nil ) then
			
				message[1] = 1;
				
				for i,k in string_gmatch(ip,"(%d+):(%d+)") do 
				
					if ( counter < 8 ) then 
				
						message[offset+counter] = i;
						message[offset+counter+1] = k;
						counter = counter + 2;
						
					end
					
				end
				
			else
				
				for i,k in string_gmatch(ip,"(%d+)%.(%d+)") do 
					
					if ( counter < 4 ) then 
				
						message[offset+counter] = i;
						message[offset+counter+1] = k;
						counter = counter + 2;
						
					end
					
				end
				
			end
			
			for i = 1,#name do
				table_insert(message,name[i]);
			end
			
			
			return message;
		
		end
		
		--	Function responsible of sending server information through a socket.
		local function	send_server_info( socket , server_index , account_id )
		
			if ( server_index > 0  and  server_index <= #server_list[1] ) then
				
				if ( server_list[2][server_index] ~= 0  and  server_list[3][server_index][1] ~= nil ) then
				
					local	characters = server_list[2][server_index]:Query(server_list[3][server_index][1],"u","u",account_id);
					local	population = server_list[2][server_index]:Query(server_list[3][server_index][2],"u");
					
					
					
					if ( characters ~= nil ) then
						server_list[4][server_index][14] = characters[1][1];
					end
					
					if ( population ~= nil ) then
						server_list[4][server_index][13] = population[1][1];
					end
					
				end
				
				socket:Send(server_list[4][server_index]);
				server_list[4][server_index][14] = 0;
				
			end
		
		end
		
		--	Function responsible of performing any server operations. It is performing cleanup of the Sockets table if necessary.
		local function	operate( id )
			default_operation();
		end
		
		--	Function responsible of performing cleanup when a socket is closed.
		local function	socket_closed( socket_id , server )

			if ( server ) then
				remove_server(find_server(socket_id));
			end
			
		end
		
		--	Function responsible of performing any relevant operation when the server shuts down.
		local function	server_shutdown()
		end
			

		--	Function responding to any incoming messages other than REQUEST_CONNECTION and SERVER_LIST_REQUEST.
		local function	default_reaction( socket , message )

			if ( socket:IsConnected() ) then
				
				socket:Send(message_not_supported);
				
				if ( not socket:IsServer() ) then
					socket:Disconnect();
				end
			
			end

		end

		--	Function responsible of handling an incoming REQUEST_CONNECTION message.
		local function	server_connected( socket , message )

			if ( socket:IsConnected()  and  not socket:IsServer() ) then
			
				if ( message[1] ~= id  and  message[1] ~= login_id  ) then 
				
					local	results = servers:Query(read_server_info,"usuuss","u",message[1]);
					
					
					
					if ( results ~= nil ) then
						
						local	address = select(1,socket:GetConnectionInfo());
						
						
						
						if ( address == results[1][2]  and  string_char(table_unpack(message,2)) == results[1][6] ) then
						
							local	world_database = 0;
							local	character_query = { nil , nil };
								
								
								
							print("Authenticated server with id "..message[1]);
							socket:Server(true);
							
							table_insert(server_list[1],socket:GetID());
							table_insert(server_list[4],create_server_message(results[1][2],results[1][3],results[1][4],results[1][1],results[1][5]));
							
							if ( results[1][1] == shard_server_type ) then
								
								local	world_database_info = databases:Query(get_databases,"susss","us",message[1],"reader");
								
								
								
								
								if ( world_database_info ~= nil ) then
									
									world_database = connect_to_database(world_database_info[1][1],world_database_info[1][2],world_database_info[1][3],world_database_info[1][4],world_database_info[1][5]);
									
									if ( world_database ~= nil ) then 
									
										character_query[1] = world_database:CreateStatement("SELECT COUNT(*) FROM Registry WHERE ACCOUNT = ?;");
										character_query[2] = world_database:CreateStatement("SELECT COUNT(*) FROM Registry WHERE LOGGED_IN = 1;");
										
									else
										world_database = 0;
									end
									
								end
								
							end
							
							table_insert(server_list[2],world_database);
							table_insert(server_list[3],character_query)
							
							message_server_list_size[1] = #server_list[1];
							socket:Send(message_server_authentication_success);
							
						else
							
							socket:Send(message_server_authentication_failure);
							socket:Disconnect();
							
						end
						
					else
						
						socket:Send(message_server_authentication_failure);
						socket:Disconnect();
					
					end
					
				else
				
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
				
				end
				
			end
			
		end
		
		--	Function responsible for handling an incoming SERVER_LIST_REQUEST message.
		local function	get_list( socket , message )
		
			if ( socket:IsConnected()  and  not socket:IsServer() ) then 
			
				local	token_result = account:Query(read_token,"s","u",message[1]);
				
				
				
				if ( token_result ~= nil   and token_result[1] ~= nil  and  token_result[1] ~= "" ) then
				
					if ( token_check(token_result[1][1],message,3) ) then
					
						print("Sending server list to "..message[1]..".");
						socket:Send(message_server_list_size);
						
						for i = 1,#server_list[1] do
							send_server_info(socket,i,message[1]);
						end
						
						socket:Disconnect();
					
					else
					
						socket:Send(message_user_failure);
						socket:Disconnect();
					
					end
				
				else
				
					socket:Send(message_user_failure);
					socket:Disconnect();
				
				end
			
			end
		
		end
		
		
		--	Create server token.
		session_token = generate_token(62);
		servers:Update(set_server_token,"su",string_char(table_unpack(session_token)),id);


		--	Registering the functions.
		RegisterFunction(OPERATION,operate);
		RegisterFunction(SOCKET_DISCONNECT,socket_closed);
		RegisterFunction(SERVER_SHUTDOWN,server_shutdown);
		RegisterFunction(DEFAULT_ACTION,default_reaction);
		RegisterFunction(REQUEST_CONNECTION,server_connected);
		RegisterFunction(SERVER_LIST_REQUEST,get_list);
	
	end
	
end