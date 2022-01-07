# cREMI
C++ port of [remi](https://github.com/dddomodossola/remi) GUI project


<p align="center">
    <img src="https://raw.githubusercontent.com/dddomodossola/remi/master/remi/res/logo.png" width="430">
</p>

<h2 align="center" style="font-weight:bolder">
    GUI library for your C++ applications
</h2>

<p align="center" style="white-space:pre">
cRemi is a GUI library for C++ applications that gets rendered in web browsers. 
This allows you to access your interface locally and remotely.
</p>

Remi enables developers to create platform independent GUI with C++. The entire GUI is rendered in your browser. **No HTML** is required, Remi automatically translates your code into HTML at runtime. When your app starts, it starts a web server that will be accessible on your network.

A basic application appears like this:
```c++
#include "remi_server.h"
#include <string.h>
#include <fstream>

#pragma execution_character_set( "utf-8" )

using namespace std;
using namespace remi;
using namespace remi::utils;

class HelloWorld : public remi::server::App {
private:
	remi::AsciiContainer* mainContainer;

	remi::Button* btn1;
	remi::Label* label;

public:

	Widget* main() {

		mainContainer = new remi::AsciiContainer(
			R"(
			|label          |
			|button         |
			)", 1.0, 1.0
		);

		mainContainer->style.set("width", "500px");
		mainContainer->style.set("height", "600px");

		label = new remi::Label("CRemi");
		label->css_background_color = "yellow";
		label->css_text_align = "center";
		label->css_font_size = "100px";
		mainContainer->append(label, "label");

		btn1 = new remi::Button("Say hello");
		/* registering a class member listener */
		LINK_EVENT_TO_CLASS_MEMBER(remi::Button::onclick, btn1->event_onclick, this, &HelloWorld::onClick);
		mainContainer->append(btn1, "button");

		return mainContainer;
	}

	void onClick(EventSource* emitter, void* userdata) {
		this->label->setText("Hello World!");
	}


};

int main() {
	remi::server::Server<HelloWorld>* srv = new remi::server::Server<HelloWorld>();
	srv->start("127.0.0.1");
	getchar();
	return 0;
}
```

Run the application then, open the browser at the address "http://127.0.0.1:91".