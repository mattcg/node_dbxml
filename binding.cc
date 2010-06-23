#include <node.h>
#include <node_events.h>
#include <stdlib.h>

#include <dbxml/DbXml.hpp>

using namespace v8;
using namespace node;
using namespace DbXml;

class Env : public EventEmitter {

	public:

		static void Initialize (v8::Handle<v8::Object> target) {
			HandleScope scope;
			
			Local<FunctionTemplate> t = FunctionTemplate::New(New);
			
			t->Inherit(EventEmitter::constructor_template);
			t->InstanceTemplate()->SetInternalFieldCount(1);
			
			NODE_SET_PROTOTYPE_METHOD(t, "createEnv", CreateEnv);
			//NODE_SET_PROTOTYPE_METHOD(t, "createXmlManager", CreateXmlManager);
						
			target->Set(String::NewSymbol("Env"), t->GetFunction());
		}

		bool CreateEnv (const char* env_home) {
			if (environment_) return false;
			
			int dberr;
			
			u_int32_t env_flags =	DB_CREATE		|
									DB_INIT_LOCK	|
									DB_INIT_LOG		|
									DB_INIT_MPOOL	|
									DB_INIT_TXN;
			
			dberr = db_env_create(&environment_, 0);
			
			if (dberr) {
				ThrowException(Exception::Error(
					String::New(strcat((char *)"Unable to create environment: ", db_strerror(dberr)) )));
				if (environment_) environment_->close(environment_, 0);
				environment_ = NULL;
				return false; //return (EXIT_FAILURE);
			}
			
			environment_->open(environment_, env_home, env_flags, 0);
			
			// BEGIN TESTING
			
			XmlManager *xmlmanager = NULL;
			
			xmlmanager = new XmlManager(environment_);

			try {
				if (xmlmanager != NULL) {
					xmlmanager->setDefaultContainerType(XmlContainer::WholedocContainer);
					
					if (xmlmanager->existsContainer("/tmp/dbxml/test.bdbxml"))
						xmlmanager->removeContainer("/tmp/dbxml/test.bdbxml");
					
					XmlContainer xmlcontainer = xmlmanager->createContainer("/tmp/dbxml/test.bdbxml");
					
					const char *xmldocstr = "<a_node><b_node>Some text</b_node></a_node>";
					const char *xmldocname = "testdoc";
					
					XmlUpdateContext updatecontext = xmlmanager->createUpdateContext();
					
					try {
						xmlcontainer.putDocument(xmldocname, xmldocstr, updatecontext, 0);
					} catch (XmlException &e) { 
						// Error handling goes here. You may want to check 
						// for XmlException::UNIQUE_ERROR, which is raised 
						// if a document with that name already exists in 
						// the container. If this exception is thrown, 
						// try the put again with a different name.
						ThrowException(Exception::Error(
							String::New("Unable to put document.")));
					}
					
					xmlmanager->removeContainer("/tmp/dbxml/test.bdbxml");
					
					delete xmlmanager;
				}
			} catch(XmlException &xe) {
				ThrowException(Exception::Error(
					String::New(strcat((char *)"Error closing XmlManager: ", xe.what()) )));
			}
			
			// END TESTING
			
			creatingenv_ = true;
	
			return true;
		}
		
	protected:
		
		static Handle<v8::Value> New (const Arguments& args) {
			HandleScope scope;
			
			Env *environment = new Env();
			environment->Wrap(args.This());
			
			return args.This();
		}
		
		static Handle<v8::Value> CreateEnv (const Arguments& args) {
			Env *environment = ObjectWrap::Unwrap<Env>(args.This());
			
			HandleScope scope;

			if (args.Length() == 0 || !args[0]->IsString()) {
				return ThrowException(Exception::Error(
					String::New("Must give env_home string as argument.")));
			}
			
			String::Utf8Value env_home(args[0]->ToString());
			
			bool r = environment->CreateEnv(*env_home);
			
			if (!r) {
				return ThrowException(Exception::Error(
					String::New("Unable to create environment.")));
			}
			
			return Undefined();
		}
		
		Env () : EventEmitter () {
			environment_ = NULL;
			
			creatingenv_ = false;
			
		}
		
		~Env () {
			environment_->close(environment_, 0);
		}
		
		DB_ENV *environment_;
		bool creatingenv_;		
};

extern "C" void init (Handle<Object> target) {
	HandleScope scope;
	
	Env::Initialize(target);
}