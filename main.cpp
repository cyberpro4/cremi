#include "remi_server.h"


#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <sys\types.h>
#include <windows.h>
//#include <winsock2.h>
#endif
#include <string.h>
#include <fstream>

#pragma execution_character_set( "utf-8" )
#pragma warning(disable : 4996)

using namespace std;
using namespace remi;
using namespace remi::utils;


void funcOnclick(EventSource* emitter, void* userdata) {
	remi::Button* btn1 = (remi::Button*)userdata;
	std::cout << "Event onClick btn1" << endl;
	remi::Button* btn1_cast = static_cast<remi::Button*>(emitter);

	btn1_cast->style.set("background-color", "orange");
}


class TestApp : public remi::server::App, 
	public FileUploader::ondata::EventListener, 
	public Event<string, string>::EventListener, 
	public Event<string>::EventListener,
	public Event<ListItem*>::EventListener {
private:
	remi::AsciiContainer* mainContainer;

	remi::Button*		btn1;
	remi::Button*		btn2;
	remi::Button*		btn3;

	remi::GenericDialog* dialog;
	remi::Label*		label;
	
	remi::TextInput*	txtInput;

	remi::Image*		image;

	remi::FileUploader* fileUploader;

	remi::Progress*		progress;

	remi::ListView*		listView;

	int counter;

public:
	void idle() {
		label->setText(utils::sformat("%d", counter));
		progress->setValue(counter % 100);
		counter++;
	}

	Widget* main() {
		counter = 0;

		mainContainer = new remi::AsciiContainer(
			R"(
			|   |image  |   | listView |
			|label | button | listView |
			|button2 |bt3   | listView |
			|button2 |txt   | listView |
			|progress       | listView |
		    |file_uploader  | listView |
			)", 1.0, 1.0
		);

		mainContainer->style.set("width", "500px");
		mainContainer->style.set("height", "600px");

		this->_staticResourcesPaths["res"] = "./res/";
		image = new Image("/res:/logo.png");
		LINK_EVENT_TO_CLASS_MEMBER(remi::Image::onclick, image->event_onclick, this, &TestApp::onClickImage);
		mainContainer->append(image, "image");

		label = new remi::Label("CRemi");
		label->css_background_color = "yellow";
		label->css_text_align = "center";
		label->css_font_size = "100px";
		mainContainer->append(label, "label");

		btn1 = new remi::Button("Show generic dialog");
		
		/* registering a class member listener */
		LINK_EVENT_TO_CLASS_MEMBER(remi::Button::onclick, btn1->event_onclick, this, &TestApp::onClick);

		/* registering a function listener */
		//btn1->event_onclick->link(&funcOnclick, btn1);

		/* registering a lambda expression listener */
		//btn1->event_onclick->link((Event::listener_function_type)[](EventSource* emitter, void* userdata){static_cast<remi::Button*>(emitter)->style.set("background-color", "green");});

		/* registering a context capturing lambda expression listener */
		//btn1->event_onclick->link([this](EventSource* emitter, void* userdata){this->btn1->style.set("background-color", "purple");});

		LINK_EVENT_TO_CLASS_MEMBER(remi::Button::onmousedown, btn1->event_onmousedown, this, &TestApp::onMouseDown);
		mainContainer->append(btn1, "button");

		btn2 = new remi::Button("bt2");
		LINK_EVENT_TO_CLASS_MEMBER(remi::Button::onclick, btn2->event_onclick, this, &TestApp::onBt2Click);

		btn3 = new remi::Button("bt3");
		mainContainer->append(btn2, "button2");
		mainContainer->append(btn3, "bt3");

		dialog = new remi::GenericDialog("POTATO", "Chips");
		LINK_EVENT_TO_CLASS_MEMBER(remi::GenericDialog::onconfirm, dialog->event_onconfirm, this, &TestApp::dialogOnConfirm);
		
		txtInput = new remi::TextInput(true, "type here");
		mainContainer->append(txtInput, "txt");
		LINK_EVENT_TO_CLASS_MEMBER(remi::TextInput::onkeyup, txtInput->event_onkeyup, this, &TestApp::txtInputOnkeyup);
		LINK_EVENT_TO_CLASS_MEMBER(remi::TextInput::onchange, txtInput->event_onchange, this, &TestApp::txtInputOnchange);
		
		progress = new remi::Progress(0, 100);
		mainContainer->append(progress, "progress");

		fileUploader = new remi::FileUploader();
		LINK_EVENT_TO_CLASS_MEMBER(remi::FileUploader::ondata, fileUploader->event_ondata, this, &TestApp::onData);
		LINK_EVENT_TO_CLASS_MEMBER(remi::FileUploader::onsuccess, fileUploader->event_onsuccess, this, &TestApp::onSuccess);
		LINK_EVENT_TO_CLASS_MEMBER(remi::FileUploader::onfail, fileUploader->event_onfail, this, &TestApp::onFail);
		mainContainer->append(fileUploader, "file_uploader");

		listView = ListView::newFromVectorOfStrings(vector<std::string>{ "item1", "item2", "item3", "item4" });
		mainContainer->append(listView, "listView");
		LINK_EVENT_TO_CLASS_MEMBER(Event<ListItem*>, listView->event_onselection, this, &TestApp::onListItemSelected);

		return mainContainer;
	}

	void onListItemSelected(EventSource* listView, ListItem* item, void* userdata) {
		std::cout << "event onselection " << item->text() << endl;
		item->setText(item->text() + "!");
	}

	void txtInputOnkeyup(EventSource* emitter, std::string newValue, std::string keycode, void* userdata){
		std::cout << "event onkeyup new value:" << newValue << " keycode:" << keycode << endl;
	}
	
	void txtInputOnchange(EventSource* emitter, std::string newValue, void* userdata){
		std::cout << "event onchange new value:" << newValue << endl;
	}

	void onBt2Click(EventSource* emitter, void* userdata) {
		label->css_color = "red";
	}

	void dialogOnConfirm(EventSource* emitter, void* userdata) {
		std::cout << "Dialog Confirmed" << endl;
		this->setRootWidget(this->mainContainer);
	}

	void onQueryResult(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata) {
		std::cout << "Event onquery result" << endl;
		for (std::string key : params->keys()) {
			std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
		}
	}

	void onClickImage(EventSource* emitter, void* userdata) {
		std::cout << "Image clicked" << endl;
	}

	void onClick(EventSource* emitter, void* userdata) {
		std::cout << "Event onClick btn1" << endl;
		this->btn1->css_background_color = "red";
		this->setRootWidget(this->dialog);
		this->executeJavascript("alert('hello');");
		/*
		std::list<std::string> _attrs = {"id", "class"};
		std::list<std::string> _styles = {"width", "height"};
		(*this->btn1->event_onqueryClientResult) >> this >> (EventListener::listener_class_member_type)&TestApp::onQueryResult;
		this->btn1->queryClient(this, _attrs, _styles);
		*/
	}

	void onMouseDown(EventSource* emitter, float x, float y, void* userdata) {
		std::cout << "Event onMouseDown btn1 x:" << x << " y:" << y << endl;
	}

	void onpageshow(void* emitter, float width, float height, void* user_data) {
		std::cout << "Event onPageShow - ";
		cout << width << " - " << height << endl;
	}

	void onpageerror(Tag* emitter, Dictionary<Buffer*>* params, void* userdata) {
		std::cout << "Event onPageError - ";
		for (std::string key : params->keys()) {
			std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
		}
	}

	void onData(EventSource* emitter, std::string filename, Buffer* data, std::string content_type, void* userdata) {
		/*
		std::cout << "Event onData - ";
		for (std::string key : params->keys()) {
			std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
		}
		*/

		remi::FileUploader* fu = static_cast<remi::FileUploader*>(emitter);
		FILE* f = fopen((fu->savePath() + "/" + filename).c_str(), "a+b");
		fwrite(data->data, sizeof(char), data->len, f);
		fclose(f);

	}
	void onSuccess(Tag* emitter, std::string filename, void* userdata) {
		std::cout << "Event onSuccess - filename:" << filename << endl;
	}
	void onFail(Tag* emitter, std::string filename, void* userdata) {
		std::cout << "Event onFail - filename:" << filename << endl;
	}

};


int main() {
#ifdef _WIN32
	SetConsoleOutputCP(65001);
#endif
	remi::server::Server<TestApp>* srv = new remi::server::Server<TestApp>();
	srv->start("127.0.0.1");

	//remi::utils::open_browser( "http://localhost:91" );

	getchar();

	return 0;
}
