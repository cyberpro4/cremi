#include "remi_server.h"


#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <sys\types.h>
//#include <winsock2.h>
#endif
#include <string.h>
#include <fstream>

using namespace std;
using namespace remi;
using namespace remi::utils;

int run_test(){

#ifdef _WIN32


	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;

	return 0;

#else

	return 1;

#endif

}


void funcOnclick(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata){
	remi::Button* btn1 = (remi::Button*)userdata;
	std::cout << "Event onClick btn1" << endl;
	remi::Button* btn1_cast = static_cast<remi::Button*>(emitter);
	
	btn1_cast->style.set("background-color", "orange");
}


class TestApp : public remi::server::App{
private:
	//remi::Widget*			mainContainer;
	remi::AsciiContainer*	mainContainer;

	remi::Button*			btn1;
	remi::Button*			btn2;
	remi::Button*			btn3;

	remi::GenericDialog*	dialog;
	remi::Label*			label;

	remi::Image*			image;

	remi::FileUploader*		fileUploader;

	int counter;

public:
    void idle(){
        label->setText(utils::sformat("%d", counter));
        counter++;
    }

	Widget* main(){
        counter = 0;
		//mainContainer = new remi::HBox();
		mainContainer = new remi::AsciiContainer(
			R"(
			|label | button |
			|button2 |bt3   |
			|button2 |bt3   |
			)", 10.0, 10.0
		);
				
		//mainContainer->addClass("myclass2");
		mainContainer->style.set("width", "500px");
		mainContainer->style.set("height", "600px");
		//mainContainer->style.set("background-color", "red");
		//tag1->setOnClickListener(this);

		label = new remi::Label("CRemi");
		mainContainer->append(label, "label");

		btn1 = new remi::Button("Show generic dialog");
		
		/* registering a class member listener */
		btn1->event_onclick->_do(this, (EventListener::listener_class_member_type)&TestApp::onClick);
		
		/* registering a function listener */
		//btn1->event_onclick->_do(&funcOnclick, btn1);
		
		/* registering a lambda expression listener */
		//btn1->event_onclick->_do((Event::listener_function_type)[](EventSource* emitter, Dictionary<Buffer*>* params, void* userdata){static_cast<remi::Button*>(emitter)->style.set("background-color", "green");});
		
		/* registering a context capturing lambda expression listener */
		//btn1->event_onclick->_do([this](EventSource* emitter, Dictionary<Buffer*>* params, void* userdata){this->btn1->style.set("background-color", "purple");});
		
		btn1->event_onmousedown->_do(this, (EventListener::listener_class_member_type)&TestApp::onMouseDown);
		//btn1->style.set("width", "100px");
		mainContainer->append(btn1, "button");
		
		btn2 = new remi::Button("bt2");
		btn2->event_onclick->_do(this, (EventListener::listener_class_member_type)&TestApp::onBt2Click);
		
		btn3 = new remi::Button("bt3");
		mainContainer->append(btn2, "button2");
		mainContainer->append(btn3, "bt3");

		dialog = new remi::GenericDialog("POTATO", "Chips");
		(*dialog->event_onconfirm) >> this >> (EventListener::listener_class_member_type)&TestApp::dialogOnConfirm;

		return mainContainer;
	}

	void onBt2Click(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata){
		label->style["color"] = "red";
	}

	void dialogOnConfirm(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata){
		std::cout << "Dialog Confirmed" << endl;
		this->setRootWidget(this->mainContainer);
	}

	void onQueryResult(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata){
		std::cout << "Event onquery result" << endl;
		for(std::string key : params->keys()){
            std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
        }
	}

	void onClick(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata){
		std::cout << "Event onClick btn1" << endl;
		this->btn1->style["background-color"] = "red";
		this->setRootWidget(this->dialog);
		this->executeJavascript("alert('hello');");
		/*
		std::list<std::string> _attrs = {"id", "class"};
		std::list<std::string> _styles = {"width", "height"};
		(*this->btn1->event_onqueryClientResult) >> this >> (EventListener::listener_class_member_type)&TestApp::onQueryResult;
		this->btn1->queryClient(this, _attrs, _styles);
		*/
	}
	
	void onMouseDown(EventSource* emitter, Dictionary<Buffer*>* params, void* userdata){
		std::cout << "Event onMouseDown btn1" << endl;
		for(std::string key : params->keys()){
            std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
        }
	}

    void onpageshow(void* emitter, Dictionary<Buffer*>* params, void* user_data){
        std::cout << "Event onPageShow - ";
        for(std::string key : params->keys()){
            std::cout << "param_name: " << key << "  value: " << params->get(key)->str() << endl;
        }
        int width = 0;
        int height = 0;
        sscanf(params->get("width")->str().c_str(), "%d", &width);
        sscanf(params->get("height")->str().c_str(), "%d", &height);
        cout << width << " - " << height << endl;
    }

    void onpageerror(Tag* emitter, Dictionary<Buffer*>* params, void* userdata){
        std::cout << "Event onPageError - ";
        for(std::string key : params->keys()){
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
	/*int i = 0;
	for(i=0;i<1000000;i++){
		remi::server::ServerResponse* s = new remi::server::ServerResponse();
		s->appendToBody("patatas fritas");
		delete s;
	}*/
	
	remi::server::Server<TestApp>* srv = new remi::server::Server<TestApp>();
	srv->start();

	//remi::utils::open_browser( "http://localhost:91" );

	(void)getchar();

	return 0;
}
