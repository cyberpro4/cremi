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


    class Tag;
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


    class EventJavascript:public Event{
        public:
            std::string _javascriptCode;
            bool        _preventDefault;
            bool        _stopPropagation;

        public:
            EventJavascript(EventSource* eventSource, const char* eventName,
                                     std::string javascriptCode,
                                     bool preventDefault, bool stopPropagation):Event::Event(eventSource,eventName){
                _javascriptCode     = javascriptCode;
                _preventDefault     = preventDefault;
                _stopPropagation    = stopPropagation;
                //static_cast<Tag*>(eventSource)->attributes[eventName] = _javascriptCode;
            };
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
        virtual std::string repr() = 0;
    };


    class StringRepresantable : public Represantable {
    public:

        StringRepresantable(std::string v );

        std::string repr();

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

		std::string innerHTML( Dictionary<Represantable*>* localChangedWidgets );

		std::string repr();

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

        std::list<Represantable *>              _render_children_list;
		std::ostringstream _backupRepr;
		std::ostringstream _reprAttributes;
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

        void redraw();

        void addChild( Represantable* child, std::string key = "" );

		void hide();

		void show( server::App* app );

		void setParentApp( server::App* app );

    private:

        void defaults();

        Widget::Layout              _layout_orientation;

		server::App*			    _parentApp;

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
