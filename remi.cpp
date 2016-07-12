//
// Created by CyberPro4 on 24/06/2016.
//


#include "remi.h"

#include <stdarg.h>
#include <memory>

#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

#include <regex>

using namespace remi;

int		remi_timestamp(){
#ifdef _WIN32

	FILETIME ft = {0};
  
    GetSystemTimeAsFileTime(&ft);

    LARGE_INTEGER li = {0};    

    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    long long int hns = li.QuadPart;

	return (int)(hns / 1000 / 1000);


#endif
}

remi_thread   remi_createThread( remi_thread_callback callback , remi_thread_param param ){

	#ifdef _WIN32
		return CreateThread( NULL , 0 , callback , param , 0 , 0 );
	#endif

	#ifdef __unix__
		pthread_t       th;
		pthread_create( &th , NULL , callback , param );
		return th;
	#endif

}

const std::string Widget::Event_OnClick = "OnClick";
const std::string Widget::Event_OnDblClick = "OnDblClick";
const std::string Widget::Event_OnMouseDown = "OnMouseDown";
const std::string Widget::Event_OnMouseMove = "OnMouseMove";
const std::string Widget::Event_OnMouseOver = "OnMouseOver";
const std::string Widget::Event_OnMouseOut = "OnMouseOut";
const std::string Widget::Event_OnMouseLeave = "OnMouseLeave";
const std::string Widget::Event_OnMouseUp = "OnMouseUp";
const std::string Widget::Event_OnTouchMove = "OnTouchMove";
const std::string Widget::Event_OnTouchStart = "OnTouchStart";
const std::string Widget::Event_OnTouchEnd = "OnTouchEnd";
const std::string Widget::Event_OnTouchEnter = "OnTouchEnter";
const std::string Widget::Event_OnTouchLeave = "OnTouchLeave";
const std::string Widget::Event_OnTouchCancel = "OnTouchCancel";
const std::string Widget::Event_OnKeyDown = "OnKeyDown";
const std::string Widget::Event_OnKeyPress = "OnKeyPress";
const std::string Widget::Event_OnKeyUp = "OnKeyUp";
const std::string Widget::Event_OnChange = "OnChange";
const std::string Widget::Event_OnFocus = "OnFocus";
const std::string Widget::Event_OnBlur = "OnBlur";
const std::string Widget::Event_OnContextMenu = "OnContextMenu";
const std::string Widget::Event_OnUpdate = "OnUpdate";

std::string remi::utils::base64( std::string str ){

  BIO *bmem, *b64;
  BUF_MEM *bptr;

  b64 = BIO_new(BIO_f_base64());
  bmem = BIO_new(BIO_s_mem());
  b64 = BIO_push(b64, bmem);
  BIO_write(b64, str.c_str(), str.length() );
  BIO_flush(b64);
  BIO_get_mem_ptr(b64, &bptr);

  char *buff = (char *)malloc(bptr->length);
  memcpy(buff, bptr->data, bptr->length-1);
  buff[bptr->length-1] = 0;

  BIO_free_all(b64);

  return buff;

}

void remi::utils::open_browser( std::string url ){

#ifdef _WIN32

	ShellExecuteA( NULL , "open" , url.c_str() , NULL , NULL , 0 );

#endif

#if defined(__unix__)

    std::string cmd = "xdg-open " + url;

    system( cmd.c_str() );

#endif

}

std::string remi::utils::url_decode( std::string from ){
	std::smatch match;
	std::string out = from;

	while( std::regex_search( out, match, std::regex("%[0-9A-F]{2}") ) ){
		int v = 0;

		sscanf( 
			out.substr( match.position(0) +1 , 2 ).c_str(),
			"%x" , &v );

		char r[2] = {0};
		r[0] = (char)v;

		out.replace( match.position(0) , match.length(0) , r );
	}

	return out;
}

std::string remi::utils::toPix( int v ){
    std::ostringstream ss;
    ss << v << "px";
    return ss.str();
}

std::list<std::string> remi::utils::split( std::string subject , std::string delimiter ){
	std::list<std::string> list;

	size_t found = 0;
	size_t start = 0;

	while( true ){

		size_t nf = subject.find( delimiter, found );
		if( nf == std::string::npos ){
			list.push_back( subject.substr( found , subject.length() - found ) );
			break;
		} else {
			list.push_back( subject.substr( found , nf - found ) );
			found = nf + delimiter.length();
		}
	}

	return list;
}

std::string remi::utils::join(std::list<std::string> stringList , std::string glue ){
    std::ostringstream ss;

    bool first = true;
    for( std::string s : stringList ){
        if( !first )
            ss << glue;
        ss << s;
        first = false;
    }

    return ss.str();
}

std::string remi::utils::string_encode(std::string text){
	return text;
}

int remi::utils::sscan( std::string from , std::string format , ... ){

	va_list ap;

	va_start( ap, format );

	int r = vsscanf( from.c_str() , format.c_str() , ap );

	va_end(ap);

	return r;
}

std::string remi::utils::sformat( std::string fmt_str , ... ){

	// From http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf

	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
    std::string str;
    std::unique_ptr<char[]> formatted;
    va_list ap;
    while(1) {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return std::string(formatted.get());

}



utils::Timer::Timer( int millisecondsInterval, TimerListener* listener ){
    _stopFlag = false;

    _millisecondsInterval = millisecondsInterval;

	_listener = listener;

	start();
}

remi_thread_result utils::Timer::thread_entry( remi_thread_param instance ){
	((utils::Timer*)instance)->tick();
	return 0;
}

double remi::utils::Timer::elapsed(){
	return std::chrono::duration_cast<second_>
            (clock_::now() - _start).count();
}

void remi::utils::Timer::setInterval( int msecInterval ){
	_millisecondsInterval = msecInterval;
}

bool utils::Timer::has_passed(){
	return elapsed() > _millisecondsInterval;
}

void remi::utils::Timer::tick(){
	while( !_stopFlag ){
		if( has_passed() ){
			_passed = true;
			if( _listener != NULL ){
				_listener->timer();
			}

			stop();
		}

		Sleep( 2 );
	}

	
}

void remi::utils::Timer::start(){
	_start = clock_::now();
    _stopFlag = false;
	_passed = false;

	if( _listener != NULL ){
		remi_createThread( (remi_thread_callback)&utils::Timer::thread_entry, this );
	}

}

void remi::utils::Timer::stop(){
    _stopFlag = true;
}




std::string remi::utils::join( Dictionary<std::string> from , std::string nameValueGlue , std::string itemsGlue ){
    std::ostringstream out;

    bool first = true;

    for( std::string key : from.keys() ){
        if( !first )
            out << itemsGlue;
        first = false;
        out << key << nameValueGlue << from.get(key);
    }

    return out.str();
}

std::string remi::utils::toCss( Dictionary<std::string> values ){
    return join( values , ": " , ";" );
}




EventManagerListener::EventManagerListener(){}

EventEmitter::EventEmitter(){}

void EventEmitter::propagate( std::string eventName, void* params ){
    if( _listeners.has( eventName ) == false )
        return;

    _listeners.get( eventName )->onEvent( eventName );
}

void EventEmitter::propagate( std::string eventName ){
    propagate( eventName , NULL );
}

void EventEmitter::registerListener( std::string eventName , EventManagerListener* listener, void* funcName ){
    _listeners.set( eventName , listener );
}

StringRepresantable::StringRepresantable(std::string v ){
    this->v = v;
}

std::string StringRepresantable::repr(){
    return v;
}



Tag::Tag(){
    _type = "div";
	attributes.set( "id" , utils::sformat( "%d", (int)this ) );
}

Tag::Tag( std::string type ){
    _type = type;
	attributes.set( "id" , utils::sformat( "%d", (int)this ) );
}

void Tag::addClass( std::string name ){
    _classes.push_front( name );
}

void Tag::removeClass( std::string name ){
    _classes.remove( name );
}

std::string Tag::getIdentifier(){
    if( attributes.has( "id" ) )
        return attributes["id"];
    return "";
}


std::string Tag::repr(){

    std::ostringstream  html;

    if( style.size() > 0 )
        this->attributes.set( "style" , utils::toCss( style ) );
    else
        this->attributes.remove( "style" );

    std::ostringstream attributes;
    bool first=true;
    for( std::string key : this->attributes.keys() ){
        if(!first)
            attributes << " ";
        attributes << key << "=\"" << this->attributes.get(key) << "\"";
        first = false;
    }

    std::ostringstream innerHtml;

    for( Represantable* represantable : _render_children_list ){
        innerHtml << represantable->repr();
    }

    html << "<" << _type <<
    " " << attributes.str() <<
    " class=\"" << utils::join(_classes, " ") << "\""
            ">" << innerHtml.str() << "</" << _type << ">";

    return html.str();
}

void Tag::addChild( std::string key, Tag* child ){

    if( child == NULL || key.length() < 1 )
        return;

    child->attributes.set( "parent_widget" , this->getIdentifier() );

    if( children.has( key ) ){
        _render_children_list.remove( children.get(key) );
    }

    _render_children_list.push_front( child );
    children.set( key, child );
}

Represantable * Tag::getChild(std::string key ){
    return children[key];
}




Widget::Widget() : Tag() {
    defaults();
}

Widget::Widget( std::string type ) {
    defaults();
}

void Widget::setSize( int width, int height ){
    style.set( "width" , utils::toPix( width ) );
    style.set( "height" , utils::toPix( height ) );
}

void Widget::setLayoutOrientation(Widget::Layout orientation){
    _layout_orientation = orientation;
}

void Widget::redraw(){
    //TODO server . update_event set()													   
}																						   
																						   
void Widget::addChild( std::string key, Tag* child ){									   
																						   
    if( _layout_orientation == Widget::Layout::Horizontal ){							   
        /*																				   
         * From python this make no sense at all:										   
         * Maybe for VersionedDictionary?												   
         *																				   
         * if 'float' in self.children[key].style.keys():								   
         *       if not (self.children[key].style['float'] == 'none'):					   
         *           self.children[key].style['float'] = 'left'							   
         *   else:																		   
         *       self.children[key].style['float'] = 'left'								   
		 *																				   
         */																				   
																						   
        child->style.set( "float" , "left" );											   
    }																					   
																						   
    Tag::addChild( key , child );														   
}

void Widget::onFocus(){
    propagate( Widget::Event_OnFocus );
}

void Widget::setOnFocusListener( void* listener , void* fname ){

    /*self.attributes[self.EVENT_ONFOCUS] = \
        "sendCallback('%s','%s');" \
        "event.stopPropagation();event.preventDefault();" \
        "return false;" % (self.identifier, self.EVENT_ONFOCUS)
    self.eventManager.register_listener(self.EVENT_ONFOCUS, listener, funcname)*/

}

void Widget::setOnClickListener( EventManagerListener* listener ){

	std::ostringstream oss;
	
	oss << "sendCallback( '" << getIdentifier() << "', '" << Widget::Event_OnClick << "' );" ;
	oss << "event.stopPropagation();event.preventDefault();";
	
	attributes.set( Widget::Event_OnClick , oss.str() );
	
	registerListener( Widget::Event_OnClick, listener );
}


void Widget::defaults(){
    _layout_orientation = Layout::Vertical;
    style.set( "margin" , "0px auto" );
}


/*void Widget::setEventListener(std::string eventName, EventManagerListener *listener) {

    attributes.set( eventName , "sendCallBack();event.stopPropagation();event.preventDefault();return false;");

    _eventManager.registerListener( eventName , listener );
}*/


HBox::HBox() : Widget(){

	style.set("display", "flex");
	style.set("justify-content" , "space-around" );
	style.set("align-items" , "center" );
	style.set("flex-direction" , "row" );

}