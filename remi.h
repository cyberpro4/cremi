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

		//std::string base64( std::string );
		std::string SHA1(std::string&);

		std::string url_decode( std::string from );

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

        Dictionary(){}

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

		void operator = ( const Dictionary<T> & d ){
			clear();
			for( DictionaryValue<T>* value : d._library ){
				_library.push_front( new DictionaryValue<T>( value->name , value->value ) );
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

		void alignVersion(){
			_lastVersion = _version;
		}

		bool isChanged(){
			return _version != _lastVersion;
		}

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

		void setUpdated();

		bool isChanged(){
			return attributes.isChanged() || style.isChanged() || children.isChanged();
		}

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

		Dictionary<std::string>			params;

	};

    class EventListener {

    public:

        EventListener();

        virtual void onEvent( std::string eventName , Event* eventData ) = 0;

    };

    class EventDispatcher : public EventListener {

    public:

        EventDispatcher();

        void registerListener( std::string eventName , EventListener* listener, void* funcName = NULL );

		virtual void onEvent( std::string eventName , Event* eventData );

    private:

        Dictionary<EventListener*>  _listeners;
    };


	namespace server {
		class App;
	}
	


    class Widget : public Tag , public EventDispatcher {
	public:
		class WidgetOnClickListener{ public: virtual void onClick(Widget*) = 0; };
		class WidgetOnDblClickListener{ public: virtual void onDblClick(Widget*) = 0; };
		class WidgetOnMouseDownListener{ public: virtual void onMouseDown(Widget*, int x, int y) = 0; };
		class WidgetOnMouseMoveListener{ public: virtual void onMouseMove(Widget*, int x, int y) = 0; };
		class WidgetOnMouseOverListener{ public: virtual void onMouseOver(Widget*) = 0; };
		class WidgetOnMouseOutListener{ public: virtual void onMouseOut(Widget*) = 0; };
		class WidgetOnMouseLeaveListener{ public: virtual void onMouseLeave(Widget*) = 0; };
		class WidgetOnMouseUpListener{ public: virtual void onMouseUp(Widget*, int x, int y) = 0; };
		class WidgetOnTouchMoveListener{ public: virtual void onTouchMove(Widget*, int x, int y) = 0; };
		class WidgetOnTouchStartListener{ public: virtual void onTouchStart(Widget*, int x, int y) = 0; };
		class WidgetOnTouchEndListener{ public: virtual void onTouchEnd(Widget*, int x, int y) = 0; };
		class WidgetOnTouchEnterListener{ public: virtual void onTouchEnter(Widget*, int x, int y) = 0; };
		class WidgetOnTouchLeaveListener{ public: virtual void onTouchLeave(Widget*) = 0; };
		class WidgetOnTouchCancelListener{ public: virtual void onTouchCancel(Widget*) = 0; };
		class WidgetOnKeyDownListener{ public: virtual void onKeyDown(Widget*) = 0; };
		class WidgetOnKeyPressListener{ public: virtual void onKeyPress(Widget*) = 0; };
		class WidgetOnKeyUpListener{ public: virtual void onKeyUp(Widget*) = 0; };
		class WidgetOnChangeListener{ public: virtual void onChange(Widget*) = 0; };
		class WidgetOnFocusListener{ public: virtual void onFocus(Widget*) = 0; };
		class WidgetOnBlurListener{ public: virtual void onBlur(Widget*) = 0; };
		class WidgetOnContextMenuListener{ public: virtual void onContextMenu(Widget*) = 0; };
		class WidgetOnUpdateListener{ public: virtual void onUpdate(Widget*) = 0; };

		WidgetOnClickListener* onClickListener;
		WidgetOnDblClickListener* onDblClickListener;
		WidgetOnMouseDownListener* onMouseDownListener;
		WidgetOnMouseMoveListener* onMouseMoveListener;
		WidgetOnMouseOverListener* onMouseOverListener;
		WidgetOnMouseOutListener* onMouseOutListener;
		WidgetOnMouseLeaveListener* onMouseLeaveListener;
		WidgetOnMouseUpListener* onMouseUpListener;
		WidgetOnTouchMoveListener* onTouchMoveListener;
		WidgetOnTouchStartListener* onTouchStartListener;
		WidgetOnTouchEndListener* onTouchEndListener;
		WidgetOnTouchEnterListener* onTouchEnterListener;
		WidgetOnTouchLeaveListener* onTouchLeaveListener;
		WidgetOnTouchCancelListener* onTouchCancelListener;
		WidgetOnKeyDownListener* onKeyDownListener;
		WidgetOnKeyPressListener* onKeyPressListener;
		WidgetOnKeyUpListener* onKeyUpListener;
		WidgetOnChangeListener* onChangeListener;
		WidgetOnFocusListener* onFocusListener;
		WidgetOnBlurListener* onBlurListener;
		WidgetOnContextMenuListener* onContextMenuListener;
		WidgetOnUpdateListener* onUpdateListener;
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

		void onEvent(std::string name, Event* event);

		void hide();

		void setParentApp( server::App* app );

    private:

        void defaults();

        Widget::Layout  _layout_orientation;

		server::App*			_parentApp;

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
		class TextInputOnEnterListener{ public: virtual void onEnter(TextInput*, std::string text) = 0; };

		TextInputOnEnterListener* onEnterListener;

	public:

		static const std::string Event_OnEnter;

		TextInput( bool single_line = true );

		void setPlaceholder( std::string text );
		std::string placeholder();

		void setOnChangeListener( EventListener* listener );

		void setOnKeyDownListener( EventListener* listener );

		void setOnEnterListener( EventListener* listener );

		virtual void onEvent( std::string name , Event* event );

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

		static const std::string Event_OnConfirm;
		static const std::string Event_OnCancel;

		GenericDialog( std::string title = "" , std::string message = "" );

		void onEvent(std::string name, Event* event);

		void addFieldWithLabel(std::string key, std::string label_description, Widget* field);

		void addField(std::string key, Widget* field);

		Widget* getField(std::string key);

	private:

		Widget*		_container;
		Button*		_confirmButton;
		Button*		_cancelButton;
		Widget*		_hLay;
		std::map<std::string, Widget*> _inputs;
	};

	class InputDialog : public GenericDialog, public TextInput::TextInputOnEnterListener {

	public:

		InputDialog( std::string title = "" , std::string message = "" );

		virtual void onEvent( std::string name , Event* event );

		std::string text();

		void setText(std::string);

		void onEnter(TextInput* w, std::string text);

	private:

		TextInput	_inputText;
	};


	class ListItem : public TextWidget {
	public:

		ListItem(std::string text = "");


	};

	class ListView : public Widget, public Widget::WidgetOnClickListener {
	public:
		class ListViewOnSelectionListener{ public: virtual void onSelection(ListView*, ListItem*) = 0; };

		ListViewOnSelectionListener* onSelectionListener;
	public:

		ListView();

		void addChild(Represantable* child, std::string key = "");

		virtual void onEvent(std::string name, Event* event);

		void selectByKey(std::string key);

		void selectItem(ListItem* item);

		void onClick(Widget*);

	public: //members
		ListItem*	selectedItem;

	};

	class Image : public Widget{
	public:

		Image(std::string url);

		void setURL(std::string);

		std::string url();

	};
	
	
}

#endif //CPORT_REMI_H
