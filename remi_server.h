
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

		

		class App {

		public:

			App();

			virtual Widget* main();

			void init();

			ServerResponse*		serve( std::string url );

		private:

			Widget* _rootWidget;

			WebsocketServer*	_webSocketServer;

		protected:

			std::string		_staticResourcesPath;
			std::string		_scriptHeader;

			std::string		_cssHeader;

		};

		class AnonymousServer {

		public:

			AnonymousServer();

			void address();

			void start( void* user_data = NULL );

			void serve_forever();

			void stop();

			ServerResponse*		serve( std::string url );

			virtual App* buildInstance() = 0;

		private:

			void* _serverInfo;

			App* _guiInstance;

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