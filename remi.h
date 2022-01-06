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
#include <functional>

//#include <thread>       //std::this_thread::sleep_for std::thread
#include <chrono>
#include <typeinfo>

#pragma pointers_to_members(full_generality, virtual_inheritance)

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
typedef     void* (*remi_thread_callback)(remi_thread_param param);


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

remi_thread		remi_createThread(remi_thread_callback callback, remi_thread_param param);

namespace remi {


	namespace utils {

		std::string toPix(int v);

		//https://en.cppreference.com/w/cpp/language/user_literal
		//permits to set values expressed as 12_px and returns a string as "12px"
		std::string operator "" _px(long double value);
		//pc stays for percent
		std::string operator "" _pc(long double value);

		std::string join(std::list<std::string> stringList, std::string glue);

		std::string string_encode(std::string text);

		std::string escape_json(const std::string& s);

		std::string sformat(std::string format, ...);

		int sscan(std::string from, std::string format, ...);

		std::list<std::string> split(std::string subject, std::string delimiter);

		std::string strip(std::string subject, char char_to_strip);

		int count(std::string subject, std::string pattern);

		unsigned long long searchIndexOf(const char* buffer, char __char, unsigned long long len, unsigned long long start);

		//std::list<char*> splitCharPointer(const char* buf, char splitChar, unsigned long long bufLen, int maxSplit);

		std::string SHA1(std::string&);

		void url_decode(const char* from, unsigned long long len, char*& converted, unsigned long long* lenConverted);

		void open_browser(std::string url);


		template< class org >
		org list_at(std::list<org> list, int index) {
			int c = 0;
			org fitem;
			for (org item : list) {
				if (c == index)
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

			Timer(int millisecondsInterval, TimerListener* listener = NULL);
			Timer();

			void stop();

			void start();

			void setInterval(int millisecondsInterval);

			void setListener(TimerListener* listener);

			//bool has_passed();

			void tick();

			//double elapsed();

		private:

			remi_thread     _t;

			TimerListener* _listener;

			int             _millisecondsInterval;

			bool            _stopFlag;

			//bool			_passed;

			typedef std::chrono::steady_clock clock_;
			typedef std::chrono::duration<double, std::ratio<1> > second_;

			//std::chrono::time_point<clock_>		_start;

			static remi_thread_result thread_entry(remi_thread_param p);
		};
	};

	//some remi_server::App functionalities have to be called by widgets. Here a common
	//interface is defined
	class CommonAppInterface {
	public:
		virtual void executeJavascript(std::string command) = 0;
	};


	template<class T> class DictionaryValue {
	public:

		DictionaryValue(std::string name) {
			this->name = name;
		}

		DictionaryValue(std::string name, T value) {
			this->name = name;
			this->value = value;
		}

		void operator = (T v) {
			value = v;
		}

		operator T() {
			return value;
		}

		operator const T() const {
			return (const T)value;
		}

		std::string     name;
		T  value;
	};


	template<class T> class Dictionary {

	public:

		Dictionary() {
		}

		Dictionary(Dictionary<T>& d) {
			for (DictionaryValue<T>* value : d._library) {
				_library.push_front(new DictionaryValue<T>(value->name, value->value));
			}
		}

		bool has(std::string name) {
			DictionaryValue<T>* value = this->getDictionaryValue(name);
			if (value != NULL)
				return true;
			return false;
		}

		long size() {
			return this->_library.size();
		}

		std::list<std::string> keys() {
			std::list<std::string> k;
			if (_library.size() > 0) {
				for (DictionaryValue<T>* currentAttribute : _library) {
					k.push_front(currentAttribute->name);
				}
			}

			return k;
		}

		DictionaryValue<T>& operator [](std::string name) {
			DictionaryValue<T>* objectAttribute = this->getDictionaryValue(name);
			if (objectAttribute == NULL) {
				objectAttribute = new DictionaryValue<T>(name);
				_library.push_front(objectAttribute);
			}
			return (*objectAttribute);
		}

		void operator = (const Dictionary<T>& d) {
			clear();
			for (DictionaryValue<T>* value : d._library) {
				_library.push_front(new DictionaryValue<T>(value->name, value->value));
			}
		}

		void update(const Dictionary<T>& d) {
			for (DictionaryValue<T>* dictionaryValue : d._library) {
				this->set(dictionaryValue->name, dictionaryValue->value);
			}

		}

		const T get(std::string name) const {
			DictionaryValue<T>* objectAttribute = this->getDictionaryValue(name);

			/*if( objectAttribute != NULL )
				return objectAttribute->value;*/

			return objectAttribute->value;
		}

		void remove(std::string name) {
			DictionaryValue<T>* dictionaryValue = this->getDictionaryValue(name);
			if (dictionaryValue != NULL) {
				_library.remove(dictionaryValue);
				delete dictionaryValue;
			}
		}

		void set(std::string name, T value) {
			DictionaryValue<T>* dictionaryValue = this->getDictionaryValue(name);
			if (dictionaryValue == NULL) {
				DictionaryValue<T>* _value = new DictionaryValue<T>(name, value);
				_library.push_front(_value);
			}
			else {
				dictionaryValue->value = value;
			}
		}

		void clear() {
			for (DictionaryValue<T>* currentAttribute : _library) {
				delete currentAttribute;
			}

			_library.clear();
		}

		~Dictionary() {
			clear();
		}

	private:

		DictionaryValue<T>* getDictionaryValue(std::string name) const {
			if (this->_library.size() == 0)
				return NULL;


			for (DictionaryValue<T>* dictionaryValue : _library) {
				if (dictionaryValue->name.compare(name) == 0) {
					return dictionaryValue;
				}
			}

			return NULL;
		}

		std::list<DictionaryValue<T>*> _library;

	};


	namespace utils {

		std::string join(Dictionary<std::string>& from, std::string nameValueGlue, std::string itemsGlue);

		std::string toCss(Dictionary<std::string>& values);

	};


	class Buffer {
	public:
		char* data;
		unsigned long long len;
	public:
		Buffer(char* _data, unsigned long long _len) {
			data = _data;
			len = _len;
		}
		~Buffer() {
			delete data;
		}
		std::string str() {
			std::string _s;
			_s.assign(data, len);
			return _s;
		}
		/*std::wstring wstr() {
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			return converter.from_bytes(data,&data[len]);
		}*/
	};



	class JavascriptEventHandler {
	public:
		JavascriptEventHandler() {}
		virtual void handle_websocket_event(Dictionary<Buffer*>* params) {
		}
	};

	class EventSource {
	public:
		Dictionary<JavascriptEventHandler*> event_handlers;
	};

	template <typename... T>
	class Event {
	public:

		class EventListener {
		public:
			typedef void (EventListener::* listener_class_member_type)(EventSource*, T..., void*);
		};

		const char* _eventName; //this is used for comparison, when a js event occurs and have to be dispatched to widgets

		EventSource* _eventSource; //the event owner, that calls the listener

	public:
		typedef void (*listener_function_type)(EventSource*, T..., void*);
		listener_function_type _listener_function;

		typename EventListener::listener_class_member_type _listener_member;
		EventListener* _listener_instance;

		typedef std::function<void(EventSource*, T..., void*)> listener_contextualized_lambda_type;
		listener_contextualized_lambda_type _listener_context_lambda;

		void* _userData;

	public:
		Event(EventSource* eventSource, const char* eventName) :_eventSource(eventSource), _eventName(eventName) {
			_listener_function = NULL;
			_listener_member = NULL;
			_listener_instance = NULL;
			_listener_context_lambda = NULL;
		}

		virtual void link(EventListener* instance, typename EventListener::listener_class_member_type listener, void* userData = 0) {
			(*this) >> instance >> listener >> userData;
		}

		virtual void link(listener_function_type listener, void* userData = 0) {
			(*this) >> listener >> userData;
		}

		virtual void link(listener_contextualized_lambda_type listener, void* userData = 0) {
			(*this) >> listener >> userData;
		}

		//event registration in stream form myevent >> listener >> userData;;
		virtual Event& operator>> (listener_function_type listener) {
			_listener_member = NULL;
			_listener_instance = NULL;
			_listener_context_lambda = NULL;

			this->_listener_function = listener;
			return *this;
		}
		virtual Event& operator>> (listener_contextualized_lambda_type listener) {
			_listener_function = NULL;
			_listener_member = NULL;
			_listener_instance = NULL;

			this->_listener_context_lambda = listener;
			return *this;
		}
		virtual Event& operator>> (EventListener* instance) {
			_listener_function = NULL;
			_listener_context_lambda = NULL;

			this->_listener_instance = instance;
			return *this;
		}
		virtual Event& operator>> (typename EventListener::listener_class_member_type listener) {
			_listener_function = NULL;
			_listener_context_lambda = NULL;

			this->_listener_member = listener;
			return *this;
		}
		virtual Event& operator>> (void* userData) {
			this->_userData = userData;
			return *this;
		}

		virtual void handle_websocket_event(Dictionary<Buffer*>* params) {
		}

		virtual void operator()(T... params) = 0; /* {
			if (this->_listener_function != NULL) {
				this->_listener_function(_eventSource, params, this->_userData);
				return;
			}
			if (this->_listener_member != NULL) {
				//(*this->_listener_instance).*(this->_listener_member)(eventSource, params, this->_userData);
				//invoke(this->_listener_member, this->_listener_instance, eventSource, params, this->_userData);
				CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, params, this->_userData);
			}
			if (this->_listener_context_lambda != NULL) {
				this->_listener_context_lambda(_eventSource, params, this->_userData);
				return;
			}
		}*/
	};

	#define LINK_EVENT_TO_CLASS_MEMBER(event_type, event, pointerToObject, pointerToMember, pointerVoidPtrUserdata) event->link(reinterpret_cast<event_type::EventListener*>(pointerToObject), reinterpret_cast<event_type::EventListener::listener_class_member_type>(pointerToMember), reinterpret_cast<void*>(pointerVoidPtrUserdata);
	#define LINK_EVENT_TO_CLASS_MEMBER(event_type, event, pointerToObject, pointerToMember) event->link(reinterpret_cast<event_type::EventListener*>(pointerToObject), reinterpret_cast<event_type::EventListener::listener_class_member_type>(pointerToMember), NULL);
	#define LINK_EVENT_TO_FUNCTION(event_type, event, pointerToFunction, pointerVoidPtrUserdata) event->link(reinterpret_cast<event_type::listener_function_type>(pointerToFunction), reinterpret_cast<void*>(pointerVoidPtrUserdata));
	#define LINK_EVENT_TO_LAMBDA(event_type, event, lambdaExpression) event->link(reinterpret_cast<event_type::listener_contextualized_lambda_type>(lambdaExpression));

	#define EVENT_VOID(NAME) class NAME:public Event<>{ \
							public: \
								NAME(EventSource* eventSource):Event::Event(eventSource, CLASS_NAME(NAME)){ \
								} \
								void operator()(){ \
									if (this->_listener_function != NULL) { \
										this->_listener_function(_eventSource, this->_userData); \
										return; \
									} \
									if (this->_listener_member != NULL) { \
										CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, this->_userData); \
									} \
									if (this->_listener_context_lambda != NULL) { \
										this->_listener_context_lambda(_eventSource, this->_userData); \
										return; \
									} \
								} \
						}* event_##NAME;


	template<class T> class VersionedDictionary : public Dictionary<T>, public EventSource {
	public:

		VersionedDictionary() {
			this->_version = 0;
			this->event_onchange = new onchange(this);
		}

		~VersionedDictionary() {
			delete this->event_onchange;
		}

		/* With this operator the dictionary stores the key of the value to be accessed
			and returns itself. The overload of operator= and operator T() allows to
			get or set the value of the element for which the key has been stored.
			This is done in order to get rid of the changes over the element making it possible
			to spawn the onchange event.
			The Dictionay::operator[] returns the DictionaryValue, and this would make
			impossible to track value changes. This is why it has been overloaded.
		*/
		VersionedDictionary& operator [](std::string name) {
			Dictionary<T>::operator[](name);
			this->_elementKey = name;
			return *this;
		}

		void operator = (T value) {
			this->set(this->_elementKey, value);
		}

		operator T() {
			return this->get(this->_elementKey);
		}

		void update(const Dictionary<T>& d) {
			_version++;
			Dictionary<T>::update(d);
			this->event_onchange->operator()();
		}

		void remove(std::string name) {
			_version++;
			Dictionary<T>::remove(name);
			this->event_onchange->operator()();
		}

		void set(std::string name, T value) {
			/*if( has( name ) ){
				if( getAttribute( name ).compare( value ) != 0 )
					_version += version_increment;
			} else
				_version += version_increment;*/
			_version++;

			Dictionary<T>::set(name, value);
			this->event_onchange->operator()();
		}

		void clear() {
			_version++;
			Dictionary<T>::clear();
			this->event_onchange->operator()();
		}

		long getVersion() {
			return _version;
		}

		void alignVersion() {
			_lastVersion = _version;
		}

		bool isChanged() {
			return _version != _lastVersion;
		}

		/*
		class onChange:public Event{
			public:
				onChange(EventSource* eventSource):Event::Event(eventSource, CLASS_NAME(onChange)){
					eventSource->event_handlers.set(this->_eventName, this);
				}
				void operator()(Dictionary<Buffer*>* parameters=NULL){
					Event::operator()(_eventSource, parameters);
				}
		}* event_onchange;
		*/
		EVENT_VOID(onchange)

	private:

		long _version;
		long _lastVersion;
		std::string _elementKey; //this is the key to be accessed from the operator[]
	};


	//A generic property class that triggers an onchange event
	template <class T>
	class Property: public EventSource {
	public:
		EVENT_VOID(onchange);

		Property() {
			this->event_onchange = new onchange(this);
		}

		~Property() {
			delete this->event_onchange;
		}

		void operator = (T value) {
			this->_value = value;
			this->event_onchange->operator()();
		}

		operator T(){
			return this->_value;
		}

	private:
		T _value;

	};

	
	class TagProperty{
	public:
		TagProperty(std::string name, VersionedDictionary<std::string>* dictionary) {
			this->_name = name;
			this->_dictionary = dictionary;
		}

		~TagProperty() {
			
		}

		void operator = (std::string value) {
			this->_dictionary->set(this->_name, value);
		}

		operator std::string() {
			return this->_dictionary->get(this->_name);
		}

	private:
		VersionedDictionary<std::string>* _dictionary;
		std::string	_name;
	};


	class Represantable {
	public:
		virtual std::string repr(Dictionary<Represantable*>* changedWidgets, bool forceUpdate = false) = 0;
	};


	class StringRepresantable : public Represantable {
	public:

		StringRepresantable(std::string v);

		std::string repr(Dictionary<Represantable*>* changedWidgets = NULL, bool forceUpdate = false);

	private:

		std::string v;
	};


	class Tag : public Represantable, public EventSource, public Event<>::EventListener {

	public:

		Tag();

		Tag(VersionedDictionary<std::string> attributes, std::string _type, std::string _class = "");

		~Tag() {
			//Children are deleted by parent
			for (std::string k : this->children.keys()) {
				delete this->children.get(k);
			}

			//Event handlers are deleted here
			for (std::string k : this->event_handlers.keys()) {
				delete this->event_handlers.get(k);
			}
		}

		void setClass(std::string name);

		void addClass(std::string name);

		void removeClass(std::string name);

		std::string getIdentifier();

		void setIdentifier(std::string newIdentifier);

		virtual std::string innerHTML(Dictionary<Represantable*>* localChangedWidgets, bool forceUpdate);

		std::string repr(Dictionary<Represantable*>* changedWidgets, bool forceUpdate);

		std::string getLatestRepr() {
			return _backupRepr.str();
		}

		void setParent(Tag* tag) {
			this->_parent = tag;
			this->attributes.set("parent_widget", this->getIdentifier());
		}

		void addChild(Represantable* child, std::string key = "");

		void addChild(std::string child, std::string key = "");

		Represantable* getChild(std::string key);

		void setUpdated();

		bool isChanged() {
			return attributes.isChanged() || style.isChanged() || children.isChanged();
		}

		virtual void _notifyParentForUpdate();

		//EventListener::listener_type _needUpdate;
		void _needUpdate(Tag* emitter, void* userdata);
		
		void disableUpdate();
		void enableUpdate();

	public:

		std::list<std::string>              _classes;

		VersionedDictionary<std::string>    attributes;
		VersionedDictionary<std::string>    style;

		VersionedDictionary<Represantable*>       children;

		std::string     type;

		Tag* _parent;

		bool ignoreUpdate;

	private:

		std::list<Represantable*>  _render_children_list;
		std::ostringstream          _backupRepr;
		std::ostringstream          _reprAttributes;
	};


	template <typename... T>
	class EventJS :public Event<T...>, public JavascriptEventHandler {
		/* This class is used to set event widget attributes only when connected to a listener */
	public:
		std::string js_code;

	public:
		EventJS(EventSource* eventSource, const char* eventName, std::string _js_code) :Event::Event(eventSource, eventName) {
			this->js_code = _js_code;
		}

		//event registration in stream form myevent >> listener >> userData;;
		Event& operator>> (listener_function_type listener) {
			Tag* emitter = static_cast<Tag*>(_eventSource);
			emitter->attributes[this->_eventName] = utils::sformat(this->js_code, emitter->getIdentifier().c_str(), this->_eventName);
			return Event::operator>>(listener);
		}
		Event& operator>> (listener_contextualized_lambda_type listener) {
			Tag* emitter = static_cast<Tag*>(_eventSource);
			emitter->attributes[this->_eventName] = utils::sformat(this->js_code, emitter->getIdentifier().c_str(), this->_eventName);
			return Event::operator>>(listener);
		}
		Event& operator>> (Event<T...>::EventListener* instance) {
			Tag* emitter = static_cast<Tag*>(_eventSource);
			emitter->attributes[this->_eventName] = utils::sformat(this->js_code, emitter->getIdentifier().c_str(), this->_eventName);
			return Event::operator>>(instance);
		}
		Event& operator>> (typename Event<T...>::EventListener::listener_class_member_type listener) {
			//not required to set attribute js code, because it will be in
			//operator>>(EventListener* instance)
			//_eventSource->attributes[this->_eventName] = utils::sformat( this->js_code, this->_eventSource->getIdentifier().c_str(), this->_eventName);
			return Event::operator>>(listener);
		}
		Event& operator>> (void* userData) {
			//not required to set attribute js code, because it will be in
			//operator>>(EventListener* instance)
			//_eventSource->attributes[this->_eventName] = utils::sformat( this->js_code, this->_eventSource->getIdentifier().c_str(), this->_eventName);
			return Event::operator>>(userData);
		}
	};


	#define EVENT_JS(NAME, JSCODE) class NAME : public EventJS{ \
                                public: \
                                    NAME(Tag* emitter):EventJS::EventJS(emitter, CLASS_NAME(NAME), utils::sformat( JSCODE, emitter->getIdentifier().c_str(), CLASS_NAME(NAME))){ \
                                        ((Tag*)emitter)->event_handlers.set(this->_eventName, this); \
                                    } \
                                    void operator()(Dictionary<Buffer*>* parameters=NULL){ \
                                        Event::operator()(parameters); \
                                    } \
                            }* event_##NAME;

	#define EVENT_JS_DO(NAME, JSCODE, DOFUNCTION)  class NAME : public EventJS{ \
	                            public: \
	                                NAME(Tag* emitter):EventJS::EventJS(emitter, CLASS_NAME(NAME), utils::sformat( JSCODE, emitter->getIdentifier().c_str(), CLASS_NAME(NAME))){ \
	                                    ((Tag*)emitter)->event_handlers.set(this->_eventName, this); \
	                                } \
	                                void operator()(Dictionary<Buffer*>* parameters=NULL)DOFUNCTION; \
	                        }* event_##NAME;


	class Widget : public Tag {
	public:

		class onclick : public EventJS<> {
		public:
			onclick(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onclick), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(onclick))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				//Event::operator()(parameters);
				operator()();
			}
			virtual void operator()() {
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
		}*event_onclick;
		//EVENT_JS(onclick, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")


		/*EVENT_JS_DO(onclick, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();",{
			std::cout << "do some code here, maybe parse parameters" << std::endl;
			Event::operator()(parameters);
		})*/

		class onblur : public EventJS<> {
		public:
			onblur(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onblur), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(onblur))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()();
			}
			virtual void operator()() {
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
		}*event_onblur;
		//EVENT_JS(onblur, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

		class onfocus : public EventJS<> {
		public:
			onfocus(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onfocus), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(onfocus))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()();
			}
			virtual void operator()() {
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
		}*event_onfocus;
		//EVENT_JS(onfocus, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

		class ondblclick : public EventJS<> {
		public:
			ondblclick(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(ondblclick), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(ondblclick))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()();
			}
			virtual void operator()() {
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
		}*event_ondblclick;
		//EVENT_JS(ondblclick, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")
		
		class oncontextmenu : public EventJS<> {
		public:
			oncontextmenu(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(oncontextmenu), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(oncontextmenu))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()();
			}
			virtual void operator()() {
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
		}*event_oncontextmenu;
		//EVENT_JS(oncontextmenu, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

		class onmousedown : public EventJS<float, float> {
		public:
			onmousedown(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onmousedown), utils::sformat(R"(var params={};
			var boundingBox = this.getBoundingClientRect();
			params['x']=event.clientX-boundingBox.left;
			params['y']=event.clientY-boundingBox.top;
			remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onmousedown))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string x = parameters->get("x")->str();
				std::string y = parameters->get("y")->str();
				float xf = std::stof(x);
				float yf = std::stof(y);
				operator()(xf, yf);
			}
			virtual void operator()(float x, float y) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, x, y, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					//(*this->_listener_instance).*(this->_listener_member)(eventSource, params, this->_userData);
					//invoke(this->_listener_member, this->_listener_instance, eventSource, params, this->_userData);
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, x, y, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, x, y, this->_userData);
					return;
				}
			}
		}*event_onmousedown;

		class onmouseup : public EventJS<float, float> {
		public:
			onmouseup(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onmouseup), utils::sformat(R"(var params={};
			var boundingBox = this.getBoundingClientRect();
			params['x']=event.clientX-boundingBox.left;
			params['y']=event.clientY-boundingBox.top;
			remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onmouseup))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string x = parameters->get("x")->str();
				std::string y = parameters->get("y")->str();
				float xf = std::stof(x);
				float yf = std::stof(y);
				operator()(xf, yf);
			}
			virtual void operator()(float x, float y) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, x, y, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					//(*this->_listener_instance).*(this->_listener_member)(eventSource, params, this->_userData);
					//invoke(this->_listener_member, this->_listener_instance, eventSource, params, this->_userData);
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, x, y, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, x, y, this->_userData);
					return;
				}
			}
		}*event_onmouseup;

		class onmouseout : public EventJS<> {
		public:
			onmouseout(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onmouseout), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(onmouseout))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()();
			}
			virtual void operator()() {
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
		}*event_onmouseout;

		class onmouseover : public EventJS<> {
		public:
			onmouseover(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onmouseover), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(onmouseover))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()();
			}
			virtual void operator()() {
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
		}*event_onmouseover;

		class onmouseleave : public EventJS<> {
		public:
			onmouseleave(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onmouseleave), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(onmouseleave))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()();
			}
			virtual void operator()() {
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
		}*event_onmouseleave;
		
		class onmousemove : public EventJS<float, float> {
		public:
			onmousemove(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onmousemove), utils::sformat(R"(var params={};
			var boundingBox = this.getBoundingClientRect();
			params['x']=event.clientX-boundingBox.left;
			params['y']=event.clientY-boundingBox.top;
			remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onmousemove))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string x = parameters->get("x")->str();
				std::string y = parameters->get("y")->str();
				float xf = std::stof(x);
				float yf = std::stof(y);
				operator()(xf, yf);
			}
			virtual void operator()(float x, float y) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, x, y, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, x, y, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, x, y, this->_userData);
					return;
				}
			}
		}*event_onmousemove;

		class ontouchmove : public EventJS<float, float> {
		public:
			ontouchmove(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(ontouchmove), utils::sformat(R"(var params={};" \
			"var boundingBox = this.getBoundingClientRect();" \
			"params['x']=parseInt(event.changedTouches[0].clientX)-boundingBox.left;" \
			"params['y']=parseInt(event.changedTouches[0].clientY)-boundingBox.top;" \
			"remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(ontouchmove))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string x = parameters->get("x")->str();
				std::string y = parameters->get("y")->str();
				float xf = std::stof(x);
				float yf = std::stof(y);
				operator()(xf, yf);
			}
			virtual void operator()(float x, float y) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, x, y, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, x, y, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, x, y, this->_userData);
					return;
				}
			}
		}*event_ontouchmove;

		class ontouchstart : public EventJS<float, float> {
		public:
			ontouchstart(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(ontouchstart), utils::sformat(R"(var params={};" \
			"var boundingBox = this.getBoundingClientRect();" \
			"params['x']=parseInt(event.changedTouches[0].clientX)-boundingBox.left;" \
			"params['y']=parseInt(event.changedTouches[0].clientY)-boundingBox.top;" \
			"remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(ontouchstart))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string x = parameters->get("x")->str();
				std::string y = parameters->get("y")->str();
				float xf = std::stof(x);
				float yf = std::stof(y);
				operator()(xf, yf);
			}
			virtual void operator()(float x, float y) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, x, y, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, x, y, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, x, y, this->_userData);
					return;
				}
			}
		}*event_ontouchstart;

		class ontouchend : public EventJS<float, float> {
		public:
			ontouchend(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(ontouchend), utils::sformat(R"(var params={};" \
			"var boundingBox = this.getBoundingClientRect();" \
			"params['x']=parseInt(event.changedTouches[0].clientX)-boundingBox.left;" \
			"params['y']=parseInt(event.changedTouches[0].clientY)-boundingBox.top;" \
			"remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(ontouchend))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string x = parameters->get("x")->str();
				std::string y = parameters->get("y")->str();
				float xf = std::stof(x);
				float yf = std::stof(y);
				operator()(xf, yf);
			}
			virtual void operator()(float x, float y) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, x, y, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, x, y, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, x, y, this->_userData);
					return;
				}
			}
		}*event_ontouchend;

		class ontouchenter : public EventJS<float, float> {
		public:
			ontouchenter(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(ontouchenter), utils::sformat(R"(var params={};" \
			"var boundingBox = this.getBoundingClientRect();" \
			"params['x']=parseInt(event.changedTouches[0].clientX)-boundingBox.left;" \
			"params['y']=parseInt(event.changedTouches[0].clientY)-boundingBox.top;" \
			"remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(ontouchenter))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string x = parameters->get("x")->str();
				std::string y = parameters->get("y")->str();
				float xf = std::stof(x);
				float yf = std::stof(y);
				operator()(xf, yf);
			}
			virtual void operator()(float x, float y) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, x, y, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, x, y, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, x, y, this->_userData);
					return;
				}
			}
		}*event_ontouchenter;

		class ontouchleave : public EventJS<> {
		public:
			ontouchleave(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(ontouchleave), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(ontouchleave))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()();
			}
			virtual void operator()() {
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
		}*event_ontouchleave;

		class ontouchcancel : public EventJS<> {
		public:
			ontouchcancel(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(ontouchcancel), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(ontouchcancel))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()();
			}
			virtual void operator()() {
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
		}*event_ontouchcancel;

		class onkeyup : public EventJS<int, bool, bool, bool> {
		public:
			onkeyup(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onkeyup), utils::sformat(R"(var params={};params['key']=event.key;
			params['keycode']=(event.which||event.keyCode);
			params['ctrl']=event.ctrlKey;
			params['shift']=event.shiftKey;
			params['alt']=event.altKey;
			remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onkeyup))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string keycode = parameters->get("keycode")->str();
				std::string ctrl = parameters->get("ctrl")->str();
				std::string shift = parameters->get("shift")->str();
				std::string alt = parameters->get("alt")->str();
				int ikeycode = std::stoi(keycode);
				float bctrl = false;
				float bshift = false;
				float balt = false;
				operator()(ikeycode, bctrl, bshift, balt);
			}
			virtual void operator()(int keycode, bool ctrl, bool shift, bool alt) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, keycode, ctrl, shift, alt, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, keycode, ctrl, shift, alt, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, keycode, ctrl, shift, alt, this->_userData);
					return;
				}
			}
		}*event_onkeyup;

		class onkeydown : public EventJS<int, bool, bool, bool> {
		public:
			onkeydown(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onkeydown), utils::sformat(R"(var params={};params['key']=event.key;
			params['keycode']=(event.which||event.keyCode);
			params['ctrl']=event.ctrlKey;
			params['shift']=event.shiftKey;
			params['alt']=event.altKey;
			remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onkeydown))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string keycode = parameters->get("keycode")->str();
				std::string ctrl = parameters->get("ctrl")->str();
				std::string shift = parameters->get("shift")->str();
				std::string alt = parameters->get("alt")->str();
				int ikeycode = std::stoi(keycode);
				float bctrl = false;
				float bshift = false;
				float balt = false;
				operator()(ikeycode, bctrl, bshift, balt);
			}
			virtual void operator()(int keycode, bool ctrl, bool shift, bool alt) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, keycode, ctrl, shift, alt, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, keycode, ctrl, shift, alt, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, keycode, ctrl, shift, alt, this->_userData);
					return;
				}
			}
		}*event_onkeydown;

		void queryClient(CommonAppInterface* appInstance, std::list<std::string>& attributes_list, std::list<std::string>& style_properties_list) {
			std::string attributes_string = "";
			for (std::string& attribute : attributes_list) {
				attributes_string += "params['" + attribute + "']=document.getElementById('" + this->getIdentifier() + "')." + attribute + ";";
			}

			std::string style_string = "";
			for (std::string& style_prop : style_properties_list) {
				style_string += "params['" + style_prop + "']=document.getElementById('" + this->getIdentifier() + "').style." + style_prop + ";";
			}

			std::string command = "var params={};" + attributes_string + style_string + utils::sformat("remi.sendCallbackParam('%s','%s',params);", this->getIdentifier().c_str(), "onqueryClientResult");

			appInstance->executeJavascript(command);
		}

		/*
		class onqueryClientResult:public Event{
			public:
				onqueryClientResult(Tag* emitter):Event::Event(emitter, CLASS_NAME(onqueryClientResult)){
					((Tag*)emitter)->event_handlers.set(this->_eventName, this);
				}
		}* event_onqueryClientResult;
		*/
		//EVENT(onqueryClientResult)
		class onqueryClientResult :public Event<Dictionary<Buffer*>*>, public JavascriptEventHandler {
		public:
			onqueryClientResult(EventSource* eventSource) :Event::Event(eventSource, CLASS_NAME(onqueryClientResult)) {
				eventSource->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()(parameters);
			}
			void operator()(Dictionary<Buffer*>* parameters) {
				if (this->_listener_function != NULL) {
						this->_listener_function(_eventSource, parameters, this->_userData);
						return;
				}
				if (this->_listener_member != NULL) {
						CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, parameters, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
						this->_listener_context_lambda(_eventSource, parameters, this->_userData);
						return;
				}
			}
		}*event_onqueryClientResult;

	public:
		Widget();
		Widget(std::string _class);
		Widget(VersionedDictionary<std::string> _attributes, VersionedDictionary<std::string> _style, std::string _type, std::string _class);

		~Widget() {
			//Event handlers are deleted by Tag destructor
			/*delete this->event_onclick;
			delete this->event_onblur;
			delete this->event_onfocus;
			delete this->event_ondblclick;
			delete this->event_oncontextmenu;
			delete this->event_onmousedown;
			delete this->event_onmouseup;
			delete this->event_onmouseout;
			delete this->event_onmouseover;
			delete this->event_onmouseleave;
			delete this->event_onmousemove;
			delete this->event_ontouchmove;
			delete this->event_ontouchstart;
			delete this->event_ontouchend;
			delete this->event_ontouchenter;
			delete this->event_ontouchleave;
			delete this->event_ontouchcancel;
			delete this->event_onkeyup;
			delete this->event_onkeydown;
			delete this->event_onqueryClientResult;*/
		}

		void setWidth(int width);
		void setHeight(int height);
		void setSize(int width, int height);

		void addChild(Represantable* child, std::string key = "");

	public: //properties
		TagProperty css_color = TagProperty("color", &style); //Text color
		TagProperty css_float = TagProperty("float", &style);
		TagProperty css_margin = TagProperty("margin", &style); //Margins allows to define spacing around element
		TagProperty css_visibility = TagProperty("visibility", &style); //Specifies whether or not an element is visible
		TagProperty css_width = TagProperty("width", &style); //Widget width
		TagProperty css_height = TagProperty("height", &style); //Widget height
		TagProperty css_left = TagProperty("left", &style); //Widget x position
		TagProperty css_right = TagProperty("right", &style); //Widget x position
		TagProperty css_top = TagProperty("top", &style); //Widget y position
		TagProperty css_bottom = TagProperty("bottom", &style); //Widget y position
		TagProperty css_position = TagProperty("position", &style); //The position property specifies the type of positioning method used for an element.''', 'DropDown', { 'possible_values': ('static', 'absolute', 'fixed', 'relative', 'initial', 'inherit') }
		TagProperty css_overflow = TagProperty("overflow", &style); //Visibility behavior in case of content does not fit in size
		TagProperty css_background_color = TagProperty("background-color", &style);
		TagProperty css_background_image = TagProperty("background-image", &style); //An optional background image
		TagProperty css_background_position = TagProperty("background-position", &style); //The position of an optional background in the form 0% 0%
		TagProperty css_background_repeat = TagProperty("background-repeat", &style); //The repeat behaviour of an optional background image
		TagProperty css_opacity = TagProperty("opacity", &style); //The opacity level describes the transparency , where 1 is not transparent at all, 0.5 is 50 % and 0 is completely transparent
		TagProperty css_border_color = TagProperty("border-color", &style);
		TagProperty css_border_width = TagProperty("border-width", &style); //Border thickness
		TagProperty css_border_style = TagProperty("border-style", &style); //{ 'possible_values': ('none', 'solid', 'dotted', 'dashed') }
		TagProperty css_border_radius = TagProperty("border-radius", &style); //Border rounding radius
		TagProperty css_font_family = TagProperty("font-family", &style);
		TagProperty css_font_size = TagProperty("font-size", &style);
		TagProperty css_font_style = TagProperty("font-style", &style); //{ 'possible_values': ('normal', 'italic', 'oblique', 'inherit') }
		TagProperty css_font_weight = TagProperty("font-weight", &style); //{ 'possible_values': ('normal', 'bold', 'bolder', 'lighter', '100', '200', '300', '400', '500', '600', '700', '800', '900', 'inherit') }
		TagProperty css_line_height = TagProperty("line-height", &style); //The line height in pixels
		TagProperty css_white_space = TagProperty("white-space", &style); //{ 'possible_values': ('normal', 'nowrap', 'pre', 'pre-line', 'pre-wrap', 'initial', 'inherit') }
		TagProperty css_letter_spacing = TagProperty("letter-spacing", &style); //Increases or decreases the space between characters in a text

		TagProperty css_flex_direction = TagProperty("flex-direction", &style); //The flex-direction property specifies the direction of the flexible items. Note: If the element is not a flexible item, the flex-direction property has no effect.''', 'DropDown', { 'possible_values': ('row', 'row-reverse', 'column', 'column-reverse', 'initial', 'inherit') }
		TagProperty css_display = TagProperty("display", &style); //The display property specifies the type of box used for an HTML element { 'possible_values': ('inline', 'block', 'contents', 'flex', 'grid', 'inline-block', 'inline-flex', 'inline-grid', 'inline-table', 'list-item', 'run-in', 'table', 'none', 'inherit') }
		TagProperty css_justify_content = TagProperty("justify-content", &style); //The justify-content property aligns the flexible container's items when the items do not use all available space on the main - axis(horizontally)''', 'DropDown', {'possible_values': ('flex - start', 'flex - end', 'center', 'space - between', 'space - around', 'initial', 'inherit')}
		TagProperty css_align_items = TagProperty("align-items", &style); //The align-items property specifies the default alignment for items inside the flexible container''', 'DropDown', { 'possible_values': ('stretch', 'center', 'flex-start', 'flex-end', 'baseline', 'initial', 'inherit') }
		TagProperty css_align_content = TagProperty("align-content", &style); //The align-content property modifies the behavior of the flex-wrap property. It is similar to align - items, but instead of aligning flex items, it aligns flex lines.Tip: Use the justify - content property to align the items on the main - axis(horizontally).Note : There must be multiple lines of items for this property to have any effect.''', 'DropDown', {'possible_values': ('stretch', 'center', 'flex - start', 'flex - end', 'space - between', 'space - around', 'initial', 'inherit')}
		TagProperty css_flex_wrap = TagProperty("flex-wrap", &style); //The flex-wrap property specifies whether the flexible items should wrap or not. Note: If the elements are not flexible items, the flex-wrap property has no effect''', 'DropDown', { 'possible_values': ('nowrap', 'wrap', 'wrap-reverse', 'initial', 'inherit') }
		TagProperty css_flex_flow = TagProperty("flex-flow", &style); //The flex-flow property is a shorthand property for the flex-direction and the flex-wrap properties. The flex-direction property specifies the direction of the flexible items.''', 'DropDown', { 'possible_values': ('flex-direction', 'flex-wrap', 'initial', 'inherit') }
		TagProperty css_order = TagProperty("order", &style); //The order property specifies the order of a flexible item relative to the rest of the flexible items inside the same container. Note: If the element is not a flexible item, the order property has no effect
		TagProperty css_align_self = TagProperty("align-self", &style); //The align-self property specifies the alignment for the selected item inside the flexible container. Note: The align-self property overrides the flexible container's align - items property''', 'DropDown', {'possible_values': ('auto', 'stretch', 'center', 'flex - start', 'flex - end', 'baseline', 'initial', 'inherit')}
		TagProperty css_flex = TagProperty("flex", &style); //The flex property specifies the length of the item, relative to the rest of the flexible items inside the same container. The flex property is a shorthand for the flex-grow, flex-shrink, and the flex-basis properties. Note: If the element is not a flexible item, the flex property has no effect

		TagProperty attr_class = TagProperty("class", &attributes); //The html class attribute, allows to assign a css style class. Multiple classes have to be separed by space.
		TagProperty attr_title = TagProperty("title", &attributes); //Advisory information for the element

	};


	class Container : public Widget {
	public:
		enum Layout {
			Horizontal = 1,
			Vertical = 0
		};

	private:
		Container::Layout _layout_orientation;

	public:
		Container(Dictionary<Widget*>* children = NULL, Container::Layout layout_orientation = Layout::Vertical);

		std::string append(Widget* w, std::string key = std::string(""));
		virtual std::string append(Dictionary<Widget*>* w);

		void setLayoutOrientation(Container::Layout orientation);
	};


	class AsciiContainer : public Container {
	public:
		Dictionary<std::map<std::string, float>*> widget_layout_map; //here are contained the position and size of each widget

	public:
		AsciiContainer(std::string asciipattern = "", float gap_horizontal = 0, float gap_vertical = 0, Dictionary<Widget*> * children = NULL);

		~AsciiContainer();

		void set_from_asciiart(std::string asciipattern, float gap_horizontal = 0, float gap_vertical = 0);

		std::string append(Widget* w, std::string key = std::string(""));

		void set_widget_layout(std::string key);
	};


	class HTML :public Tag {
	public: //Events
		class onrequiredupdate :public Event<>{
		public:
			onrequiredupdate(EventSource* eventSource) :Event::Event(eventSource, CLASS_NAME(onrequiredupdate)) {
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
		}*event_onrequiredupdate;

	public:
		HTML();

		~HTML();

		void _notifyParentForUpdate();

		std::string repr(Dictionary<Represantable*>* changed_widgets, bool forceUpdate = false);
	};


	class HEAD :public Tag {
	public:
		class onerror :public Event<std::string, std::string, std::string, std::string, std::string>, public JavascriptEventHandler {
		public:
			onerror(EventSource* eventSource) :Event::Event(eventSource, CLASS_NAME(onerror)) {
				//THIS IS NOT A JAVASCRIPT EVENT HANDLER eventSource->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string message = parameters->get("message")->str();
				std::string source = parameters->get("source")->str();
				std::string lineno = parameters->get("lineno")->str();
				std::string colno = parameters->get("colno")->str();
				std::string error = parameters->get("error")->str();
				operator()(message, source, lineno, colno, error);
			}
			void operator()(std::string message, std::string source, std::string lineno, std::string colno, std::string error) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, message, source, lineno, colno, error, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, message, source, lineno, colno, error, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, message, source, lineno, colno, error, this->_userData);
					return;
				}
			} 
		}*event_onerror;

	public:
		HEAD(std::string title);

		~HEAD();

		void setIconFile(std::string filename, std::string rel = "icon", std::string mimetype = "image/png");

		void setIconData(std::string base64_data, std::string mimetype = "image/png", std::string rel = "icon");

		void setInternalJs(std::string net_interface_ip, unsigned short pending_messages_queue_length, unsigned short websocket_timeout_timer_ms);

		void setTitle(std::string title);

		std::string repr(Dictionary<Represantable*>* changed_widgets, bool forceUpdate);
	};


	class BODY :public Container {
	public:
		class onload : public EventJS<> {
		public:
			onload(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onload), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(onload))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()();
			}
			virtual void operator()() {
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
		}*event_onload;


		class ononline : public EventJS<> {
		public:
			ononline(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(ononline), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(ononline))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()();
			}
			virtual void operator()() {
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
		}*event_ononline;

		class onpagehide : public EventJS<> {
		public:
			onpagehide(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onpagehide), utils::sformat("remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();", emitter->getIdentifier().c_str(), CLASS_NAME(onpagehide))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				operator()();
			}
			virtual void operator()() {
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
		}*event_onpagehide;

		class onpageshow : public EventJS<float, float> {
		public:
			onpageshow(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onpageshow), utils::sformat(R"(
					var params={};
					params['width']=window.innerWidth;
					params['height']=window.innerHeight;
					remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onpageshow))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string width = parameters->get("width")->str();
				std::string height = parameters->get("height")->str();
				float fwidth = std::stof(width);
				float fheight = std::stof(height);
				operator()(fwidth, fheight);
			}
			virtual void operator()(float width, float height) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, width, height, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, width, height, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, width, height, this->_userData);
					return;
				}
			}
		}*event_onpageshow;

		class onresize : public EventJS<float, float> {
		public:
			onresize(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onresize), utils::sformat(R"(
					var params={};
					params['width']=window.innerWidth;
					params['height']=window.innerHeight;
					remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onresize))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string width = parameters->get("width")->str();
				std::string height = parameters->get("height")->str();
				float fwidth = std::stof(width);
				float fheight = std::stof(height);
				operator()(fwidth, fheight);
			}
			virtual void operator()(float width, float height) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, width, height, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, width, height, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, width, height, this->_userData);
					return;
				}
			}
		}*event_onresize;

	public:

		BODY();

		~BODY();

	};


	class HBox : public Container {
	public:
		HBox();
	};


	class VBox : public Container {
	public:
		VBox();
	};

	class TextWidget : public Widget {
	public:
		TagProperty css_writing_mode = TagProperty("writing-mode", &style); //'none', 'horizontal-tb', 'vertical-rl', 'vertical-lr'
		TagProperty css_text_align = TagProperty("text-align", &style); //'none', 'center', 'left', 'right', 'justify'
		TagProperty css_direction = TagProperty("direction", &style); //'none', 'ltr', 'rtl'
			
	public:
		void setText(std::string text);
		std::string text();
	};

	class Button : public TextWidget {
	public:

		Button(std::string text);

		void setEnabled(bool en);
		bool enabled();
	};


	class Label : public TextWidget {
	public:
		Label(std::string text = "");
	};
	
	
	class TextInput : public TextWidget {
	    /*Editable multiline/single_line text area widget. You can set the content by means of the function set_text or
	     retrieve its content with get_text.
	    */
	public:
		/*Called when the user changes the TextInput content.
With single_line=True it fires in case of focus lost and Enter key pressed.
With single_line=False it fires at each key released.

Args:
	new_value (str): the new string content of the TextInput.
*/
		class onchange : public EventJS<std::string> {
		public:
			onchange(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onchange), utils::sformat(R"(var params={};
			params['new_value']=this.value;
			remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onchange))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				static_cast<TextInput*>(_eventSource)->disableUpdate();
				static_cast<TextInput*>(_eventSource)->setValue(parameters->get("new_value")->str());
				static_cast<TextInput*>(_eventSource)->enableUpdate();
				operator()(parameters->get("new_value")->str());
			}
			virtual void operator()(std::string newValue) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, newValue, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, newValue, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, newValue, this->_userData);
					return;
				}
			}
		}*event_onchange;

		/*Called when user types and releases a key into the TextInput

		Note: This event can't be registered together with Widget.onchange.

		Args:
			new_value (str): the new string content of the TextInput
			keycode (str): the numeric char code
		*/
		class onkeyup : public EventJS<std::string, std::string> {
		public:
			onkeyup(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onkeyup), utils::sformat(R"(var elem=this;
            var params={};params['new_value']=elem.value;params['keycode']=(event.which||event.keyCode);
			remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onkeyup))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string newValue = parameters->get("new_value")->str();
				std::string keycode = parameters->get("keycode")->str();
				operator()(newValue, keycode);
			}
			virtual void operator()(std::string newValue, std::string keycode) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, newValue, keycode, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, newValue, keycode, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, newValue, keycode, this->_userData);
					return;
				}
			}
		}*event_onkeyup;

		/*Called when the user types a key into the TextInput.

		Note: This event can't be registered together with Widget.onchange.

		Args:
			new_value (str): the new string content of the TextInput.
			keycode (str): the numeric char code
		*/
		class onkeydown : public EventJS<std::string, std::string> {
		public:
			onkeydown(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onkeydown), utils::sformat(R"(var elem=this;
            var params={};params['new_value']=elem.value;params['keycode']=(event.which||event.keyCode);
			remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onkeydown))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string newValue = parameters->get("new_value")->str();
				std::string keycode = parameters->get("keycode")->str();
				operator()(newValue, keycode);
			}
			virtual void operator()(std::string newValue, std::string keycode) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, newValue, keycode, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, newValue, keycode, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, newValue, keycode, this->_userData);
					return;
				}
			}
		}*event_onkeydown;

	public:
		TagProperty attr_maxlength = TagProperty("maxlength", &attributes);
		TagProperty attr_placeholder = TagProperty("placeholder", &attributes);
		TagProperty attr_rows = TagProperty("rows", &attributes);
		TagProperty attr_autocomplete = TagProperty("autocomplete", &attributes);
		TagProperty css_resize = TagProperty("resize", &style);
		
	public:
		/*Args:
			single_line (bool): Determines if the TextInput have to be single_line. A multiline TextInput have a gripper
								that allows the resize.
			hint (str): Sets a hint using the html placeholder attribute.*/
		TextInput(bool singleLine = true, std::string hint = "");

		/*Sets the text content.

		Args:
			text (str): The string content that have to be appended as standard child identified by the key 'text'
		*/
		void setValue(std::string text);
    	
		/*Returns:
				str: The text content of the TextInput. You can set the text content with set_text(text).
		*/
		std::string getValue();
	    
    };


	class Progress : public Widget {
		/* Progress bar widget. */
	public:
		TagProperty attr_value = TagProperty("value", &attributes);
		TagProperty attr_max = TagProperty("max", &attributes);

	public:
		Progress(int value = 0, int max = 100) {
			/*
			Args :
				value(int) : The actual progress value.
				max(int) : The maximum progress value.
			*/
			type = "progress";
			setValue(value);
			this->attr_max = std::to_string(max);
		}

		void setValue(int value) {
			this->attr_value = std::to_string(value);
		}

		int getValue() {
			return std::atoi(((std::string)this->attr_value).c_str());
		}

		void setMax(int value) {
			this->attr_max = std::to_string(value);
		}

		int getMax() {
			return std::atoi(((std::string)this->attr_max).c_str());
		}
	};


	class GenericDialog : public Container {
	public:
		class onconfirm :public Event<>, public Button::onclick::EventListener {
		public:
			onconfirm(EventSource* eventSource) :Event::Event(eventSource, CLASS_NAME(onconfirm)) {
				//THIS IS NOT A JAVASCRIPT EVENT HANDLER eventSource->event_handlers.set(this->_eventName, this);
			}
			void onclick(EventSource* source, void* params) {
				operator()();
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
		}*event_onconfirm;

		class oncancel :public Event<>, public Button::onclick::EventListener {
		public:
			oncancel(EventSource* eventSource) :Event::Event(eventSource, CLASS_NAME(oncancel)) {
				//THIS IS NOT A JAVASCRIPT EVENT HANDLER eventSource->event_handlers.set(this->_eventName, this);
			}
			void onclick(EventSource* source, void* params) {
				operator()();
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
		}*event_oncancel;

	public:

		GenericDialog(std::string title = "", std::string message = "");

		void addFieldWithLabel(std::string key, std::string label_description, Widget* field);

		void addField(std::string key, Widget* field);

		Widget* getField(std::string key);

	private:

		Container* _container;
		Button* _confirmButton;
		Button* _cancelButton;
		HBox* _hLay;
		std::map<std::string, Widget*> _inputs;
	};


	class Image : public Widget {
	public:
		TagProperty attr_src = TagProperty("src", &attributes);
	public:
		Image(std::string url);
		void setURL(std::string);
		std::string url();

	};


	class Input : public Widget {
	public:
		Input();

		void setValue(std::string value);
		std::string getValue();

		void setEnable(bool on);
		bool isEnable();

		void setReadOnly(bool on);
		bool isReadOnly();
	};

	class FileUploader : public Widget {
	public:
		class onsuccess :public Event<std::string>, public JavascriptEventHandler {
		public:
			onsuccess(EventSource* eventSource) :Event::Event(eventSource, CLASS_NAME(onsuccess)) {
				eventSource->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string filename = parameters->get("filename")->str();
				operator()(filename);
			}
			void operator()(std::string filename) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, filename, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, filename, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, filename, this->_userData);
					return;
				}
			}
		}*event_onsuccess;

		class onfail :public Event<std::string>, public JavascriptEventHandler {
		public:
			onfail(EventSource* eventSource) :Event::Event(eventSource, CLASS_NAME(onfail)) {
				eventSource->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string filename = parameters->get("filename")->str();
				operator()(filename);
			}
			void operator()(std::string filename) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, filename, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, filename, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, filename, this->_userData);
					return;
				}
			}
		}*event_onfail;

		class ondata :public Event<std::string, Buffer*, std::string>, public JavascriptEventHandler {
		public:
			ondata(EventSource* eventSource) :Event::Event(eventSource, CLASS_NAME(ondata)) {
				eventSource->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				std::string filename = parameters->get("filename")->str();
				Buffer* data = parameters->get("data");
				std::string content_type = parameters->get("content_type")->str();
				operator()(filename, data, content_type);
			}
			void operator()(std::string filename, Buffer* data, std::string content_type) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, filename, data, content_type, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, filename, data, content_type, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, filename, data, content_type, this->_userData);
					return;
				}
			}
		}*event_ondata;
	
	public:
		FileUploader(std::string path = "./", bool multipleSelectionAllowed = true);

		void setSavePath(std::string path);
		std::string savePath();

		void setMultipleSelectionAllowed(bool value);
		bool multipleSelectionAllowed();

	private:
		std::string _path;
		bool _multipleSelectionAllowed;

	};
}

#endif //CPORT_REMI_H
