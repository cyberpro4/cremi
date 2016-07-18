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


class TestApp : public remi::server::App , public EventManagerListener {

public:

	virtual Widget* main(){
		remi::Widget *tag1 = new remi::Widget("div");

		tag1->addClass("myclass2");
		tag1->style.set("width", "100px");
		tag1->style.set("height", "200px");
		tag1->style.set("background-color", "red");
		tag1->setOnClickListener(this);

		remi::TextInput* ti1 = new remi::TextInput();
		tag1->addChild( ti1 );

		remi::Button* btn1 = new remi::Button("Press me");
		tag1->addChild( btn1 );
	
		return tag1;
	}

	virtual void onEvent( std::string eventName ){
		cout << "App::" << eventName;
	}

};



int main() {


	remi::server::Server<TestApp>* srv = new remi::server::Server<TestApp>();
	srv->start();

	//remi::utils::open_browser( "http://localhost:91" );

	(void)getchar();

	return 0;
}
