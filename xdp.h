
#define XDP_BEGIN_DATA struct __xdp_deep_struct {

#define XDP_END_DATA int __xdp_deep_data_c; }; __xdp_deep_struct *__xdp_deep_data;

#define XDP_INIT __xdp_deep_data = new __xdp_deep_struct(); __xdp_deep_data->__xdp_deep_data_c = 0;

#define XDP_DELETE if( __xdp_deep_data->__xdp_deep_data_c < 1 ) delete __xdp_deep_data; else __xdp_deep_data->__xdp_deep_data_c--;

#define XDP_COPY(from) __xdp_deep_data = from.__xdp_deep_data;__xdp_deep_data->__xdp_deep_data_c++;

#define XDP_DEFER															\
	if( __xdp_deep_data->__xdp_deep_data_c != 0 ){							\
		__xdp_deep_struct *dt = __xdp_deep_data;							\
		__xdp_deep_data = new __xdp_deep_struct();							\
		memcpy(__xdp_deep_data, dt, sizeof(__xdp_deep_struct));				\
		__xdp_deep_data->__xdp_deep_data_c = 0;								\
		dt->__xdp_deep_data_c--;											\
	}																		\

#define XDP __xdp_deep_data


/*


class MyClass {

public:

	MyClass(){
		XDP_INIT;
		XDP->a = 1000;
	}

	~MyClass(){
		XDP_DELETE;
	}

	MyClass( MyClass & d ){
		XDP_COPY(d);
	}

	void operator = ( const MyClass & d ){
		XDP_COPY(d);
	}

	void set(int a){
		XDP_DEFER;
		XDP->a;
	}

	int get(){
		return XDP->a;
	}

private:

	XDP_BEGIN_DATA
		int a;
	XDP_END_DATA

};

*/