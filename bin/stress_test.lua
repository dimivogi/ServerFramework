local	login_server_sockets = {};
local	shard_list_server_sockets = {};
local	shard_server_sockets = {};
local	chat_server_server_sockets = {};
local	current_time = {};
local	previous_time = {};
local	authenticated = {};
local	got_list = {};
local	connected = {};
local	logged_in = {};
local	chat_connected = {};
local	tokens = {};
local	authentication_request = {};
local	list_request = {};
local	connect_request = {};
local	login_request = {};
local	chat_connect_request = {};
local	CLIENTS = 1;
local	starting_id = 502;
local	BROADCAST_TIME = 250;
local	sockets = Sockets;
local	default_operation = default_operate;
local	math_random = math.random;
local	math_seed = math.randomseed;
local	math_modf = math.modf;
local	get_time = time;
local	get_difftime = difftime;
local	string_len = string.len;
local	string_format = string.format;
local	string_byte = string.byte;
local	table_insert = table.insert;
local	table_concat = table.concat;



for i = 1,CLIENTS do

	--	Server socket creation.
	--	Format: { <ip address either IPv4 or IPv6> , <port number> , <if it's TCP or not> , <if it's IPv6 or not> }
	local login_socket = CreateServerSocket("120.0.0.1",9171,true,false);
	
	
	
	if ( login_socket ~= nil ) then
	
		login_socket:AvailableOperations(1);
		table_insert(login_server_sockets,login_socket:GetID());
		
	else
		table_insert(login_server_sockets,0);
	end
	
	table_insert(tokens,0);
	table_insert(authenticated,false);
	table_insert(got_list,false);
	table_insert(connected,false);
	table_insert(logged_in,false);
	table_insert(chat_connected,false);
	table_insert(authentication_request,false);
	table_insert(list_request,false);
	table_insert(connect_request,false);
	table_insert(login_request,false);
	table_insert(chat_connect_request,false);

end

for i = 1,CLIENTS do

	--	Server socket creation.
	--	Format: { <ip address either IPv4 or IPv6> , <port number> , <if it's TCP or not> , <if it's IPv6 or not> }
	local shard_list_socket = CreateServerSocket("120.0.0.1",9172,true,false);
	
	
	
	if ( shard_list_socket ~= nil ) then
	
		shard_list_socket:AvailableOperations(1);
		table_insert(shard_list_server_sockets,shard_list_socket:GetID());
		
	else
		table_insert(shard_list_server_sockets,0);
	end

end

for i = 1,CLIENTS do

	--	Server socket creation.
	--	Format: { <ip address either IPv4 or IPv6> , <port number> , <if it's TCP or not> , <if it's IPv6 or not> }
	local shard_socket = CreateServerSocket("120.0.0.1",9173,true,false);
	
	
	
	if ( shard_socket ~= nil ) then
		table_insert(shard_server_sockets,shard_socket:GetID());
	else
		table_insert(shard_server_sockets,0);
	end

end

for i = 1,CLIENTS do

	--	Server socket creation.
	--	Format: { <ip address either IPv4 or IPv6> , <port number> , <if it's TCP or not> , <if it's IPv6 or not> }
	local chat_socket = CreateServerSocket("120.0.0.1",9174,true,false);
	
	
	
	if ( chat_socket ~= nil ) then
		table_insert(chat_server_server_sockets,chat_socket:GetID());
	else
		table_insert(chat_server_server_sockets,0);
	end

end


local function	operate( id ) 

	current_time[id] = get_time("ms");
	
	if ( previous_time[id] == nil ) then
		previous_time[id] = current_time[id];
	end
	
	math_seed(current_time[id]);
	default_operation();
	
	
	if ( #current_time > 0  and  get_difftime(current_time[id],previous_time[id]) >= BROADCAST_TIME ) then
	
		local	per_id = select(1,math_modf(CLIENTS / #current_time));
		local	remaining = CLIENTS % #current_time;
		local	not_first = 1;
		
		
		
		if ( id == 1 ) then
		
			per_id = per_id + remaining;
			not_first = 0;
			
		end
		
		for i = 1,per_id do
		
			local	target = (id-1)*per_id + not_first*remaining + i;
			
			
			
			if ( target <= CLIENTS ) then

				if ( not authentication_request[target]  and  login_server_sockets[target] > 0  and  sockets[login_server_sockets[target]]:IsConnected() ) then
			
					local	login_message = {
												[0] = AUTHENTICATION_REQUEST , 
											};
					local	username = "TestUser"..(target+starting_id-1);
					local	password = "TestPassword";
					local	username_len = string_len(username);
					local	password_len = string_len(password);
					
					
					
					for i = 1,username_len do
						login_message[i] = string.byte(username,i);
					end
					
					login_message[username_len+1] = 0;
					
					for i = 1,password_len do
						login_message[username_len+1+i] = string.byte(password,i);
					end
					
					sockets[login_server_sockets[target]]:Send(login_message);
					authentication_request[target] = true;
				
				elseif ( authenticated[target] ) then
				
					if ( not list_request[target]  and  shard_list_server_sockets[target] > 0  and  sockets[shard_list_server_sockets[target]]:IsConnected() ) then
					
						tokens[target][0] = SERVER_LIST_REQUEST;
						sockets[shard_list_server_sockets[target]]:Send(tokens[target]);
						list_request[target] = true;
					
					elseif ( got_list[target] ) then
					
						if ( not connect_request[target]  and  shard_server_sockets[target] > 0  and  sockets[shard_server_sockets[target]]:IsConnected() ) then
						
							tokens[target][0] = USER_CONNECT;
							sockets[shard_server_sockets[target]]:Send(tokens[target]);
							connect_request[target] = true;
						
						elseif ( connected[target] ) then
						
							if ( not login_request[target] ) then
							
								sockets[shard_server_sockets[target]]:Send({[0]=CHARACTER_LOGIN,[1]=1});
								login_request[target] = true;
							
							elseif ( logged_in[target] ) then
							
								if ( not chat_connect_request[target]  and  chat_server_server_sockets[target] > 0  and  sockets[chat_server_server_sockets[target]]:IsConnected() ) then
								
									tokens[target][0] = USER_CONNECT;
									tokens[target][2] = 1;
									sockets[chat_server_server_sockets[target]]:Send(tokens[target]);
									chat_connect_request[target] = true;
								
								elseif ( chat_connected[target] ) then
								
									local	percentage = math_random()*100;
									
									
									
									if ( percentage >= 25 ) then
									
										sockets[shard_server_sockets[target]]:Send({	
																						[0] = CHARACTER_MOVEMENT , 
																						current_time[id],
																						math_random(1,2) , 
																						math_random(0,40) - 20 , 
																						3 , 
																						math_random(0,40) - 20 , 
																						math_random(0,360)
																					});
										
										if ( percentage >= 50 ) then
										
											sockets[shard_server_sockets[target]]:Send( {
																							[0] = CHARACTER_INFO , 
																							1 ,
																							math_random(1,100)
																						});
										
											if ( percentage >= 66 ) then

												local	chat_message = {
																			[0] = CHAT_MESSAGE , 
																			0 , 
																			49 , 
																			50 , 
																			51 , 
																			52 ,
																			53 ,
																			54 , 
																			55 ,
																		};
												local	chat_command = math_random(1,10);
												local	channel_id = math_random(1,11*CLIENTS);



												if ( chat_command > 4 ) then
													chat_message[1] = channel_id;
												elseif ( chat_command > 2 ) then
												
													chat_message[0] = CHAT_USER_REQUEST_JOIN_CHANNEL;
													chat_message[1] = string_byte("M");
													chat_message[2] = string_byte("a");
													chat_message[3] = string_byte("p");
													chat_message[4] = 0;
													chat_message[5] = 0;
													chat_message[6] = 0;
													chat_message[7] = 0;
													chat_message[8] = 0;
													sockets[chat_server_server_sockets[target]]:Send(chat_message);
													
													chat_message[0] = CHAT_USER_REQUEST_JOIN_CHANNEL;
													chat_message[1] = math_random(49,255);
													chat_message[2] = math_random(49,255);
													chat_message[3] = math_random(49,255);
													chat_message[4] = math_random(49,255);
													chat_message[5] = math_random(49,255);
													chat_message[6] = math_random(49,255);
													chat_message[7] = math_random(49,255);
													chat_message[8] = math_random(49,255);
												
												else
												
													chat_message[0] = CHAT_USER_REQUEST_LEAVE_CHANNEL;
													chat_message[1] = channel_id;
												
												end

												sockets[chat_server_server_sockets[target]]:Send(chat_message);


												if ( percentage >= 95 ) then
													sockets[shard_server_sockets[target]]:Send({ [0] = CHARACTER_TRANSPORT, math_random(1,2) });
												end
											
											end
											
										end
										
									end
								
								end
							
							end
							
						end
					
					end
				
				end
				
			end
		
		end
		
		previous_time[id] = current_time[id];
	
	end

end

--	Function performing the default reaction of the server.
local function	reaction( socket , message )
	
	if ( message[0] ~= CHARACTER_FAILURE  and  message[0] ~= CHAT_FAILURE ) then
	
		local	output = {};
		
		
		
		output[1] = "Received on socket "..socket:GetID()..": "..string_format("0x%x",message[0]);
		
		for i = 1,#message do
			table_insert(output,message[i]);
		end
		
		print(table_concat(output," ",1,#output));
		
	end
	
end

--	Function hanlding AUTHENTICATION_SUCCESS messages.
local function	user_authenticated( socket , message )

	local	socket_id = socket:GetID();
	local	index = socket_id - 2;
	
	
	
	if ( index > 3*CLIENTS ) then
		index = index - 3*CLIENTS;
	elseif ( index > 2*CLIENTS ) then
		index = index - 2*CLIENTS;
	elseif ( index > CLIENTS ) then
		index = index - CLIENTS;
	end
	
	tokens[index] = message;
	authenticated[index] = true;
	reaction(socket,message);
	
end

--	Function handling SERVER_LIST_SIZE messages.
local function	got_server_list( socket , message )

	local	socket_id = socket:GetID();
	local	index = socket_id - 2;
	
	
	
	if ( index > 3*CLIENTS ) then
		index = index - 3*CLIENTS;
	elseif ( index > 2*CLIENTS ) then
		index = index - 2*CLIENTS;
	elseif ( index > CLIENTS ) then
		index = index - CLIENTS;
	end
	
	got_list[index] = true;
	reaction(socket,message);

end

--	Function handling USER_QUEUE_POSITION messages.
local function	user_queue( socket , message )

	local	socket_id = socket:GetID();
	local	index = socket_id - 2;
	
	
	
	if ( index > 3*CLIENTS ) then
		index = index - 3*CLIENTS;
	elseif ( index > 2*CLIENTS ) then
		index = index - 2*CLIENTS;
	elseif ( index > CLIENTS ) then
		index = index - CLIENTS;
	end
	
	connected[index] = true;
	reaction(socket,message);

end

--	Function handling CHARACTER_SUCCESS messages.
local function	character_success( socket , message )

	if ( message[1] == CHARACTER_LOGIN ) then
	
		local	socket_id = socket:GetID();
		local	index = socket_id - 2;
		
		
		
		if ( index > 3*CLIENTS ) then
			index = index - 3*CLIENTS;
		elseif ( index > 2*CLIENTS ) then
			index = index - 2*CLIENTS;
		elseif ( index > CLIENTS ) then
			index = index - CLIENTS;
		end
		
		logged_in[index] = true;
		
	end
	
	reaction(socket,message);

end

--	Function handling USER_SUCCESS messages.
local function	user_success( socket , message )

	if ( message[1] == USER_CONNECT ) then
	
		local	socket_id = socket:GetID();
		local	index = socket_id - 2;
		
		
		
		if ( index > 3*CLIENTS ) then
			index = index - 3*CLIENTS;
		elseif ( index > 2*CLIENTS ) then
			index = index - 2*CLIENTS;
		elseif ( index > CLIENTS ) then
			index = index - CLIENTS;
		end
		
		chat_connected[index] = true;
		
	end
	
	reaction(socket,message);

end


RegisterFunction(OPERATION,operate);
RegisterFunction(DEFAULT_ACTION,reaction);
RegisterFunction(AUTHENTICATION_SUCCESS,user_authenticated);
RegisterFunction(SERVER_LIST_SIZE,got_server_list);
RegisterFunction(USER_QUEUE_POSITION,user_queue);
RegisterFunction(CHARACTER_SUCCESS,character_success);
RegisterFunction(USER_SUCCESS,user_success);
