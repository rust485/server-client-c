Compiling:
	To Compile my code, you must cd into ./client and run make, 	and then cd into ./server and run make. Running make will 	delete the old binary files and re-compile the files

Running:
  NOTE: Must run the server before the client.

  Server:
    cd into server and run ./server. This will start the server       
    on port 8080.

  Client:
    cd into client and run ./client [-h] [-d 
    <days>:<hours>:<minutes>] http://<hostname>[:<port number>] 
    <resource>
      
      -h will request only the header from the host
      
      -d <time interval> will request to receive the object only 
      if the requested object was modified between the last days, 
      hours, and minutes ago from now

Description:
  Server:
    The server will run and wait for clients to request to the 
    server.

    After a client sends a request, the server creates a thread 
    to parse and handle the clients request while the main thread 
    returns to the accept command to wait for another child to 
    make a request.

    The server first parses the request, if the Request starts 
    with "HEAD", then the server will only send the header for 
    the file requested. If the client request starts with "GET", 
    then a header along with the requested resource will be 
    written to the client.

    If the request includes an "If-Modified-Since: " tag, then 
    the server will only write the object back only if the file    
    was modified since the specified date.

    Date comparison is done by converting the date in the request 
    to time_t and retrieving the last-modified attribute of the 
    requested resource, then using difftime to compare the two 
    times

Client:
  The client will make a request to the specified host, at the 
  specified port (default 80), for the specified resource. The 
  client will form the header, establish a connection with the 
  host, and send the request to the server.

  Afterwards, the client waits for the server to respond.

  The server response is written to the response file, which is 
  removed and then recreated before writing to it, inside of the 
  client directory.
