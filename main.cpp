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
	remi::Widget*			mainContainer;

	remi::Button*			btn1;
	remi::Button*			btn2;

	remi::GenericDialog*	dialog;
	remi::Label*			label;

	remi::Image*			image;

	remi::FileUploader*		fileUploader;

	int counter;

public:
    /*void idle(){
        label->setText(utils::sformat("%d", counter));
        counter++;
    }*/

	Widget* main(){
        counter = 0;
		mainContainer = new remi::HBox();
		
		//mainContainer->addClass("myclass2");
		mainContainer->style.set("width", "500px");
		mainContainer->style.set("height", "600px");
		//mainContainer->style.set("background-color", "red");
		//tag1->setOnClickListener(this);

		label = new remi::Label("CRemi");
		mainContainer->addChild(label);

		btn1 = new remi::Button("Show generic dialog");
		
		btn1->event_onclick->_do(this, (EventListener::listener_class_member_type)&TestApp::onClick);
		btn1->event_onclick->_do((Event::listener_function_type)&funcOnclick, btn1);
		btn1->style.set("width", "100px");
		mainContainer->addChild(btn1);

		return mainContainer;
	}


	void onClick(Tag* emitter, Dictionary<Buffer*>* params, void* userdata){
		std::cout << "Event onClick btn1" << endl;
		this->btn1->style.set("background-color", "red");
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

	remi::server::Server<TestApp>* srv = new remi::server::Server<TestApp>();
	srv->start();

	//remi::utils::open_browser( "http://localhost:91" );

	(void)getchar();

	return 0;
}
