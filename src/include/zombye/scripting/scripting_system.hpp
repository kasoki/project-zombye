#ifndef __ZOMBYE_SCRIPTING_SYSTEM_HPP__
#define __ZOMBYE_SCRIPTING_SYSTEM_HPP__

#include <memory>

#include <angelscript.h>
#include <scriptbuilder/scriptbuilder.h>

namespace zombye {
	class game;
}

namespace zombye {
	class scripting_system {
	private:
		game& game_;
		std::unique_ptr<asIScriptEngine, void(*)(asIScriptEngine*)> script_engine_;
		std::unique_ptr<asIScriptContext, void(*)(asIScriptContext*)> script_context_;
		std::unique_ptr<CScriptBuilder> script_builder_;

	public:
		scripting_system(game& game);
		~scripting_system() = default;

		scripting_system(const scripting_system& rhs) = delete;
		scripting_system& operator=(const scripting_system& rhs) = delete;

		scripting_system(scripting_system&& rhs) = delete;
		scripting_system& operator=(scripting_system&& rhs) = delete;

		void begin_module(const std::string& module_name);
		void load_script(const std::string& file_name);
		void end_module();
		void exec(const std::string& function_decl, const std::string& module_name);

		template<typename t>
		void register_function(const std::string function_decl, const t& function) {
			auto result = script_engine_->RegisterGlobalFunction(function_decl.c_str(), asFUNCTION(function), asCALL_CDECL);
			if (result < 0) {
				throw std::runtime_error("Could not register function " + function_decl);
			}
		}
	};
}

#endif
