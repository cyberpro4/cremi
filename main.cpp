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


class TestApp : public remi::server::App , 
				public EventListener, 
				public ListView::ListViewOnSelectionListener, 
				public GenericDialog::GenericDialogOnConfirmListener,
				public GenericDialog::GenericDialogOnCancelListener {
private:
	remi::Widget*			mainContainer;
	remi::TextInput*		ti1;

	remi::GenericDialog*	dialog;
	remi::Label*			label;

	remi::ListView*			listView;

public:

	virtual Widget* main(){
		mainContainer = new remi::VBox();

		//mainContainer->addClass("myclass2");
		mainContainer->style.set("width", "300px");
		mainContainer->style.set("height", "500px");
		//mainContainer->style.set("background-color", "red");
		//tag1->setOnClickListener(this);

		listView = new ListView();
		listView->addChild(new ListItem("item1"));
		listView->addChild(new ListItem("item2"));
		listView->onSelectionListener = this;
		listView->style["padding"] = "0px";
		listView->style["width"] = "200px";
		mainContainer->addChild(listView);

		
		label = new remi::Label("CRemi");
		mainContainer->addChild(label);

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
		dialog->onConfirmListener = this;
		dialog->onCancelListener = this;

		return mainContainer;
	}

	virtual void onEvent( std::string eventName , Event* eventData ){
		std::ostringstream o;
		if ( eventName == Widget::Event_OnClick ){
			std::cout << "TestApp." << eventName << endl ;
			show(dialog);
		}
	}

	void onSelection(ListView* list, ListItem* item){
		std::ostringstream o;
		o << "ListView selection: " << item->text();
		label->setText(o.str());
	}

	void onConfirm(GenericDialog* dialog){
		std::ostringstream o;
		o << "Dialog confirmed: " << ((remi::TextInput*)dialog->get_field("input"))->text();
		label->setText(o.str());
		((remi::TextInput*)dialog->get_field("input"))->setText("");
		show(mainContainer);
	}

	void onCancel(GenericDialog* dialog){
		label->setText("Dialog canceled.");
		((remi::TextInput*)dialog->get_field("input"))->setText("");
		show(mainContainer);
	}

};



int main() {


	remi::server::Server<TestApp>* srv = new remi::server::Server<TestApp>();
	srv->start();
	
	//remi::utils::open_browser( "http://localhost:91" );

	(void)getchar();

	return 0;
}
