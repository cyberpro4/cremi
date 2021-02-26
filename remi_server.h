
#ifndef __REMI_SERVER_H
#define __REMI_SERVER_H

#include "remi.h"
#include "websocket.h"

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#endif

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



		class App:public EventListener {

		public:

			App();

			virtual Widget* main();

			void init(std::string host_address);

			ServerResponse*		serve( std::string url );

			void setRootWidget(Widget* widget);

			bool update();

            void _notifyParentForUpdate(EventSource*, Dictionary<Buffer*>*, void* );

		private:
			Widget*             _rootWidget;

			WebsocketServer*	_webSocketServer;

			bool                _needUpdateFlag;

		protected:

			std::string		_staticResourcesPath;
			std::string		_scriptHeader;

			std::string		_cssHeader;

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

			void serve_forever();

			void stop();

			void onTimer();

			ServerResponse*		serve( std::string url, struct MHD_Connection *connection );

			virtual App* buildInstance() = 0;

		private:

			void* _serverInfo;

			App* _guiInstance;

			utils::Timer	_updateTimer;

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
