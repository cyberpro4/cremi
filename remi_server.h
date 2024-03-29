
#ifndef __REMI_SERVER_H
#define __REMI_SERVER_H

#include "remi.h"
#include "websocket.h"

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "TinySHA1.hpp"
#include <map>
#include <string>

#pragma pointers_to_members(full_generality, virtual_inheritance)

namespace remi {

	namespace server {
		//Dictionary<Tag*> runtimeInstances;

		class ServerResponse {

		public:

			ServerResponse();
			ServerResponse(int code);
			ServerResponse(std::string body);

			~ServerResponse();

			void setHeader(std::string name, std::string value);

			void appendToBody(std::string content);

			void appendToBody(const char* buffer, int buffer_size);

			int		getCode();
			void	setCode(int code);

			const char* getBodyBuffer();
			int				getBodyBufferSize();

		private:

			Dictionary<std::string>		_headers;

			char* _body_buffer;
			int		_body_buffer_size;

			int		_code;

			int		prepareSize(int new_size);
		};



		class App :public remi::Event<>::EventListener,
			public remi::Event<float,float>::EventListener,
			public remi::HEAD::onerror::EventListener,
			public CommonAppInterface, 
			private utils::TimerListener,
			public remi::EventSource{
		public: //events
			class onexpired :public Event<>{
			public:
				onexpired(EventSource* eventSource) :Event::Event(eventSource, CLASS_NAME(onexpired)) {
					//THIS IS NOT A JAVASCRIPT EVENT HANDLER eventSource->event_handlers.set(this->_eventName, this);
				}
				void operator()() {
					if (this->_listener_function != NULL) {
						this->_listener_function(_eventSource, this->_userData);
						return;
					}
					if (this->_listener_member != NULL) {
						CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, this->_userData);
					}
					if (this->_listener_context_lambda != NULL) {
						this->_listener_context_lambda(_eventSource, this->_userData);
						return;
					}
				}
			}*event_onexpired;
		public:

			App();

			~App() {
				delete event_onexpired;
				for (WebsocketClientInterface* wci : _webSocketClients) {
					delete wci;
				}
			}

			virtual void idle() {
				// Idle function is called before each update cycle
			};

			void init(std::string host_address);

			virtual Widget* main();

			ServerResponse* serve(std::string url);

			void setRootWidget(Widget* widget);

			void _notifyParentForUpdate(EventSource*, void*);

			std::string getStaticFile(std::string filename);

			void setExpireTimeout(int seconds);

			/* these method must handle a mutex for the access to websocket list */
			void addWebsocketClientInterface(WebsocketClientInterface* wci);
			void sendMessageToAllClients(std::string message);
			void executeJavascript(std::string command);

			virtual void onpageerror(void* emitter, std::string message, std::string source, std::string lineno, std::string colno, std::string error, void* user_data);
			virtual void onload(void* emitter, void* user_data);
			virtual void ononline(void* emitter, void* user_data);
			virtual void onpagehide(void* emitter, void* user_data);
			virtual void onpageshow(void* emitter, float width, float height, void* user_data);
			virtual void onresize(void* emitter, float width, float height, void* user_data);

			void onTimer();

			void update();

		private:
			utils::Timer	    _updateTimer;

			Widget* _rootWidget;

			bool                _needUpdateFlag;

			list<WebsocketClientInterface*>	_webSocketClients;

			bool                _mutex_blocked_webSocketClients;

			int					_expireTimeoutSeconds;
			long long int		_secondsSinceLastWebsocketClientDropped;

		protected:

			//std::string		_staticResourcesPath;
			std::string		_scriptHeader;

			std::string		_cssHeader;

			std::map<std::string, std::string> _staticResourcesPaths;

		public:
			remi::HTML* html;
			remi::HEAD* head;
			remi::BODY* body;
		};

		class AnonymousServer : private utils::TimerListener, public App::onexpired::EventListener {

		public:

			AnonymousServer();

			void address();

			void start(const char* address, int port = 91);

			void onTimer();

			virtual App* buildInstance() = 0;

			void onAppExpired(EventSource* emitter, void* userdata);

		private:

			void* _serverInfo;

			utils::Timer	_updateTimer;

		public:
			Dictionary<App*> _guiInstances;

		};

		template< class AppClass >
		class Server : public AnonymousServer {

		public:

			App* buildInstance() {
				App* _app = (App*)(new AppClass());
				return _app;
			}

		};

	}
}

#endif
