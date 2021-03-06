#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <scriptstdstring/scriptstdstring.h>
#include <scriptarray/scriptarray.h>

#include <zombye/assets/asset.hpp>
#include <zombye/assets/asset_manager.hpp>
#include <zombye/core/game.hpp>
#include <zombye/scripting/scripting_system.hpp>
#include <zombye/utils/logger.hpp>

namespace zombye {
	scripting_system::scripting_system(game& game)
	: game_{game}, script_engine_{nullptr, +[](asIScriptEngine*){}} {
		script_engine_ = std::unique_ptr<asIScriptEngine, void(*)(asIScriptEngine*)>(
			asCreateScriptEngine(ANGELSCRIPT_VERSION),
			+[](asIScriptEngine* se) { se->ShutDownAndRelease(); }
		);

		auto result = script_engine_->SetMessageCallback(
			asFUNCTION(+[](const asSMessageInfo* msg, void* param) {
				auto message = std::string{msg->message} + " in section " + msg->section + ":"
					+ std::to_string(msg->row) + ":" + std::to_string(msg->col);
				switch(msg->type) {
					case asEMsgType::asMSGTYPE_INFORMATION:
						log(LOG_INFO, message);
						break;
					case asEMsgType::asMSGTYPE_WARNING:
						log(LOG_WARNING, message);
						break;
					case asEMsgType::asMSGTYPE_ERROR:
						log(LOG_ERROR, message);
						break;
				}
			}),
			nullptr,
			asCALL_CDECL
		);
		if (result < 0) {
			log(LOG_ERROR, "Could not register message callback for script system");
		}

		context_pool_.reserve(10);
		for (auto i = 0; i < 10; ++i) {
			context_pool_.emplace_back(
				script_engine_->RequestContext(),
				+[](asIScriptContext* context) {context->Release();}
			);
		}

		used_context_.reserve(15);

		RegisterStdString(script_engine_.get());
		RegisterScriptArray(script_engine_.get(), true);

		script_builder_ = std::make_unique<CScriptBuilder>();

		register_function("void print(const string& in)", +[](const std::string& in) {log(in);});

		static std::function<float()> width_function_ptr = [this]() { return game_.width(); };
		register_function("float width()", width_function_ptr);
		static std::function<float()> height_function_ptr = [this]() { return game_.height(); };
		register_function("float height()", height_function_ptr);

		register_glm();
	}

	void scripting_system::begin_module(const std::string& module_name) {
		auto result = script_builder_->StartNewModule(script_engine_.get(), module_name.c_str());
		if (result < 0) {
			throw std::runtime_error("Could not create new module " + module_name);
		}
	}

	void scripting_system::load_script(const std::string& file_name) {
		auto asset = game_.asset_manager().load(file_name);
		if (!asset) {
			throw std::runtime_error("Could not load script " + file_name);
		}
		auto content = asset->content();
		auto result = script_builder_->AddSectionFromMemory(file_name.c_str(), content.data(), content.size(), 0);
		if (result < 0) {
			throw std::runtime_error("Could not add section from memory " + file_name);
		}
	}

	void scripting_system::end_module() {
		auto result = script_builder_->BuildModule();
		if (result < 0) {
			throw std::runtime_error("Could not build module");
		}
	}

	void scripting_system::exec(const std::string& function_decl, const std::string& module_name) {
		auto mod = script_engine_->GetModule(module_name.c_str());
		if (!mod) {
			throw std::runtime_error("No module named " + module_name);
		}
		auto func = mod->GetFunctionByDecl(function_decl.c_str());
		if (!func) {
			throw std::runtime_error("No function with signature " + function_decl + " in module " + module_name);
		}

		allocate_context();

		prepare(*func);
		exec();
	}

	void scripting_system::exec() {
		auto& script_context = used_context_.back();

		auto result = script_context->Execute();
		if (result != asEXECUTION_FINISHED) {
			if (result == asEXECUTION_EXCEPTION) {
				throw std::runtime_error(script_context->GetExceptionString());
			}
		}

		context_pool_.emplace_back(std::move(used_context_.back()));
		used_context_.pop_back();
	}

	void scripting_system::prepare(asIScriptFunction& function) {
		used_context_.emplace_back(std::move(context_pool_.back()));
		context_pool_.pop_back();
		auto& script_context = used_context_.back();

		script_context->Prepare(&function);
	}

	template <>
	void scripting_system::argument<float>(int position, float& arg) {
		allocate_context();
		used_context_.back()->SetArgFloat(position, arg);
	}

	void scripting_system::register_destructor(const std::string& type_name, void(*function)(void*)) {
		auto result = script_engine_->RegisterObjectBehaviour(type_name.c_str(), asBEHAVE_DESTRUCT,
			"void f()", asFUNCTION(function), asCALL_CDECL_OBJFIRST);
		if (result < 0) {
			throw std::runtime_error("Could not register destructor at type " + type_name);
		}
	}

	void scripting_system::register_member(const std::string& type_name, const std::string& member_decl, size_t offset) {
		auto result = script_engine_->RegisterObjectProperty(type_name.c_str(), member_decl.c_str(), offset);
		if (result < 0 ) {
			throw std::runtime_error("Could not register member " + member_decl + " at type " + type_name);
		}
	}

	void scripting_system::allocate_context() {
		if (context_pool_.size() == 0) {
			for (auto i = 0; i < 5; ++i) {
				context_pool_.emplace_back(std::move(
					std::unique_ptr<asIScriptContext, void(*)(asIScriptContext*)>{
						script_engine_->RequestContext(),
						+[](asIScriptContext* context) {context->Release();}
					}
				));
			}
		}
	}

	void scripting_system::register_glm() {
		script_engine_->SetDefaultNamespace("glm");

		register_pod_type<glm::vec3>("vec3");

		register_constructor("vec3", "void f()",
			+[](void* memory) { *reinterpret_cast<glm::vec3*>(memory) = glm::vec3{}; });
		register_constructor("vec3", "void f(float a)",
			+[](void* memory, float a) { *reinterpret_cast<glm::vec3*>(memory) = glm::vec3(a); });
		register_constructor("vec3", "void f(float a, float b, float c)",
			+[](void* memory, float a, float b, float c) { *reinterpret_cast<glm::vec3*>(memory) = glm::vec3(a, b, c); });

		register_destructor("vec3", +[](void* memory) {});

		register_member("vec3", "float x", asOFFSET(glm::vec3, x));
		register_member("vec3", "float y", asOFFSET(glm::vec3, y));
		register_member("vec3", "float z", asOFFSET(glm::vec3, z));

		register_member_function("vec3", "vec3& opAssign(const vec3& in)",
			+[](glm::vec3& lhs, const glm::vec3& rhs) -> glm::vec3& { return lhs = rhs; });
		register_member_function("vec3", "vec3& opAddAssign(const vec3& in)",
			+[](glm::vec3& lhs, const glm::vec3& rhs) -> glm::vec3& { return lhs += rhs; });
		register_member_function("vec3", "vec3& opSubAssign(const vec3& in)",
			+[](glm::vec3& lhs, const glm::vec3& rhs) -> glm::vec3& { return lhs -= rhs; });
		register_member_function("vec3", "vec3& opMulAssign(float)",
			+[](glm::vec3& lhs, float rhs) -> glm::vec3& { return lhs *= rhs; });
		register_member_function("vec3", "vec3& opDivAssign(float)",
			+[](glm::vec3& lhs, float rhs) -> glm::vec3& { return lhs /= rhs; });
		register_member_function("vec3", "vec3 opAdd(const vec3& in)",
			+[](const glm::vec3& lhs, const glm::vec3& rhs) { return lhs + rhs; });
		register_member_function("vec3", "vec3 opSub(const vec3& in)",
			+[](const glm::vec3& lhs, const glm::vec3& rhs) { return lhs - rhs; });
		register_member_function("vec3", "vec3 opMul(float)",
			+[](float lhs, const glm::vec3& rhs) { return lhs * rhs; },
			asCALL_CDECL_OBJLAST);
		register_member_function("vec3", "vec3 opMul_r(float)",
			+[](const glm::vec3& lhs, float rhs) { return lhs * rhs; });
		register_member_function("vec3", "vec3 opDiv(float)",
			+[](const glm::vec3& lhs, float rhs) { return lhs / rhs; });

		register_function("float length(const vec3& in)",
			+[](const glm::vec3& v) {return glm::length(v);});
		register_function("float dot(const vec3& in, const vec3& in)",
			+[](const glm::vec3& v1, const glm::vec3& v2) {return glm::dot(v1, v2);});
		register_function("vec3 cross(const vec3& in, const vec3& in)",
			+[](const glm::vec3& v1, const glm::vec3& v2) {return glm::cross(v1, v2);});
		register_function("vec3 normalize(const vec3& in)",
			+[](const glm::vec3& v) {return glm::normalize(v);});


		register_pod_type<glm::quat>("quat");

		register_constructor("quat", "void f()",
			+[](void* memory) { *reinterpret_cast<glm::quat*>(memory) = glm::quat{}; });
		register_constructor("quat", "void f(float w, float x, float y, float z)",
			+[](void* memory, float w, float x, float y, float z) { *reinterpret_cast<glm::quat*>(memory) = glm::quat(w, x, y, z); });
		register_constructor("quat", "void f(float angle, const vec3& in axis)",
			+[](void* memory, float angle, const glm::vec3& axis) { *reinterpret_cast<glm::quat*>(memory) = glm::angleAxis(angle, axis); } );

		register_destructor("quat", +[](void* memory) {});

		register_member("quat", "float x", asOFFSET(glm::quat, x));
		register_member("quat", "float y", asOFFSET(glm::quat, y));
		register_member("quat", "float z", asOFFSET(glm::quat, z));
		register_member("quat", "float w", asOFFSET(glm::quat, w));

		register_member_function("quat", "quat& opAssign(const quat& in)",
			+[](glm::quat& lhs, const glm::quat& rhs) -> glm::quat& { return lhs = rhs; });


		register_function("float glm::radians(float)",
			+[](float degree) { return glm::radians(degree); });

		register_pod_type<glm::mat4>("mat4");

		register_function("mat4 glm::ortho(float, float, float, float, float, float)",
			+[](float left, float right, float bottom, float top, float z_near, float z_far) {
				return glm::ortho(left, right, bottom, top, z_near, z_far);
			});
		register_function("mat4 glm::perspectiveFov(float, float, float, float, float)",
			+[](float fov, float width, float height, float z_near, float z_far) {
				return glm::perspectiveFov(fov, width, height, z_near, z_far);
			}
		);

		script_engine_->SetDefaultNamespace("");
		register_function("void print(const glm::vec3& in)", +[](const glm::vec3& in) {log(glm::to_string(in));});
	}
}
