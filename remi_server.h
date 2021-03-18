
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


namespace remi {

	namespace server {
		//Dictionary<Tag*> runtimeInstances;

		class ServerResponse {

		public:

			ServerResponse();
			ServerResponse( int code );
			ServerResponse( std::string body );

			~ServerResponse();

			void setHeader( std::string name , std::string value );

			void appendToBody( std::string content );

			void appendToBody( const char* buffer , int buffer_size );

			int		getCode();
			void	setCode( int code );

			const char*		getBodyBuffer();
			int				getBodyBufferSize();

		private:

			Dictionary<std::string>		_headers;

			char*	_body_buffer;
			int		_body_buffer_size;

			int		_code;

			int		prepareSize( int new_size );
		};



		class App:public EventListener, private utils::TimerListener {

		public:

			App();

			~App(){
                for(WebsocketClientInterface* wci : _webSocketClients){
                    delete wci;
                }
			}

			virtual void idle(){
                // Idle function is called before each update cycle
			};

			void init(std::string host_address);

			virtual Widget* main();

			ServerResponse*	serve( std::string url );

			void setRootWidget(Widget* widget);

			void _notifyParentForUpdate(EventSource*, Dictionary<Buffer*>*, void* );

            std::string getStaticFile(std::string filename);

            /* these method must handle a mutex for the access to websocket list */
            void addWebsocketClientInterface(WebsocketClientInterface* wci);
            void sendMessageToAllClients(std::string message);
			void executeJavascript(std::string command);
			
            virtual void onpageerror(void* emitter, Dictionary<Buffer*>* params, void* user_data);
            virtual void onload(void* emitter, Dictionary<Buffer*>* params, void* user_data);
            virtual void ononline(void* emitter, Dictionary<Buffer*>* params, void* user_data);
            virtual void onpagehide(void* emitter, Dictionary<Buffer*>* params, void* user_data);
            virtual void onpageshow(void* emitter, Dictionary<Buffer*>* params, void* user_data);
            virtual void onresize(void* emitter, Dictionary<Buffer*>* params, void* user_data);

            void onTimer();

            void update();

		private:
		    utils::Timer	    _updateTimer;

			Widget*             _rootWidget;

			bool                _needUpdateFlag;

			list<WebsocketClientInterface*>	_webSocketClients;

			bool                _mutex_blocked_webSocketClients;

		protected:

			//std::string		_staticResourcesPath;
			std::string		_scriptHeader;

			std::string		_cssHeader;

			std::map<std::string, std::string> _staticResourcesPaths;

        public:
			remi::HTML*      html;
			remi::HEAD*      head;
			remi::BODY*      body;

		};

		class AnonymousServer : private utils::TimerListener {

		public:

			AnonymousServer();

			void address();

			void start( void* user_data = NULL );

			void onTimer();

			virtual App* buildInstance() = 0;

		private:

			void* _serverInfo;

			utils::Timer	_updateTimer;

        public:
            Dictionary<App*> _guiInstances;

		};

		template< class AppClass >
		class Server : public AnonymousServer {

		public:

			App* buildInstance(){
				App* _app = (App*)(new AppClass());
				return _app;
			}

		};

	}
}

#endif
