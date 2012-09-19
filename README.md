ServerFramework
===============

The server framework and the servers that were developed during my dissertation for Msc. Games Programming

===============

Installation guide


In order for the framework to compile and run correctly you need Lua v5.2, MySQL Connector C and PDCurses library.
Download the libraries at www.lua.org, www.mysql.com  and pdcurses.sourceforge.net.
Once the libraries have been downloaded, put them in the Libraries directory in SharedFiles directory and put any relevant .dll files
in the bin directory. If you need to build lua you can easily use the Lua solution file. The final folder structure should be


	SharedFiles

		Libraries
		
			Lua
			MySQLConnectorC
			pdcurses
		
		SourceCode
	

The vs2012 folders contain solutions and project files for visual studio 2012, the other folders contain solutions and projects
for visual studio 2010.

The bin directory contains the configuration and script files for each server. Theses files can be used as a template if a new type of 
server needs to be created.

=================

Contents

The aim of this project was to develop an environment suitable for a Massively Multiplayer Online Game. 
The development was split into three parts: Server Infrastructure, Rendering and User Interface. 
The set goal was to create a framework that a game could be built upon. 
This repository contains the Server Infrastructure of the project. 
The server framework utilizes networking via TCP and UDP using overlapped I/O, scripting in Lua and database data through connections to MySQL databases. 
By using Lua scripts one can define the reaction of the server to any incoming messages or to connect to a database.


=================

Available Lua functions


1. Default Libraries and functions 

• Lua Bitwise Library (bit32)
• Lua Math Library (math)
• Lua String Library (string)
• Lua Table Library (table)
• Assert function (assert)
• Error function (error)
• Get Metatable function (getmetatable)
• Ipairs function (ipairs)
• Next function (next)
• Pairs function (pairs)
• Print function (print) - Modified
• Print Error function (printerror) - Added
• Print Warning function (printwarning) - Added
• Raw Equal function (rawequal)
• Raw Length function (rawlen)
• Raw Get function (rawget)
• Raw Set function (rawset)
• Select function (select)
• Set Metatable function (setmetatable)
• To number function (tonumber)
• To string function (tostring)
• To Boolean function (toboolean) – Added
• Type function (type)
• Clock function (clock, originally os.clock)
• Date function (date, originally os.date )
• Difftime function (difftime, originally os.difftime)
• Time function (time, originally os.time) – Modified


2. Framework functions

2.1. Global functions

• Create server socket function (CreateServerSocket)
• Connect to database function (ConnectToDatabase)
• Functionality function register function (RegisterFunction)
• Functionality function unregister function (UnregisterFunction)
• Encrypt function (Encrypt)
• Decrypt function (Decrypt)
• Send to all clients function (SendToClients)
• Send to all servers function (SendToServers)
• Send to all sockets function (SendToAll)
• Sleep function (Sleep)

2.2. Socket functions

• Send message function(Send)
• Server status alteration function (Server)
• Available operation count alteration function (AvailableOperations)
• Disconnect socket function (Disconnect)
• Socket ID retrieval function (GetID)
• Connection info retrieval info (GetConnectionInfo)
• Server status retrieval function (IsServer)
• Available operation count retrieval function (AvailableOperationsCount)
• Socket connection status function (IsConnected)

2.3. Database functions

• Prepared statement creation function (CreatePreparedStatement)
• Database update function (Update)
• Database query function (Query)
• Database connection status (IsConnected)
