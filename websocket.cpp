
#include "websocket.h"

#include <string>
#include <stdio.h>

#include <openssl\sha.h>

using namespace remi::server;

remi_thread_result WebsocketClientInterface_threadEntry( remi_thread_param param ){
	((WebsocketClientInterface*)param)->_run();
	return 0;
}

WebsocketClientInterface::WebsocketClientInterface( remi_socket clientSock , struct sockaddr_in clientAddr ){
    _sock = clientSock;
    _stopFlag = false;
	_handshakeDone = false;
	_t = remi_createThread( (remi_thread_callback)&WebsocketClientInterface_threadEntry , this );

	_secondsSinceLastPing = remi_timestamp();
}

void WebsocketClientInterface::stop(){
	_stopFlag = true;
}

void* WebsocketClientInterface::_run(){
    //here the recv send is managed
    //  at first glance, the handshake have to be performed
    //  after that, new messages an managed in on_message
    while( !_stopFlag ){
        
		/*char buffer[64] = {0};

		int recv_bytes = recv( _sock , buffer , 64 , 0 );

		if( recv_bytes < 1 )
			continue;

		if( recv_bytes == SOCKET_ERROR )
            break;

		std::cout << buffer;*/

		if( !_handshakeDone ){
			handshake();
		} else {
			if( !readNextMessage() ){
				_handshakeDone = false;
				std::cout << "ws: read next message has failed";
			}
		}
    }

	return NULL;
}

bool WebsocketClientInterface::readNextMessage(){

	char length_buf[8] = {0};

	if( recv( _sock , length_buf, 2 , 0 ) != 2 )
		return false;

	unsigned long long int payload_len = 0;
	unsigned int _l = length_buf[1] & 127;
	payload_len = _l;

	bool mask_enable = length_buf[1] & (1 << 7);

	// manage payload len 126 / 127
	if( payload_len == 126 ){
		if( recv( _sock , length_buf, 2 , 0 ) != 2 )
			return false;

		unsigned short _l = 0;
		memcpy( &_l , length_buf , 2 );
		payload_len = _l;

	} else if( payload_len == 127 ){
		if( recv( _sock , length_buf, 8 , 0 ) != 8 )
			return false;

		unsigned long long int _l = 0;
		memcpy( &_l , length_buf , 8 );
		payload_len = _l;
	}

	char mask[4];

	if( mask_enable ){
		if( recv( _sock , mask , 4 , 0 ) != 4 )
			return false;
	}

	char *buf = new char [ payload_len +1 ];
	memset( buf , 0 , payload_len+1 );

	int _rv = -1;
	if( (_rv = recv( _sock , buf , payload_len , 0 )) != payload_len ){
		delete buf;
		return false;
	}

	for(int l = 0; l < payload_len; l++ ){
		buf[l] = buf[l] ^ mask[ l % 4 ];
	}

	on_message( remi::utils::url_decode( buf ) );

	delete buf;

	return true;
}

void WebsocketClientInterface::handshake(){

	char buffer[1024] = {0};

	recv( _sock , buffer , 1024 , 0 );

	std::string sbuff = buffer;

	std::list<std::string> sec_pieces = utils::split( sbuff , "Sec-WebSocket-Key: " );
	if( sec_pieces.size() != 2 )
		return;

	std::list<std::string> pieces = utils::split( utils::list_at( sec_pieces, 1 ), "\r\n" );
	if( pieces.size() < 1 )
		return;

	std::string key = utils::list_at( pieces, 0 );
	key.append( "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" );

	unsigned char key_sha1[64] = {0};

	SHA1( (const unsigned char*)key.c_str() , key.length() , key_sha1 );
	
	std::string sha1( (const char*)key_sha1 );

	std::string b64 = utils::base64( sha1 );
	

	std::cout << b64 << std::endl;

	std::ostringstream response_s;
	response_s 
		<< "HTTP/1.1 101 Switching Protocols\r\n"
		<< "Upgrade: websocket\r\n"
		<< "Connection: Upgrade\r\n"
		<< "Sec-WebSocket-Accept: " << b64 << "\r\n\r\n";/* % digest.decode("utf-8");*/
	std::string response = response_s.str();

	send( _sock , response.c_str() , response.length() , 0 );

	_handshakeDone = true;
}

void WebsocketClientInterface::on_message( std::string message){

	std::cout << "ws: " << message << std::endl;

	if( message == "pong" )
		return;

	send_message("ack");

	std::list<std::string> chunks = utils::split( message, "/" );

	if( chunks.size() > 3 ){

		if( utils::list_at( chunks , 0 ) == "callback" ){
			std::string widget_id = utils::list_at( chunks , 1 );
			std::string function_name = utils::list_at( chunks , 2 );

			std::cout << "ws: call id = " << widget_id << " . " << function_name << std::endl;
		}

	}

}

void WebsocketClientInterface::send_message( std::string message){

	size_t message_length = message.length();

	// Prepare the buffer length for the maximum size condition
	// +1 byte for fix 129
	// +1 for length
	// +8 for extreme payload condition
	// buffer_length will be decreased later
	size_t buffer_length = message_length +1 +1 +8;

	unsigned char* buf = new unsigned char[ buffer_length ];

	unsigned char* lpmsg = &buf[0];

	*lpmsg = 129;
	lpmsg++;

	if( message_length <= 125 ) {

		*lpmsg = message_length;
		buffer_length -= 8;

	} else if( message_length >= 126 && message_length <= 65535 ){

		buffer_length -= 4;
		*lpmsg = 126;lpmsg++;
		unsigned short _m = message_length;
		memcpy( lpmsg , &_m , 2 );
		lpmsg += 2;

	} else {

		*lpmsg = 127;lpmsg++;
		unsigned long long int _m = message_length;
		memcpy( lpmsg , &_m , 8 );
		lpmsg += 8;

	}

	memcpy( lpmsg , message.c_str() , message_length );

	send( _sock , (const char*)buf, buffer_length, 0 );
	
}



remi_thread_result WebsocketServer_threadEntry( remi_thread_param WebsocketServerInstance ){
	((WebsocketServer*)WebsocketServerInstance)->_listenAsync(NULL);
	return 0;
}

WebsocketServer::WebsocketServer(int port){
    _port = port;
    _stopFlag = false;
    _socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(_socketFd < 0){
        //cerr << "WebsocketServer::WebsocketServer - cannot open socket" << endl;
        //return 0;
		return;
    }
    
    memset((void*) &_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;
	remi_socket_setaddr( _address.sin_addr, INADDR_ANY );
	_address.sin_port = htons(_port);

	if( bind( _socketFd , (SOCKADDR*)&_address, sizeof( SOCKADDR_IN ) ) < 0 ){
		return;
	}

    if( listen( _socketFd , SOMAXCONN ) < 0 ){
        return;
    }
    
    _t = remi_createThread( (remi_thread_callback)&WebsocketServer_threadEntry, (void*)this );
}

void* WebsocketServer::_listenAsync(void* data){

	while( true ){
 
        SOCKADDR_IN         clientSock;
        remi_socket_len		clientLen = sizeof( clientSock );
 
        remi_socket		client = accept( _socketFd , (SOCKADDR*)&clientSock , &clientLen );

        if( client != INVALID_SOCKET ){

			//std::string key = uniqueClientKey(clientSock);
			std::string key = "remove_me";
            _clients.set(key , new WebsocketClientInterface( client , clientSock ) );

            //_log( "new client" );

            /*NetServer_ClientThread_Data*    data = new NetServer_ClientThread_Data;
            data->socket = client;
            data->server = netClass;
            //CreateThread( 0 , 0 , (LPTHREAD_START_ROUTINE)&NetServer_ClientThread , data , 0 , 0 );*/
            //remi_createThread( (remi_thread_callback)&NetServer_ClientThread , data );
        }
 
        Sleep( 100 );
	}

	return NULL;
}