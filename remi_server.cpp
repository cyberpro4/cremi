
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
		ServerResponse* serverResponse = ((AnonymousServer*)cls)->serve( url, connection );


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
	_updateTimer.setInterval( 100 );
	_updateTimer.setListener( this );
	_updateTimer.start();
}

void AnonymousServer::address(){
}

void AnonymousServer::onTimer(){
	_guiInstance->update();
}

void AnonymousServer::start(void* user_data){

	struct MHD_Daemon *daemon;

	int		port = 91;

	std::cout << "cRemi Http server listening on port " << port << std::endl;

	daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port, NULL, NULL,
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

	} else if (url == "/" ){
	    Dictionary<Represantable*> changedWidgets;
		output << html->repr(&changedWidgets);
		cout << "PAGE: " << output.str() << endl;
	}

	return new ServerResponse( utils::string_encode( output.str() ) );
}

bool App::update(bool avoid_update_because_new_subchild){
	if (this == NULL)return false;
	if (this->_rootWidget == NULL)return false;
	if (this->_rootWidget->isChanged()){

        Dictionary<Represantable*>* local_changed_widgets = new Dictionary<Represantable*>();
		std::string _html = remi::utils::string_encode(remi::utils::escape_json(html->repr(local_changed_widgets)));
		std::ostringstream output;

		output << "update_widget," << this->_rootWidget->getIdentifier().c_str() << "," << _html;
		this->_webSocketServer->sendToAllClients(output.str());

		//update children dictionaries __version__ in order to avoid nested updates
		this->_rootWidget->setUpdated();
		return true;

	}

	/*bool changed_or = false;

	//checking if subwidgets changed
	for (std::string key : child_tag->children.keys()){
		Represantable* represantable = child_tag->children.get(key);
		if (dynamic_cast<Tag*>(represantable) != 0){
			changed_or |= this->update((remi::Widget*)represantable, avoid_update_because_new_subchild);
		}
	}*/

	//propagating the children changed flag
	return false;
}

App::App(){
	_rootWidget = NULL;
}

remi::Widget* App::main(){
	return 0;
}

void App::init(std::string host_address){

	_staticResourcesPath = "./res/";

    html = new remi::HTML();

    //head.setTitle(self.server.title);

    // use the default css, but append a version based on its hash, to stop browser caching
    head = new remi::HEAD(std::string("Remi App"));
    head->addChild("<link href='/res:style.css' rel='stylesheet' />\n", "internal_css");
    struct sockaddr *so;

    //head->setInternalJs(utils::sformat("%d", (int)this), host_address, 20, 3000);
    head->setInternalJs(utils::sformat("%d", (int)this), "127.0.0.1:92", 20, 3000);

    body = new remi::BODY();
    body->addClass("remi-main");
    /*body.onload.connect(self.onload)
    body.ononline.connect(self.ononline)
    body.onpagehide.connect(self.onpagehide)
    body.onpageshow.connect(self.onpageshow)
    body.onresize.connect(self.onresize)*/

    html->addChild(head, "head");
    html->addChild(body, "body");

    _webSocketServer = new WebsocketServer( 92 );

    setRootWidget(this->main());
}

void App::setRootWidget(Widget* widget){
    body->append(widget, "root");
    _rootWidget = widget;
    //_rootWidget.disable_refresh()
    //_rootWidget->attributes["data-parent-widget"] = str(id(self));
    //_rootWidget->setParent(&body);
    //_rootWidget.enable_refresh()
    Dictionary<Represantable*> changedWidgets;
    std::ostringstream msg;
    msg << "0" << _rootWidget->getIdentifier().c_str() << ',' << remi::utils::string_encode(remi::utils::escape_json(body->innerHTML(&changedWidgets)));
    cout << "message 0 to websocket: " << msg.str() << endl;
    this->_webSocketServer->sendToAllClients(msg.str());
}

void App::_notifyParentForUpdate(){
    cout << "APP need update" << endl<<endl<<endl;
}


void AnonymousServer::serve_forever(){
}

void AnonymousServer::stop(){
}

ServerResponse* AnonymousServer::serve(std::string url, struct MHD_Connection *connection){
    cout << ">>>>>> url:" << url << endl << endl;
    const char* value = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Host");
    cout << ">>>>>> host:" << value << endl << endl;
	if (_guiInstance == NULL){
		_guiInstance = (App*)buildInstance();
		_guiInstance->init(std::string(value));
	}

	return _guiInstance->serve(url);
}




