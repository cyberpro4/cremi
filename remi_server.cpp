
#include <microhttpd.h>

#include "remi_server.h"

#include <stdlib.h>
#include <regex>
#include <stdio.h>
#include <string>


using namespace remi::server;
using namespace remi;

ServerResponse::ServerResponse() {
	_body_buffer = NULL;
	_body_buffer_size = 0;
	_code = -1;
}

ServerResponse::ServerResponse(int code) {
	_body_buffer = NULL;
	_body_buffer_size = 0;
	_code = code;
}

ServerResponse::ServerResponse(std::string body) {

	_body_buffer = NULL;
	_body_buffer_size = 0;

	appendToBody(body);

	_code = 200;
}

ServerResponse::~ServerResponse() {
	if (_body_buffer != NULL)
		delete[] _body_buffer;
}

int	ServerResponse::getCode() {
	return _code == -1 ? 500 : _code;
}

void ServerResponse::setCode(int code) {
	_code = code;
}

void setHeader(std::string name, std::string value);

void ServerResponse::appendToBody(std::string content) {

	int old_size = prepareSize(_body_buffer_size + content.length() + 1);

	memcpy(&_body_buffer[old_size], content.c_str(), content.length() + 1);
}

void ServerResponse::appendToBody(const char* buffer, int buffer_size) {
	int old_size = prepareSize(_body_buffer_size + buffer_size);

	memcpy(&_body_buffer[old_size], buffer, buffer_size);
}

const char* ServerResponse::getBodyBuffer() {
	return _body_buffer;
}

int	ServerResponse::getBodyBufferSize() {
	return _body_buffer_size;
}

int ServerResponse::prepareSize(int new_size) {

	if (_code == -1)
		_code = 200;

	if (new_size <= _body_buffer_size)
		return _body_buffer_size;

	char* _nb = new char[new_size];
	int old_size = _body_buffer_size;

	if (_body_buffer != NULL) {
		memcpy(_nb, _body_buffer, _body_buffer_size);
		delete[] _body_buffer;
	}

	_body_buffer = _nb;
	_body_buffer_size = new_size;

	return old_size;
}


struct POST_Handler_Connection_Info_Struct {
	std::string filename;
	std::string listenerFuncName;
	remi::FileUploader* fu;
	struct MHD_PostProcessor* postprocessor;
};

static int __remi_server_connection_post_handler(void* cls,
	enum MHD_ValueKind kind,
	const char* key,
	const char* filename,
	const char* content_type,
	const char* transfer_encoding,
	const char* data,
	uint64_t off,
	size_t size
	/*void* coninfo_cls, enum MHD_ValueKind kind, const char* key,
	const char* filename, const char* content_type,
	const char* transfer_encoding, const char* data,
	uint64_t off, size_t size*/) {
	struct POST_Handler_Connection_Info_Struct* ci = (POST_Handler_Connection_Info_Struct*)cls;

	if (ci->fu->event_handlers.has(ci->listenerFuncName)) {
		Dictionary<Buffer*> dict;
		Buffer* param = new Buffer((char*)filename, strlen(filename));
		dict.set("filename", param);

		param = new Buffer((char*)data, size);
		dict.set("data", param);

		if (!(content_type == NULL)) {
			param = new Buffer((char*)content_type, strlen(content_type));
			dict.set("content_type", param);
		}

		if (!(transfer_encoding == NULL)) {
			param = new Buffer((char*)transfer_encoding, strlen(transfer_encoding));
			dict.set("transfer_encoding", param);
		}
		reinterpret_cast<Event*>(ci->fu->event_handlers[ci->listenerFuncName].value)->operator()(&dict);

		//dict.clear();
		/*for (std::string key : dict.keys()) {
			delete (dict.get(key));
		}*/
		return MHD_YES;
	}
	return MHD_NO;
}

void /* *MHD_UpgradeHandler*/ __remi_server_connection_upgrade_handler(void* remi_application, struct MHD_Connection* connection,
	void* param, const char* extra_in, size_t extra_in_size,
	MHD_socket sock, struct MHD_UpgradeResponseHandle* urh) {

	WebsocketClientInterface* wci = new WebsocketClientInterface(sock, false);
	((App*)remi_application)->addWebsocketClientInterface(wci);
}

static MHD_Result
__remi_server_answer(void* cls, struct MHD_Connection* connection,
	const char* url, const char* method,
	const char* version, const char* upload_data,
	unsigned int* upload_data_size, void** con_cls) {

	struct MHD_Response* response;
	MHD_Result ret;

	const char* sessionCookieValue = MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "remi_session");
	if (strcmp(method, "POST") == 0) {
		//this is a POST request, in remi it is used normally for file upload requests
		
		ServerResponse* serverResponse;
		
		if (sessionCookieValue == NULL || ((AnonymousServer*)cls)->_guiInstances.has(sessionCookieValue) == false) {
			//if the application has still not established a session, this is a strange situation, quit
			serverResponse = new ServerResponse(401); //Unauthorized
		} else {
			//if this is a new POST request
			if(*con_cls == NULL){
				const char* filename = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "filename");
				const char* listener = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "listener");
				const char* listener_function = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "listener_function");
				*con_cls = new POST_Handler_Connection_Info_Struct;
				((POST_Handler_Connection_Info_Struct*)*con_cls)->filename = filename;
				((POST_Handler_Connection_Info_Struct*)*con_cls)->listenerFuncName = listener_function;
				((POST_Handler_Connection_Info_Struct*)*con_cls)->fu = (remi::FileUploader*)((void*)atoi(listener));
				((POST_Handler_Connection_Info_Struct*)*con_cls)->postprocessor = MHD_create_post_processor(connection, 2048, (MHD_PostDataIterator)__remi_server_connection_post_handler, *con_cls);

				MHD_post_process(((POST_Handler_Connection_Info_Struct*)*con_cls)->postprocessor, upload_data, *upload_data_size);
				*upload_data_size = 0;

				return MHD_YES;
			}

			//if there are data to handle
			if (*upload_data_size != 0){
				MHD_post_process(((POST_Handler_Connection_Info_Struct*)*con_cls)->postprocessor, upload_data, *upload_data_size);
				*upload_data_size = 0;

				return MHD_YES;
			}
			
			//Data upload terminated
			MHD_destroy_post_processor(((POST_Handler_Connection_Info_Struct*)*con_cls)->postprocessor);
			
			App* _guiInstance = ((AnonymousServer*)cls)->_guiInstances.get(sessionCookieValue);
			serverResponse = _guiInstance->serve(url);
			serverResponse->setCode(200);

			//https://www.gnu.org/software/libmicrohttpd/tutorial.html#Processing-POST-data
		}

		response = MHD_create_response_from_buffer(serverResponse->getBodyBufferSize(), (void*)serverResponse->getBodyBuffer(), MHD_RESPMEM_MUST_COPY);

		ret = MHD_queue_response(connection, serverResponse->getCode(), response);

		MHD_destroy_response(response);
		delete serverResponse;

	} else if(strcmp(method, "GET") == 0) {

		if (cls != NULL) {
			const char* value = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Connection");
			//if(strcmp(value, "Upgrade")==0){
			if (remi::utils::count(value, "Upgrade") > 0) {
				const char* upgrade_kind = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Upgrade");
				if (strcmp(upgrade_kind, "websocket") == 0) {

					if (sessionCookieValue != NULL && ((AnonymousServer*)cls)->_guiInstances.has(sessionCookieValue)) {
						response = MHD_create_response_for_upgrade(&__remi_server_connection_upgrade_handler, (void*)((AnonymousServer*)cls)->_guiInstances.get(sessionCookieValue));
						if (!response) {
							cout << "__remi_server_answer - failed to create upgrade response" << endl;
						}

						const char* websocket_key = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Sec-WebSocket-Key");
						std::list<std::string> pieces = remi::utils::split(std::string(websocket_key), "\r\n");
						std::string key = remi::utils::list_at(pieces, 0);
						key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

						unsigned char key_sha1[64] = { 0 };

						std::string sha1_1(remi::utils::SHA1(key));

						std::string b64 = base64_encode((unsigned char*)sha1_1.c_str(), sha1_1.length());
						cout << "b64:" << b64.c_str() << endl;

						//headers from https://gitlab.univ-nantes.fr/milliat-a/Logger/blob/8a4cfcb90bee798fef2ef8576b618bc00f47514f/HTTP/Sources/HTTPHandler.cpp
						ret = MHD_add_response_header(response, MHD_HTTP_HEADER_UPGRADE, "websocket");
						ret = MHD_add_response_header(response, MHD_HTTP_HEADER_CONNECTION, MHD_HTTP_HEADER_UPGRADE);
						ret = MHD_add_response_header(response, "Sec-WebSocket-Accept", b64.c_str());
						ret = MHD_queue_response(connection, MHD_HTTP_SWITCHING_PROTOCOLS, response);
						MHD_destroy_response(response);
					}

				}
			} else {

				ostringstream session_key_value;
				bool newSession = false;
				char newSessionValue[80] = { 0 };
				if (sessionCookieValue == NULL || ((AnonymousServer*)cls)->_guiInstances.has(sessionCookieValue) == false) {
					//session is not valid
					newSession = true;
					time_t rawtime;
					struct tm* timeinfo;
					time(&rawtime);
					timeinfo = localtime(&rawtime);

					strftime(newSessionValue, 80, "%d%m%Y%H%M%S", timeinfo);

					session_key_value << "remi_session=" << newSessionValue << "; SameSite=Lax";

					App* _guiInstance = (App*)((AnonymousServer*)cls)->buildInstance();
					const char* host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Host");
					cout << ">>>>>> host:" << host << endl << endl;
					_guiInstance->init(std::string(host));
					((AnonymousServer*)cls)->_guiInstances.set(newSessionValue, _guiInstance);
				}

				ServerResponse* serverResponse;
				App* _guiInstance = ((AnonymousServer*)cls)->_guiInstances.get(newSession ? newSessionValue : sessionCookieValue);
				serverResponse = _guiInstance->serve(url);
				serverResponse->setCode(200);

				response = MHD_create_response_from_buffer(serverResponse->getBodyBufferSize(), (void*)serverResponse->getBodyBuffer(), MHD_RESPMEM_MUST_COPY);

				//if this is a newly created session, set the cookie
				if (newSession) {
					/*
					if(MHD_NO == MHD_set_connection_value (connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_SET_COOKIE, session.str().c_str())){
						cout << "AnonymousServer::serve - ERROR unable to set session value." << endl;
						return MHD_NO;
					}
					*/
					cout << "SESSION COOKIE: " << session_key_value.str() << endl;
					ret = MHD_add_response_header(response, "Set-Cookie", session_key_value.str().c_str());
					//MHD_set_connection_value (connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_SET_COOKIE, "SameSite=Lax");
				}

				ret = MHD_queue_response(connection, serverResponse->getCode(), response);

				MHD_destroy_response(response);
				delete serverResponse;

			}
		}
	}
	return ret;
}

AnonymousServer::AnonymousServer() {
	_updateTimer.setInterval(100);
	_updateTimer.setListener(this);
	_updateTimer.start();
}

void AnonymousServer::address() {
}

void AnonymousServer::onTimer() {
	/* here maybe we can delete expired App instances */
}

void AnonymousServer::start(const char* address, int port) {

	struct MHD_Daemon* daemon;

	struct sockaddr_in daemon_ip_addr;
	memset(&daemon_ip_addr, 0, sizeof(struct sockaddr_in));
	daemon_ip_addr.sin_family = AF_INET;
	daemon_ip_addr.sin_port = htons(port);
	//daemon_ip_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	daemon_ip_addr.sin_addr.s_addr = inet_addr(address);

	std::cout << "cRemi Http server listening on port " << port << std::endl;
	/*unsigned int flags,
	  uint16_t port,
	  MHD_AcceptPolicyCallback apc, void *apc_cls,
	  MHD_AccessHandlerCallback dh, void *dh_cls,*/
	daemon = MHD_start_daemon((unsigned int)MHD_USE_SELECT_INTERNALLY | MHD_ALLOW_SUSPEND_RESUME | MHD_ALLOW_UPGRADE,
		(uint16_t)port,
		(MHD_AcceptPolicyCallback)NULL, (void*)NULL,
		(MHD_AccessHandlerCallback)& __remi_server_answer, this,
		MHD_OPTION_SOCK_ADDR, &daemon_ip_addr,
		MHD_OPTION_END);

	_serverInfo = (void*)daemon;

}


App::App() {
	_rootWidget = NULL;
}

void App::init(std::string host_address) {
	html = new remi::HTML();

	//head.setTitle(self.server.title);

	// use the default css, but append a version based on its hash, to stop browser caching
	head = new remi::HEAD(std::string("Remi App"));
	head->addChild("<link href='/res:style.css' rel='stylesheet' />\n", "internal_css");
	struct sockaddr* so;

	//head->setInternalJs(utils::sformat("%d", (int)this), host_address, 20, 3000);
	head->setInternalJs("127.0.0.1:91", 20, 3000);
	LINK_EVENT_TO_CLASS_MEMBER(head->event_onerror, this, &App::onpageerror);

	body = new remi::BODY();
	body->addClass("remi-main");
	LINK_EVENT_TO_CLASS_MEMBER(body->event_onload, this, &App::onload);
	LINK_EVENT_TO_CLASS_MEMBER(body->event_ononline, this, &App::ononline);
	LINK_EVENT_TO_CLASS_MEMBER(body->event_onpagehide, this, &App::onpagehide);
	LINK_EVENT_TO_CLASS_MEMBER(body->event_onpageshow, this, &App::onpageshow);
	LINK_EVENT_TO_CLASS_MEMBER(body->event_onresize, this, &App::onresize);

	html->addChild(head, "head");
	html->addChild(body, "body");
	LINK_EVENT_TO_CLASS_MEMBER(html->event_onrequiredupdate, this, &App::_notifyParentForUpdate);

	setRootWidget(this->main());

	_updateTimer.setInterval(100);
	_updateTimer.setListener(this);
	_updateTimer.start();
}

remi::Widget* App::main() {
	return 0;
}

std::string App::getStaticFile(std::string filename) {
	//cout << "App::getStaticFile - filename: " << filename << endl;
	if (filename.find("..") > -1) {
		filename = filename.replace(filename.find(".."), 2, ""); //avoid backdirs
	}
	int __i = filename.find(':');
	if (__i < 0) {
		return std::string("");
	}
	std::string key = filename.substr(0, __i);
	std::string path = filename.substr(__i + 1, filename.length() - 1);
	key = key.replace(key.find("/"), 1, "");

	map<std::string, std::string> paths;
	paths["res"] = std::string("./res/");
	for (auto& p : this->_staticResourcesPaths) {
		paths[p.first] = p.second;
	}

	if (paths.count(key) < 1) {
		return std::string("");
	}
	std::string result = paths[key] + "/" + path;
	while (result.find("//") > -1) {
		result = result.replace(result.find("//"), 2, "/");
	}
	return result;
}

ServerResponse* App::serve(std::string url) {

	std::ostringstream output;

	//std::regex::

	std::smatch attr_call_match;
	bool attr_call = std::regex_match(url, attr_call_match, std::regex("^/*(\\w+)\\/(\\w+)\\?{0,1}(\\w*\\={1}(\\w|\\.)+\\&{0,1})*$")); //std::regex("^\\/*(\\w+)\\/(\\w+)\\?{0,1}(\\w*\\={1}\\w+\\&{0,1})*$"));

	std::smatch static_file_match;
	bool static_file = std::regex_match(url, static_file_match, std::regex("^([\\/]*[\\w\\d]+:[-_. $@?#£'%=()\\/\\[\\]!+°§^,\\w\\d]+)")); //std::regex("^/*res\\/(.*)$") );

	if (static_file && static_file_match.size() == 2) {

		ServerResponse* response = new ServerResponse();

		std::string resFilePath = this->getStaticFile(static_file_match[1].str()); //utils::sformat("%s/%s", _staticResourcesPath.c_str() , static_file_match[1].str().c_str() );
		//cout << "App::serve - resource file path: " << resFilePath << endl;
		FILE* resFile = fopen(resFilePath.c_str(), "rb");
		if (!resFile)
			return new ServerResponse(404); // FIXME

		char buf[2048];

		while (!feof(resFile)) {
			size_t r = fread(buf, 1, 2048, resFile);
			response->appendToBody(buf, r);
		}

		fclose(resFile);

		return response;

	}
	else if (attr_call) {

	}
	else if (url == "/") {
		Dictionary<Represantable*> changedWidgets;
		output << html->repr(&changedWidgets, true);
		//cout << "PAGE: " << output.str() << endl;
	}

	return new ServerResponse(utils::string_encode(output.str()));
}

void App::addWebsocketClientInterface(WebsocketClientInterface* wci) {
	while (this->_mutex_blocked_webSocketClients)
		Sleep(1);

	_mutex_blocked_webSocketClients = true;
	this->_webSocketClients.push_back(wci);
	_mutex_blocked_webSocketClients = false;

}

void App::sendMessageToAllClients(std::string message) {
	while (this->_mutex_blocked_webSocketClients)
		Sleep(1);

	_mutex_blocked_webSocketClients = true;
	for (WebsocketClientInterface* wci : _webSocketClients) {
		wci->send_message(message);
	}
	_mutex_blocked_webSocketClients = false;
}

void App::executeJavascript(std::string command) {
	this->sendMessageToAllClients(WebsocketClientInterface::packExecuteJavascriptMessage(command));
}

void App::onTimer() {
	//dropping died websocket clients
	while (this->_mutex_blocked_webSocketClients)
		Sleep(1);
	_mutex_blocked_webSocketClients = true;
	std::list<WebsocketClientInterface*> diedClients;
	for (WebsocketClientInterface* wci : _webSocketClients) {
		if (wci->isDead())
			diedClients.push_back(wci);
	}
	for (WebsocketClientInterface* wci : diedClients) {
		_webSocketClients.remove(wci);
		std::cout << "Deleting died websocket client" << std::endl;
		delete wci;
	}
	_mutex_blocked_webSocketClients = false;

	//update ui
	this->idle();
	this->update();
}

void App::update() {
	if (this->_rootWidget == NULL) {
		return;
	}
	if (this->_needUpdateFlag) {
		Dictionary<Represantable*>* local_changed_widgets = new Dictionary<Represantable*>();
		std::string _html = remi::utils::string_encode(remi::utils::escape_json(this->_rootWidget->repr(local_changed_widgets, false)));

		for (std::string identifier : local_changed_widgets->keys()) {
			_html = ((Tag*)local_changed_widgets->get(identifier))->getLatestRepr();
			//cout << "App::update - update message: " << WebsocketClientInterface::packUpdateMessage(identifier, _html).c_str() << endl;
			this->sendMessageToAllClients(WebsocketClientInterface::packUpdateMessage(identifier, remi::utils::string_encode(remi::utils::escape_json(_html))));
		}

		delete local_changed_widgets;
		_needUpdateFlag = false;
		return;
	}
}

void App::onpageerror(void* emitter, remi::Dictionary<remi::Buffer*>* params, void* user_data) {
	std::cout << "Event onPageError - ";
	for (std::string key : params->keys()) {
		std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
	}
}
void App::onload(void* emitter, Dictionary<Buffer*>* params, void* user_data) {
}
void App::ononline(void* emitter, Dictionary<Buffer*>* params, void* user_data) {
}
void App::onpagehide(void* emitter, Dictionary<Buffer*>* params, void* user_data) {
}
void App::onpageshow(void* emitter, Dictionary<Buffer*>* params, void* user_data) {
	std::cout << "Event onPageShow - ";
	for (std::string key : params->keys()) {
		std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
	}
	int width = 0;
	int height = 0;
	sscanf(params->get("width")->str().c_str(), "%d", &width);
	sscanf(params->get("height")->str().c_str(), "%d", &height);
	cout << width << " - " << height << endl;
}
void App::onresize(void* emitter, Dictionary<Buffer*>* params, void* user_data) {
}

void App::setRootWidget(Widget* widget) {
	body->append(widget, "root");
	_rootWidget = widget;
	Dictionary<Represantable*> changedWidgets;
	std::ostringstream msg;
	msg << "0" << _rootWidget->getIdentifier().c_str() << ',' << remi::utils::string_encode(remi::utils::escape_json(body->innerHTML(&changedWidgets, true)));

	this->sendMessageToAllClients(msg.str());

}

void App::_notifyParentForUpdate(EventSource* source, Dictionary<Buffer*>* params, void* user_data) {
	this->_needUpdateFlag = true;
}






