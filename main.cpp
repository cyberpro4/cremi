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



void test(){
    cout << "tick" << endl;
}

class TestApp : public remi::server::App , public remi::EventManagerListener {

public:

	virtual Widget* main(){
		remi::Widget *tag1 = new remi::Widget("div");

		tag1->addClass("myclass2");
		tag1->style.set("width", "100px");
		tag1->style.set("height", "200px");
		tag1->style.set("background-color", "red");
		tag1->setOnClickListener(this);
	
		return tag1;
	}

	virtual void onEvent( std::string eventName ){
	}

};



int main() {

	remi::server::Server<TestApp>* srv = new remi::server::Server<TestApp>();
	srv->start();

	(void)getchar();

	return 0;
}
