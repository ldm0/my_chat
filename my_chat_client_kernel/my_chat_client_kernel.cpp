#include<node.h>
#include"services.h"
#include<string>

namespace client_kernel {
/*
	v8::Persistent<v8::Function> recv_msg_callback;

	void on_recv_msg(const char *_msg_recv)
	{
		if (recv_msg_callback.IsEmpty())
			return;
		v8::Isolate* isolate = v8::Isolate::GetCurrent();
		v8::Local<v8::Context> context = isolate->GetCurrentContext(); // maybe a bug here
		v8::Local<v8::Function> callback = v8::Local<v8::Function>::New(isolate, recv_msg_callback);

		if (!callback.IsEmpty())
			return;
		const unsigned argc = 1;
		v8::Local<v8::Value> argv[argc] = {
			v8::String::NewFromUtf8(isolate, _msg_recv, v8::NewStringType::kNormal).ToLocalChecked()};
		callback->Call(context, Null(isolate), argc, argv);
	}

	void set_recv_msg_callback(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::Isolate *isolate = args.GetIsolate();
		if (!args[0]->IsFunction()) {
			isolate->ThrowException(v8::Exception::TypeError(
				v8::String::NewFromUtf8(isolate, "argument is not a function!", v8::NewStringType::kNormal).ToLocalChecked()));
			return;
		}
		v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(args[0]);
		v8::Function *func_ptr = *func;
		recv_msg_callback.Reset(isolate, func);
		for (int i = 0; i < 1000; ++i)
			on_recv_msg("hhhhhh_test");
	}
*/
	void init(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		services_init();
	}

	void close(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		services_close();
	}

	void send_msg(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::Isolate *isolate = args.GetIsolate();
		int args_length = args.Length();
		if (args_length <= 0)
			return;
		for (int i = 0; i < args_length; ++i) {
			if (!args[i]->IsString())
				isolate->ThrowException(v8::Exception::TypeError(
					v8::String::NewFromUtf8(isolate, "one of the args is not string", v8::NewStringType::kNormal).ToLocalChecked()
				));
			v8::String::Utf8Value tmp_msg_send(isolate, args[i]);
			services_send_msg(*(tmp_msg_send));
		}
	}

	void get_msg_recv(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::Isolate *isolate = args.GetIsolate();
		std::string msg_recv;
		services_get_msg_recv(msg_recv);
		args.GetReturnValue().Set(
			v8::String::NewFromUtf8(isolate, msg_recv.c_str(), v8::NewStringType::kNormal).ToLocalChecked()
		);
	}

	void Init(v8::Local<v8::Object> exports)
	{
		NODE_SET_METHOD(exports, "init", init);
		NODE_SET_METHOD(exports, "close", close);
		NODE_SET_METHOD(exports, "send_msg", send_msg);
		NODE_SET_METHOD(exports, "get_msg_recv", get_msg_recv);
	}
}

NODE_MODULE(NODE_GYP_MODULE_NAME, client_kernel::Init)
