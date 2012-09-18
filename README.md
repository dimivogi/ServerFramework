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