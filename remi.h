//
// Created by CyberPro4 on 24/06/2016.
//

#ifndef CPORT_REMI_H
#define CPORT_REMI_H
#include <iostream>
#include <list>
#include <sstream>
#include <map>
#include <codecvt>
#include <string>
#include <locale>
#include <iomanip>
#include <stdio.h>
#include <stdarg.h>

//#include <thread>       //std::this_thread::sleep_for std::thread
#include <chrono>
#include <typeinfo>

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

#define HAVE_STRUCT_TIMESPEC // ?? TODO: WTF

#ifdef WIN32

    #include <Windows.h>

    #define     remi_socket               SOCKET
    #define     remi_socket_len           int


	#define     remi_socket_setaddr( sck , adr )      sck.S_un.S_addr = adr
	#define     remi_socket_addr( sck )               sck.S_un.S_addr

    #define     remi_thread_result        DWORD
    #define     remi_thread_callback      LPTHREAD_START_ROUTINE
    #define     remi_thread_param         LPVOID
    #define     remi_thread               HANDLE

	#define _CRT_SECURE_NO_WARNINGS 1

#endif

#if defined(__unix__) || defined(__MACH__)

    #define     SOCKET_ERROR    -1
    #define     SOCKADDR        sockaddr
    #define     SOCKADDR_IN     sockaddr_in
    #define     INVALID_SOCKET  -1
    #define     remi_socket       int
    #define     remi_socket_len   socklen_t
    #define     remi_socket_setaddr( sck , adr )      sck.s_addr = adr
    #define     remi_socket_addr( sck )               sck.s_addr

    #define     remi_thread               pthread_t
    #define     remi_thread_result        void*
    #define     remi_thread_param         void*
    typedef     void* (*remi_thread_callback)( remi_thread_param param );


    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <stdio.h>
    #include <string.h>

    #define     Sleep(a)    usleep( a * 1000 )

#endif

#define CLASS_NAME(c) #c

long long int	remi_timestamp();

remi_thread		remi_createThread( remi_thread_callback callback , remi_thread_param param );

namespace remi {


    namespace utils {

        std::string toPix( int v );

        std::string join(std::list<std::string> stringList , std::string glue );

		std::string string_encode(std::string text);

		std::string escape_json(const std::string &s);

		std::string sformat( std::string format , ... );

		int sscan( std::string from , std::string format , ... );

		std::list<std::string> split( std::string subject , std::string delimiter );

		unsigned long long searchIndexOf(const char* buffer, char __char, unsigned long long len, unsigned long long start);

		//std::list<char*> splitCharPointer(const char* buf, char splitChar, unsigned long long bufLen, int maxSplit);

		std::string SHA1(std::string&);

		void url_decode(const char* from, unsigned long long len, char*& converted, unsigned long long* lenConverted);

		void open_browser( std::string url );


		template< class org >
		org list_at( std::list<org> list , int index ){
			int c = 0;
            org fitem;
			for( org item : list ){
				if( c == index )
					fitem = item;
				c++;
			}

            return fitem;
		}

		class TimerListener {
		public:
			virtual void onTimer() = 0;
		};

        //Timer utility. Calls a function (in a parallel std::thread) at a predefined 'millisecondsInterval' until stopFlag turns True.
        class Timer {
        public:

            Timer( int millisecondsInterval , TimerListener* listener = NULL );
			Timer();

            void stop();

			void start();

			void setInterval( int millisecondsInterval );

			void setListener( TimerListener* listener );

			bool has_passed();

			void tick();

			double elapsed();

        private:

            remi_thread     _t;

			TimerListener*	_listener;

            int             _millisecondsInterval;

            bool            _stopFlag;

			bool			_passed;

			typedef std::chrono::high_resolution_clock clock_;
			typedef std::chrono::duration<double, std::ratio<1> > second_;

			std::chrono::time_point<clock_>		_start;

			static remi_thread_result thread_entry( remi_thread_param p );
        };
    };

    template<class T> class DictionaryValue {
    public:

		DictionaryValue( std::string name ){
			this->name = name;
		}

        DictionaryValue( std::string name , T value ){
            this->name = name;
            this->value = value;
        }

		void operator = ( T v ){
			value = v;
		}

		operator T (){
			return value;
		}

		operator const T () const {
			return (const T )value;
		}

        std::string     name;
        T  value;
    };

    template<class T> class Dictionary {

    public:

        Dictionary(){
        }

		Dictionary( Dictionary<T> & d ){
			for( DictionaryValue<T>* value : d._library ){
				_library.push_front( new DictionaryValue<T>( value->name , value->value ) );
			}
		}

        bool has( std::string name ){
            DictionaryValue<T>* value = this->getDictionaryValue( name );
            if( value != NULL )
                return true;
            return false;
        }

        long size(){
            return this->_library.size();
        }

        std::list<std::string> keys(){
            std::list<std::string> k;
            if(_library.size()>0){
                for( DictionaryValue<T>* currentAttribute : _library ){
                    k.push_front( currentAttribute->name );
                }
            }

            return k;
        }

		DictionaryValue<T> & operator [] ( std::string name ) {
			DictionaryValue<T>* objectAttribute = this->getDictionaryValue( name );
			if( objectAttribute == NULL ){
				objectAttribute = new DictionaryValue<T>( name );
				_library.push_front( objectAttribute );
			}
			return (*objectAttribute);
		}

		void operator = ( const Dictionary<T> & d ){
			clear();
			for( DictionaryValue<T>* value : d._library ){
				_library.push_front( new DictionaryValue<T>( value->name , value->value ) );
			}
		}

		void update( const Dictionary<T>& d ) {
			for (DictionaryValue<T>* dictionaryValue : d._library) {
				this->set(dictionaryValue->name, dictionaryValue->value);
			}

		}

        const T get( std::string name ) const {
            DictionaryValue<T>* objectAttribute = this->getDictionaryValue( name );

            /*if( objectAttribute != NULL )
                return objectAttribute->value;*/

            return objectAttribute->value;
        }

        void remove( std::string name ){
            DictionaryValue<T>* dictionaryValue = this->getDictionaryValue( name );
            if( dictionaryValue != NULL ) {
                _library.remove( dictionaryValue );
                delete dictionaryValue;
            }
        }

        void set( std::string name, T value ){
            DictionaryValue<T>* dictionaryValue = this->getDictionaryValue(name );
            if( dictionaryValue == NULL ) {
                DictionaryValue<T> *_value = new DictionaryValue<T>(name, value);
                _library.push_front(_value);
            } else {
                dictionaryValue->value = value;
            }
        }

        void clear(){
            for( DictionaryValue<T>* currentAttribute : _library ){
				delete currentAttribute;
            }

            _library.clear();
        }

		~Dictionary(){
			clear();
		}

    private:

        DictionaryValue<T>* getDictionaryValue( std::string name ) const {
            if( this->_library.size() == 0 )
                return NULL;


            for( DictionaryValue<T>* dictionaryValue : _library ){
                if( dictionaryValue->name.compare( name ) == 0 ){
                    return dictionaryValue;
                }
            }

            return NULL;
        }

        std::list<DictionaryValue<T>*> _library;

    };

    namespace utils {

        std::string join( Dictionary<std::string>& from , std::string nameValueGlue , std::string itemsGlue );

        std::string toCss( Dictionary<std::string>& values );

    };


    class Buffer{
        public:
            char* data; unsigned long long len;
        public:
            Buffer(char* _data, unsigned long long _len){
                data = _data; len = _len;
            }
            ~Buffer(){
                delete data;
            }
            std::string str(){
                std::string _s; _s.assign(data, len);
                return _s;
            }
    };

    class Event;


    class EventSource{
        public:
            Dictionary<Event*> event_handlers;
    };

    class EventListener{
        public:
            typedef void (EventListener::*listener_type)(EventSource*, Dictionary<Buffer*>*, void* );
    };


    class Event{
        public:
            const char* _eventName; //this is used for comparison, when a js event occurs and have to be dispatched to widgets

            EventSource* _eventSource; //the event owner, that calls the listener

        public:
            typedef void (*listener_type)(EventSource*, Dictionary<Buffer*>*, void* );
            listener_type _listener;
            EventListener::listener_type _listener_member;
            EventListener* _listener_instance;

            void* _userData;

        public:
            Event(EventSource* eventSource, const char* eventName):_eventSource(eventSource),_eventName(eventName){
                _listener = NULL;
                _listener_member = NULL;
                _listener_instance = NULL;
            };

            //event registration in explicit form myevent._do(listener, userData);
            void _do(EventListener* instance, EventListener::listener_type listener, void* userData=0){
                this->_listener_instance = instance;
                this->_listener_member = listener;
                this->_userData = userData;
            }

            void _do(listener_type listener, void* userData=0){
                this->_listener = listener;
                this->_userData = userData;
            }

            //event registration in stream form myevent >> listener >> userData;;
            Event& operator>> (listener_type listener){
                this->_listener = listener;
                return *this;
            }
            Event& operator>> (EventListener* instance){
                this->_listener_instance = instance;
                return *this;
            }
            Event& operator>> (EventListener::listener_type listener){
                this->_listener_member = listener;
                return *this;
            }
            Event& operator>> (void* userData){
                this->_userData = userData;
                return *this;
            }

            void operator()(EventSource* eventSource, Dictionary<Buffer*>* params){
                if(this->_listener!=NULL){
                    this->_listener(eventSource, params, this->_userData);
                    return;
                }
                if(this->_listener_member!=NULL){
                    //(*this->_listener_instance).*(this->_listener_member)(eventSource, params, this->_userData);
                    //invoke(this->_listener_member, this->_listener_instance, eventSource, params, this->_userData);
                    CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(eventSource, params, this->_userData);
                }
            }

            virtual void operator()(Dictionary<Buffer*>*)=0;
    };


    template<class T> class VersionedDictionary : public Dictionary<T>, public EventSource {
    public:

        VersionedDictionary(){
            this->_version = 0;
            this->event_onchange = new onChange(this);
        }

        void set( std::string name , T value, int version_increment = 1 ){
            /*if( has( name ) ){
                if( getAttribute( name ).compare( value ) != 0 )
                    _version += version_increment;
            } else
                _version += version_increment;*/
            _version += version_increment;

            Dictionary<T>::set(name, value);
            this->event_onchange->operator()(NULL);
        }

        void clear( int version_increment = 1 ){
            _version += version_increment;
            Dictionary<T>::clear();
            this->event_onchange->operator()(NULL);
        }

        long getVersion(){
            return _version;
        }

		void alignVersion(){
			_lastVersion = _version;
		}

		bool isChanged(){
			return _version != _lastVersion;
		}

        class onChange:public Event{
            friend class Widget;
            public:
                //int status = 0;
                //evt(void* emitter):Event::Event(emitter, abi::__cxa_demangle(typeid(this).name(),0,0,&status)){
                onChange(EventSource* eventSource):Event::Event(eventSource, CLASS_NAME(onChange)){
                    eventSource->event_handlers.set(this->_eventName, this);
                }
                void operator()(Dictionary<Buffer*>* parameters=NULL){
                    Event::operator()(_eventSource, parameters);
                }
        }* event_onchange;
    private:

        long _version;
		long _lastVersion;
    };


    class Represantable {
    public:
        virtual std::string repr(Dictionary<Represantable*>* changedWidgets) = 0;
    };


    class StringRepresantable : public Represantable {
    public:

        StringRepresantable(std::string v );

        std::string repr(Dictionary<Represantable*>* changedWidgets = NULL);

    private:

        std::string v;
    };


    class Tag : public Represantable, public EventSource, public EventListener  {

    public:

        Tag();

        Tag(VersionedDictionary<std::string> attributes, std::string _type, std::string _class="");

        void addClass( std::string name );

        void removeClass( std::string name );

        std::string getIdentifier();

		void setIdentifier( std::string newIdentifier );

		virtual std::string innerHTML( Dictionary<Represantable*>* localChangedWidgets );

        std::string repr(Dictionary<Represantable*>* changedWidgets);

        void addChild( Represantable* child, std::string key = "" );

		void addChild( std::string child, std::string key = "" );

        Represantable * getChild(std::string key );

		void setUpdated();

		bool isChanged(){
			return attributes.isChanged() || style.isChanged() || children.isChanged();
		}

		void _notifyParentForUpdate();

		//EventListener::listener_type _needUpdate;
		void _needUpdate(Tag* emitter, Dictionary<Buffer*>* params, void* userdata);

    public:

        std::list<std::string>              _classes;

        VersionedDictionary<std::string>    attributes;
        VersionedDictionary<std::string>    style;

        VersionedDictionary<Represantable *>       children;

        std::string     type;

		Tag* _parent;

		bool ignoreUpdate;

    private:

        std::list<Represantable *>  _render_children_list;
		std::ostringstream          _backupRepr;
		std::ostringstream          _reprAttributes;
    };



	namespace server {
		class App;
	}


    class Widget : public Tag {
	public:
        class onclick:public Event{
            friend class Widget;
            public:
                //int status = 0;
                //evt(void* emitter):Event::Event(emitter, abi::__cxa_demangle(typeid(this).name(),0,0,&status)){
                onclick(Widget* emitter):Event::Event(emitter, CLASS_NAME(onclick)){
                    ((Widget*)emitter)->event_handlers.set(this->_eventName, this);
                    emitter->attributes["onclick"] = utils::sformat( "sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), this->_eventName);
                }
                void operator()(Dictionary<Buffer*>* parameters=NULL){
                    Event::operator()(_eventSource, parameters);
                }
        }* event_onclick;


    public:

        enum Layout {
            Horizontal = 1,
            Vertical = 0
        };


        Widget();

        Widget( std::string type );

		void setWidth( int width );
		void setHeight( int height );
        void setSize( int width, int height );

        void setLayoutOrientation( Widget::Layout orientation );

        void addChild( Represantable* child, std::string key = "" );

		void setParent( Tag* tag ){
            this->_parent = tag;
		}

		std::string append(Widget* w, std::string key=std::string(""));

    private:

        void defaults();

        Widget::Layout              _layout_orientation;

    };


    class HTML:public Tag{
        public:
            HTML(){
                type = "html";
            }

            std::string repr(Dictionary<Represantable*>* changed_widgets){
                /*It is used to automatically represent the object to HTML format
                packs all the attributes, children and so on.

                Args:
                    changed_widgets (dict): A dictionary containing a collection of tags that have to be updated.
                        The tag that have to be updated is the key, and the value is its textual repr.
                */

                Dictionary<Represantable*>* local_changed_widgets = new Dictionary<Represantable*>();

                std::ostringstream  result;

                result << "<" << type << ">\n" << this->innerHTML(local_changed_widgets) << "\n</" << type << ">";
                this->setUpdated();
                //delete changed_widgets;
                std::cout << "HTML content:" << result.str()<<std::endl;
                delete local_changed_widgets;
                return result.str();
            }
    };


    class HEAD:public Tag{
        public:
            HEAD(std::string title){
                type = "head";
                this->addChild("<meta content='text/html;charset=utf-8' http-equiv='Content-Type'> \
                        <meta content='utf-8' http-equiv='encoding'> \
                        <meta name='viewport' content='width=device-width, initial-scale=1.0'>", "meta");

                this->setTitle(title);
            }

            void setIconFile(std::string filename, std::string rel="icon", std::string mimetype="image/png"){
                /* Allows to define an icon for the App

                    Args:
                        filename (str): the resource file name (ie. "/res:myicon.png")
                        rel (str): leave it unchanged (standard "icon")
                */
                //mimetype, encoding = mimetypes.guess_type(filename)
                this->addChild("favicon", utils::sformat("<link rel='%s' href='%s' type='%s' />", rel.c_str(), filename.c_str(), mimetype.c_str()));
            }

            void setIconData(std::string base64_data, std::string mimetype="image/png", std::string rel="icon"){
                /* Allows to define an icon for the App

                    Args:
                        base64_data (str): base64 encoded image data  (ie. "data:image/x-icon;base64,AAABAAEAEBA....")
                        mimetype (str): mimetype of the image ("image/png" or "image/x-icon"...)
                        rel (str): leave it unchanged (standard "icon")
                */
                this->addChild("favicon", utils::sformat("<link rel='%s' href='%s' type='%s' />", rel.c_str(), base64_data.c_str(), mimetype.c_str()));
            }

            void setInternalJs(std::string app_identifier, std::string net_interface_ip, unsigned short pending_messages_queue_length, unsigned short websocket_timeout_timer_ms){
                /* NOTE here are used raw string literals
                    R"( ... )"
                */
                this->addChild(utils::sformat(R"(
                        <script>
                        /*'use strict';*/

                        var Remi = function() {
                        this._pendingSendMessages = [];
                        this._ws = null;
                        this._comTimeout = null;
                        this._failedConnections = 0;
                        this._openSocket();
                        };

                        // from http://stackoverflow.com/questions/5515869/string-length-in-bytes-in-javascript
                        // using UTF8 strings I noticed that the javascript .length of a string returned less
                        // characters than they actually were
                        Remi.prototype._byteLength = function(str) {
                            // returns the byte length of an utf8 string
                            var s = str.length;
                            for (var i=str.length-1; i>=0; i--) {
                                var code = str.charCodeAt(i);
                                if (code > 0x7f && code <= 0x7ff) s++;
                                else if (code > 0x7ff && code <= 0xffff) s+=2;
                                if (code >= 0xDC00 && code <= 0xDFFF) i--; //trail surrogate
                            }
                            return s;
                        };

                        Remi.prototype._paramPacketize = function (ps){
                            var ret = '';
                            for (var pkey in ps) {
                                if( ret.length>0 )ret = ret + '|';
                                var pstring = pkey+'='+ps[pkey];
                                var pstring_length = this._byteLength(pstring);
                                pstring = pstring_length+'|'+pstring;
                                ret = ret + pstring;
                            }
                            return ret;
                        };

                        Remi.prototype._openSocket = function(){
                            var ws_wss = "ws";
                            try{
                                ws_wss = document.location.protocol.startsWith('https')?'wss':'ws';
                            }catch(ex){}

                            var self = this;
                            try{
                                this._ws = new WebSocket(ws_wss + '://%s/');
                                console.debug('opening websocket');

                                this._ws.onopen = function(evt){
                                    if(self._ws.readyState == 1){
                                        self._ws.send('connected');

                                        try {
                                            document.getElementById("loading").style.display = 'none';
                                        } catch(err) {
                                            console.log('Error hiding loading overlay ' + err.message);
                                        }

                                        self._failedConnections = 0;

                                        while(self._pendingSendMessages.length>0){
                                            self._ws.send(self._pendingSendMessages.shift()); /*without checking ack*/
                                        }
                                    }
                                    else{
                                        console.debug('onopen fired but the socket readyState was not 1');
                                    }
                                };

                                this._ws.onmessage = function(evt){
                                    var received_msg = evt.data;

                                    if( received_msg[0]=='0' ){ /*show_window*/
                                        var index = received_msg.indexOf(',')+1;
                                        /*var idRootNodeWidget = received_msg.substr(0,index-1);*/
                                        var content = received_msg.substr(index,received_msg.length-index);

                                        document.body.innerHTML = decodeURIComponent(content);
                                    }else if( received_msg[0]=='1' ){ /*update_widget*/
                                        var focusedElement=-1;
                                        var caretStart=-1;
                                        var caretEnd=-1;
                                        if (document.activeElement)
                                        {
                                            focusedElement = document.activeElement.id;
                                            try{
                                                caretStart = document.activeElement.selectionStart;
                                                caretEnd = document.activeElement.selectionEnd;
                                            }catch(e){console.debug(e.message);}
                                        }
                                        var index = received_msg.indexOf(',')+1;
                                        var idElem = received_msg.substr(1,index-2);
                                        var content = received_msg.substr(index,received_msg.length-index);

                                        var elem = document.getElementById(idElem);
                                        try{
                                            elem.insertAdjacentHTML('afterend',decodeURIComponent(content));
                                            elem.parentElement.removeChild(elem);
                                        }catch(e){
                                            /*Microsoft EDGE doesn't support insertAdjacentHTML for SVGElement*/
                                            var ns = document.createElementNS("http://www.w3.org/2000/svg",'tmp');
                                            ns.innerHTML = decodeURIComponent(content);
                                            elem.parentElement.replaceChild(ns.firstChild, elem);
                                            console.debug(e.message);
                                        }

                                        var elemToFocus = document.getElementById(focusedElement);
                                        if( elemToFocus != null ){
                                            elemToFocus.focus();
                                            try{
                                                elemToFocus = document.getElementById(focusedElement);
                                                if(caretStart>-1 && caretEnd>-1) elemToFocus.setSelectionRange(caretStart, caretEnd);
                                            }catch(e){console.debug(e.message);}
                                        }
                                    }else if( received_msg[0]=='2' ){ /*javascript*/
                                        var content = received_msg.substr(1,received_msg.length-1);
                                        try{
                                            eval(content);
                                        }catch(e){console.debug(e.message);};
                                    }else if( received_msg[0]=='3' ){ /*ack*/
                                        self._pendingSendMessages.shift() /*remove the oldest*/
                                        if(self._comTimeout!==null)
                                            clearTimeout(self._comTimeout);
                                    }
                                };

                                this._ws.onclose = function(evt){
                                    /* websocket is closed. */
                                    console.debug('Connection is closed... event code: ' + evt.code + ', reason: ' + evt.reason);
                                    // Some explanation on this error: http://stackoverflow.com/questions/19304157/getting-the-reason-why-websockets-closed
                                    // In practice, on a unstable network (wifi with a lot of traffic for example) this error appears
                                    // Got it with Chrome saying:
                                    // WebSocket connection to 'ws://x.x.x.x:y/' failed: Could not decode a text frame as UTF-8.
                                    // WebSocket connection to 'ws://x.x.x.x:y/' failed: Invalid frame header

                                    try {
                                        document.getElementById("loading").style.display = '';
                                    } catch(err) {
                                        console.log('Error hiding loading overlay ' + err.message);
                                    }

                                    self._failedConnections += 1;

                                    console.debug('failed connections=' + self._failedConnections + ' queued messages=' + self._pendingSendMessages.length);

                                    if(self._failedConnections > 3) {

                                        // check if the server has been restarted - which would give it a new websocket address,
                                        // new state, and require a reload
                                        console.debug('Checking if GUI still up ' + location.href);

                                        var http = new XMLHttpRequest();
                                        http.open('HEAD', location.href);
                                        http.onreadystatechange = function() {
                                            if (http.status == 200) {
                                                // server is up but has a new websocket address, reload
                                                location.reload();
                                            }
                                        };
                                        http.send();

                                        self._failedConnections = 0;
                                    }

                                    if(evt.code == 1006){
                                        self._renewConnection();
                                    }
                                };

                                this._ws.onerror = function(evt){
                                    /* websocket is closed. */
                                    /* alert('Websocket error...');*/
                                    console.debug('Websocket error... event code: ' + evt.code + ', reason: ' + evt.reason);
                                };

                            }catch(ex){this._ws=false;alert('websocketnot supported or server unreachable');}
                        }


                        /*this uses websockets*/
                        Remi.prototype.sendCallbackParam = function (widgetID,functionName,params /*a dictionary of name:value*/){
                            var paramStr = '';
                            if(params!==null) paramStr=this._paramPacketize(params);
                            var message = encodeURIComponent(unescape('callback' + '/' + widgetID+'/'+functionName + '/' + paramStr));
                            this._pendingSendMessages.push(message);
                            if( this._pendingSendMessages.length < %d ){
                                if (this._ws !== null && this._ws.readyState == 1)
                                    this._ws.send(message);
                                    if(this._comTimeout===null)
                                        this._comTimeout = setTimeout(this._checkTimeout, %d);
                            }else{
                                console.debug('Renewing connection, this._ws.readyState when trying to send was: ' + this._ws.readyState)
                                this._renewConnection();
                            }
                        };

                        /*this uses websockets*/
                        Remi.prototype.sendCallback = function (widgetID,functionName){
                            this.sendCallbackParam(widgetID,functionName,null);
                        };

                        Remi.prototype._renewConnection = function(){
                            // ws.readyState:
                            //A value of 0 indicates that the connection has not yet been established.
                            //A value of 1 indicates that the connection is established and communication is possible.
                            //A value of 2 indicates that the connection is going through the closing handshake.
                            //A value of 3 indicates that the connection has been closed or could not be opened.
                            if( this._ws.readyState == 1){
                                try{
                                    this._ws.close();
                                }catch(err){};
                            }
                            else if(this._ws.readyState == 0){
                            // Don't do anything, just wait for the connection to be stablished
                            }
                            else{
                                this._openSocket();
                            }
                        };

                        Remi.prototype._checkTimeout = function(){
                            if(this._pendingSendMessages.length > 0)
                                this._renewConnection();
                        };

                        Remi.prototype.uploadFile = function(widgetID, eventSuccess, eventFail, eventData, file){
                            var url = '/';
                            var xhr = new XMLHttpRequest();
                            var fd = new FormData();
                            xhr.open('POST', url, true);
                            xhr.setRequestHeader('filename', file.name);
                            xhr.setRequestHeader('listener', widgetID);
                            xhr.setRequestHeader('listener_function', eventData);
                            xhr.onreadystatechange = function() {
                                if (xhr.readyState == 4 && xhr.status == 200) {
                                    /* Every thing ok, file uploaded */
                                    var params={};params['filename']=file.name;
                                    remi.sendCallbackParam(widgetID, eventSuccess,params);
                                    console.log('upload success: ' + file.name);
                                }else if(xhr.status == 400){
                                    var params={};params['filename']=file.name;
                                    remi.sendCallbackParam(widgetID,eventFail,params);
                                    console.log('upload failed: ' + file.name);
                                }
                            };
                            fd.append('upload_file', file);
                            xhr.send(fd);
                        };

                        window.onerror = function(message, source, lineno, colno, error) {
                            var params={};params['message']=message;
                            params['source']=source;
                            params['lineno']=lineno;
                            params['colno']=colno;
                            params['error']=JSON.stringify(error);
                            remi.sendCallbackParam('%s','%s',params);
                            return false;
                        };

                        window.remi = new Remi();

                        </script>)", net_interface_ip.c_str(), pending_messages_queue_length, websocket_timeout_timer_ms, app_identifier.c_str(), "onerror"), "internal_js");
            }

            void setTitle(std::string title){
                this->addChild(utils::sformat("<title>%s</title>", title.c_str()), "title");
            }

            std::string repr(Dictionary<Represantable*>* changed_widgets){
                /*It is used to automatically represent the object to HTML format
                packs all the attributes, children and so on.

                Args:
                    changed_widgets (dict): A dictionary containing a collection of tags that have to be updated.
                        The tag that have to be updated is the key, and the value is its textual repr.
                */

                Dictionary<Represantable*>* local_changed_widgets = new Dictionary<Represantable*>();

                std::ostringstream  result;

                result << "<" << type << ">\n" << this->innerHTML(local_changed_widgets) << "\n</" << type << ">";
                this->setUpdated();
                //delete changed_widgets;
                delete local_changed_widgets;
                return result.str();
            }
    };


    class BODY:public Widget{
        public:
            const char* EVENT_ONLOAD = "onload";
            const char* EVENT_ONERROR = "onerror";
            const char* EVENT_ONONLINE = "ononline";
            const char* EVENT_ONPAGEHIDE = "onpagehide";
            const char* EVENT_ONPAGESHOW = "onpageshow";
            const char* EVENT_ONRESIZE = "onresize";

            BODY(){
                type = "body";
                Widget* loading_anim = new Widget();
                loading_anim->style.remove("margin");
                loading_anim->setIdentifier("loading-animation");

                Widget* loading_container = new Widget();
                loading_container->append(loading_anim, "loading_animation");
                loading_container->style.set("display", "none");
                loading_container->style.remove("margin");
                loading_container->setIdentifier("loading");

                this->append(loading_container, "loading_container");
            }
            /*
            @decorate_set_on_listener("(self, emitter)")
            @decorate_event_js("""remi.sendCallback('%(emitter_identifier)s','%(event_name)s');""")
            def onload(self):
                """Called when page gets loaded."""
                return ()

            @decorate_set_on_listener("(self, emitter)")
            @decorate_event_js("""remi.sendCallback('%(emitter_identifier)s','%(event_name)s');""")
            def ononline(self):
                return ()

            @decorate_set_on_listener("(self, emitter)")
            @decorate_event_js("""remi.sendCallback('%(emitter_identifier)s','%(event_name)s');""")
            def onpagehide(self):
                return ()

            @decorate_set_on_listener("(self, emitter)")
            @decorate_event_js("""
                    var params={};
                    params['width']=window.innerWidth;
                    params['height']=window.innerHeight;
                    remi.sendCallbackParam('%(emitter_identifier)s','%(event_name)s',params);""")
            def onpageshow(self, width, height):
                return (width, height)

            @decorate_set_on_listener("(self, emitter)")
            @decorate_event_js("""
                    var params={};
                    params['width']=window.innerWidth;
                    params['height']=window.innerHeight;
                    remi.sendCallbackParam('%(emitter_identifier)s','%(event_name)s',params);""")
            def onresize(self, width, height):
                return (width, height)
            */
    };


	class HBox : public Widget {

	public:

		HBox();

	};

	class VBox : public Widget {

	public:

		VBox();

	};

	class TextWidget : public Widget {

	public:

		void setText( std::string text );

		std::string text();

	};

	class Button : public TextWidget {
	public:

		Button( std::string text );

		void setEnabled( bool en );
		bool enabled();

	};


	class Label : public TextWidget {

	public:

		Label( std::string text = "" );

	};


	class GenericDialog : public Widget {
	public:
		class GenericDialogOnConfirmListener{ public: virtual void onConfirm(GenericDialog*) = 0; };
		class GenericDialogOnCancelListener{ public: virtual void onCancel(GenericDialog*) = 0; };

		GenericDialogOnConfirmListener* onConfirmListener;
		GenericDialogOnCancelListener* onCancelListener;
	public:

		GenericDialog( std::string title = "" , std::string message = "" );

		void addFieldWithLabel(std::string key, std::string label_description, Widget* field);

		void addField(std::string key, Widget* field);

		Widget* getField(std::string key);

		void onClick(Widget*);

	private:

		Widget*		_container;
		Button*		_confirmButton;
		Button*		_cancelButton;
		Widget*		_hLay;
		std::map<std::string, Widget*> _inputs;
	};


	class Image : public Widget{
	public:

		Image(std::string url);

		void setURL(std::string);

		std::string url();

	};


	class Input : public Widget {

	public:

		Input();

		void setValue( std::string value );
		std::string getValue();

		void setEnable( bool on );
		bool isEnable();

		void setReadOnly( bool on );
		bool isReadOnly();
	};

	class FileUploader : public Widget {
	public:
		static const std::string Event_OnSuccess;
		static const std::string Event_OnFail;
		static const std::string Event_OnData;
		//static const std::string Event_OnProgress;

		class FileUploaderOnSuccessListener{ public: virtual void onSuccess(FileUploader*) = 0; };
		FileUploaderOnSuccessListener* onSuccessListener;
		class FileUploaderOnFailListener{ public: virtual void onFail(FileUploader*) = 0; };
		FileUploaderOnFailListener* onFailListener;
		class FileUploaderOnDataListener{ public: virtual void onData( FileUploader*, std::string fileName, const char* data, unsigned long long len ) = 0; };
		FileUploaderOnDataListener* onDataListener;

	public:
		FileUploader( std::string path = "./", bool multipleSelectionAllowed = true );

		void setSavePath( std::string path );
		std::string savePath();

		void setMultipleSelectionAllowed( bool value );
		bool multipleSelectionAllowed();

		//virtual void onEvent(std::string name, Event* event);

	private:
		std::string _path;
		bool _multipleSelectionAllowed;

	};
}

#endif //CPORT_REMI_H
