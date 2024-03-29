//
// Created by CyberPro4 on 24/06/2016.
//

#ifndef CPORT_REMI_H
#define CPORT_REMI_H
#include <iostream>
#include <list>
#include <sstream>
#include <map>
#include <deque>
#include <codecvt>
#include <string>
#include <locale>
#include <iomanip>
#include <stdio.h>
#include <stdarg.h>
#include <functional>
#include <vector>

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

	
	class TagProperty {
	protected:
		VersionedDictionary<std::string>* _dictionary;
		std::string	_name;
	public:
		TagProperty(std::string name, VersionedDictionary<std::string>* dictionary) {
			this->_name = name;
			this->_dictionary = dictionary;
		}

		~TagProperty() {

		}

		void operator = (const char* value) const {
			this->_dictionary->set(this->_name, value);
		}

		void operator = (const char* value) {
			this->_dictionary->set(this->_name, value);
		}

		void operator = (std::string value) const {
			this->_dictionary->set(this->_name, value.c_str());
		}

		/*void operator = (const std::string& value) { //bool assignment gets called instead
			this->_dictionary->set(this->_name, value);
		}*/

		operator std::string() {
			return this->_dictionary->get(this->_name);
		}

		void operator = (bool value) const {
			this->_dictionary->set(this->_name, value ? "true" : "false");
		}

		operator bool() {
			std::string v = this->_dictionary->get(this->_name);
			if (v.length() < 1)return false;
			return (v[0] == 'T') || (v[0] == 't') || (v[0] == '1');
		}

		void operator = (int value) const {
			std::stringstream s;
			s << value;
			this->_dictionary->set(this->_name, s.str());
		}

		operator int() {
			std::stringstream value;
			value << this->_dictionary->get(this->_name);
			int res;
			value >> res;
			return res;
		}

		void operator = (float value) const {
			std::stringstream s;
			s << value;
			this->_dictionary->set(this->_name, s.str());
		}

		operator float() {
			std::stringstream value;
			value << this->_dictionary->get(this->_name);
			float res;
			value >> res;
			return res;
		}

		void remove() {
			//if (this->_dictionary->has(this->_name)) { //redundant
			this->_dictionary->remove(this->_name);
			//}
		}

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


	class Tag : public Represantable, 
		public EventSource, 
		public Event<>::EventListener, 
		public Event<std::string>::EventListener,
		public Event<Represantable*>::EventListener {

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

		Represantable* removeChild(std::string key);

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


	#define EVENT_JS_VOID(NAME, JSCODE) class NAME : public EventJS<>{ \
                                public: \
                                    NAME(Tag* emitter):EventJS::EventJS(emitter, CLASS_NAME(NAME), utils::sformat( JSCODE, emitter->getIdentifier().c_str(), CLASS_NAME(NAME))){ \
                                        ((Tag*)emitter)->event_handlers.set(this->_eventName, this); \
                                    } \
									void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) { \
										operator()(); \
									} \
									virtual void operator()() { \
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

	#define EVENT_JS_DO(NAME, JSCODE, DOFUNCTION)  class NAME : public EventJS<>{ \
	                            public: \
	                                NAME(Tag* emitter):EventJS::EventJS(emitter, CLASS_NAME(NAME), utils::sformat( JSCODE, emitter->getIdentifier().c_str(), CLASS_NAME(NAME))){ \
	                                    ((Tag*)emitter)->event_handlers.set(this->_eventName, this); \
	                                } \
	                                void operator()(Dictionary<Buffer*>* parameters=NULL)DOFUNCTION; \
	                        }* event_##NAME;


	class Widget : public Tag {
	public:
		/*
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
		*/
		EVENT_JS_VOID(onclick, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")


		/*EVENT_JS_DO(onclick, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();",{
			std::cout << "do some code here, maybe parse parameters" << std::endl;
			Event::operator()(parameters);
		})*/

		EVENT_JS_VOID(onblur, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

		EVENT_JS_VOID(onfocus, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

		EVENT_JS_VOID(ondblclick, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")
		
		EVENT_JS_VOID(oncontextmenu, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

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

		EVENT_JS_VOID(onmouseout, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

		EVENT_JS_VOID(onmouseover, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

		EVENT_JS_VOID(onmouseleave, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")
		
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

		EVENT_JS_VOID(ontouchleave, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

		EVENT_JS_VOID(ontouchcancel, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

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

		void empty();

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
		EVENT_JS_VOID(onload, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

		EVENT_JS_VOID(ononline, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

		EVENT_JS_VOID(onpagehide, "remi.sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();")

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
			this->attr_max = std::to_string(max).c_str();
		}

		void setValue(int value) {
			this->attr_value = std::to_string(value).c_str();
		}

		int getValue() {
			return std::atoi(((std::string)this->attr_value).c_str());
		}

		void setMax(int value) {
			this->attr_max = std::to_string(value).c_str();
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


	template <class T>
	class Input : public Widget {
	public:
		TagProperty attr_value = TagProperty("value", &attributes);
		TagProperty attr_type = TagProperty("type", &attributes);
		TagProperty attr_autocomplete = TagProperty("autocomplete", &attributes);

		class onchange : public EventJS<T> {
		public:
			onchange(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onchange), utils::sformat(R"(var params={};
				params['value']=this.value;
				remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onchange))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				static_cast<Input*>(_eventSource)->disableUpdate();
				static_cast<Input*>(_eventSource)->attr_value = parameters->get("value")->str();
				static_cast<Input*>(_eventSource)->enableUpdate();
				operator()(static_cast<Input*>(_eventSource)->attr_value);
			}
			virtual void operator()(T newValue) {
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
	public:
		Input(std::string input_type, T defaultValue);

		void setEnable(bool on);
		bool isEnable();

		void setReadOnly(bool on);
		bool isReadOnly();

	};


	class CheckBox : public Input<bool> {
	public:
		class TagPropertyChecked:public TagProperty{
			void operator = (const char* value){
				/*operator = (string) overloading to intercept and call the boolean one*/
				TagProperty::operator=(value);
				this->operator=((bool)(*this));
			}
			void operator = (bool value) {
				if (value) {
					this->_dictionary->set(this->_name, "checked");
				}else{
					remove();
				}
			}

			operator bool() {
				return this->_dictionary->has(this->_name);
			}
		};
		TagProperty attr_value = TagProperty("checked", &attributes);

		class onchange : public EventJS<bool> {
		public:
			onchange(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onchange), utils::sformat(R"(var params={};
				params['value']=this.checked;
				remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onchange))) {
				((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				static_cast<CheckBox*>(_eventSource)->disableUpdate();
				static_cast<CheckBox*>(_eventSource)->attr_value = parameters->get("value")->str().c_str();
				if (((bool)static_cast<CheckBox*>(_eventSource)->attr_value) == false) {
					static_cast<CheckBox*>(_eventSource)->attr_value.remove();
					static_cast<CheckBox*>(_eventSource)->enableUpdate();
					operator()(false);
				}else {
					static_cast<CheckBox*>(_eventSource)->enableUpdate();
					operator()(true);
				}
			}
			virtual void operator()(bool newValue) {
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

	public:
		CheckBox();
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


	class ListItem : public TextWidget {
		/*
		List item widget for the ListView.

		ListItems are characterized by a textual content.They can be selected from
		the ListView.Do NOT manage directly its selection by registering set_on_click_listener, use instead the events of
		the ListView.
		*/
	public:
		TagProperty attr_selected = TagProperty("selected", &attributes); //Selection status
	public:
		ListItem(std::string text = "");

	};

	class ListView : public Container, public ListItem::onclick::EventListener {
		/*
		List widget it can contain ListItems.Add items to it by using the standard append(item, key) function or
		generate a filled list from a string list by means of the function new_from_list.Use the list in conjunction of
		its onselection event.Register a listener with ListView.onselection.connect.
		*/
	private:
		bool		selectable;
		ListItem* selectedItem;
		std::string selectedKey;
	public:
		ListView(bool selectable = true);

		ListView(std::vector<std::string> values, bool selectable = true);

		void onItemSelected(EventSource* source, void* params) {
			for (std::string key : this->children.keys()) {
				if (static_cast<ListItem*>(source) == this->children[key]) {
					this->selectByKey(key);
					break;
				}
			}
			this->event_onselection->operator()(this->selectedItem);
		}
		class onselection :public Event<ListItem*> {
		public:
			onselection(EventSource* eventSource) :Event::Event(eventSource, CLASS_NAME(onselection)) {
				//THIS IS NOT A JAVASCRIPT EVENT HANDLER eventSource->event_handlers.set(this->_eventName, this);
			}
			void operator()(ListItem* item) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, item, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, item, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, item, this->_userData);
					return;
				}
			}
		}*event_onselection;

		std::string append(ListItem* w, std::string key = std::string("")) {
			LINK_EVENT_TO_CLASS_MEMBER(ListItem::onclick, w->event_onclick, this, &ListView::onItemSelected);
			w->attr_selected = "false";
			return Container::append(w, key);
		}

		void empty() {
			/*
			* Removes all children from the list
			*/
			this->resetSelection();
			Container::empty();
		}

		void resetSelection() {
			if (this->selectedItem != NULL) {
				this->selectedItem->attr_selected = "false";
			}
			this->selectedItem = NULL;
			this->selectedKey = "";
		}

		bool selectByKey(std::string key) {
			this->resetSelection();
			if (!this->children.has(key))return false;
			this->selectedItem = reinterpret_cast<ListItem*>(this->getChild(key));
			this->selectedKey = key;
			if (this->selectable) {
				this->selectedItem->attr_selected = "true";
			}
			return true;
		}

		ListItem* get_item() {
			/*
				Returns :
				ListItem* : The selected item or NULL
			*/
			return this->selectedItem;
		}

		std::string get_value() {
			/*
				Returns :
				std::string : The value of the selected item or None
			*/
			if (this->selectedItem == NULL)return "";
			return this->selectedItem->text();
		}

		std::string get_key() {
			/*
				Returns :
				std::string : The key of the selected item or "" if no item is selected.
			*/
			return this->selectedKey;
		}

	};

	class DropDownItem : public TextWidget {
		/*
		Item widget for the DropDown.

		DropDownItems are characterized by a textual content.
		*/
	public:
		TagProperty attr_selected = TagProperty("selected", &attributes); //Selection status
	public:
		DropDownItem(std::string text = "");

	};

	class DropDown : public Container {
		/* 
		Drop down selection widget.Implements the onchange(value) event.
		*/
	private:
		bool			selectable;
		DropDownItem*	selectedItem;
		std::string		selectedKey;

	public:
		class onchange : public EventJS<std::string> {
		public:
			onchange(Tag* emitter) :EventJS::EventJS(emitter, CLASS_NAME(onchange), utils::sformat(R"(var params={};
				params['value']=this.value;
				remi.sendCallbackParam('%s','%s',params);)", emitter->getIdentifier().c_str(), CLASS_NAME(onchange))) {
					((Tag*)emitter)->event_handlers.set(this->_eventName, this);
			}
			void handle_websocket_event(Dictionary<Buffer*>* parameters = NULL) {
				static_cast<DropDown*>(_eventSource)->disableUpdate();
				static_cast<DropDown*>(_eventSource)->selectByValue(parameters->get("value")->str());
				static_cast<DropDown*>(_eventSource)->enableUpdate();
				operator()(parameters->get("value")->str());
			}
			virtual void operator()(std::string value) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, value, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, value, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, value, this->_userData);
					return;
				}
			}
		}*event_onchange;

		DropDown(){
			type = "select";
			setClass(CLASS_NAME(DropDown));
			this->selectedItem = NULL;
			this->selectedKey = "";
			this->event_onchange = new DropDown::onchange(this);
		}

		DropDown(std::vector<std::string> values):DropDown(){
			for (std::string value : values) {
				this->append(new DropDownItem(value));
			}
		}

		DropDownItem* get_item() {
			/*
				Returns :
				DropDownItem* : The selected item or NULL
			*/
			return this->selectedItem;
		}

		std::string get_value() {
			/*
				Returns :
				std::string : The value of the selected item or None
			*/
			if (this->selectedItem == NULL)return "";
			return this->selectedItem->text();
		}

		std::string get_key() {
			/*
				Returns :
				std::string : The key of the selected item or "" if no item is selected.
			*/
			return this->selectedKey;
		}

		std::string append(DropDownItem* w, std::string key = std::string("")) {
			key = Container::append(w, key);
			if (this->children.size() == 1) {
				this->selectByKey(key);
			}
			return key;
		}

		void empty() {
			/*
			* Removes all children from the list
			*/
			this->resetSelection();
			Container::empty();
		}

		void resetSelection() {
			if (this->selectedItem != NULL) {
				this->selectedItem->attr_selected.remove();
			}
			this->selectedItem = NULL;
			this->selectedKey = "";
		}

		bool selectByKey(std::string key) {
			this->resetSelection();
			if (!this->children.has(key))return false;
			this->selectedItem = reinterpret_cast<DropDownItem*>(this->getChild(key));
			this->selectedKey = key;
			if (this->selectable) {
				this->selectedItem->attr_selected = "true";
			}
			return true;
		}

		bool selectByValue(std::string value) {
			this->resetSelection();
			for (std::string key : this->children.keys()) {
				if (reinterpret_cast<DropDownItem*>(this->getChild(key))->text() == value) {
					this->selectByKey(key);
				}
			}
			return (this->selectedItem != NULL);
		}
	};

	class TableItem : public TextWidget {
	public:
		TableItem(std::string text = "");
	};

	class TableTitle : public TableItem {
	public:
		TableTitle(std::string text = "");
	};

	class TableRow : public Container, public TableItem::onclick::EventListener {
	public:
		void onTableItemClick(EventSource* source, void* userdata) {
			std::string key = "";
			for (std::string _key : this->children.keys()) {
				if (reinterpret_cast<TableItem*>(source) == this->children[_key]) {
					key = _key;
					break;
				}
			}
			this->event_onrowitemclick->operator()(static_cast<TableItem*>(source));
		}
		class onrowitemclick :public Event<TableItem*> {
		public:
			onrowitemclick(EventSource* eventSource) :Event::Event(eventSource, CLASS_NAME(onrowitemclick)) {
				//THIS IS NOT A JAVASCRIPT EVENT HANDLER eventSource->event_handlers.set(this->_eventName, this);
			}
			void operator()(TableItem* item) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, item, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, item, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, item, this->_userData);
					return;
				}
			}
		}*event_onrowitemclick;
	public:
		TableRow();

		std::string append(TableItem* w, std::string key = std::string("")) {
			LINK_EVENT_TO_CLASS_MEMBER(TableItem::onclick, w->event_onclick, this, &TableRow::onTableItemClick);
			
			return Container::append(w, key);
		}

	};

	class Table : public Container, public TableRow::onrowitemclick::EventListener {
		/*
			table widget - it will contains TableRow
		*/
	public:
		void onRowItemClick(EventSource* source, TableItem* item, void* userdata) {
			std::string key = "";
			for (std::string _key : this->children.keys()) {
				if (static_cast<TableRow*>(source) == this->children[_key]) {
					key = _key;
					break;
				}
			}
			this->event_ontablerowclick->operator()(static_cast<TableRow*>(source), item);
		}
		class ontablerowclick :public Event<TableRow*, TableItem*> {
		public:
			ontablerowclick(EventSource* eventSource) :Event::Event(eventSource, CLASS_NAME(ontablerowclick)) {
				//THIS IS NOT A JAVASCRIPT EVENT HANDLER eventSource->event_handlers.set(this->_eventName, this);
			}
			void operator()(TableRow* row, TableItem* item) {
				if (this->_listener_function != NULL) {
					this->_listener_function(_eventSource, row, item, this->_userData);
					return;
				}
				if (this->_listener_member != NULL) {
					CALL_MEMBER_FN(*this->_listener_instance, this->_listener_member)(_eventSource, row, item, this->_userData);
				}
				if (this->_listener_context_lambda != NULL) {
					this->_listener_context_lambda(_eventSource, row, item, this->_userData);
					return;
				}
			}
		}*event_ontablerowclick;
	public:
		Table();
			
		void appendFromVectorOfVectorOfStrings(std::vector<std::vector<std::string>> data, bool fillTitle = false) {
			unsigned int row_index = 0;
			for (std::vector<std::string> row : data) {
				unsigned int col_index = 0;
				TableRow* tr = new TableRow();
				for (std::string item : row) {
					TableItem* ti = NULL;
					if (row_index == 0 && fillTitle) {
						ti = new TableTitle(item);
					} else {
						ti = new TableItem(item);
					}
					tr->append(ti, remi::utils::sformat("%d", col_index));
					col_index++;
				}
				this->append(tr, remi::utils::sformat("%d", row_index));
				row_index++;
			}
		}

		std::string append(TableRow* w, std::string key = std::string("")) {
			LINK_EVENT_TO_CLASS_MEMBER(TableRow::onrowitemclick, w->event_onrowitemclick, this, &Table::onRowItemClick);

			return Container::append(w, key);
		}

	};



	class ISvgStroke {
	public:
		//@editor_attribute_decorator("WidgetSpecific", '''Color for svg elements.''', 'ColorPicker', {})
		TagProperty attr_stroke;
		TagProperty attr_stroke_width;

		ISvgStroke(Tag* tagInstance) :
			attr_stroke(TagProperty("stroke", &tagInstance->attributes)),
			attr_stroke_width(TagProperty("stroke-width", &tagInstance->attributes))
		{}

		void set_stroke(float width = 1, std::string color = "black") {
			/*
			Sets the stroke properties.

			Args :
			width(float) : stroke width
			color(str) : stroke color
			*/
			this->attr_stroke = color;
			this->attr_stroke_width = std::to_string(width);
		}
	};

	class ISvgTransformable {
	public:
		//@editor_attribute_decorator("Transformation", """Transform commands (i.e. rotate(45), translate(30,100)).""", str, {})
		TagProperty css_transform;
		//@editor_attribute_decorator("Transformation", """Transform origin as percent or absolute x,y pair value or ["center","top","bottom","left","right"] .""", str, {})
		TagProperty css_transform_origin;
		//@editor_attribute_decorator("Transformation", """Alters the behaviour of tranform and tranform-origin by defining the transform box.""", "DropDown", { "possible_values": ("content-box","border-box","fill-box","stroke-box","view-box") })
		TagProperty css_transform_box;

		ISvgTransformable(Tag* tagInstance) :
			css_transform(TagProperty("transform", &tagInstance->style)),
			css_transform_origin(TagProperty("transform-origin", &tagInstance->style)),
			css_transform_box(TagProperty("transform-box", &tagInstance->style))
		{}
	};

	class ISvgFill {
	public:
		//@editor_attribute_decorator("WidgetSpecific", """Fill color for svg elements.""", "ColorPicker", {})
		TagProperty attr_fill;
		//@editor_attribute_decorator("WidgetSpecific", """Fill opacity for svg elements.""", float, { "possible_values": "", "min" : 0.0, "max" : 1.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_fill_opacity;

		ISvgFill(Tag* tagInstance) :
			attr_fill(TagProperty("fill", &tagInstance->attributes)),
			attr_fill_opacity(TagProperty("fill-opacity", &tagInstance->attributes)) {

		}

		void set_fill(std::string color = "black") {
			/*Sets the fill color.

			Args :
			color(str) : stroke color
			*/
			this->attr_fill = color;
		}
	};

	class ISvgPosition {
	public:
		//@editor_attribute_decorator("WidgetSpecific", """Coordinate for Svg element.""", float, { "possible_values": "", "min" : -65635.0, "max" : 65635.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_x;
		//@editor_attribute_decorator("WidgetSpecific", """Coordinate for Svg element.""", float, { "possible_values": "", "min" : -65635.0, "max" : 65635.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_y;

		ISvgPosition(Tag* tagInstance) :
			attr_x(TagProperty("x", &tagInstance->attributes)),
			attr_y(TagProperty("y", &tagInstance->attributes)) {

		}

		void set_position(float x, float y) {
			/*
			Sets the shape position.

			Args :
			x(float) : the x coordinate
			y(float) : the y coordinate
			*/
			this->attr_x = x;
			this->attr_y = y;
		}
	};

	class ISvgSize {
	public:
		//@editor_attribute_decorator("WidgetSpecific", """Width for Svg element.""", float, { "possible_values": "", "min" : 0.0, "max" : 65635.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_width;
		//@editor_attribute_decorator("WidgetSpecific", """Height for Svg element.""", float, { "possible_values": "", "min" : 0.0, "max" : 65635.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_height;

		ISvgSize(Tag* tagInstance) :
			attr_width(TagProperty("width", &tagInstance->attributes)),
			attr_height(TagProperty("height", &tagInstance->attributes)) {

		}

		void set_size(float w, float h) {
			/*
			Sets the rectangle size.

			Args :
			w(int) : width of the rectangle
			h(int) : height of the rectangle
			*/
			this->attr_width = w;
			this->attr_height = h;
		}
	};

	class SvgStop :public Tag {
	public:
		//@editor_attribute_decorator("WidgetSpecific", """Gradient color""", "ColorPicker", {})
		TagProperty css_stop_color = TagProperty("stop-color", &style);
		/*@editor_attribute_decorator("WidgetSpecific", """The opacity property sets the opacity level for the gradient.
				The opacity - level describes the transparency - level, where 1 is not transparent at all, 0.5 is 50 % see - through, and 0 is completely transparent.""", float, {"possible_values": "", "min": 0.0, "max": 1.0, "default": 1.0, "step": 0.1})
		*/
		TagProperty css_stop_opactity = TagProperty("stop-opacity", &style);
		//@editor_attribute_decorator("WidgetSpecific", """The offset value for the gradient stop. It is in percentage""", float, { "possible_values": "", "min" : 0, "max" : 100, "default" : 0, "step" : 1 })
		TagProperty attr_offset = TagProperty("offset", &attributes);

		SvgStop(std::string offset = "0%", std::string color = "rgb(255,255,0)", float opacity = 1.0) : Tag::Tag() {
			this->type = "stop";
			this->attr_offset = offset;
			this->css_stop_color = color;
			this->css_stop_opactity = opacity;
		}
	};

	class SvgGradientLinear :public Tag {
	public:
		//@editor_attribute_decorator("WidgetSpecific", """Gradient coordinate value. It is expressed in percentage""", float, { "possible_values": "", "min" : 0, "max" : 100, "default" : 0, "step" : 1 })
		TagProperty attr_x1 = TagProperty("x1", &attributes);
		//@editor_attribute_decorator("WidgetSpecific", """Gradient coordinate value. It is expressed in percentage""", float, { "possible_values": "", "min" : 0, "max" : 100, "default" : 0, "step" : 1 })
		TagProperty attr_y1 = TagProperty("y1", &attributes);
		//@editor_attribute_decorator("WidgetSpecific", """Gradient coordinate value. It is expressed in percentage""", float, { "possible_values": "", "min" : 0, "max" : 100, "default" : 0, "step" : 1 })
		TagProperty attr_x2 = TagProperty("x2", &attributes);
		//@editor_attribute_decorator("WidgetSpecific", """Gradient coordinate value. It is expressed in percentage""", float, { "possible_values": "", "min" : 0, "max" : 100, "default" : 0, "step" : 1 })
		TagProperty attr_y2 = TagProperty("y2", &attributes);

		SvgGradientLinear(std::string x1, std::string y1, std::string x2, std::string y2) : Tag::Tag() {
			this->type = "linearGradient";
			this->attr_x1 = x1;
			this->attr_y1 = y1;
			this->attr_x2 = x2;
			this->attr_y2 = y2;
		}
	};

	class SvgGradientRadial :public Tag {
	public:
		//@editor_attribute_decorator("WidgetSpecific", """Gradient coordinate value. It is expressed in percentage""", float, { "possible_values": "", "min" : 0, "max" : 100, "default" : 0, "step" : 1 })
		TagProperty attr_cx = TagProperty("cx", &attributes);
		//@editor_attribute_decorator("WidgetSpecific", """Gradient coordinate value. It is expressed in percentage""", float, { "possible_values": "", "min" : 0, "max" : 100, "default" : 0, "step" : 1 })
		TagProperty attr_cy = TagProperty("cy", &attributes);
		//@editor_attribute_decorator("WidgetSpecific", """Gradient coordinate value. It is expressed in percentage""", float, { "possible_values": "", "min" : 0, "max" : 100, "default" : 0, "step" : 1 })
		TagProperty attr_fx = TagProperty("fx", &attributes);
		//@editor_attribute_decorator("WidgetSpecific", """Gradient coordinate value. It is expressed in percentage""", float, { "possible_values": "", "min" : 0, "max" : 100, "default" : 0, "step" : 1 })
		TagProperty attr_fy = TagProperty("fy", &attributes);
		//@editor_attribute_decorator("WidgetSpecific", """Gradient radius value. It is expressed in percentage""", float, { "possible_values": "", "min" : 0, "max" : 100, "default" : 0, "step" : 1 })
		TagProperty attr_r = TagProperty("r", &attributes);

		SvgGradientRadial(std::string cx = "20%", std::string cy = "30%", std::string r = "30%", std::string fx = "50%", std::string fy = "50%") : Tag::Tag() {

			this->type = "radialGradient";
			this->attr_cx = cx;
			this->attr_cy = cy;
			this->attr_fx = fx;
			this->attr_fy = fy;
			this->attr_r = r;
		}
	};

	class SvgDefs :public Tag {
	public:
		SvgDefs() {
			this->type = "defs";
		}
	};

	class Svg :public Container {
		//svg widget - is a container for graphic widgets such as SvgCircle, SvgLine and so on.
	public:
		//@editor_attribute_decorator("WidgetSpecific", """preserveAspectRatio" """, "DropDown", {"possible_values": ("none","xMinYMin meet","xMidYMin meet","xMaxYMin meet","xMinYMid meet","xMidYMid meet","xMaxYMid meet","xMinYMax meet","xMidYMax meet","xMaxYMax meet","xMinYMin slice","xMidYMin slice","xMaxYMin slice","xMinYMid slice","xMidYMid slice","xMaxYMid slice","xMinYMax slice","xMidYMax slice","xMaxYMax slice")})
		TagProperty attr_preserveAspectRatio = TagProperty("preserveAspectRatio", &attributes);
		//@editor_attribute_decorator("WidgetSpecific", """viewBox of the svg drawing. es="x, y, width, height" """, "str", {})
		TagProperty attr_viewBox = TagProperty("viewBox", &attributes);

		Svg() :
			Container::Container() {
			this->type = "svg";
			setClass(CLASS_NAME(Svg));
		}

		void set_viewbox(float x, float y, float w, float h) {
			/*
			Sets the originand size of the viewbox, describing a virtual view area.

			Args :
			x(int) : x coordinate of the viewbox origin
			y(int) : y coordinate of the viewbox origin
			w(int) : width of the viewBox
			h(int) : height of the viewBox
			*/
			std::stringstream ss;
			ss << x << " " << y << " " << w << " " << h;
			this->attr_viewBox = ss.str();
			this->attr_preserveAspectRatio = "none";
		}
	};

	class SvgSubcontainer : public Svg, public ISvgPosition, public ISvgSize {
		//svg widget to nest within another Svg element - is a container for graphic widgets such as SvgCircle, SvgLineand so on.
	public:
		SvgSubcontainer(float x = 0, float y = 0, float width = 100, float height = 100) :
			Svg(),
			ISvgPosition(this),
			ISvgSize(this) {
			/*
			Args :
			width(float) : the viewBox width in pixel
			height(float) : the viewBox height in pixel
			*/
			this->set_position(x, y);
			this->set_size(width, height);
			this->setClass(CLASS_NAME(SvgSubcontainer));
		}
	};

	class SvgGroup :public Container, public ISvgStroke, public ISvgFill, public ISvgTransformable {
		/* svg group - a non visible container for svg widgets,
			this have to be appended into Svg elements. */
		SvgGroup() : Container(),
			ISvgStroke(this),
			ISvgFill(this),
			ISvgTransformable(this) {
			Tag::type = "g";
		}
	};

	class SvgRectangle :public Widget, public ISvgPosition, public ISvgSize, public ISvgStroke, public ISvgFill, public ISvgTransformable {
	public:
		//@editor_attribute_decorator("WidgetSpecific", """Horizontal round corners value.""", float, { "possible_values": "", "min" : 0.0, "max" : 10000.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_round_corners_h;
		//@editor_attribute_decorator("WidgetSpecific", """Vertical round corners value. Defaults to attr_round_corners_h.""", float, { "possible_values": "", "min" : 0.0, "max" : 10000.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_round_corners_y;

		SvgRectangle(float x = 0, float y = 0, float w = 100, float h = 100) :
			Widget::Widget(CLASS_NAME(SvgRectangle)),
			ISvgPosition(this),
			ISvgSize(this),
			ISvgStroke(this),
			ISvgFill(this),
			ISvgTransformable(this),
			attr_round_corners_h(TagProperty("rx", &this->attributes)),
			attr_round_corners_y(TagProperty("ry", &this->attributes)) {
			/*
			Args :
			x(float) : the x coordinate of the top left corner of the rectangle
			y(float) : the y coordinate of the top left corner of the rectangle
			w(float) : width of the rectangle
			h(float) : height of the rectangle
			*/
			ISvgPosition::set_position(x, y);
			ISvgSize::set_size(w, h);
			this->type = "rect";

		}
	};

	class SvgImage :public Widget, public ISvgPosition, public ISvgSize, public ISvgTransformable {
		/*svg image - a raster image element for svg graphics,
			this have to be appended into Svg elements.*/
	public:
		//@editor_attribute_decorator("WidgetSpecific", """preserveAspectRatio" """, "DropDown", {"possible_values": ("none","xMinYMin meet","xMidYMin meet","xMaxYMin meet","xMinYMid meet","xMidYMid meet","xMaxYMid meet","xMinYMax meet","xMidYMax meet","xMaxYMax meet","xMinYMin slice","xMidYMin slice","xMaxYMin slice","xMinYMid slice","xMidYMid slice","xMaxYMid slice","xMinYMax slice","xMidYMax slice","xMaxYMax slice")})
		TagProperty attr_preserveAspectRatio;
		//@editor_attribute_decorator("WidgetSpecific", """Image data or url  or a base64 data string, html attribute xlink:href""", "base64_image", {})
		TagProperty image_data;

		SvgImage(std::string image_data = "", float x = 0, float y = 0, float w = 100, float h = 100) :
			Widget::Widget(CLASS_NAME(SvgImage)),
			ISvgPosition(this),
			ISvgSize(this),
			ISvgTransformable(this),
			attr_preserveAspectRatio(TagProperty("preserveAspectRatio", &this->attributes)),
			image_data(TagProperty("xlink:href", &this->attributes)) {
			/*
			Args :
			image_data(str) : an url to an image
			x(float) : the x coordinate of the top left corner of the rectangle
			y(float) : the y coordinate of the top left corner of the rectangle
			w(float) : width of the rectangle
			h(float) : height of the rectangle
			kwargs : See Widget.__init__()
			*/
			Tag::type = "image";
			this->image_data = image_data;
			ISvgPosition::set_position(x, y);
			ISvgSize::set_size(w, h);
		}
	};

	class SvgCircle :public Widget, public ISvgStroke, public ISvgFill, public ISvgTransformable {
	public:
		//@editor_attribute_decorator("WidgetSpecific", """Center coordinate for SvgCircle.""", float, { "possible_values": "", "min" : -65535.0, "max" : 65535.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_cx;
		//@editor_attribute_decorator("WidgetSpecific", """Center coordinate for SvgCircle.""", float, { "possible_values": "", "min" : -65535.0, "max" : 65535.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_cy;
		//@editor_attribute_decorator("WidgetSpecific", """Radius of SvgCircle.""", float, { "possible_values": "", "min" : 0.0, "max" : 65535.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_r;

		SvgCircle(float x = 0, float y = 0, float radius = 50) :
			Widget::Widget(CLASS_NAME(SvgCircle)),
			ISvgStroke(this),
			ISvgFill(this),
			ISvgTransformable(this),
			attr_cx(TagProperty("cx", &this->attributes)),
			attr_cy(TagProperty("cy", &this->attributes)),
			attr_r(TagProperty("r", &this->attributes)) {
			/*
			Args :
			x(float) : the x center point of the circle
			y(float) : the y center point of the circle
			radius(float) : the circle radius
			*/

			this->set_position(x, y);
			this->set_radius(radius);
			this->type = "circle";
		}

		void set_radius(float radius) {
			/*
			Sets the circle radius.

			Args :
			radius(int) : the circle radius
			*/
			this->attr_r = radius;
		}

		void set_position(float x, float y) {
			/*
			Sets the circle position.

			Args :
			x(int) : the x coordinate
			y(int) : the y coordinate
			*/
			this->attr_cx = x;
			this->attr_cy = y;
		}
	};

	class SvgEllipse :public Widget, public ISvgStroke, public ISvgFill, public ISvgTransformable {
	public:
		//@editor_attribute_decorator("WidgetSpecific", """Coordinate for SvgEllipse.""", float, { "possible_values": "", "min" : -65535.0, "max" : 65535.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_cx;
		//@editor_attribute_decorator("WidgetSpecific", """Coordinate for SvgEllipse.""", float, { "possible_values": "", "min" : -65535.0, "max" : 65535.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_cy;
		//@editor_attribute_decorator("WidgetSpecific", """Radius of SvgEllipse.""", float, { "possible_values": "", "min" : 0.0, "max" : 10000.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_rx;
		//@editor_attribute_decorator("WidgetSpecific", """Radius of SvgEllipse.""", float, { "possible_values": "", "min" : 0.0, "max" : 65535.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_ry;
		SvgEllipse(float x = 0, float y = 0, float rx = 50, float ry = 30) :
			Widget::Widget(CLASS_NAME(SvgEllipse)),
			ISvgStroke(this),
			ISvgFill(this),
			ISvgTransformable(this),
			attr_cx(TagProperty("cx", &this->attributes)),
			attr_cy(TagProperty("cy", &this->attributes)),
			attr_rx(TagProperty("rx", &this->attributes)),
			attr_ry(TagProperty("ry", &this->attributes)) {
			/*
			Args :
			x(float) : the x center point of the ellipse
			y(float) : the y center point of the ellipse
			rx(float) : the ellipse radius
			ry(float) : the ellipse radius
			*/
			this->set_position(x, y);
			this->set_radius(rx, ry);
			this->type = "ellipse";
		}

		void set_radius(float rx, float ry) {
			/*
			Sets the ellipse radius.

			Args :
			rx(int) : the ellipse radius
			ry(int) : the ellipse radius
			*/
			this->attr_rx = rx;
			this->attr_ry = ry;
		}

		void set_position(float x, float y) {
			/*
			Sets the ellipse position.

			Args :
			x(int) : the x coordinate
			y(int) : the y coordinate
			*/
			this->attr_cx = x;
			this->attr_cy = y;
		}
	};

	class SvgLine :public Widget, public ISvgStroke, public ISvgTransformable {
	public:
		//@editor_attribute_decorator("WidgetSpecific", """P1 coordinate for SvgLine.""", float, { "possible_values": "", "min" : -65535.0, "max" : 65535.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_x1;
		//@editor_attribute_decorator("WidgetSpecific", """P1 coordinate for SvgLine.""", float, { "possible_values": "", "min" : -65535.0, "max" : 65535.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_y1;
		//@editor_attribute_decorator("WidgetSpecific", """P2 coordinate for SvgLine.""", float, { "possible_values": "", "min" : -65535.0, "max" : 65535.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_x2;
		//@editor_attribute_decorator("WidgetSpecific", """P2 coordinate for SvgLine.""", float, { "possible_values": "", "min" : -65535.0, "max" : 65535.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_y2;

		SvgLine(float x1 = 0, float y1 = 0, float x2 = 50, float y2 = 50) :
			Widget::Widget(CLASS_NAME(SvgEllipse)),
			ISvgStroke(this),
			ISvgTransformable(this),
			attr_x1(TagProperty("x1", &this->attributes)),
			attr_y1(TagProperty("y1", &this->attributes)),
			attr_x2(TagProperty("x2", &this->attributes)),
			attr_y2(TagProperty("y2", &this->attributes)) {

			this->set_coords(x1, y1, x2, y2);
			Tag::type = "line";
		}

		void set_coords(float x1, float y1, float x2, float y2) {
			this->set_p1(x1, y1);
			this->set_p2(x2, y2);
		}

		void set_p1(float x1, float y1) {
			this->attr_x1 = x1;
			this->attr_y1 = y1;
		}

		void set_p2(float x2, float y2) {
			this->attr_x2 = x2;
			this->attr_y2 = y2;
		}
	};

	class SvgPolyline :public Widget, public ISvgStroke, public ISvgFill, public ISvgTransformable {
	public:
		//@editor_attribute_decorator("WidgetSpecific", """Defines the maximum values count.""", int, { "possible_values": "", "min" : 0, "max" : 65535, "default" : 0, "step" : 1 })
		std::deque<float> coordsX;
		std::deque<float> coordsY;
		int __maxlen;
		int maxlen() {
			return this->__maxlen;
		}
		void maxlen(int value) {
			this->__maxlen = value;
			this->coordsX = std::deque<float>(this->__maxlen);
			this->coordsY = std::deque<float>(this->__maxlen);
		}

		SvgPolyline(int _maxlen = 1000) :
			Widget::Widget(CLASS_NAME(SvgPolyline)),
			ISvgStroke(this),
			ISvgFill(this),
			ISvgTransformable(this) {
			this->__maxlen = 0;
			Tag::type = "polyline";
			this->maxlen(_maxlen);
			Tag::attributes["points"] = "";
			Tag::attributes["vector-effect"] = "non-scaling-stroke";
		}

		void add_coord(float x, float y) {
			int spacepos = 0;
			if (this->coordsX.size() == this->maxlen()) {
				spacepos = static_cast<std::string>(Tag::attributes["points"]).find(" ", 0);
			}
			if (spacepos > 0) {
				Tag::attributes["points"] = static_cast<std::string>(Tag::attributes["points"]).substr(spacepos + 1);
			}
			this->coordsX.push_back(x);
			this->coordsY.push_back(y);
			std::stringstream ss;
			ss << static_cast<std::string>(Tag::attributes["points"]) << std::to_string(x) << "," << std::to_string(y) << " ";
			Tag::attributes["points"] = ss.str();
		}
	};

	class SvgPolygon :public SvgPolyline {
	public:
		SvgPolygon(int _maxlen = 1000) : SvgPolyline(_maxlen) {
			Tag::type = "polygon";
			Widget::setClass(CLASS_NAME(SvgPolygon));
		}
	};

	class SvgText : public TextWidget, public ISvgPosition, public ISvgStroke, public ISvgFill, public ISvgTransformable {

	public:
		//@editor_attribute_decorator("WidgetSpecific", """Length for svg text elements.""", int, { "possible_values": "", "min" : 0.0, "max" : 10000.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_textLength;
		//@editor_attribute_decorator("WidgetSpecific", """Controls how text is stretched to fit the length.""", "DropDown", { "possible_values": ("spacing","spacingAndGlyphs") })
		TagProperty attr_lengthAdjust;
		//@editor_attribute_decorator("WidgetSpecific", """Rotation angle for svg elements.""", float, { "possible_values": "", "min" : -360.0, "max" : 360.0, "default" : 1.0, "step" : 0.1 })
		TagProperty attr_rotate;
		//@editor_attribute_decorator("WidgetSpecific", """Description.""", "DropDown", { "possible_values": ("start", "middle", "end") })
		TagProperty attr_text_anchor;
		//@editor_attribute_decorator("WidgetSpecific", """Description.""", "DropDown", { "possible_values": ("auto", "text-bottom", "alphabetic", "ideographic", "middle", "central", "mathematical", "hanging", "text-top") })
		TagProperty attr_dominant_baseline;

		SvgText(float x = 10, float y = 10, std::string text = "svg text") :
			TextWidget(),
			ISvgPosition(this),
			ISvgStroke(this),
			ISvgFill(this),
			ISvgTransformable(this),
			attr_textLength(TagProperty("textLength", &this->attributes)),
			attr_lengthAdjust(TagProperty("lengthAdjust", &this->attributes)),
			attr_rotate(TagProperty("rotate", &this->attributes)),
			attr_text_anchor(TagProperty("text-anchor", &this->attributes)),
			attr_dominant_baseline(TagProperty("dominant-baseline", &this->attributes)) {
			setClass(CLASS_NAME(SvgText)),
				this->type = "text";
			ISvgPosition::set_position(x, y);
			this->setText(text);
		}
	};

	class SvgPath :public Widget, public ISvgStroke, public ISvgFill, public ISvgTransformable {
	public:
		//@editor_attribute_decorator("WidgetSpecific", """Instructions for SvgPath.""", str, {})
		TagProperty attr_d;

		SvgPath(std::string path_value = "") :
			Widget::Widget(CLASS_NAME(SvgPath)),
			ISvgStroke(this),
			ISvgFill(this),
			ISvgTransformable(this),
			attr_d(TagProperty("d", &this->attributes)) {
			this->type = "path";
			static_cast<Widget*>(this)->attributes["d"] = path_value;
		}

		void add_position(float x, float y) {
			std::stringstream ss;
			ss << static_cast<std::string>(Widget::attributes["d"]) << "M " << x << " " << y;
			Widget::attributes["d"] = ss.str();
		}

		void add_arc(float x, float y, float rx, float ry, float x_axis_rotation, bool large_arc_flag, bool sweep_flag) {
			// A rx ry x - axis - rotation large - arc - flag sweep - flag x y
			std::stringstream ss;
			//"A %(rx)s %(ry)s, %(x-axis-rotation)s, %(large-arc-flag)s, %(sweep-flag)s, %(x)s %(y)s"% {"x":x, "y" : y, "rx" : rx, "ry" : ry, "x-axis-rotation" : x_axis_rotation, "large-arc-flag" : large_arc_flag, "sweep-flag" : sweep_flag}
			ss << static_cast<std::string>(Widget::attributes["d"]) << "A " << rx << " " << ry << ", " << x_axis_rotation << ", " << large_arc_flag << ", " << sweep_flag << ", " << x << " " << y;
			Widget::attributes["d"] = ss.str();
		}
	};

}

#endif //CPORT_REMI_H
