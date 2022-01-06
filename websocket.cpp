#include "websocket.h"

#include <string>
#include <stdio.h>

#include <regex>

using namespace remi::server;
using namespace remi;

remi_thread_result WebsocketClientInterface_threadEntry(remi_thread_param param) {
	((WebsocketClientInterface*)param)->_run();
	return 0;
}

WebsocketClientInterface::WebsocketClientInterface(remi_socket clientSock, bool doHandshake = true) {
	_sock = clientSock;
	/*Since the socket already exists ( it comes from microhttpd ) we cannot set WSA_FLAG_OVERLAPPED
		and so we cannot set a timeout. *select* function will be used instead
		This occurs with WSAsockets*/
		/*
		DWORD timeout = 3000;
		setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		setsockopt(_sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
		*/
	_stopFlag = false;
	_handshakeDone = !doHandshake;

	_t = remi_createThread((remi_thread_callback)& WebsocketClientInterface_threadEntry, this);

	_secondsSinceLastPing = remi_timestamp();
}

void WebsocketClientInterface::stop() {
	_stopFlag = true;
}

void* WebsocketClientInterface::_run() {
	//here the recv send is managed
	//  at first glance, the handshake have to be performed
	//  after that, new messages an managed in on_message
	while (!_stopFlag) {

		/*char buffer[64] = {0};

		int recv_bytes = recv( _sock , buffer , 64 , 0 );

		if( recv_bytes < 1 )
			continue;

		if( recv_bytes == SOCKET_ERROR )
			break;

		std::cout << buffer;*/

		if (!_handshakeDone) {
			handshake();
		}
		else {
			if (!readNextMessage()) {
				_handshakeDone = false;
				std::cout << "ws: read next message has failed";
				_stopFlag = true;
			}
		}
	}

	return NULL;
}

int waitForSocketDataAvailable(int sockHandle, int timeout) {
	fd_set fds;
	struct timeval tv;
	// Set up the file descriptor set.
	FD_ZERO(&fds);
	FD_SET(sockHandle, &fds);
	// Set up the struct timeval for the timeout.
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	// Wait until timeout or data received.
	return select(sockHandle, &fds, NULL, NULL, &tv);
}

bool WebsocketClientInterface::readNextMessage() {
	bool fin = false;
	unsigned char opcode = 0; //opcode==0 indicates continuationFrame
	std::list<char*> chunks;
	std::list<unsigned long long> sizes;
	unsigned long long entireMsgLen = 0;
	bool skip = false; //this is used to store that the algorithm is interrupted and the memory have to be released
	while (opcode == 0) {
		char length_buf[8] = { 0 };

		if (waitForSocketDataAvailable(_sock, 3) == 0) {
			continue;
		}

		if (recv(_sock, length_buf, 2, 0) != 2) {
			std::cout << "recv failed: " << "first two bytes not recv" << endl;
			_stopFlag = true;
			skip = true;
			break;
			//return false;
		}
		fin = length_buf[0] & 1;
		opcode = (length_buf[0] >> 4) & 0xf;

		unsigned long long payload_len = 0;
		unsigned int _l = length_buf[1] & 127;
		payload_len = _l;

		bool mask_enable = (length_buf[1] & (1 << 7)) != 0;
		//std::cout << "recv: " << "payload_len" << payload_len << endl;
		// manage payload len 126 / 127
		if (payload_len == 126) {
			if (recv(_sock, length_buf, 2, 0) != 2) {
				std::cout << "recv failed: " << "length_buf, 2" << endl;
				skip = true;
				break;
				//return false;
			}

			payload_len = 0;
			int _i = 0;
			payload_len += (unsigned char)length_buf[_i++]; payload_len = (payload_len << 8);
			payload_len += (unsigned char)length_buf[_i++];
			//std::cout << "payload 127: " << payload_len << "      length_buf: " << length_buf[_i++] << " " << length_buf[_i++] << endl;

		}
		else if (payload_len == 127) {
			if (recv(_sock, length_buf, 8, 0) != 8) {
				std::cout << "recv failed: " << "length_buf, 8" << endl;
				skip = true;
				break;
				//return false;
			}

			payload_len = 0;
			int _i = 0;
			payload_len += (unsigned char)length_buf[_i++]; payload_len = (payload_len << 8);
			payload_len += (unsigned char)length_buf[_i++]; payload_len = (payload_len << 8);
			payload_len += (unsigned char)length_buf[_i++]; payload_len = (payload_len << 8);
			payload_len += (unsigned char)length_buf[_i++]; payload_len = (payload_len << 8);
			payload_len += (unsigned char)length_buf[_i++]; payload_len = (payload_len << 8);
			payload_len += (unsigned char)length_buf[_i++]; payload_len = (payload_len << 8);
			payload_len += (unsigned char)length_buf[_i++]; payload_len = (payload_len << 8);
			payload_len += (unsigned char)length_buf[_i++];
			_i = 0;
			/*std::cout << "payload 127: " << payload_len << "      length_buf: " <<
				length_buf[_i++] << " "
				<< length_buf[_i++] << " "
				<< length_buf[_i++] << " "
				<< length_buf[_i++] << " "
				<< length_buf[_i++] << " "
				<< length_buf[_i++] << " "
				<< length_buf[_i++] << " "
				<< length_buf[_i++] << endl;*/
		}

		char mask[4];
		//std::cout << "recv: " << "mask enb:" << mask_enable << endl;
		if (mask_enable) {
			if (recv(_sock, mask, 4, 0) != 4) {
				std::cout << "recv failed: " << "mask recv not received" << endl;
				skip = true;
				break;
				//return false;
			}
		}
		//cout << "payload: " << payload_len << endl;
		char* buf = new char[payload_len + 1];
		std::memset(buf, 0, payload_len + 1);

		size_t _rv = -1;
		if ((_rv = recv(_sock, buf, payload_len, 0)) != (size_t)payload_len) {
			std::cout << "recv failed: " << "recv size mismatch" << endl;
			delete[] buf;
			skip = true;
			break;
			//return false;
		}

		for (size_t l = 0; l < _rv; l++) {
			buf[l] = buf[l] ^ mask[l % 4];
		}

		chunks.push_back(buf);
		sizes.push_back(_rv);
		entireMsgLen += _rv;
		//delete[] buf; //this is deleted below
	}

	if (skip) {
		//deleting already stored chunks
		for (char* _pointer : chunks) {
			delete[] _pointer;
		}
		return false;
	}

	char* entireMsg = new char[entireMsgLen];
	unsigned long long _offset = 0;
	for (char* _pointer : chunks) {
		memcpy(&entireMsg[_offset], _pointer, utils::list_at(sizes, 0));
		_offset += utils::list_at(sizes, 0);
		sizes.pop_front();
		delete[] _pointer;
	}

	char* convertedMessage;
	unsigned long long convertedMessageLen = 0;
	remi::utils::url_decode(entireMsg, entireMsgLen, convertedMessage, &convertedMessageLen);
	on_message(convertedMessage, convertedMessageLen);

	delete[] entireMsg;
	delete[] convertedMessage;

	return true;
}

void WebsocketClientInterface::handshake() {

	char buffer[1024] = { 0 };

	recv(_sock, buffer, 1024, 0);

	std::string sbuff = buffer;

	std::list<std::string> sec_pieces = utils::split(sbuff, "Sec-WebSocket-Key: ");
	if (sec_pieces.size() != 2)
		return;

	std::list<std::string> pieces = utils::split(utils::list_at(sec_pieces, 1), "\r\n");
	if (pieces.size() < 1)
		return;

	std::string key = utils::list_at(pieces, 0);
	key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

	unsigned char key_sha1[64] = { 0 };

	std::string sha1_1(remi::utils::SHA1(key));

	std::string b64 = base64_encode((unsigned char*)sha1_1.c_str(), sha1_1.length());
	cout << "b64:" << b64.c_str() << endl;
	std::ostringstream response_s;
	response_s
		<< "HTTP/1.1 101 Switching Protocols\r\n"
		<< "Upgrade: websocket\r\n"
		<< "Connection: Upgrade\r\n"
		<< "Sec-WebSocket-Accept: " << b64 << "\r\n\r\n";/* % digest.decode("utf-8");*/
	std::string response = response_s.str();

	send(_sock, response.c_str(), response.length(), 0);

	_handshakeDone = true;
}

Dictionary<Buffer*>* WebsocketClientInterface::parseParams(const char* paramString, unsigned long len) {
	/*std::smatch match;
	std::string out = paramString;*/
	Dictionary<Buffer*>* ret = new Dictionary<Buffer*>();

	unsigned long long __start = 0;
	unsigned long long __pipe = 0;
	unsigned long long __eq = 0;
	unsigned long long dataLen = 0;

	__pipe = utils::searchIndexOf(paramString, '|', len, 0) - 1;
	__eq = utils::searchIndexOf(paramString, '=', len, __pipe) - 1;
	while (__start < __pipe && __pipe < __eq) {
		std::string sdataLen(&paramString[__start], __pipe - __start);
		dataLen = atoll(sdataLen.c_str());
		if (dataLen > 0) {
			unsigned long long fieldNameLen = __eq - __pipe - 1;
			unsigned long long fieldDataLen = dataLen - __eq + __pipe;
			std::string fieldName; fieldName.assign(&paramString[__pipe + 1], fieldNameLen);

			char* fieldData = new char[dataLen - __eq + __pipe];
			memcpy(fieldData, &paramString[__eq + 1], fieldDataLen);

			Buffer* buf = new Buffer(fieldData, fieldDataLen);

			ret->set(fieldName, buf);

		}
		__start = __pipe + 2 + dataLen;
		__pipe = utils::searchIndexOf(paramString, '|', len, __start) - 1;  //paramString.find_first_of("|", __start);
		__eq = utils::searchIndexOf(paramString, '=', len, __pipe) - 1;  //paramString.find_first_of("=", __pipe);
	}

	return ret;
}

#define _MSG_ACK    "3"
#define _MSG_JS     "2"
#define _MSG_UPDATE "1"
void WebsocketClientInterface::on_message(const char* message, unsigned long long len) {

	//std::cout << "ws: " << message;
	send_message(_MSG_ACK);

	int _slashOffset1 = utils::searchIndexOf(message, '/', len, 0);
	int _slashOffset2 = utils::searchIndexOf(message, '/', len, _slashOffset1);
	int _slashOffset3 = utils::searchIndexOf(message, '/', len, _slashOffset2);

	if (_slashOffset3 > _slashOffset2 && _slashOffset2 > _slashOffset1) { // msgtype,widget,function,params

		if (memcmp(message, "callback", _slashOffset1 - 1) == 0) {

			std::string s_widget_id; s_widget_id.assign(&message[_slashOffset1], _slashOffset2 - _slashOffset1 - 1);

			std::string function_name; function_name.assign(&message[_slashOffset2], _slashOffset3 - _slashOffset2 - 1);

			std::cout << "ws: call id = " << s_widget_id << "." << function_name << std::endl;

			int widget_id;
			if (utils::sscan(s_widget_id, "%d", &widget_id) != 1)
				return;



			Widget* widget = (Widget*)((void*)widget_id);

			/*Event* event = new Event( function_name );
			event->source = widget;*/

			Dictionary<Buffer*>* params = NULL;
			if (_slashOffset3 < len) { //so there is a last chunk
				params = parseParams(&message[_slashOffset3], len - _slashOffset3);
			}
			if (widget->event_handlers.has(function_name)) {
				reinterpret_cast<JavascriptEventHandler*>(widget->event_handlers[function_name].value)->handle_websocket_event(params);
			}
			else {
				cout << "WebsocketClientInterface::on_message - ERROR listener function not implemented" << endl;
			}
			if (params != NULL) {
				for (std::string key : params->keys()) {
					delete (params->get(key));
				}
			}
			delete params;
		}

	}

}

void WebsocketClientInterface::send_message(std::string message) {

	size_t message_length = message.length();

	// Prepare the buffer length for the maximum size condition
	// +1 byte for fix 129
	// +1 for length
	// +8 for extreme payload condition
	// buffer_length will be decreased later
	size_t buffer_length = message_length + 1 + 1 + 8;

	unsigned char* buf = new unsigned char[buffer_length];

	unsigned char* lpmsg = &buf[0];

	*lpmsg = 129;
	lpmsg++;

	if (message_length <= 125) {

		*lpmsg = message_length;
		lpmsg++;
		buffer_length -= 8;

	}
	else if (message_length >= 126 && message_length <= 65535) {

		buffer_length -= 6;
		*lpmsg = 126; lpmsg++;
		unsigned short _m = message_length;
		lpmsg[0] = (_m >> 8) & 0xff;
		lpmsg[1] = (_m >> 0) & 0xff;
		lpmsg += 2;

	}
	else {

		*lpmsg = 127; lpmsg++;
		unsigned long long int _m = message_length;
		memcpy(lpmsg, &_m, 8);
		lpmsg += 8;

	}

	memcpy(lpmsg, message.c_str(), message_length);

	send(_sock, (const char*)buf, buffer_length, 0);

	delete[] buf;

}

std::string WebsocketClientInterface::packUpdateMessage(std::string tagToUpdateIdentifier, std::string htmlContent) {
	std::ostringstream output;
	output << _MSG_UPDATE << tagToUpdateIdentifier.c_str() << "," << htmlContent.c_str();
	return output.str();
}

std::string WebsocketClientInterface::packExecuteJavascriptMessage(std::string command) {
	std::ostringstream output;
	output << _MSG_JS << command.c_str();
	return output.str();
}
