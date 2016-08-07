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


class TestApp : public remi::server::App , 
				public Widget::WidgetOnClickListener,
				public Widget::WidgetOnChangeListener,
				public ListView::ListViewOnSelectionListener, 
				public GenericDialog::GenericDialogOnConfirmListener,
				public GenericDialog::GenericDialogOnCancelListener,
				public FileUploader::FileUploaderOnDataListener{
private:
	remi::Widget*			mainContainer;
	
	remi::Button*			btn1;
	remi::Button*			btn2;
	
	remi::TextInput*		ti1;

	remi::GenericDialog*	dialog;
	remi::Label*			label;

	remi::ListView*			listView;

	remi::InputDialog*		inputDialog;

	remi::Image*			image;

	remi::FileUploader*		fileUploader;

public:

	virtual Widget* main(){
		mainContainer = new remi::VBox();

		//mainContainer->addClass("myclass2");
		mainContainer->style.set("width", "500px");
		mainContainer->style.set("height", "600px");
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
		ti1->onChangeListener = this;
		ti1->style.set("width", "100px");
		ti1->style.set("height", "24px");
		mainContainer->addChild(ti1);

		btn1 = new remi::Button("Show generic dialog");
		btn1->onClickListener = this;
		btn1->style.set("width", "100px");
		mainContainer->addChild(btn1);

		btn2 = new remi::Button("Show input dialog");
		btn2->onClickListener = this;
		btn2->style.set("width", "100px");
		mainContainer->addChild(btn2);

		dialog = new remi::GenericDialog("Dialog", "This is a generic input dialog");
		dialog->style["width"] = "500px";
		remi::TextInput* ti2 = new remi::TextInput(true);
		dialog->addFieldWithLabel("input", "Insert a text", ti2);
		dialog->onConfirmListener = this;
		dialog->onCancelListener = this;

		inputDialog = new remi::InputDialog("Input dialog", "Type a value:");
		inputDialog->style["width"] = "500px";
		inputDialog->onCancelListener = this;
		inputDialog->onConfirmListener = this;

		image = new remi::Image("http://vignette1.wikia.nocookie.net/matrix/images/f/f5/Matrix_Wiki_Seraph_Matrix.png/revision/latest?cb=20120808072031");
		image->setSize(400, 300);
		mainContainer->addChild(image);

		fileUploader = new remi::FileUploader();
		fileUploader->onDataListener = this;
		mainContainer->addChild(fileUploader);

		return mainContainer;
	}


	void onSelection(ListView* list, ListItem* item){
		std::ostringstream o;
		o << "ListView selection: " << item->text();
		label->setText(o.str());
	}

	void onConfirm(GenericDialog* dialog){
		if (dialog == this->dialog){
			std::ostringstream o;
			o << "Dialog confirmed: " << ((remi::TextInput*)dialog->getField("input"))->text();
			label->setText(o.str());
			((remi::TextInput*)dialog->getField("input"))->setText("");
		}else if(dialog == this->inputDialog){
			std::ostringstream o;
			o << "Input confirmed: " << inputDialog->text();
			label->setText(o.str());
			inputDialog->setText("");
		}
		show(mainContainer);
	}

	void onCancel(GenericDialog* dialog){
		if (dialog == this->dialog){
			label->setText("Dialog canceled.");
			((remi::TextInput*)dialog->getField("input"))->setText("");
		}else if (dialog == this->inputDialog){
			label->setText("Input canceled.");
			inputDialog->setText("");
		}
		show(mainContainer);
	}

	void onClick(Widget* w){
		if (w == this->btn1){
			std::cout << "Event onClick" << endl;
			show(dialog);
		}else if(w == this->btn2){
			std::cout << "Event onClick" << endl;
			inputDialog->show( this );
		}
	}

	void onChange(Widget* w){
		std::ostringstream o;
		o << "Text changed: " << ti1->text();
		label->setText(o.str());
	}

	void onData(FileUploader* w, std::string fileName, const char* data, unsigned long long len){
		std::ostringstream filePathName;
		filePathName << w->savePath() << "/" << fileName;
		label->setText("File: " + filePathName.str());
		FILE* myFile = fopen(filePathName.str().c_str(), "wb");
		fwrite(data, sizeof(char), len, myFile);
		fclose(myFile);
		//label->setText(data);
	}
};



int main() {


	remi::server::Server<TestApp>* srv = new remi::server::Server<TestApp>();
	srv->start();
	
	//remi::utils::open_browser( "http://localhost:91" );

	(void)getchar();

	return 0;
}
