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

using namespace std;
using namespace remi;
using namespace remi::utils;


void funcOnclick(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata) {
	remi::Button* btn1 = (remi::Button*)userdata;
	std::cout << "Event onClick btn1" << endl;
	remi::Button* btn1_cast = static_cast<remi::Button*>(emitter);

	btn1_cast->style.set("background-color", "orange");
}


class TestApp : public remi::server::App {
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
			|   |image  |   |
			|label | button |
			|button2 |bt3   |
			|button2 |txt   |
			|progress       |
			)", 1.0, 1.0
		);

		mainContainer->style.set("width", "500px");
		mainContainer->style.set("height", "600px");

		this->_staticResourcesPaths["res"] = "./res/";
		image = new Image("/res:/logo.png");
		LINK_EVENT_TO_CLASS_MEMBER(image->event_onclick, this, &TestApp::onClickImage);
		mainContainer->append(image, "image");

		label = new remi::Label("CRemi");
		label->css_background_color = "yellow";
		label->css_text_align = "center";
		label->css_font_size = "100px";
		mainContainer->append(label, "label");

		btn1 = new remi::Button("Show generic dialog");
		
		/* registering a class member listener */
		LINK_EVENT_TO_CLASS_MEMBER(btn1->event_onclick, this, &TestApp::onClick);

		/* registering a function listener */
		//btn1->event_onclick->link(&funcOnclick, btn1);

		/* registering a lambda expression listener */
		//btn1->event_onclick->link((Event::listener_function_type)[](EventSource* emitter, Dictionary<Buffer*>* params, void* userdata){static_cast<remi::Button*>(emitter)->style.set("background-color", "green");});

		/* registering a context capturing lambda expression listener */
		//btn1->event_onclick->link([this](EventSource* emitter, Dictionary<Buffer*>* params, void* userdata){this->btn1->style.set("background-color", "purple");});

		LINK_EVENT_TO_CLASS_MEMBER(btn1->event_onmousedown, this, &TestApp::onMouseDown);
		mainContainer->append(btn1, "button");

		btn2 = new remi::Button("bt2");
		LINK_EVENT_TO_CLASS_MEMBER(btn2->event_onclick, this, &TestApp::onBt2Click);

		btn3 = new remi::Button("bt3");
		mainContainer->append(btn2, "button2");
		mainContainer->append(btn3, "bt3");

		dialog = new remi::GenericDialog("POTATO", "Chips");
		LINK_EVENT_TO_CLASS_MEMBER( dialog->event_onconfirm, this, &TestApp::dialogOnConfirm);
		
		txtInput = new remi::TextInput(true, "type here");
		mainContainer->append(txtInput, "txt");
		LINK_EVENT_TO_CLASS_MEMBER(txtInput->event_onkeyup, this, &TestApp::txtInputOnkeyup);
		LINK_EVENT_TO_CLASS_MEMBER(txtInput->event_onchange, this, &TestApp::txtInputOnchange);
		
		progress = new remi::Progress(0, 100);
		mainContainer->append(progress, "progress");

		fileUploader = new remi::FileUploader();
		LINK_EVENT_TO_CLASS_MEMBER(fileUploader->event_ondata, this, &TestApp::onData);
		LINK_EVENT_TO_CLASS_MEMBER(fileUploader->event_onsuccess, this, &TestApp::onSuccess);
		LINK_EVENT_TO_CLASS_MEMBER(fileUploader->event_onfail, this, &TestApp::onFail);
		mainContainer->append(fileUploader);

		return mainContainer;
	}

	void txtInputOnkeyup(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata){
		std::cout << "event onkeyup" << endl;
		for (std::string key : params->keys()) {
			std::cout << "param_name: " << key << "  value: ";
			std::cout << params->get(key)->str() << endl;
		}
	}
	
	void txtInputOnchange(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata){
		std::cout << "event onchange" << endl;
		for (std::string key : params->keys()) {
			std::cout << "param_name: " << key << "  value: ";
			std::cout << params->get(key)->str() << endl;
		}
	}

	void onBt2Click(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata) {
		label->css_color = "red";
	}

	void dialogOnConfirm(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata) {
		std::cout << "Dialog Confirmed" << endl;
		this->setRootWidget(this->mainContainer);
	}

	void onQueryResult(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata) {
		std::cout << "Event onquery result" << endl;
		for (std::string key : params->keys()) {
			std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
		}
	}

	void onClickImage(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata) {
		std::cout << "Image clicked" << endl;
	}

	void onClick(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata) {
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

	void onMouseDown(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata) {
		std::cout << "Event onMouseDown btn1" << endl;
		for (std::string key : params->keys()) {
			std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
		}
	}

	void onpageshow(void* emitter, Dictionary<Buffer*>* params, void* user_data) {
		std::cout << "Event onPageShow - ";
		for (std::string key : params->keys()) {
			std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
		}
		int width = 0;
		int height = 0;
		sscanf(params->get("width")->str().c_str(), "%d", &width);
		sscanf(params->get("height")->str().c_str(), "%d", &height);
		cout << width << " - " << height << endl;
	}

	void onpageerror(Tag* emitter, Dictionary<Buffer*>* params, void* userdata) {
		std::cout << "Event onPageError - ";
		for (std::string key : params->keys()) {
			std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
		}
	}

	void onData(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata) {
		/*
		std::cout << "Event onData - ";
		for (std::string key : params->keys()) {
			std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
		}
		*/
		const char* data = params->get("data")->str().c_str();
		std::string filename = params->get("filename")->str();
		if (params->has("content_type"));
		if (params->has("transfer_encoding"));

		remi::FileUploader* fu = static_cast<remi::FileUploader*>(emitter);
		FILE* f = fopen((fu->savePath() + "/" + filename).c_str(), "w+b");
		fwrite(data, sizeof(char), params->get("data")->len, f);
		fclose(f);

	}
	void onSuccess(Tag* emitter, Dictionary<Buffer*>* params, void* userdata) {
		std::cout << "Event onSuccess - ";
		for (std::string key : params->keys()) {
			std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
		}
	}
	void onFail(Tag* emitter, Dictionary<Buffer*>* params, void* userdata) {
		std::cout << "Event onFail - ";
		for (std::string key : params->keys()) {
			std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
		}
	}

	/*void onData(FileUploader* w, std::string fileName, const char* data, unsigned long long len){
		std::ostringstream filePathName;
		filePathName << w->savePath() << "/" << fileName;
		label->setText("File: " + filePathName.str());
		FILE* myFile = fopen(filePathName.str().c_str(), "wb");
		fwrite(data, sizeof(char), len, myFile);
		fclose(myFile);
		//label->setText(data);
	}*/
};


int main() {
#ifdef _WIN32
	SetConsoleOutputCP(65001);
#endif
	remi::server::Server<TestApp>* srv = new remi::server::Server<TestApp>();
	srv->start();

	//remi::utils::open_browser( "http://localhost:91" );

	getchar();

	return 0;
}
