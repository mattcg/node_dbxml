#include <node.h>
#include <node_events.h>
#include <stdlib.h>

#include <dbxml/DbXml.hpp>

using namespace v8;
using namespace node;
using namespace DbXml;
using namespace std;

static Persistent<String> error_symbol;

class Env : public EventEmitter {

	public:
		
		static void Initialize (v8::Handle<v8::Object> target) {
			HandleScope scope;
			
			Local<FunctionTemplate> t = FunctionTemplate::New(New);
			
			t->Inherit(EventEmitter::constructor_template);
			t->InstanceTemplate()->SetInternalFieldCount(1);
			
			error_symbol = NODE_PSYMBOL("error");
			
			NODE_SET_PROTOTYPE_METHOD(t, "createEnv", CreateEnv);
						
			target->Set(String::NewSymbol("Env"), t->GetFunction());
		}

		bool CreateEnv (const char* env_home) {
						
			if (environment_) return false;
			
			int dberr;
			
			u_int32_t env_flags =	DB_CREATE		|
									//DB_INIT_LOCK	|
									DB_INIT_LOG		|
									DB_INIT_MPOOL;//	|
									//DB_INIT_TXN;
									
			DbXml::setLogLevel(DbXml::LEVEL_ALL, true);
			DbXml::setLogCategory(DbXml::CATEGORY_ALL, true);
			
			dberr = db_env_create(&environment_, 0);
			
			//environment_->set_errpfx(environment_, "BDB error: ");
			// AARGH could never get this to work!
			//environment_->set_errcall(environment_, BdbErrorCallback);
			//environment_->set_msgcall(environment_, xyz);
			
			if (dberr) {
				ThrowException(Exception::Error(
					String::New(strcat((char *)"Unable to create environment: ", db_strerror(dberr)) )));
				if (environment_) environment_->close(environment_, 0);
				environment_ = NULL;
				return false;
			}
			
			try {
				environment_->open(environment_, env_home, env_flags, 0);
			} catch (XmlException &e) { 
				ThrowException(Exception::Error(
					String::New( strcat((char *)"Unable to open environment: ", e.what()) )));
				return false;
			}
			
			// BEGIN TESTING
			
			XmlManager *xmlmanager = NULL;
			
			XmlContainer xmlcontainer;
			
			xmlmanager = new XmlManager(environment_);
			
			// XmlManager *xmlmanager = new XmlManager(environment_);
			
			try {
				xmlmanager->setDefaultContainerType(XmlContainer::WholedocContainer);
			} catch (XmlException &e) { 
				ThrowException(Exception::Error(
					String::New( strcat((char *)"Unable to set default container type: ", e.what()) )));
			}
			
			if (xmlmanager->existsContainer("/tmp/dbxml/test.bdbxml")) {
				try {
					xmlmanager->removeContainer("/tmp/dbxml/test.bdbxml");
				} catch (XmlException &e) { 
					ThrowException(Exception::Error(
						String::New( strcat((char *)"Unable to remove existing container: ", e.what()) )));
				}	
			}
			
			try {
				xmlcontainer = xmlmanager->createContainer("/tmp/dbxml/test.bdbxml");
			} catch (XmlException &e) { 
				ThrowException(Exception::Error(
					String::New( strcat((char *)"Unable to create container: ", e.what()) )));
			}
			
			const char *xmldocstr = "<a_node><b_node>Some text</b_node></a_node>";
			const char *xmldocname = "testdoc";
			
			XmlUpdateContext updatecontext = xmlmanager->createUpdateContext();
			
			try {
				xmlcontainer.putDocument(xmldocname, xmldocstr, updatecontext, 0);
			//} catch (std::exception &e) {
			} catch (XmlException &e) { 
				// Error handling goes here. You may want to check 
				// for XmlException::UNIQUE_ERROR, which is raised 
				// if a document with that name already exists in 
				// the container. If this exception is thrown, 
				// try the put again with a different name.
				ThrowException(Exception::Error(
					String::New( strcat((char *)"Unable to put document: ", e.what()) )));
			} catch (...) {
				ThrowException(Exception::Error(
					String::New("UNABLE TO PUT DOC!")));
			}
			
			/*
			xmlcontainer.close();
			
			try {
				xmlmanager->removeContainer("/tmp/dbxml/test.bdbxml");
			} catch (XmlException &e) {
				ThrowException(Exception::Error(
					//String::New( strcat((char *)"Unable to close container: ", e.what()) )));
					String::New(e.what())));
					//String::New("UNABLE TO CLOSE CONTAINER!")));
			}
			*/
			
			try {
				if (xmlmanager != NULL) {		
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
		
		/*static void BdbErrorCallback (const DB_ENV *dbenv, const char *errpfx, const char *msg) {						
			HandleScope scope;
			
			Local<v8::Value> exception;
			exception = Exception::Error(String::New(msg));
			environment->Emit(error_symbol, 1, &exception);
		}*/
		
		Env () : EventEmitter () {
			environment_ = NULL;
			creatingenv_ = false;			
		}
		
		~Env () {
			environment_->close(environment_, 0);
		}
		
	private:
		
		DB_ENV *environment_;
		bool creatingenv_;	
};

extern "C" void init (Handle<Object> target) {
	HandleScope scope;
	
	Env::Initialize(target);
}