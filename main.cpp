#include "remi_server.h"


#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <sys\types.h>
//#include <winsock2.h>
#endif
#include <string.h>

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


class TestApp : public remi::server::App , public EventListener {
private:
	remi::Widget *mainContainer;
	remi::TextInput* ti1;

	remi::GenericDialog* dialog;

public:

	virtual Widget* main(){
		mainContainer = new remi::VBox();

		//mainContainer->addClass("myclass2");
		mainContainer->style.set("width", "200px");
		mainContainer->style.set("height", "200px");
		//mainContainer->style.set("background-color", "red");
		//tag1->setOnClickListener(this);

		ti1 = new remi::TextInput();
		ti1->setOnChangeListener(this);
		ti1->style.set("width", "100px");
		ti1->style.set("height", "24px");
		mainContainer->addChild(ti1);

		remi::Button* btn1 = new remi::Button("Show dialog");
		btn1->setOnClickListener( this );
		btn1->style.set("width", "100px");
		mainContainer->addChild(btn1);

		dialog = new remi::GenericDialog("Dialog", "This is a generic input dialog");
		remi::TextInput* ti2 = new remi::TextInput(true);
		dialog->add_field_with_label("input", "Insert a text", ti2);
		dialog->setOnConfirmListener(this);
		dialog->setOnCancelListener(this);

		return mainContainer;
	}

	virtual void onEvent( std::string eventName , Event* eventData ){
		if ( eventName == Widget::Event_OnClick ){
			std::cout << "TestApp." << eventName ;
			show(dialog);
		}

		if ( eventName == GenericDialog::Event_OnConfirm ){
			std::cout << "Inserted text:" << eventName;
			show(mainContainer);
		}
	}

};

Dictionary<int> t(){
	Dictionary<int> a1;
	a1["a1"] = 10;

	return a1;
}

int main() {

	Dictionary<int> a2 = t();

	remi::server::Server<TestApp>* srv = new remi::server::Server<TestApp>();
	srv->start();
	
	//remi::utils::open_browser( "http://localhost:91" );

	(void)getchar();

	return 0;
}
