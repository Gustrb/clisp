# This is supposed to be an end-to-end test scenario for serialization

In this one, we will have a server and a client. The server will parse a program and send it to the client.
The client will deserializer_deserialize the program and then send it over the wire to the server
The server will match that what he sent is the same as what he received from the client
