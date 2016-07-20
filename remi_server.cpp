
#include <microhttpd.h>

#include "remi_server.h"

#include <stdlib.h>
#include <regex>
#include <stdio.h>
#include <string>

using namespace remi::server;


ServerResponse::ServerResponse(){
	_body_buffer = NULL;
	_body_buffer_size = 0;
	_code = -1;
}

ServerResponse::ServerResponse( int code ){
	_body_buffer = NULL;
	_body_buffer_size = 0;
	_code = code;
}

ServerResponse::ServerResponse( std::string body ){
	
	_body_buffer = NULL;
	_body_buffer_size = 0;

	appendToBody( body );

	_code = 200;
}

ServerResponse::~ServerResponse(){
	if( _body_buffer != NULL )
		delete _body_buffer;
}

int	ServerResponse::getCode(){
	return _code == -1 ? 500 : _code;
}

void ServerResponse::setCode( int code ){
	_code = code;
}

void setHeader( std::string name , std::string value );

void ServerResponse::appendToBody( std::string content ){

	int old_size = prepareSize( _body_buffer_size + content.length() + 1 );
	
	memcpy( &_body_buffer[old_size] , content.c_str() , content.length() + 1 );
}

void ServerResponse::appendToBody( const char* buffer , int buffer_size ){
	int old_size = prepareSize( _body_buffer_size + buffer_size );
	
	memcpy( &_body_buffer[old_size] , buffer , buffer_size );
}

const char*		ServerResponse::getBodyBuffer(){
	return _body_buffer;
}

int	ServerResponse::getBodyBufferSize(){
	return _body_buffer_size;
}

int ServerResponse::prepareSize( int new_size ){

	if( _code == -1 )
		_code = 200;

	if( new_size <= _body_buffer_size )
		return _body_buffer_size;

	char* _nb = new char [ new_size ];
	int old_size = _body_buffer_size;

	if( _body_buffer != NULL ){
		memcpy( _nb , _body_buffer , _body_buffer_size );
		delete _body_buffer;
	}

	_body_buffer = _nb;
	_body_buffer_size = new_size;

	return old_size;
}


static int
__remi_server_answer(void *cls, struct MHD_Connection *connection,
const char *url, const char *method,
const char *version, const char *upload_data,
size_t *upload_data_size, void **con_cls)
{
	
	struct MHD_Response *response;
	int ret;

	if (cls != NULL){
		ServerResponse* serverResponse = ((AnonymousServer*)cls)->serve( url );
	

		response =
			MHD_create_response_from_buffer( 
				serverResponse->getBodyBufferSize() , 
				(void *)serverResponse->getBodyBuffer(),
				MHD_RESPMEM_MUST_COPY
			);

		ret = MHD_queue_response(connection, serverResponse->getCode(), response);

		MHD_destroy_response(response);

		delete serverResponse;

	}

	return ret;
}

AnonymousServer::AnonymousServer(){
	_guiInstance = NULL;
}

void AnonymousServer::address(){
}

void AnonymousServer::start(void* user_data){

	struct MHD_Daemon *daemon;

	daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 91, NULL, NULL,
		&__remi_server_answer, this, MHD_OPTION_END);

	_serverInfo = (void*)daemon;

}

ServerResponse* App::serve(std::string url){

	std::ostringstream output;

	//std::regex::

	std::smatch attr_call_match;
	bool attr_call = std::regex_match(url, attr_call_match, std::regex("^\\/*(\\w+)\\/(\\w+)\\?{0,1}(\\w*\\={1}\\w+\\&{0,1})*$"));

	std::smatch static_file_match;
	bool static_file = std::regex_match( url, static_file_match, std::regex("^/*res\\/(.*)$") );

	if( static_file && static_file_match.size() == 2 ){

		ServerResponse* response = new ServerResponse();

		std::string resFilePath = utils::sformat("%s/%s", _staticResourcesPath.c_str() , static_file_match[1].str().c_str() );

		FILE* resFile = fopen( resFilePath.c_str() , "rb" );
		if( !resFile )
			return new ServerResponse(404); // FIXME

		char buf[ 2048 ];

		while( !feof( resFile ) ){
			size_t r = fread( buf , 1 , 2048 , resFile );
			response->appendToBody( buf , r );
		}
	
		fclose( resFile );

		return response;

	} else if ( attr_call ){

	} else if (_rootWidget != NULL && url == "/" ){
		output 
			<< "<!DOCTYPE html>"
			<< "<html><head>" 

			<< "<meta content = 'text/html;charset=utf-8' http - equiv = 'Content-Type'>"
			<< "<meta content = 'utf-8' http - equiv = 'encoding'>"
			<< "<meta name = \"viewport\" content = \"width=device-width, initial-scale=1.0\">"

			<< "<link href=\"res/style.css\" rel=\"stylesheet\" / >"

			<< "<script>var net_interface_ip = 'localhost';</script>"
			<< "<script>var wsport = 92;</script>"
			<< "<script>var pending_messages_queue_length = 10;</script>"
			<< "<script>var websocket_timeout_timer_ms = 200;</script>"
			<< "<script src=\"res/remi.js\"></script>"

			<< "</head><body>"
			<< _rootWidget->repr()
			<< "</body></html>";
	}

	return new ServerResponse( utils::string_encode( output.str() ) );
}

void AnonymousServer::serve_forever(){
}

void AnonymousServer::stop(){
}

App::App(){
	_rootWidget = NULL;
}


remi::Widget* App::main(){
	return 0;
}

void App::init(){

	_staticResourcesPath = "./res/";

	_rootWidget = this->main();

	_webSocketServer = new WebsocketServer( 92 );
}

ServerResponse* AnonymousServer::serve(std::string url){
	
	if (_guiInstance == NULL){
		_guiInstance = (App*)buildInstance();
		_guiInstance->init();
	}

	return _guiInstance->serve(url);
}




