
#ifndef _REMI_WEBSOCKET__
#define _REMI_WEBSOCKET__

#include "remi.h"
#include "base64.h"
#include "TinySHA1.hpp"
#include <iostream>
#include <vector>

using namespace std;

namespace remi {

	namespace server {

		class WebsocketClientInterface{

		public:
			WebsocketClientInterface(remi_socket clientSock, bool doHandshake);
			void* _run();

			void stop();

			void handshake();

			void on_message(const char* message, unsigned long long len);

			void send_message( std::string message);

			bool readNextMessage();

			static std::string packUpdateMessage(std::string tagToUpdateIdentifier, std::string htmlContent);

		private:

			Dictionary<Buffer*>*		parseParams(const char* paramString, unsigned long len);

			remi_thread		_t;

			remi_socket		_sock;

			bool	_handshakeDone;

			bool	_stopFlag;

			int		_secondsSinceLastPing;

		};

	}

}

#endif
