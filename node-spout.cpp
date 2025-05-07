#define NAPI_DISABLE_CPP_EXCEPTIONS 1

#include <node_api.h> 
#include <napi.h>

#include <assert.h>
#include <stdio.h> 
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <vector>

#include "SpoutGL\SpoutSender.h"
#include "SpoutGL\SpoutReceiver.h"

struct Sender : public Napi::ObjectWrap<Sender> {

    SpoutSender sender;    // Spout sender object

    Sender(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Sender>(info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        if (info.Length() > 0 && info[0].IsString()) {
            sender.SetSenderName(info[0].ToString().Utf8Value().c_str());
        }
	}

    ~Sender() {
		printf("release sender\n");
        sender.ReleaseSender();
    }

    Napi::Value setName(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        if (info.Length() > 0 && info[0].IsString()) {
            sender.SetSenderName(info[0].ToString().Utf8Value().c_str());
        }
        return This;
    }

    Napi::Value sendFbo(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        if (info.Length() > 2 
            && info[0].IsNumber()
            && info[1].IsNumber()
            && info[2].IsNumber()) {
            //   The fbo must be bound for read.
            //   The invert option is false because the fbo is already flipped in y.
            //GLuint fboID, unsigned int width, height, bool invert;
            sender.SendFbo(info[0].ToNumber().Uint32Value(), info[1].ToNumber().Uint32Value(), info[2].ToNumber().Uint32Value(), info[3].ToBoolean().Value());
        }
        return This;
    }

    Napi::Value sendTexture(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        if (info.Length() > 3 
            && info[0].IsNumber()
            && info[1].IsNumber()
            && info[2].IsNumber()
            && info[3].IsNumber()) {
            
            //GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO
            sender.SendTexture(
                info[0].ToNumber().Uint32Value(),   // texid
                info[1].ToNumber().Uint32Value(),   // texture target
                info[2].ToNumber().Uint32Value(),   // width
                info[3].ToNumber().Uint32Value(),   // height
                info[4].ToBoolean().Value(),        // invert
                info[5].ToNumber().Uint32Value());  // host FBO
        }
        return This;
    }

    Napi::Value setMetadata(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();

        // assume that 1st arg is a string or uint8array that we will send

        if (info.Length() > 0) {
            const char * name = sender.GetName();
            int current_size = sender.GetMemoryBufferSize(name);
            size_t size = 0;
            char * data = nullptr;
            if (info[0].IsTypedArray()) {
                size = info[0].As<Napi::TypedArray>().ByteLength();
                data = (char *)info[0].As<Napi::TypedArray>().ArrayBuffer().Data();
            } else if (info[0].IsString()) {
                // convert info[0] to string and send that
                std::string str = info[0].As<Napi::String>().Utf8Value();
                size = str.length()+1; // plus one for null terminator
                data = (char *)str.c_str();
            }

            if (size && data) {
                // reallocate if needed
                if (size > current_size) {
                    sender.CreateMemoryBuffer(name, size);
                }

                // send
                // bool =
                sender.WriteMemoryBuffer(name, data, size);
            }
        }

        return This;
    }
};


struct Receiver : public Napi::ObjectWrap<Receiver> {

    SpoutReceiver receiver;    // Spout object
    Napi::TypedArrayOf<uint8_t> metadata;

    Receiver(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Receiver>(info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        metadata = Napi::TypedArrayOf<uint8_t>::New(env, 128, napi_uint8_array);
        This.Set("metadata", metadata);

        if (info.Length() > 0 && info[0].IsString()) {
            receiver.SetActiveSender(info[0].ToString().Utf8Value().c_str());
        }
	}

    ~Receiver() {
		printf("release receiver\n");
        receiver.ReleaseReceiver();
    }

    Napi::Value setName(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        if (info.Length() > 0 && info[0].IsString()) {
            receiver.SetReceiverName(info[0].ToString().Utf8Value().c_str());
        }
        return This;
    }

    Napi::Value getSenders(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        char SenderName[256];
        int count = receiver.GetSenderCount();
        Napi::Array ret = Napi::Array::New(env, count);
        for (int i=0; i<count; i++) {
            receiver.GetSender(i, SenderName);
            ret.Set(i, Napi::String::New(env, SenderName));
        }
        return ret;
    }

    Napi::Value setActiveSender(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        if (info.Length() > 0 && info[0].IsString()) {
            //printf("seetting active sender %s\n", info[0].ToString().Utf8Value().c_str());
            receiver.SetActiveSender(info[0].ToString().Utf8Value().c_str());
        }
        return This;
    }


    Napi::Value isUpdated(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        return Napi::Boolean::New(env, receiver.IsUpdated());
    }

    Napi::Value isFrameNew(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        return Napi::Boolean::New(env, receiver.IsFrameNew());
    }

    Napi::Value isConnected(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        return Napi::Boolean::New(env, receiver.IsConnected());
    }

    Napi::Value getSenderWidth(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        return Napi::Number::New(env, (double)receiver.GetSenderWidth());
    }

    Napi::Value getSenderHeight(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        return Napi::Number::New(env, (double)receiver.GetSenderHeight());
    }

    Napi::Value getSenderFormat(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        return Napi::Number::New(env, receiver.GetSenderFormat());
    }

    Napi::Value getSenderFps(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        return Napi::Number::New(env, receiver.GetSenderFps());
    }

    Napi::Value getSenderFrame(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        return Napi::Number::New(env, receiver.GetSenderFrame());
    }

    Napi::Value getSenderName(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        return Napi::String::New(env, receiver.GetSenderName());
    }

    Napi::Value receiveTexture(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();

        // also get metadata:
        //getMetadata(info);

        if (info.Length() >= 2 
            && info[0].IsNumber()
            && info[1].IsNumber()) {
            //SpoutReceiver::ReceiveTexture(GLuint TextureID, GLuint TextureTarget, bool bInvert = false, GLuint HostFbo = 0)
            return Napi::Boolean::New(env, receiver.ReceiveTexture(
                info[0].ToNumber().Uint32Value(), // texid
                info[1].ToNumber().Uint32Value(),  // texture target
                info[2].ToBoolean().Value(),        // invert
                info[3].ToNumber().Uint32Value()));  // host FBO, optional
        }
        return Napi::Boolean::New(env, false);
    }

    Napi::Value getMetadata(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();

        int size = receiver.GetMemoryBufferSize(receiver.GetSenderName());
        if (size) {

            // let's assume the user passed in a uint8array:
            if (info[0].IsTypedArray()) {
                auto arr = info[0].As<Napi::TypedArray>();
                size_t len = arr.ByteLength();
                void * dst = arr.ArrayBuffer().Data();

                receiver.ReadMemoryBuffer(receiver.GetSenderName(), (char *)dst, len);
            }

            // if (size != metadata.ElementLength()) {
            //     // reallocate
            //     printf("reallocating for %d bytes of metadata\n", size);
            //     metadata = Napi::TypedArrayOf<uint8_t>::New(env, size, napi_uint8_array);
            //     This.Set("metadata", metadata);
            // }
            // receiver.ReadMemoryBuffer(receiver.GetSenderName(), (char *)metadata.Data(), metadata.ElementLength());
            // 
            //printf("received %d %s\n", strlen((char *)metadata.Data()), (char *)metadata.Data());
        }
        return This;
    }
};

class Module : public Napi::Addon<Module> {
public:

	// /*
	// 	Returns array
	// */
	// Napi::Value devices(const Napi::CallbackInfo& info) {
	// 	Napi::Env env = info.Env();
	// 	Napi::Object devices = Napi::Array::New(env);

	// 	std::vector<sl::DeviceProperties> devList = sl::Camera::getDeviceList();
	// 	for (int i = 0; i < devList.size(); i++) {
	// 		Napi::Object device = Napi::Object::New(env);
	// 		device.Set("id", devList[i].id);
	// 		device.Set("serial", devList[i].serial_number);
	// 		device.Set("model", sl::toString(devList[i].camera_model).get());
	// 		device.Set("state", sl::toString(devList[i].camera_state).get());
	// 		devices[i] = device;
	// 	}
	// 	return devices;
	// }

	// Napi::Value open(const Napi::CallbackInfo& info) {
	// 	Napi::Env env = info.Env();
	// }

   // EnableSpoutLogFile("node_spout.log", true);
	
	Module(Napi::Env env, Napi::Object exports) {

        //EnableSpoutLog();

		// See https://github.com/nodejs/node-addon-api/blob/main/doc/class_property_descriptor.md
		DefineAddon(exports, {
		// 	// InstanceMethod("start", &Module::start),
		// 	// InstanceMethod("end", &Module::end),
		// 	// //InstanceMethod("test", &Module::test),
		// 	// // InstanceValue
		// 	// // InstanceAccessor
		// 	InstanceAccessor<&Module::devices>("devices"),
		// 	// InstanceAccessor<&Module::Gett>("t"),
		// 	// InstanceAccessor<&Module::GetSamplerate>("samplerate"),
		});
		
		{
            // This method is used to hook the accessor and method callbacks
            Napi::Function sender_ctor = Sender::DefineClass(env, "Sender", {
                Sender::InstanceMethod<&Sender::setName>("setName"),
                Sender::InstanceMethod<&Sender::sendFbo>("sendFbo"),
                Sender::InstanceMethod<&Sender::sendTexture>("sendTexture"),
                Sender::InstanceMethod<&Sender::setMetadata>("setMetadata"),
            });

            // Create a persistent reference to the class constructor.
            Napi::FunctionReference* constructor = new Napi::FunctionReference();
            *constructor = Napi::Persistent(sender_ctor);
            exports.Set("Sender", sender_ctor);
            env.SetInstanceData<Napi::FunctionReference>(constructor);
        }

        {// This method is used to hook the accessor and method callbacks
            Napi::Function receiver_ctor = Receiver::DefineClass(env, "Receiver", {
                Receiver::InstanceMethod<&Receiver::setName>("setName"),
                Receiver::InstanceMethod<&Receiver::setActiveSender>("setActiveSender"),
                Receiver::InstanceMethod<&Receiver::getSenders>("getSenders"),
                Receiver::InstanceMethod<&Receiver::getSenderFormat>("getSenderFormat"),
                Receiver::InstanceMethod<&Receiver::getSenderWidth>("getSenderWidth"),
                Receiver::InstanceMethod<&Receiver::getSenderHeight>("getSenderHeight"),
                Receiver::InstanceMethod<&Receiver::getSenderFps>("getSenderFps"),
                Receiver::InstanceMethod<&Receiver::getSenderFrame>("getSenderFrame"),
                Receiver::InstanceMethod<&Receiver::getSenderName>("getSenderName"),
                Receiver::InstanceMethod<&Receiver::isConnected>("isConnected"),
                Receiver::InstanceMethod<&Receiver::isUpdated>("isUpdated"),
                Receiver::InstanceMethod<&Receiver::isFrameNew>("isFrameNew"),
                Receiver::InstanceMethod<&Receiver::receiveTexture>("receiveTexture"),
                Receiver::InstanceMethod<&Receiver::getMetadata>("getMetadata"),
            });

            // Create a persistent reference to the class constructor.
            Napi::FunctionReference* constructor = new Napi::FunctionReference();
            *constructor = Napi::Persistent(receiver_ctor);
            exports.Set("Receiver", receiver_ctor);
            env.SetInstanceData<Napi::FunctionReference>(constructor);
        }
	}
};

NODE_API_ADDON(Module)