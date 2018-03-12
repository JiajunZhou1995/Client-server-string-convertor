# cs454a3

Tonghui Ma, Ruijie Zhang
20376486, 20487924
July 12, 2017


How to Run:
	1. use command "make" to create 3 executables "client" , “server”,  and “binder”
	2.  The binder needs to run first, to run, use ./binder
		it should print two statements, one is the BINDER_ADDRESS of which the 			binder is running on.
		the other is dynamically allocated port number BINDER_PORT

	3.  Set environment variable on both server machine and client machine using the 	
	     following commands:
		export BINDER_ADDRESS=<get from binder output>
		export BINDER_PORT=<get from server output>
		
	4.  Launch server using command: 
		./server
		the server will try to register 4 functions to binder and waiting for client 
		to connect.
		if something is wrong, appropriate error code will be returned.
		see the Error code section in this doc for details
   5.   Launch client using command:
		./client
		the client will first send location request message to the binder to get the server
		and port information about the server which provides this service.
		Appropriate status information will be printed to the screen
		if the previous action is successful, then the client will talk to the server
		to call the remote procedure that it needed.

		if any of the previous step encounters an error, then an appropriate error code 
		will be returned and printed.

		see the error code section for details

