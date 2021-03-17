//
// Created by CyberPro4 on 24/06/2016.
//


#include "remi_server.h"
#include "remi.h"

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

void remi::utils::url_decode(const char* from, unsigned long long len, char*& converted, unsigned long long* lenConverted ){
	std::smatch match;
	std::string out = from;
	char r[2] = { 0 };
	int v = 0;

	/*while( std::regex_search( out, match, std::regex("%[0-9A-F]{2}") ) ){
		sscanf(
			out.substr( match.position(0) +1 , 2 ).c_str(),
			"%x" , &v );
		r[0] = r[1] = 0;
		r[0] = (char)v;

		out.replace( match.position(0) , match.length(0) , r );
	}*/
	if (len < 3){
		converted = new char[len];
		memcpy(converted, from, len);
		return;
	}

	converted = new char[len];
	unsigned long long int __i = 0;
	unsigned short val = 0;

	char* original = converted;
	for (__i = 0; __i < len-2; __i++){
		converted[0] = from[__i];
		if (converted[0] == '%'){
			converted[1] = from[__i+1];
			converted[2] = from[__i+2];
			if ((converted[1] >= '0' && converted[1] <= '9') || (converted[1] >= 'A' && converted[1] <= 'F')){
				if ((converted[2] >= '0' && converted[2] <= '9') || (converted[2] >= 'A' && converted[2] <= 'F')){
					val = ((converted[1] - ((converted[1]>'9') ? (55) : '0')) * 16) + ((converted[2] - ((converted[2]>'9') ? (55) : '0')));
					converted[0] = val;
					__i += 2;
				}
			}
		}
		converted++;
		(*lenConverted)++;
	}
	//the iteration terminated 2 steps before allowing to avoid additional len checks, but here we fill the remaining chars
	converted[0] = from[__i++];
	converted[1] = from[__i++];
	converted = original;
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

unsigned long long remi::utils::searchIndexOf(const char* buffer, char __char, unsigned long long len, unsigned long long start){
	unsigned long long __i = start;
	while (__i < len && buffer[__i++] != __char);
	return __i;
}

/*std::list<char*> utils::splitCharPointer(const char* buf, char splitChar, unsigned long long bufLen, int maxSplit){
	std::list<char*> ret;
	unsigned long long __i = -1;
	unsigned long long __last = 0;
	while ((__i = searchIndexOf(buf, splitChar, bufLen, __last))!=__last && maxSplit>0){
		char* chunk = new char[__i - __last];
		memcpy(chunk, buf, __i - __last);
		ret.push_back(chunk);
		__last = __i;
		maxSplit--;
	}
	if (maxSplit == 0){
		char* chunk = new char[bufLen - __i - 1];
		memcpy(chunk, buf, bufLen - __i - 1);
		ret.push_back(chunk);
	}
	return ret;
}*/

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
	std::locale loc(std::locale(), new std::codecvt_utf8<wchar_t>);
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
			_start = clock_::now();
		}

		Sleep( 2 );
	}
    cout << "remi::utils::Timer::tick - TIMER STOPPED" << endl;

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


StringRepresantable::StringRepresantable(std::string v ){
    this->v = v;
}

std::string StringRepresantable::repr(Dictionary<Represantable*>* changedWidgets, bool forceUpdate){
    return v;
}


Tag::Tag(){
    this->attributes.event_onchange->_do(this, (EventListener::listener_class_member_type)&Tag::_needUpdate);
	this->style.event_onchange->_do(this, (EventListener::listener_class_member_type)&Tag::_needUpdate);
	this->children.event_onchange->_do(this, (EventListener::listener_class_member_type)&Tag::_needUpdate);

    _parent = NULL;

	ignoreUpdate = false;

    type = "div";
	setIdentifier(utils::sformat("%llu", (unsigned long long)this));

    int status = 0;
	std::string _class = std::string( abi::__cxa_demangle(typeid(*this).name(),0,0,&status) );
	int pos = _class.rfind(":");
	_class.erase(0, pos+1);
    addClass( _class );
}

Tag::Tag(VersionedDictionary<std::string> _attributes, std::string _type, std::string _class):Tag(){

    type = _type;

	attributes.update(_attributes);

    setClass( _class );
}

void Tag::setClass(std::string name) {
    _classes.clear();
	_classes.push_front(name);
}

void Tag::addClass(std::string name) {
	_classes.push_front(name);
}

void Tag::removeClass(std::string name) {
	_classes.remove(name);
}

std::string Tag::getIdentifier() {
	if (attributes.has("id"))
		return attributes["id"];
	return "";
}

void Tag::setIdentifier(std::string newIdentifier) {
	attributes.set("id", newIdentifier);
	//remi::server::runtimeInstances[this->getIdentifier()] = this;
}

std::string Tag::innerHTML(Dictionary<Represantable*>* localChangedWidgets, bool forceUpdate) {
	std::ostringstream ret;
	for (std::string k : this->children.keys()) {
        cout << "representing: " << k << endl;
		Represantable* s = children.get(k);
		ret << s->repr(localChangedWidgets, forceUpdate);
	}
	//cout << ret.str() << endl;
	return ret.str();
}

std::string Tag::repr(Dictionary<Represantable*>* changedWidgets, bool forceUpdate) {
	Dictionary<Represantable*>* localChangedWidgets = new Dictionary<Represantable*>();
	std::ostringstream  _innerHtml;

	_innerHtml << innerHTML(localChangedWidgets, forceUpdate);
    cout << "Tag::repr - representing: " << this->type << " localChangedWidgets->size() = " << localChangedWidgets->size() << endl;
	if (this->isChanged() || (localChangedWidgets->size() > 0 || forceUpdate)) {
		_backupRepr.str(std::string());
		_backupRepr << "<" << type <<
			" " << _reprAttributes.str() <<
			" class=\"" << utils::join(_classes, " ") << "\""
			">" << _innerHtml.str() << "</" << type << ">";
	}

	if (this->isChanged()) {
		changedWidgets->set(this->getIdentifier(), this);
		this->setUpdated();
	}else {
		changedWidgets->update(*localChangedWidgets);
	}

    delete localChangedWidgets;

	return _backupRepr.str();
}

void Tag::_notifyParentForUpdate() {
	if (!ignoreUpdate) {
		if (this->_parent) {
			this->_parent->_notifyParentForUpdate();
		}
	}
}

void Tag::_needUpdate(Tag* emitter, Dictionary<Buffer*>* params, void* userdata) {
    Dictionary<std::string> tmp;
    tmp.update(this->attributes);

	if (style.size() > 0)
		tmp.set("style", utils::toCss(style));


    this->_reprAttributes.str(std::string());
    cout << this->_reprAttributes.str() << endl<<endl;
	for (std::string k : tmp.keys()) {
		this->_reprAttributes << k << "=\"" << tmp[k].value << "\"";
	}
	cout << this->_reprAttributes.str() << endl<<endl;
	_notifyParentForUpdate();
}

void Tag::addChild( Represantable* child , std::string key ){

    if( child == NULL )
        return;

	std::string _key = key;
	if( _key.length() < 1 ){
		_key = utils::sformat( "%llu" , (unsigned long long)(void*)child );
	}

	if( dynamic_cast<Tag*>(child) != 0 ){
		Tag* _tag = dynamic_cast<Tag*>(child);
		_tag->setParent( this );
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


Widget::Widget() : Widget::Widget(CLASS_NAME(Widget)) {
    
}

Widget::Widget(std::string _class) : Tag::Tag() {
    this->event_onclick = new Widget::onclick(this);
	this->event_onblur = new Widget::onblur(this);
	this->event_onfocus = new Widget::onfocus(this);
	this->event_ondblclick = new Widget::ondblclick(this);
	this->event_oncontextmenu = new Widget::oncontextmenu(this);
	this->event_onmousedown = new Widget::onmousedown(this);
	this->event_onmouseup = new Widget::onmouseup(this);
	this->event_onmouseout = new Widget::onmouseout(this);
	this->event_onmouseover = new Widget::onmouseover(this);
	this->event_onmouseleave = new Widget::onmouseleave(this);
	this->event_onmousemove = new Widget::onmousemove(this);
	this->event_ontouchmove = new Widget::ontouchmove(this);
	this->event_ontouchstart = new Widget::ontouchstart(this);
	this->event_ontouchend = new Widget::ontouchend(this);
	this->event_ontouchenter = new Widget::ontouchenter(this);
	this->event_ontouchleave = new Widget::ontouchleave(this);
	this->event_ontouchcancel = new Widget::ontouchcancel(this);
	this->event_onkeyup = new Widget::onkeyup(this);
	this->event_onkeydown = new Widget::onkeydown(this);
	
    style.set( "margin" , "0px auto" );

    setClass(_class);
}

Widget::Widget(VersionedDictionary<std::string> _attributes, VersionedDictionary<std::string> _style, std::string _type, std::string _class):Widget(_class) {
    this->attributes.update(_attributes);
    this->style.update(_style);
    this->type = _type;
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

void Widget::addChild( Represantable* child, std::string key ){
    Tag::addChild( child , key );
}


Container::Container(Dictionary<Widget*>* children, Container::Layout layout_orientation):Widget(CLASS_NAME(Container)){
    this->setLayoutOrientation(layout_orientation);
    if(!(children==NULL)){
        for( std::string k : children->keys() ){
            this->append(children->get(k), k);
        }
    }
}

std::string Container::append(Widget* w, std::string key){
    if(key.length()<1){
        key = w->attributes["id"];
    }
    this->addChild(w, key);
    return key;
}

std::string Container::append(Dictionary<Widget*>* _children){
    for( std::string k : _children->keys() ){
        this->append(_children->get(k), k);
    }
    return "";
}

void Container::setLayoutOrientation(Container::Layout orientation){
    _layout_orientation = orientation;
}


HBox::HBox() : Container(){

	style["display"] = "flex";

	style["justify-content"] = "space-around";

	style["align-items"] = "center";

	style["flex-direction"] = "row";

	setClass(CLASS_NAME(HBox));
}

VBox::VBox() : Container(){

	style["display"] = "flex";

	style["justify-content"] = "space-around";

	style["align-items"] = "center";

	style["flex-direction"] = "column";

	setClass(CLASS_NAME(VBox));

}

Button::Button( std::string text ) : TextWidget(){

	type = "button";

	setText(text);

	setClass(CLASS_NAME(Button));
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


void TextWidget::setText(std::string text){
	((Tag*)this)->addChild(text, std::string("text"));
}

std::string TextWidget::text(){
	Represantable* repr = getChild("text");
	return ((StringRepresantable*)repr)->repr();
}

Label::Label( std::string text ){
	type = "p";
	setText( text );
	setClass(CLASS_NAME(Label));
}

GenericDialog::GenericDialog( std::string title , std::string message ){
    setClass(CLASS_NAME(GenericDialog));
	setLayoutOrientation( Container::Layout::Vertical );
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

	_container = new Container();
	_container->style["display"] = "block";
	_container->style["overflow"] = "auto";
	_container->style["margin"] = "5px";
	_container->setLayoutOrientation(Container::Layout::Vertical);

	_confirmButton = new Button("Ok");
	_confirmButton->setSize(100, 30);
	_confirmButton->style["margin"] = "3px";
	//_confirmButton->onClickListener = this;

	_cancelButton = new Button("Cancel");
	_cancelButton->setSize(100, 30);
	_cancelButton->style["margin"] = "3px";
	//_cancelButton->onClickListener = this;

	_hLay = new HBox();
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
	Container* container = new Container();
	container->style.set("display", "block");
	container->style.set("overflow", "auto");
	container->style.set("padding", "3px");
	container->setLayoutOrientation(Container::Layout::Horizontal);
	container->addChild(this->_inputs[key], key);
	this->_container->addChild(container, key);
}

Widget* GenericDialog::getField(std::string key){
	return this->_inputs[key];
}



Image::Image(std::string url){
	type = "img";
    setClass(CLASS_NAME(Image));
	setURL(url);
}

void Image::setURL(std::string url){
	this->attributes["src"] = url;
}

std::string Image::url(){
	return this->attributes["src"];
}

Input::Input(){
	type = "input";
    /*
	attributes[Event_OnClick] = "";
	attributes[Event_OnChange] = utils::sformat( \
            "var params={};params['value']=document.getElementById('%(id)s').value;" \
            "sendCallbackParam('%s','%s',params);" , getIdentifier().c_str() , Event_OnChange.c_str() );
    */
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

	type = "input";
	setClass(CLASS_NAME(FileUploader));
	this->attributes["type"] = "file";
	this->attributes["accept"] = "*.*";
/*
	attributes[Widget::Event_OnChange] = utils::sformat(
		"var files = this.files;" \
		"for(var i=0; i<files.length; i++){" \
		"uploadFile('%s','%s','%s','%s',files[i]);}",
		getIdentifier().c_str(), FileUploader::Event_OnSuccess.c_str(), FileUploader::Event_OnFail.c_str(), FileUploader::Event_OnData.c_str()
	);

	this->attributes[Widget::Event_OnClick] = "event.stopPropagation();";
	this->attributes[Widget::Event_OnDblClick] = "event.stopPropagation();";
*/
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
/*
void FileUploader::onEvent(std::string name, Event* event){
	if ( name == FileUploader::Event_OnData ){
		if (onDataListener != NULL)onDataListener->onData(this, event->params.get("file_name")->str(), event->params.get("file_data")->data, event->params.get("file_data")->len );
	}else if ( name == FileUploader::Event_OnSuccess ){
		if (onSuccessListener != NULL)onSuccessListener->onSuccess(this);
	}else if ( name == FileUploader::Event_OnFail ){
		if (onFailListener != NULL)onFailListener->onFail(this);
	}

	Widget::onEvent(name, event);
}
*/
