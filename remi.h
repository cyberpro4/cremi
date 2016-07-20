//
// Created by CyberPro4 on 24/06/2016.
//

#ifndef CPORT_REMI_H
#define CPORT_REMI_H

#include <iostream>
#include <list>
#include <sstream>

//#include <thread>       //std::this_thread::sleep_for std::thread
#include <chrono>

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

int				remi_timestamp();

remi_thread		remi_createThread( remi_thread_callback callback , remi_thread_param param );

namespace remi {

    namespace utils {

        std::string toPix( int v );

        std::string join(std::list<std::string> stringList , std::string glue );

		std::string string_encode(std::string text);

		std::string sformat( std::string format , ... );

		int sscan( std::string from , std::string format , ... );

		std::list<std::string> split( std::string subject , std::string delimiter );

		std::string base64( std::string );

		std::string url_decode( std::string from );

		void open_browser( std::string url );

		template< class org >
		org list_at( std::list<org> list , int index ){
			int c = 0;
			for( org item : list ){
				if( c == index )
					return item;
				c++;
			}
		}

		class TimerListener {
		public:
			virtual void timer() = 0;
		};

        //Timer utility. Calls a function (in a parallel std::thread) at a predefined 'millisecondsInterval' until stopFlag turns True.
        class Timer {
        public:
            
            Timer( int millisecondsInterval , TimerListener* listener = NULL );

            void stop();

			void start();

			void setInterval( int millisecondsInterval );

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

        Dictionary(){}

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
            for( DictionaryValue<T>* currentAttribute : _library ){
                k.push_front( currentAttribute->name );
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

        const T get( std::string name ) const {
            DictionaryValue<T>* objectAttribute = this->getDictionaryValue( name );
            if( objectAttribute != NULL )
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


    template<class T> class VersionedDictionary : public Dictionary<T> {

    public:

        VersionedDictionary(){
            this->_version = 0;
        }

        void set( std::string name , T value, int version_increment = 1 ){
            /*if( has( name ) ){
                if( getAttribute( name ).compare( value ) != 0 )
                    _version += version_increment;
            } else
                _version += version_increment;*/
            _version += version_increment;

            return Dictionary<T>::set(name, value);
        }

        void clear( int version_increment = 1 ){
            _version += version_increment;
            Dictionary<T>::clear();
        }

        long getVersion(){
            return _version;
        }

    private:

        long _version;
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

    class Tag : public Represantable {

    public:

        Tag();

        Tag( std::string type );

        void addClass( std::string name );

        void removeClass( std::string name );

        std::string getIdentifier();


        std::string repr();

        void addChild( Represantable* child, std::string key = "" );

		void addChild( std::string child, std::string key = "" );

        Represantable * getChild(std::string key );

    public:

        std::list<std::string>              _classes;

        VersionedDictionary<std::string>    attributes;
        VersionedDictionary<std::string>    style;

        VersionedDictionary<Represantable *>       children;

        std::string     _type;

    private:

        std::list<Represantable *>              _render_children_list;

    };


	class Event {
	public:

		Event();
		Event( std::string name );

		Tag*			source;

		std::string		name;

		std::string		params;

	};

    class EventManagerListener {

    public:

        EventManagerListener();

        virtual void onEvent( std::string eventName , Event* eventData ) = 0;

    };

    class EventEmitter {

    public:

        EventEmitter();

        void propagate( Event* eventData );

        void registerListener( std::string eventName , EventManagerListener* listener, void* funcName = NULL );

    private:

        Dictionary<EventManagerListener*>  _listeners;
    };



    class Widget : public Tag , public EventEmitter {

    public:

        enum Layout {
            Horizontal = 1,
            Vertical = 0
        };

        static const std::string Event_OnClick;
        static const std::string Event_OnDblClick;
        static const std::string Event_OnMouseDown;
        static const std::string Event_OnMouseMove;
        static const std::string Event_OnMouseOver;
        static const std::string Event_OnMouseOut;
        static const std::string Event_OnMouseLeave;
        static const std::string Event_OnMouseUp;
        static const std::string Event_OnTouchMove;
        static const std::string Event_OnTouchStart;
        static const std::string Event_OnTouchEnd;
        static const std::string Event_OnTouchEnter;
        static const std::string Event_OnTouchLeave;
        static const std::string Event_OnTouchCancel;
        static const std::string Event_OnKeyDown;
        static const std::string Event_OnKeyPress;
        static const std::string Event_OnKeyUp;
        static const std::string Event_OnChange;
        static const std::string Event_OnFocus;
        static const std::string Event_OnBlur;
        static const std::string Event_OnContextMenu;
        static const std::string Event_OnUpdate;

        Widget();

        Widget( std::string type );

		void setWidth( int width );
		void setHeight( int height );
        void setSize( int width, int height );

        void setLayoutOrientation( Widget::Layout orientation );

        void redraw();

        void addChild( Represantable* child, std::string key = "" );

        void onFocus();

        void setOnFocusListener( void* listener , void* fname );

		void setOnClickListener( EventManagerListener* listener );

    private:

        void defaults();

        Widget::Layout  _layout_orientation;

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

	class TextInput : public TextWidget {
	public:

		static const std::string Event_OnEnter;

		TextInput( bool single_line = true );

		void setPlaceholder( std::string text );
		std::string placeholder();

		void setOnChangeListener( EventManagerListener* listener );

		void setOnKeyDownListener( EventManagerListener* listener );

		void setOnEnterListener( EventManagerListener* listener );

	};

	class Label : public TextWidget {

	public:

		Label( std::string text = "" );

	};


	class GenericDialog : public Widget {

	public:

		static const std::string Event_OnConfirm;
		static const std::string Event_OnCancel;

		GenericDialog( std::string title = "" , std::string message = "" );

		void setOnConfirmListener(EventManagerListener* listener);

	private:

		Widget*		_container;
		Button*		_confirmButton;
		Button*		_cancelButton;
		Widget*		_hLay;

	};

}

#endif //CPORT_REMI_H
