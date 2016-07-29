

#include "websocket.h"


#include <string>
#include <stdio.h>

#include <openssl/sha.h>

#include <regex>

using namespace remi::server;
using namespace remi;

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

	bool mask_enable = (length_buf[1] & (1 << 7)) != 0;

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

	size_t _rv = -1;
	if( (_rv = recv( _sock , buf , payload_len , 0 )) != (size_t)payload_len ){
		delete buf;
		return false;
	}

	for(size_t l = 0; l < _rv; l++ ){
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

Dictionary<std::string>	WebsocketClientInterface::parseParams( std::string paramString ){
	/*std::smatch match;
	std::string out = paramString;*/
	Dictionary<std::string> ret;

	bool firstMatch = true;

	//std::regex_search( out, match, std::regex("[^\\|]+\\|?" ) );

	for( std::string m : utils::split(paramString,"|") ){

		if( firstMatch ){
			firstMatch = false;
			continue;
		}

		std::list<std::string> parts = utils::split(m , "=");

		if( parts.size() != 2 )
			continue;

		ret[utils::list_at(parts, 0)] = utils::list_at(parts,1);

	}

	return ret;
}

void WebsocketClientInterface::on_message( std::string message ){

	//std::cout << "ws: " << message;

	if( message == "pong" )
		return;

	send_message("ack");

	std::list<std::string> chunks = utils::split( message, "/" );

	if( chunks.size() > 3 ){ // msgtype,widget,function,params

		if( utils::list_at( chunks , 0 ) == "callback" ){
			std::string s_widget_id = utils::list_at( chunks , 1 );

			std::string function_name = utils::list_at( chunks , 2 );

			std::cout << "ws: call id = " <<  s_widget_id << "." << function_name << std::endl;
				;

			int widget_id;
			if( utils::sscan( s_widget_id , "%d" , &widget_id  ) != 1 )
				return;

			

			Widget* widget = (Widget*)( (void*)widget_id );

			Event* event = new Event( function_name );
			event->source = widget;
			
			if( chunks.size() >= 4 )
				//event->params = utils::list_at( chunks , 4 );
				event->params = parseParams( utils::list_at( chunks , 3 ) );

			//widget->propagate( event );
			widget->onEvent( event->name , event );

			delete event;
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
		lpmsg++;
		buffer_length -= 8;

	} else if( message_length >= 126 && message_length <= 65535 ){

		buffer_length -= 6;
		*lpmsg = 126;lpmsg++;
		unsigned short _m = message_length;
		lpmsg[0] = (_m >> 8) & 0xff;
		lpmsg[1] = (_m >> 0) & 0xff;
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

WebsocketServer::WebsocketServer( int port ){
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

#ifdef WIN32
			// Clients keys are "[IP_ADDRESS]:[PORT]"
			std::string key = utils::sformat( "%d.%d.%d.%d:%d" , 
				(int)clientSock.sin_addr.S_un.S_un_b.s_b1,
				(int)clientSock.sin_addr.S_un.S_un_b.s_b2,
				(int)clientSock.sin_addr.S_un.S_un_b.s_b3,
				(int)clientSock.sin_addr.S_un.S_un_b.s_b4,
				(int)clientSock.sin_port
			);
#else
			std::string key = utils::sformat( "%s:%d" , 
				inet_ntoa( clientSock.sin_addr ),
				clientSock.sin_port );
#endif

            _clients.set(key , new WebsocketClientInterface( client , clientSock ) );

        }
 
        Sleep( 100 );
	}

	return NULL;
}

void WebsocketServer::sendToAllClients(std::string message){
	//for (std::list<std::string>::iterator key_iterator = _clients.keys().begin(); key_iterator != _clients.keys().end(); key_iterator++){
	for (std::string key : _clients.keys()){
		WebsocketClientInterface* ws = _clients.get(key);

		//a client may die, destroy here the instance of send fails
		//try{
		//cout << message << endl;
			ws->send_message(message.c_str());
		/*}
		catch (Exception e){}*/
	}
}