//
// Created by CyberPro4 on 24/06/2016.
//


#include "remi.h"
#include "remi_server.h"

#include <stdarg.h>
#include <memory>

#include "TinySHA1.hpp"

#include <regex>

using namespace remi;

long long int	remi_timestamp(){
#ifdef _WIN32

	FILETIME ft = {0};
  
    GetSystemTimeAsFileTime(&ft);

    LARGE_INTEGER li = {0};    

    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    long long int hns = li.QuadPart;

	return (int)(hns / 1000 / 1000);
#else

	return time( NULL );

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

const std::string TextInput::Event_OnEnter = "OnEnter";

const std::string FileUploader::Event_OnSuccess = "OnSuccess";
const std::string FileUploader::Event_OnFail = "OnFail";
const std::string FileUploader::Event_OnData = "OnData";

std::string remi::utils::SHA1(std::string& val){
	sha1::SHA1 s;
	s.processBytes(val.c_str(), val.size());
	uint32_t digest[5];
	s.getDigest(digest);

	char sha1_data[21];
	sha1_data[3] = (digest[0] >> 0) & 0xff;
	sha1_data[2] = (digest[0] >> 8) & 0xff;
	sha1_data[1] = (digest[0] >> 16) & 0xff;
	sha1_data[0] = (digest[0] >> 24) & 0xff;

	sha1_data[7] = (digest[1] >> 0) & 0xff;
	sha1_data[6] = (digest[1] >> 8) & 0xff;
	sha1_data[5] = (digest[1] >> 16) & 0xff;
	sha1_data[4] = (digest[1] >> 24) & 0xff;

	sha1_data[11] = (digest[2] >> 0) & 0xff;
	sha1_data[10] = (digest[2] >> 8) & 0xff;
	sha1_data[9] = (digest[2] >> 16) & 0xff;
	sha1_data[8] = (digest[2] >> 24) & 0xff;

	sha1_data[15] = (digest[3] >> 0) & 0xff;
	sha1_data[14] = (digest[3] >> 8) & 0xff;
	sha1_data[13] = (digest[3] >> 16) & 0xff;
	sha1_data[12] = (digest[3] >> 24) & 0xff;

	sha1_data[19] = (digest[4] >> 0) & 0xff;
	sha1_data[18] = (digest[4] >> 8) & 0xff;
	sha1_data[17] = (digest[4] >> 16) & 0xff;
	sha1_data[16] = (digest[4] >> 24) & 0xff;

	sha1_data[20] = 0;
	
	return std::string(sha1_data);
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
	std::locale loc(std::locale(), new std::codecvt_utf8<char>);
	std::ostringstream o;
	o.imbue(loc);
	o << text;
	return o.str();
}

std::string remi::utils::escape_json(const std::string &s) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (char i : s) {
		std::string::value_type c = i;

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << std::uppercase;
		escaped << '%' << std::setw(2) << int((unsigned char)c);
		escaped << std::nouppercase;
	}

	return escaped.str();
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

utils::Timer::Timer(){
	_stopFlag = false;

    _millisecondsInterval = 0;

	_listener = NULL;
}

void utils::Timer::setListener( TimerListener* listener ){
	_listener = listener;
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
	return elapsed() > (_millisecondsInterval/1000.f);
}

void remi::utils::Timer::tick(){
	while( !_stopFlag ){
		if( has_passed() ){
			_passed = true;
			if( _listener != NULL ){
				_listener->onTimer();
			}

			//stop();
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




std::string remi::utils::join( Dictionary<std::string>& from , std::string nameValueGlue , std::string itemsGlue ){
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

std::string remi::utils::toCss( Dictionary<std::string>& values ){
    return join( values , ": " , ";" );
}


Event::Event(){
	source = NULL;
}

Event::Event( std::string name ){
	source = NULL;
	this->name = name;
}

EventListener::EventListener(){}

EventDispatcher::EventDispatcher(){}

void EventDispatcher::onEvent( std::string eventName , Event* eventData ){

	if( _listeners.has( eventData->name ) == false )
        return;

    _listeners.get( eventData->name )->onEvent( eventData->name , eventData );
}

void EventDispatcher::registerListener( std::string eventName , EventListener* listener, void* funcName ){
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

void Tag::addChild( Represantable* child , std::string key ){

    if( child == NULL )
        return;

	std::string _key = key;
	if( _key.length() < 1 ){
		_key = utils::sformat( "%d" , (int)(void*)child );
	}

	if( dynamic_cast<Tag*>(child) != 0 ){
		Tag* _tag = dynamic_cast<Tag*>(child);
		_tag->attributes.set( "parent_widget" , this->getIdentifier() );
	}
    

    if( children.has( _key ) ){
        _render_children_list.remove( children.get(_key) );
    }

    _render_children_list.push_back( child );
    children.set( _key, child );
}

void Tag::addChild( std::string child, std::string key ){
	addChild( new StringRepresantable( child ) , key );
}

Represantable * Tag::getChild(std::string key ){
    return children[key];
}

void Tag::setUpdated(){
	attributes.alignVersion(); style.alignVersion(); children.alignVersion();
	for (Represantable* represantable : _render_children_list){
		if (dynamic_cast<Tag*>(represantable) != 0){
			Tag* _tag = dynamic_cast<Tag*>(represantable);
			_tag->setUpdated();
		}
	}
}


Widget::Widget() : Tag() {
    defaults();
}

Widget::Widget( std::string type ) {
    defaults();
}

void Widget::setWidth( int width ){
	style.set( "width" , utils::toPix( width ) );
}

void Widget::setHeight( int height ){
	style.set( "height" , utils::toPix( height ) );
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
								
void Widget::setParentApp( remi::server::App* app ){

	_parentApp = app;

	for( std::string key : children.keys() ){

		Widget* widget = dynamic_cast<Widget*>(children.get(key));

		if( widget == 0 )
			continue;

		widget->setParentApp( app );
	}

}

void Widget::hide(){
	if( _parentApp != NULL )
		_parentApp->showRoot();
}

void Widget::show( server::App* app ){
	if( app != NULL ){
		_parentApp = app;
		_parentApp->show( this );
	}
}

void Widget::addChild( Represantable* child, std::string key ){					   
																						   
    if( _layout_orientation == Widget::Layout::Horizontal && dynamic_cast<Tag*>(child) != NULL ){							   
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
																						   
        dynamic_cast<Tag*>(child)->style.set( "float" , "left" );											   
	}

	if( dynamic_cast<Widget*>(child) != 0 ){
		dynamic_cast<Widget*>(child)->setParentApp( _parentApp );
	}
																						   
    Tag::addChild( child , key );														   
}

void Widget::onEvent(std::string name, Event* event){

	if (name == Widget::Event_OnClick){
		if(onClickListener!=NULL)onClickListener->onClick(this);
	}
	else if (name == Widget::Event_OnDblClick){
		if(onDblClickListener != NULL)onDblClickListener->onDblClick(this);
	}
	else if (name == Widget::Event_OnChange){
		if (onChangeListener != NULL)onChangeListener->onChange(this);
	}
	/*
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
	*/
}

void Widget::defaults(){
    _layout_orientation = Layout::Vertical;
    style.set( "margin" , "0px auto" );

	attributes[Widget::Event_OnClick] = utils::sformat(
		"sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();",
		getIdentifier().c_str(), Widget::Event_OnClick.c_str());

	attributes[Widget::Event_OnDblClick] = utils::sformat(
		"sendCallback( '%s', '%s' );event.stopPropagation();event.preventDefault();",
		getIdentifier().c_str(), Widget::Event_OnDblClick.c_str());


	onClickListener = NULL;
	onDblClickListener = NULL;
	onChangeListener = NULL;
	onMouseDownListener = NULL;
	onMouseMoveListener = NULL;
	onMouseOverListener = NULL;
	onMouseOutListener = NULL;
	onMouseLeaveListener = NULL;
	onMouseUpListener = NULL;
	onTouchMoveListener = NULL;
	onTouchStartListener = NULL;
	onTouchEndListener = NULL;
	onTouchEnterListener = NULL;
	onTouchLeaveListener = NULL;
	onTouchCancelListener = NULL;
	onKeyDownListener = NULL;
	onKeyPressListener = NULL;
	onKeyUpListener = NULL;
	onChangeListener = NULL;
	onFocusListener = NULL;
	onBlurListener = NULL;
	onContextMenuListener = NULL;
	onUpdateListener = NULL;

	_parentApp = 0;
}




HBox::HBox() : Widget(){

	style["display"] = "flex";

	style["justify-content"] = "space-around";

	style["align-items"] = "center";

	style["flex-direction"] = "row";

}

VBox::VBox() : Widget(){

	style["display"] = "flex";

	style["justify-content"] = "space-around";

	style["align-items"] = "center";

	style["flex-direction"] = "column";

}

Button::Button( std::string text ) : TextWidget(){

	_type = "button";

	setText(text);

	attributes[Widget::Event_OnClick] = utils::sformat(
		"sendCallback('%s','%s');",
		getIdentifier().c_str(), Widget::Event_OnClick.c_str()
	);
}

void Button::setEnabled( bool en ){
	if( en ){
		attributes["disabled"] = "disabled";
	} else {
		attributes.remove("disabled");
	}
}

bool Button::enabled(){
	return attributes.has("disabled");
}

TextInput::TextInput( bool single_line ){

	_type = "textarea";

	attributes[Widget::Event_OnClick] = "";
	attributes[Widget::Event_OnChange] = utils::sformat(
		"var params={};params['new_value']=document.getElementById('%s').value; \
		sendCallbackParam('%s','%s',params);", 
		getIdentifier().c_str(), getIdentifier().c_str(), Widget::Event_OnChange.c_str()
		);

	//The onEnter event will be managed only in case of single_line
	if (single_line){
		attributes[TextInput::Event_OnKeyDown] = utils::sformat("\
		if (event.keyCode == 13) { \
			var params={};\
			params['new_value']=document.getElementById('%s').value; \
			document.getElementById('%s').value = ''; \
			document.getElementById('%s').onchange = ''; \
			sendCallbackParam('%s','%s',params); \
			return false; \
		}",
		getIdentifier().c_str(), getIdentifier().c_str(),
		getIdentifier().c_str(), getIdentifier().c_str(),
		TextInput::Event_OnEnter.c_str()
		);
	}else{
		attributes[Widget::Event_OnKeyDown] = utils::sformat(
			"var params={};params['new_value']=document.getElementById('%s').value;" \
			"sendCallbackParam('%s','%s',params);",
			getIdentifier().c_str(), getIdentifier().c_str(), Widget::Event_OnKeyDown.c_str()
			);
	}

	setText("");

	if( single_line ){
		attributes["resize"] = "none";
		attributes["rows"] = "1";
	}

	// TODO: hint?
}

void TextInput::onEvent( std::string name , Event* event ){

	if( name == Widget::Event_OnChange && event->params.has("new_value") ){
		setText( event->params["new_value"] );
		if (onChangeListener!=NULL)onChangeListener->onChange(this);
	}else if (name == TextInput::Event_OnEnter && event->params.has("new_value")){
		setText(event->params["new_value"]);
		if(onEnterListener!=NULL)onEnterListener->onEnter(this, event->params["new_value"]);
	}else if (name == TextInput::Event_OnKeyDown && event->params.has("new_value")){
		setText(event->params["new_value"]); 
		//setting up this value with setText causes the refresh of clients and so the focus gets lost on the input field
		this->setUpdated();
		if (onKeyDownListener != NULL)onKeyDownListener->onKeyDown(this);
	}

	Widget::onEvent( name , event );
}

void TextInput::setPlaceholder( std::string text ){
	attributes["placeholder"] = text;
}

std::string TextInput::placeholder(){
	return  attributes["placeholder"];
}

void TextWidget::setText(std::string text){
	((Tag*)this)->addChild(text, std::string("text"));
}

std::string TextWidget::text(){
	Represantable* repr = getChild("text");
	return ((StringRepresantable*)repr)->repr();
}

Label::Label( std::string text ){
	_type = "p";
	setText( text );
}

GenericDialog::GenericDialog( std::string title , std::string message ){
	setLayoutOrientation( Widget::Layout::Vertical );
	style["display"] = "block";
	style["overflow"] = "auto";

	onConfirmListener = NULL;
	onCancelListener = NULL;

	if( title.length() > 0 ){
		Label *l = new Label( title );
		l->addClass( "DialogTitle" ); //FIXME: css class named "DialogTitle" should be "GenericDialogTitle"?
		addChild( l );
	}

	if( message.length() > 0 ){
		Label *l = new Label( message );
		l->style["margin"] = "5px";
		addChild( l );
	}

	_container = new Widget();
	_container->style["display"] = "block";
	_container->style["overflow"] = "auto";
	_container->style["margin"] = "5px";
	_container->setLayoutOrientation(Widget::Layout::Vertical);

	_confirmButton = new Button("Ok");
	_confirmButton->setSize(100, 30);
	_confirmButton->style["margin"] = "3px";
	_confirmButton->onClickListener = this;

	_cancelButton = new Button("Cancel");
	_cancelButton->setSize(100, 30);
	_cancelButton->style["margin"] = "3px";
	_cancelButton->onClickListener = this;

	_hLay = new Widget();
	_hLay->setHeight( 35 );
	_hLay->style["display"] = "block";
	_hLay->style["overflow"] = "visible";
    _hLay->addChild(_confirmButton);
	_hLay->addChild(_cancelButton);

	_confirmButton->style["float"] = "right";
	_cancelButton->style["float"] = "right";

	addChild(_container);
	addChild(_hLay);

}

void GenericDialog::onClick(Widget* w){
	if (w==_confirmButton){
		if (onConfirmListener!= NULL)onConfirmListener->onConfirm(this);
	}else if (w==_cancelButton){
		if (onCancelListener!=NULL)onCancelListener->onCancel(this);
	}
}

void GenericDialog::addFieldWithLabel(std::string key, std::string label_description, Widget* field){
	this->_inputs[key] = field;
	Label* label = new Label(label_description);
	label->style.set("margin", "0px 5px");
	label->style.set("min - width", "30%");
	HBox* container = new HBox();
	container->style.set("overflow", "auto");
	container->style.set("padding", "3px");
	container->addChild(label, "lbl" + key);
	container->addChild(this->_inputs[key], key);
	this->_container->addChild(container, key);
}

void GenericDialog::addField(std::string key, Widget* field){
	this->_inputs[key] = field;
	Widget* container = new Widget();
	container->style.set("display", "block");
	container->style.set("overflow", "auto");
	container->style.set("padding", "3px");
	container->setLayoutOrientation(Widget::Layout::Horizontal);
	container->addChild(this->_inputs[key], key);
	this->_container->addChild(container, key);
}

Widget* GenericDialog::getField(std::string key){
	return this->_inputs[key];
}


InputDialog::InputDialog(std::string title, std::string message) : GenericDialog(title, message), _inputText(true){
	
	_inputText.onEnterListener = this;

	addField("textinput", &_inputText);

	_inputText.setText("");
}

std::string InputDialog::text(){
	return _inputText.text();
}

void InputDialog::setText(std::string text){
	_inputText.setText(text);
}

//lisener function for TextInputOnEnterListener interface
void InputDialog::onEnter(TextInput* w, std::string text){
	onConfirmListener->onConfirm(this);
	hide();
}

ListView::ListView(){
	_type = "ul";

	onSelectionListener = NULL;

	this->selectedItem = NULL;
}

void ListView::addChild(Represantable* child, std::string key){
	((ListItem*)child)->onClickListener = this;
	Tag::addChild(child, key);
}

void ListView::onEvent(std::string name, Event* event){
}

void ListView::onClick(Widget* widget){
	selectItem(dynamic_cast<ListItem*>(widget));

	if (onSelectionListener != NULL)onSelectionListener->onSelection(this, this->selectedItem);
}

void ListView::selectByKey(std::string key){
	
	if (!children.has(key))return;
	
	selectItem(dynamic_cast<ListItem*>(children.get(key)));
	
}

void ListView::selectItem(ListItem* item){
	if (this->selectedItem != NULL)this->selectedItem->attributes.remove("selected");

	this->selectedItem = item;
	this->selectedItem->attributes["selected"] = "true";
}



ListItem::ListItem(std::string text){
	_type = "li";
	setText( text );
}

Image::Image(std::string url){
	_type = "img";

	setURL(url);
}

void Image::setURL(std::string url){
	this->attributes["src"] = url;
}

std::string Image::url(){
	return this->attributes["src"];
}

Input::Input(){
	_type = "input";

	attributes[Event_OnClick] = "";
	attributes[Event_OnChange] = utils::sformat( \
            "var params={};params['value']=document.getElementById('%(id)s').value;" \
            "sendCallbackParam('%s','%s',params);" , getIdentifier().c_str() , Event_OnChange.c_str() );
}

void Input::setValue( std::string value ){
	attributes["value"] = value;
}

std::string Input::getValue(){
	return attributes["value"];
}

void Input::setEnable( bool on ){

	if( on )
		attributes.remove("disabled");
	else
		attributes["disabled"] = "disabled";

}

bool Input::isEnable(){
	return attributes.has("disabled");
}

void Input::setReadOnly( bool on ){

	if( on )
		attributes.remove("readonly");
	else
		attributes["readonly"] = "readonly";

}

bool Input::isReadOnly(){
	return attributes.has("readonly");
}


FileUploader::FileUploader( std::string path, bool multipleSelectionAllowed ){
	
	onSuccessListener = NULL;
	onFailListener = NULL;
	onDataListener = NULL;

	setSavePath(path);
	setMultipleSelectionAllowed(multipleSelectionAllowed);

	_type = "input";
	this->attributes["type"] = "file";
	this->attributes["accept"] = "*.*";

	attributes[Widget::Event_OnChange] = utils::sformat(
		"var files = this.files;" \
		"for(var i=0; i<files.length; i++){" \
		"uploadFile('%s','%s','%s','%s',files[i]);}",
		getIdentifier().c_str(), FileUploader::Event_OnSuccess.c_str(), FileUploader::Event_OnFail.c_str(), FileUploader::Event_OnData.c_str()
	);

	this->attributes[Widget::Event_OnClick] = "event.stopPropagation();";
	this->attributes[Widget::Event_OnDblClick] = "event.stopPropagation();";
}

void FileUploader::setSavePath(std::string path){
	this->_path = path;
}
std::string FileUploader::savePath(){
	return this->_path;
}

void FileUploader::setMultipleSelectionAllowed(bool value){
	this->_multipleSelectionAllowed = value;
	if ( this->_multipleSelectionAllowed ){
		this->attributes["multiple"] = "multiple";
	}else{
		if (this->attributes.has("multiple")){
			this->attributes.remove("multiple");
		}
	}
}
bool FileUploader::multipleSelectionAllowed(){
	return this->_multipleSelectionAllowed;
}

void FileUploader::onEvent(std::string name, Event* event){
	if ( name == FileUploader::Event_OnData ){
		if (onDataListener != NULL)onDataListener->onData(this, event->params["file_name"], event->params["file_data"]);
	}else if ( name == FileUploader::Event_OnSuccess ){
		if (onSuccessListener != NULL)onSuccessListener->onSuccess(this);
	}else if ( name == FileUploader::Event_OnFail ){
		if (onFailListener != NULL)onFailListener->onFail(this);
	}

	Widget::onEvent(name, event);
}