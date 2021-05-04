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

long long int	remi_timestamp() {
#ifdef _WIN32

	FILETIME ft = { 0 };

	GetSystemTimeAsFileTime(&ft);

	LARGE_INTEGER li = { 0 };

	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	long long int hns = li.QuadPart;

	return (int)(hns / 1000 / 1000);
#else

	return time(NULL);

#endif
}

remi_thread   remi_createThread(remi_thread_callback callback, remi_thread_param param) {

#ifdef _WIN32
	return CreateThread(NULL, 0, callback, param, 0, 0);
#endif

#ifdef __unix__
	pthread_t       th;
	pthread_create(&th, NULL, callback, param);
	return th;
#endif

}


std::string remi::utils::SHA1(std::string& val) {
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

void remi::utils::open_browser(std::string url) {

#ifdef _WIN32

	ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, 0);

#endif

#if defined(__unix__)

	std::string cmd = "xdg-open " + url;

	system(cmd.c_str());

#endif

}

void remi::utils::url_decode(const char* from, unsigned long long len, char*& converted, unsigned long long* lenConverted) {
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
	converted = new char[len];
	if (len < 3) {
		//converted = new char[len];
		memcpy(converted, from, len);
		return;
	}

	unsigned long long int __i = 0;
	unsigned short val = 0;

	char* original = converted;
	for (__i = 0; __i < len - 2; __i++) {
		converted[0] = from[__i];
		if (converted[0] == '%') {
			converted[1] = from[__i + 1];
			converted[2] = from[__i + 2];
			if ((converted[1] >= '0' && converted[1] <= '9') || (converted[1] >= 'A' && converted[1] <= 'F')) {
				if ((converted[2] >= '0' && converted[2] <= '9') || (converted[2] >= 'A' && converted[2] <= 'F')) {
					val = ((converted[1] - ((converted[1] > '9') ? (55) : '0')) * 16) + ((converted[2] - ((converted[2] > '9') ? (55) : '0')));
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

std::string remi::utils::toPix(int v) {
	std::ostringstream ss;
	ss << v << "px";
	return ss.str();
}

std::string remi::utils::operator "" _px(long double value) {
	return remi::utils::sformat("%.2fpx", (float)value);
}
//pc stays for percent
std::string remi::utils::operator "" _pc(long double value) {
	return remi::utils::sformat("%.2f%%", (float)value);
}

std::list<std::string> remi::utils::split(std::string subject, std::string delimiter) {
	std::list<std::string> list;

	size_t found = 0;

	while (true) {

		size_t nf = subject.find(delimiter, found);
		if (nf == std::string::npos) {
			list.push_back(subject.substr(found, subject.length() - found));
			break;
		}
		else {
			list.push_back(subject.substr(found, nf - found));
			found = nf + delimiter.length();
		}
	}

	return list;
}

std::string remi::utils::strip(std::string subject, char char_to_strip) {
	std::string result;
	const char* cstr = subject.c_str();

	int length = subject.length();
	while (((char)cstr[length - 1]) == char_to_strip && length > 0) {
		length--;
	}

	bool found_a_different_char = false;
	for (int i = 0; i < length; i++) {
		if (((char)cstr[i]) != char_to_strip) {
			found_a_different_char = true;
		}
		if (found_a_different_char) {
			result += ((char)cstr[i]);
		}
	}

	return result;
}

int remi::utils::count(std::string subject, std::string pattern) {
	int result = 0;
	int pos = -1;
	while ((pos = subject.find(pattern, (pos + 1))) < subject.length()) {
		result++;
	}
	return result;
}

unsigned long long remi::utils::searchIndexOf(const char* buffer, char __char, unsigned long long len, unsigned long long start) {
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

std::string remi::utils::join(std::list<std::string> stringList, std::string glue) {
	std::ostringstream ss;

	bool first = true;
	for (std::string s : stringList) {
		if (!first)
			ss << glue;
		ss << s;
		first = false;
	}

	return ss.str();
}

std::string remi::utils::string_encode(std::string text) {
	std::codecvt_utf8<wchar_t>* _cdcv = new std::codecvt_utf8<wchar_t>;
	std::locale loc(std::locale(), _cdcv);
	std::ostringstream o;
	o.imbue(loc);
	o << text;
	//delete _cdcv;
	return o.str();
}

std::string remi::utils::escape_json(const std::string& s) {
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

int remi::utils::sscan(std::string from, std::string format, ...) {

	va_list ap;

	va_start(ap, format);

	int r = vsscanf(from.c_str(), format.c_str(), ap);

	va_end(ap);

	return r;
}

std::string remi::utils::sformat(std::string fmt_str, ...) {

	// From http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf

	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
	std::string str;
	std::unique_ptr<char[]> formatted;
	va_list ap;
	while (1) {
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


utils::Timer::Timer(int millisecondsInterval, TimerListener* listener) {
	_stopFlag = false;

	_millisecondsInterval = millisecondsInterval;

	_listener = listener;

	start();
}

utils::Timer::Timer() {
	_stopFlag = false;

	_millisecondsInterval = 0;

	_listener = NULL;
}

void utils::Timer::setListener(TimerListener* listener) {
	_listener = listener;
}

remi_thread_result utils::Timer::thread_entry(remi_thread_param instance) {
	((utils::Timer*)instance)->tick();
	return 0;
}
/*
double remi::utils::Timer::elapsed(){
	double res = std::chrono::duration_cast<second_>
			(clock_::now() - _start).count();
	return res;
}
*/
void remi::utils::Timer::setInterval(int msecInterval) {
	_millisecondsInterval = msecInterval;
}
/*
bool utils::Timer::has_passed(){
	return elapsed() > (_millisecondsInterval/1000.f);
}
*/
void remi::utils::Timer::tick() {
	while (!_stopFlag) {
		/*if( has_passed() ){
			_passed = true;
			if( _listener != NULL ){
				_listener->onTimer();
			}

			//stop();
			_start = clock_::now();
		}
		Sleep( 2 );
		*/

		Sleep(_millisecondsInterval);
		if (_listener != NULL) {
			_listener->onTimer();
		}
	}

}

void remi::utils::Timer::start() {
	//_start = clock_::now();
	_stopFlag = false;
	//_passed = false;

	if (_listener != NULL) {
		remi_createThread((remi_thread_callback)& utils::Timer::thread_entry, this);
	}

}

void remi::utils::Timer::stop() {
	_stopFlag = true;
}


std::string remi::utils::join(Dictionary<std::string>& from, std::string nameValueGlue, std::string itemsGlue) {
	std::ostringstream out;

	bool first = true;

	for (std::string key : from.keys()) {
		if (!first)
			out << itemsGlue;
		first = false;
		out << key << nameValueGlue << from.get(key);
	}

	return out.str();
}

std::string remi::utils::toCss(Dictionary<std::string>& values) {
	return join(values, ": ", ";");
}


StringRepresantable::StringRepresantable(std::string v) {
	this->v = v;
}

std::string StringRepresantable::repr(Dictionary<Represantable*>* changedWidgets, bool forceUpdate) {
	return v;
}


Tag::Tag() {
	LINK_EVENT_TO_CLASS_MEMBER(this->attributes.event_onchange, this, &Tag::_needUpdate);
	LINK_EVENT_TO_CLASS_MEMBER(this->style.event_onchange, this, &Tag::_needUpdate);
	LINK_EVENT_TO_CLASS_MEMBER(this->children.event_onchange, this, &Tag::_needUpdate);

	_parent = NULL;

	ignoreUpdate = false;

	type = "div";
	setIdentifier(utils::sformat("%llu", (unsigned long long)this));

	int status = 0;
	std::string _class = std::string(typeid(*this).name());
	int pos = _class.rfind(":");
	_class.erase(0, pos + 1);
	addClass(_class);
}

Tag::Tag(VersionedDictionary<std::string> _attributes, std::string _type, std::string _class) :Tag() {

	type = _type;

	attributes.update(_attributes);

	setClass(_class);
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
		//cout << "representing: " << k << endl;
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
	//cout << "Tag::repr - representing: " << this->type << " localChangedWidgets->size() = " << localChangedWidgets->size() << endl;
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
	}
	else {
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
	//cout << this->_reprAttributes.str() << endl<<endl;
	for (std::string k : tmp.keys()) {
		this->_reprAttributes << k << "=\"" << tmp[k].value << "\"";
	}
	//cout << this->_reprAttributes.str() << endl<<endl;
	_notifyParentForUpdate();
}

void Tag::addChild(Represantable* child, std::string key) {

	if (child == NULL)
		return;

	std::string _key = key;
	if (_key.length() < 1) {
		_key = utils::sformat("%llu", (unsigned long long)(void*)child);
	}

	if (dynamic_cast<Tag*>(child) != 0) {
		Tag* _tag = dynamic_cast<Tag*>(child);
		_tag->setParent(this);
	}


	if (children.has(_key)) {
		_render_children_list.remove(children.get(_key));
	}

	_render_children_list.push_back(child);
	children.set(_key, child);
}

void Tag::addChild(std::string child, std::string key) {
	addChild(new StringRepresantable(child), key);
}

Represantable* Tag::getChild(std::string key) {
	return children[key];
}

void Tag::setUpdated() {
	attributes.alignVersion(); style.alignVersion(); children.alignVersion();
	for (Represantable* represantable : _render_children_list) {
		if (dynamic_cast<Tag*>(represantable) != 0) {
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
	this->event_onqueryClientResult = new Widget::onqueryClientResult(this);
	
	style.set("margin", "0px auto");

	setClass(_class);
}

Widget::Widget(VersionedDictionary<std::string> _attributes, VersionedDictionary<std::string> _style, std::string _type, std::string _class) :Widget(_class) {
	this->attributes.update(_attributes);
	this->style.update(_style);
	this->type = _type;
}

void Widget::setWidth(int width) {
	style.set("width", utils::toPix(width));
}

void Widget::setHeight(int height) {
	style.set("height", utils::toPix(height));
}

void Widget::setSize(int width, int height) {
	style.set("width", utils::toPix(width));
	style.set("height", utils::toPix(height));
}

void Widget::addChild(Represantable* child, std::string key) {
	Tag::addChild(child, key);
}


Container::Container(Dictionary<Widget*>* children, Container::Layout layout_orientation) :Widget(CLASS_NAME(Container)) {
	this->setLayoutOrientation(layout_orientation);
	if (!(children == NULL)) {
		for (std::string k : children->keys()) {
			this->append(children->get(k), k);
		}
	}
}

std::string Container::append(Widget* w, std::string key) {
	if (key.length() < 1) {
		key = w->attributes["id"];
	}
	this->addChild(w, key);
	return key;
}

std::string Container::append(Dictionary<Widget*>* _children) {
	for (std::string k : _children->keys()) {
		this->append(_children->get(k), k);
	}
	return "";
}

void Container::setLayoutOrientation(Container::Layout orientation) {
	_layout_orientation = orientation;
}


AsciiContainer::AsciiContainer(std::string asciipattern, float gap_horizontal, float gap_vertical, Dictionary<Widget*> * children) :Container(children) {
		this->style["position"] = "relative";
		this->set_from_asciiart(asciipattern, gap_horizontal, gap_vertical);
	}

AsciiContainer::~AsciiContainer() {
	for (std::string key : widget_layout_map.keys()) {
		delete widget_layout_map.get(key);
	}
}

void AsciiContainer::set_from_asciiart(std::string asciipattern, float gap_horizontal, float gap_vertical) {
	/*
		asciipattern (std::string): a multiline string representing the layout
			| widget1               |
			| widget1               |
			| widget2 | widget3     |
		gap_horizontal (float): a percent value (0..100)
		gap_vertical (float): a percent value
	*/

	//deleting old map
	for (std::string key : widget_layout_map.keys()) {
		delete widget_layout_map.get(key);
	}
	widget_layout_map.clear();

	std::list<std::string> pattern_rows = utils::split(asciipattern, "\n");
	pattern_rows.remove_if([](std::string value) {
		return (utils::strip(utils::strip(value, ' '), '\t').length() < 1);
		}
	);

	int layout_height_in_chars = pattern_rows.size();

	float row_index = 0.0;
	for (std::string r : pattern_rows) {
		std::string row = utils::strip(r, ' ');
		row = utils::strip(row, '\t');

		if (row.length() < 2)
			continue;  //at least two pipes
		int pipes_count = utils::count(row, "|");
		float row_width = row.length() - pipes_count; //the row width is calculated without pipes
		row = row.substr(1, row.length() - 2); //removing |pipes at beginning and end

		std::list<std::string> columns = utils::split(row, "|");

		float left_value = 0.0;
		for (std::string c : columns) {
			std::string widget_key = utils::strip(c, ' ');
			float widget_width = (c).length();

			if (!widget_layout_map.has(widget_key)) {
				/*width is calculated in percent
					height is instead initialized at 1 and incremented by 1 each row the key is present
					at the end of algorithm the height will be converted in percent
				*/
				widget_layout_map[widget_key] = new std::map<std::string, float>{ {"width", widget_width / (row_width) * 100.0 - gap_horizontal},
					{"height", 1},
					{"top", (row_index / layout_height_in_chars) * 100.0 + (gap_vertical / 2.0)},
					{"left", (left_value / row_width) * 100.0 + (gap_horizontal / 2.0)}
				};
			}
			else {
				(*(std::map<std::string, float>*)widget_layout_map.get(widget_key))["height"] += 1;
			}
			left_value += widget_width;
		}
		row_index++;
	}

	//converting height values in percent string
	for (std::string key : widget_layout_map.keys()) {
		std::map<std::string, float>* m = widget_layout_map.get(key);
		(*m)["height"] = ((*m)["height"] / layout_height_in_chars) * 100.0 - gap_vertical;
	}

	for (std::string key : widget_layout_map.keys()) {
		set_widget_layout(key);
	}
}

std::string AsciiContainer::append(Widget* w, std::string key) {
	Container* c = static_cast<Container*>(this);
	c->append(w, key);
	set_widget_layout(key);
	return key;
}

void AsciiContainer::set_widget_layout(std::string key) {
	if (children.has(key) == false || widget_layout_map.has(key) == false) {
		return;
	}
	static_cast<Tag*>(children.get(key))->style.set("position", "absolute");

	static_cast<Tag*>(children.get(key))->style.set("width", utils::sformat("%.2f%%", (*(std::map<std::string, float>*)widget_layout_map.get(key))["width"]));
	static_cast<Tag*>(children.get(key))->style.set("height", utils::sformat("%.2f%%", (*(std::map<std::string, float>*)widget_layout_map.get(key))["height"]));
	static_cast<Tag*>(children.get(key))->style.set("left", utils::sformat("%.2f%%", (*(std::map<std::string, float>*)widget_layout_map.get(key))["left"]));
	static_cast<Tag*>(children.get(key))->style.set("top", utils::sformat("%.2f%%", (*(std::map<std::string, float>*)widget_layout_map.get(key))["top"]));
}


HTML::HTML() {
	event_onrequiredupdate = new onrequiredupdate(this);
	type = "html";
}

HTML::~HTML() {
	//Event handlers are deleted by Tag destructor
	//delete this->event_onrequiredupdate;
}

void HTML::_notifyParentForUpdate() {
	//this condition prevents the event to be called in
	//onrequiredupdate::onrequiredupdate(), where the .attributes dictionary gets
	//changed
	if (this->event_handlers.has("onrequiredupdate")) {
		(*this->event_onrequiredupdate)();
	}
}

std::string HTML::repr(Dictionary<Represantable*>* changed_widgets, bool forceUpdate) {
	/*It is used to automatically represent the object to HTML format
	packs all the attributes, children and so on.

	Args:
		changed_widgets (dict): A dictionary containing a collection of tags that have to be updated.
			The tag that have to be updated is the key, and the value is its textual repr.
	*/

	Dictionary<Represantable*>* local_changed_widgets = new Dictionary<Represantable*>();

	std::ostringstream  result;

	result << "<" << type << ">\n" << this->innerHTML(local_changed_widgets, forceUpdate) << "\n</" << type << ">";
	this->setUpdated();
	delete local_changed_widgets;
	return result.str();
}


HEAD::HEAD(std::string title) {
	type = "head";
	this->addChild("<meta content='text/html;charset=utf-8' http-equiv='Content-Type'> \
							<meta content='utf-8' http-equiv='encoding'> \
							<meta name='viewport' content='width=device-width, initial-scale=1.0'>", "meta");

	this->setTitle(title);
	this->event_onerror = new onerror(this);
}

HEAD::~HEAD() {
	//Event handlers are deleted by Tag destructor
	//delete this->event_onerror;
}

void HEAD::setIconFile(std::string filename, std::string rel, std::string mimetype) {
	/* Allows to define an icon for the App

		Args:
			filename (str): the resource file name (ie. "/res:myicon.png")
			rel (str): leave it unchanged (standard "icon")
	*/
	//mimetype, encoding = mimetypes.guess_type(filename)
	this->addChild("favicon", utils::sformat("<link rel='%s' href='%s' type='%s' />", rel.c_str(), filename.c_str(), mimetype.c_str()));
}

void HEAD::setIconData(std::string base64_data, std::string mimetype, std::string rel) {
	/* Allows to define an icon for the App

		Args:
			base64_data (str): base64 encoded image data  (ie. "data:image/x-icon;base64,AAABAAEAEBA....")
			mimetype (str): mimetype of the image ("image/png" or "image/x-icon"...)
			rel (str): leave it unchanged (standard "icon")
	*/
	this->addChild("favicon", utils::sformat("<link rel='%s' href='%s' type='%s' />", rel.c_str(), base64_data.c_str(), mimetype.c_str()));
}

void HEAD::setInternalJs(std::string net_interface_ip, unsigned short pending_messages_queue_length, unsigned short websocket_timeout_timer_ms) {
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
				}else{
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
					if (document.activeElement){
						focusedElement = document.activeElement.id;
						try{
							caretStart = document.activeElement.selectionStart;
							caretEnd = document.activeElement.selectionEnd;
						}catch(e){
							console.debug(e.message);
						}
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
					if (elemToFocus != null) {
						elemToFocus.focus();
						try {
							elemToFocus = document.getElementById(focusedElement);
							if (caretStart>-1 && caretEnd>-1)
								elemToFocus.setSelectionRange(caretStart, caretEnd);
						} catch (e) {
							console.debug(e.message);
						}
					}
				}else if (received_msg[0]=='2') { /*javascript*/
					var content = received_msg.substr(1,received_msg.length-1);
					try {
						eval(content);
					} catch (e) {
						console.debug(e.message);
					};
				} else if (received_msg[0]=='3') { /*ack*/
					self._pendingSendMessages.shift() /*remove the oldest*/
					if (self._comTimeout!==null)
						clearTimeout(self._comTimeout);
				}
			};

			this._ws.onclose = function(evt) {
				/* websocket is closed. */
				console.debug('Connection is closed... event code: ' + evt.code + ', reason: ' + evt.reason);
				// Some explanation on this error: http://stackoverflow.com/questions/19304157/getting-the-reason-why-websockets-closed
				// In practice, on a unstable network (wifi with a lot of traffic for example) this error appears
				// Got it with Chrome saying:
				// WebSocket connection to 'ws://x.x.x.x:y/' failed: Could not decode a text frame as UTF-8.
				// WebSocket connection to 'ws://x.x.x.x:y/' failed: Invalid frame header

				try {
					document.getElementById("loading").style.display = '';
				} catch (err) {
					console.log('Error hiding loading overlay ' + err.message);
				}

				self._failedConnections += 1;

				console.debug('failed connections=' + self._failedConnections + ' queued messages=' + self._pendingSendMessages.length);

				if (self._failedConnections > 3) {

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

				if (evt.code == 1006) {
					self._renewConnection();
				}
			};

			this._ws.onerror = function(evt) {
				/* websocket is closed. */
				/* alert('Websocket error...');*/
				console.debug('Websocket error... event code: ' + evt.code + ', reason: ' + evt.reason);
			};

		} catch (ex) {
			this._ws=false;
			alert('websocketnot supported or server unreachable');
		}
	}


	/*this uses websockets*/
	Remi.prototype.sendCallbackParam = function(widgetID,functionName,params /*a dictionary of name:value*/) {
		var paramStr = '';
		if (params!==null)
			paramStr=this._paramPacketize(params);
		var message = encodeURIComponent(unescape('callback' + '/' + widgetID+'/'+functionName + '/' + paramStr));
		this._pendingSendMessages.push(message);
		if (this._pendingSendMessages.length < %d) {
			if (this._ws !== null && this._ws.readyState == 1)
				this._ws.send(message);
			if (this._comTimeout===null)
				this._comTimeout = setTimeout(this._checkTimeout, %d);
		} else {
			console.debug('Renewing connection, this._ws.readyState when trying to send was: ' + this._ws.readyState)
			this._renewConnection();
		}
	};

	/*this uses websockets*/
	Remi.prototype.sendCallback = function(widgetID,functionName) {
		this.sendCallbackParam(widgetID,functionName,null);
	};

	Remi.prototype._renewConnection = function() {
		// ws.readyState:
		//A value of 0 indicates that the connection has not yet been established.
		//A value of 1 indicates that the connection is established and communication is possible.
		//A value of 2 indicates that the connection is going through the closing handshake.
		//A value of 3 indicates that the connection has been closed or could not be opened.
		if (this._ws.readyState == 1) {
			try {
				this._ws.close();
			} catch (err) {};
		} else
			if (this._ws.readyState == 0) {
				// Don't do anything, just wait for the connection to be stablished
			} else {
				this._openSocket();
			}
	};

	Remi.prototype._checkTimeout = function() {
		if (this._pendingSendMessages.length > 0)
			this._renewConnection();
	};

	Remi.prototype.uploadFile = function(widgetID, eventSuccess, eventFail, eventData, file) {
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
				var params={};
				params['filename']=file.name;
				remi.sendCallbackParam(widgetID, eventSuccess,params);
				console.log('upload success: ' + file.name);
			} else
				if (xhr.status == 400) {
					var params={};
					params['filename']=file.name;
					remi.sendCallbackParam(widgetID,eventFail,params);
					console.log('upload failed: ' + file.name);
				}
		};
		fd.append('upload_file', file);
		xhr.send(fd);
	};

	window.onerror = function(message, source, lineno, colno, error) {
		var params={};
		params['message']=message;
		params['source']=source;
		params['lineno']=lineno;
		params['colno']=colno;
		params['error']=JSON.stringify(error);
		remi.sendCallbackParam('%s','%s',params);
		return false;
	};

	window.remi = new Remi();

	</script>)", net_interface_ip.c_str(), pending_messages_queue_length, websocket_timeout_timer_ms, this->getIdentifier().c_str(), "onerror"), "internal_js");
}

void HEAD::setTitle(std::string title) {
	this->addChild(utils::sformat("<title>%s</title>", title.c_str()), "title");
}

std::string HEAD::repr(Dictionary<Represantable*>* changed_widgets, bool forceUpdate) {
	/*It is used to automatically represent the object to HTML format
	packs all the attributes, children and so on.

	Args:
	changed_widgets (dict): A dictionary containing a collection of tags that have to be updated.
	The tag that have to be updated is the key, and the value is its textual repr.
	*/

	Dictionary<Represantable*>* local_changed_widgets = new Dictionary<Represantable*>();

	std::ostringstream  result;

	result << "<" << type << ">\n" << this->innerHTML(local_changed_widgets, forceUpdate) << "\n</" << type << ">";
	this->setUpdated();
	//delete changed_widgets;
	delete local_changed_widgets;
	return result.str();
}


BODY::BODY() :Container::Container() {
	type = "body";
	Widget* loading_anim = new Widget();
	loading_anim->style.remove("margin");
	loading_anim->setIdentifier("loading-animation");

	Container* loading_container = new Container();
	loading_container->append(loading_anim, "loading_animation");
	loading_container->style.set("display", "none");
	loading_container->style.remove("margin");
	loading_container->setIdentifier("loading");

	this->append(loading_container, "loading_container");

	this->event_onload = new onload(this);
	this->event_ononline = new ononline(this);
	this->event_onpagehide = new onpagehide(this);
	this->event_onpageshow = new onpageshow(this);
	this->event_onresize = new onresize(this);
}

BODY::~BODY() {
	//Event handlers are deleted by Tag destructor
	/*delete this->event_onload;
	delete this->event_ononline;
	delete this->event_onpagehide;
	delete this->event_onpageshow;
	delete this->event_onresize;*/

	//children are deleted by Tag destructor
	delete this->children.get("loading_container");
}


HBox::HBox() : Container() {

	style["display"] = "flex";

	style["justify-content"] = "space-around";

	style["align-items"] = "center";

	style["flex-direction"] = "row";

	setClass(CLASS_NAME(HBox));
}

VBox::VBox() : Container() {

	style["display"] = "flex";

	style["justify-content"] = "space-around";

	style["align-items"] = "center";

	style["flex-direction"] = "column";

	setClass(CLASS_NAME(VBox));

}


Button::Button(std::string text) : TextWidget() {

	type = "button";

	setText(text);

	setClass(CLASS_NAME(Button));
}

void Button::setEnabled(bool en) {
	if (en) {
		attributes["disabled"] = "disabled";
	}
	else {
		attributes.remove("disabled");
	}
}

bool Button::enabled() {
	return attributes.has("disabled");
}


void TextWidget::setText(std::string text) {
	((Tag*)this)->addChild(text, std::string("text"));
}

std::string TextWidget::text() {
	Represantable* repr = getChild("text");
	return ((StringRepresantable*)repr)->repr();
}

Label::Label(std::string text) {
	type = "p";
	setText(text);
	setClass(CLASS_NAME(Label));
}

GenericDialog::GenericDialog(std::string title, std::string message) :Container::Container() {
	setClass(CLASS_NAME(GenericDialog));
	setLayoutOrientation(Container::Layout::Vertical);
	style["display"] = "block";
	style["overflow"] = "auto";

	this->event_onconfirm = new onconfirm(this);
	this->event_oncancel = new oncancel(this);

	if (title.length() > 0) {
		Label* l = new Label(title);
		l->addClass("DialogTitle"); //FIXME: css class named "DialogTitle" should be "GenericDialogTitle"?
		addChild(l);
	}

	if (message.length() > 0) {
		Label* l = new Label(message);
		l->style["margin"] = "5px";
		addChild(l);
	}

	_container = new Container();
	_container->style["display"] = "block";
	_container->style["overflow"] = "auto";
	_container->style["margin"] = "5px";
	_container->setLayoutOrientation(Container::Layout::Vertical);

	_confirmButton = new Button("Ok");
	_confirmButton->setSize(100, 30);
	_confirmButton->style["margin"] = "3px";
	(*_confirmButton->event_onclick) >> (EventListener*)this->event_onconfirm >> (EventListener::listener_class_member_type) & GenericDialog::onconfirm::operator();

	_cancelButton = new Button("Cancel");
	_cancelButton->setSize(100, 30);
	_cancelButton->style["margin"] = "3px";
	(*_cancelButton->event_onclick) >> (EventListener*)this->event_oncancel >> (EventListener::listener_class_member_type) & GenericDialog::oncancel::operator();

	_hLay = new HBox();
	_hLay->setHeight(35);
	_hLay->style["display"] = "block";
	_hLay->style["overflow"] = "visible";
	_hLay->addChild(_confirmButton);
	_hLay->addChild(_cancelButton);

	_confirmButton->style["float"] = "right";
	_cancelButton->style["float"] = "right";

	addChild(_container);
	addChild(_hLay);

}

void GenericDialog::addFieldWithLabel(std::string key, std::string label_description, Widget* field) {
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

void GenericDialog::addField(std::string key, Widget* field) {
	this->_inputs[key] = field;
	Container* container = new Container();
	container->style.set("display", "block");
	container->style.set("overflow", "auto");
	container->style.set("padding", "3px");
	container->setLayoutOrientation(Container::Layout::Horizontal);
	container->addChild(this->_inputs[key], key);
	this->_container->addChild(container, key);
}

Widget* GenericDialog::getField(std::string key) {
	return this->_inputs[key];
}



Image::Image(std::string url) {
	type = "img";
	setClass(CLASS_NAME(Image));
	setURL(url);
}

void Image::setURL(std::string url) {
	this->attributes["src"] = url;
}

std::string Image::url() {
	return this->attributes["src"];
}

Input::Input() {
	type = "input";
	/*
	attributes[Event_OnClick] = "";
	attributes[Event_OnChange] = utils::sformat( \
			"var params={};params['value']=document.getElementById('%(id)s').value;" \
			"sendCallbackParam('%s','%s',params);" , getIdentifier().c_str() , Event_OnChange.c_str() );
	*/
}

void Input::setValue(std::string value) {
	attributes["value"] = value;
}

std::string Input::getValue() {
	return attributes["value"];
}

void Input::setEnable(bool on) {

	if (on)
		attributes.remove("disabled");
	else
		attributes["disabled"] = "disabled";

}

bool Input::isEnable() {
	return attributes.has("disabled");
}

void Input::setReadOnly(bool on) {

	if (on)
		attributes.remove("readonly");
	else
		attributes["readonly"] = "readonly";

}

bool Input::isReadOnly() {
	return attributes.has("readonly");
}


FileUploader::FileUploader(std::string path, bool multipleSelectionAllowed) {
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

void FileUploader::setSavePath(std::string path) {
	this->_path = path;
}
std::string FileUploader::savePath() {
	return this->_path;
}

void FileUploader::setMultipleSelectionAllowed(bool value) {
	this->_multipleSelectionAllowed = value;
	if (this->_multipleSelectionAllowed) {
		this->attributes["multiple"] = "multiple";
	}
	else {
		if (this->attributes.has("multiple")) {
			this->attributes.remove("multiple");
		}
	}
}
bool FileUploader::multipleSelectionAllowed() {
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
