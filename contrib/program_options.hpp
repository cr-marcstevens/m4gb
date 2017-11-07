/*********************************************************************************\
*                                                                                 *
* https://github.com/cr-marcstevens/snippets/tree/master/cxxheaderonly            *
*                                                                                 *
* program_options.hpp - A header only C++ boost-like program options class        *
* Copyright (c) 2017 Marc Stevens                                                 *
*                                                                                 *
* MIT License                                                                     *
*                                                                                 *
* Permission is hereby granted, free of charge, to any person obtaining a copy    *
* of this software and associated documentation files (the "Software"), to deal   *
* in the Software without restriction, including without limitation the rights    *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell       *
* copies of the Software, and to permit persons to whom the Software is           *
* furnished to do so, subject to the following conditions:                        *
*                                                                                 *
* The above copyright notice and this permission notice shall be included in all  *
* copies or substantial portions of the Software.                                 *
*                                                                                 *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
* SOFTWARE.                                                                       *
*                                                                                 *
\*********************************************************************************/

#ifndef PROGRAM_OPTIONS_HPP
#define PROGRAM_OPTIONS_HPP

#include <stdexcept>
#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <map>

/****************************** example usage ************************************\
grep "^//test.cpp" program_options.hpp -A72 > test.cpp
grep "^#file.cfg" program_options.hpp -A11 > file.cfg
g++ -std=c++11 -o test test.cpp

//test.cpp:
#include "program_options.hpp"
#include <fstream>
namespace po = program_options;

int main(int argc, char** argv)
{
	std::vector<std::string> inputfiles;
	std::string outputfile, configfile = "file.cfg";
	unsigned param1 = 0;
	int param2 = 0;
	std::size_t param3 = 0;

	po::options_description cmdopts("Command line options"), opts("Common options"), cfgopts("Config file options");
	cmdopts.add_options()
		("help,h", "Show options") // short option & long option
		("configfile,f", po::value<std::string>(&configfile), "Set configfile (default: file.cfg)")
		;
	opts.add_options()
		("dowork", "Do work")      // only long option
		("v", "Verbose")           // only short option

		// option that takes a string and can be given multiple times, values are automatically stored in inputfiles
		("inputfile,i", po::value<std::vector<std::string>>(&inputfiles), "Add input file")

		// option that takes a string with a default value of "file.tmp" that is automatically stored in outputfile
		("outputfile,o", po::value<std::string>(&outputfile)->default_value("file.tmp"), "Set outputfile")

		// options with different integer arguments (with default value and/or store variable)
		("param1", po::value<unsigned>(), "Param 1")
		("param2", po::value<int>(&param2)->default_value(-1), "Param 2")
		("param3", po::value<std::size_t>()->default_value(5), "Param 3")
		;
	cmdopts.add(opts);
	cfgopts.add(opts);
	po::variables_map vm;

	bool allow_unregistered = true, allow_positional = true;
	// parse command line
	po::store(po::parse_command_line(argc, argv, cmdopts, allow_unregistered, allow_positional), vm);
	// optional: parse configuration file
	if (!configfile.empty())
	{
		std::ifstream is(configfile.c_str());
		po::store(po::parse_config_file(is, cfgopts, allow_unregistered), vm);
	}
	// set default values if option was not given, and store arguments in variables
	po::notify(vm);

	if (vm.count("help") || (inputfiles.size() == 0 && vm.count("dowork") == 0))
	{
		std::cout << cmdopts;
		return 0;
	}
	if (vm.count("dowork"))
		std::cout << "Do work!" << std::endl;
	if (vm.count("v"))
		std::cout << "Be verbose!" << std::endl;
	if (vm.count("param1"))
		param1 = vm["param1"].as<unsigned>();
	if (vm.count("param3"))
		param3 = vm["param3"].as<std::size_t>();
	for (auto& inputfile : inputfiles)
		std::cout << "in: " << inputfile << std::endl;
	std::cout << "out: " << outputfile << std::endl;
	std::cout << "params: " << param1 << " " << param2 << " " << param3 << std::endl;
	for (auto& other_option : vm.unrecognized)
		std::cout << "unrecognized option: " << other_option << std::endl;
	for (auto& positional_argument : vm.positional)
		std::cout << "positional argument: " << positional_argument.as<std::string>() << std::endl;
	return 0;
}

#file.cfg:
# comments are ignored, whitespace is trimmed
# format is longopt=argument or just longopt
dowork
inputfile = file.1
inputfile = file.2
param3    = 6
# sections: all following options are prefixed with "sectionname."
[section1]
dowork     # section1.dowork
[section2.subsection3]
param2=5   # section2.subsection3.param2=5

\**************************** end example usage **********************************/

namespace program_options {

	namespace detail {
		// parse: provide string to other types conversion
		void parse(const std::string& str, std::string& ret)
		{
			ret = str;
		}
		template<typename Type>
		void parse(const std::string& str, Type& ret)
		{
			std::stringstream strstr(str);
			strstr >> ret;
			if (!strstr)
				throw std::runtime_error("Could not parse program option argument: " + str);
			// retrieving one more char should lead to EOF and failbit set
			strstr.get();
			if (!!strstr)
				throw std::runtime_error("Could not fully parse program option argument: " + str);
		}
		// parse: vector is treated special: converted value is appended to vector
		template<typename Type, typename A>
		void parse(const std::string& str, std::vector<Type,A>& ret)
		{
			ret.emplace_back();
			parse(str, ret.back());
		}

		// to_string: like std::to_string but outputs a stack of strings
		// extended with std::string (passthrough) and std::vector (make list)
		std::vector<std::string> to_string(const std::string& t)
		{
			return std::vector<std::string>(1,t);
		}
		std::vector<std::string> to_string(const std::vector<std::string>& vs)
		{
			return vs;
		}
		template<typename Type>
		std::vector<std::string> to_string(const Type& t)
		{
			std::stringstream strstr;
			strstr << t;
			return std::vector<std::string>(1,strstr.str());
		}
		template<typename Type, typename A>
		std::vector<std::string> to_string(const std::vector<Type,A>& vs)
		{
			std::vector<std::string> ret;
			for (auto& v : vs)
			{
				std::vector<std::string> tmp = to_string(v);
				for (std::size_t i = 0; i < tmp.size(); ++i)
					ret.emplace_back(std::move(tmp[i]));
			}
			return ret;
		}

		/* base interface to wrapper around variables and default values */
		class parser;
		class value_base {
		public:
			virtual ~value_base() {}
			virtual bool _hasdefaultvalue() = 0;
			virtual std::vector<std::string> _defaultvaluestr() = 0;
			virtual void _parse(const parser& arg) = 0;
		};

		/* parser class contains a stack of strings
		   parses each to a type on demand via 'as<type>()' and 'to(var)'
		   use pop_front() and empty() to safely traverse stack */
		class parser {
		public:
			bool empty() const
			{
				return _values.empty();
			}

			template<typename Type>
			Type as() const
			{
				if (empty())
					throw std::runtime_error("program_options::detail::parser::as(): parsing empty value");
				Type ret;
				parse(_values.front(), ret);
				return ret;
			}

			template<typename Type>
			void to(Type& target) const
			{
				if (empty())
					throw std::runtime_error("program_options::detail::parser::to(): parsing empty value");
				parse(_values.front(), target);
			}

			template<typename Type, typename A>
			void to(std::vector<Type,A>& target) const
			{
				target.resize(_values.size());
				for (std::size_t i = 0; i < _values.size(); ++i)
					parse(_values[i], target[i]);
			}

			void pop_front()
			{
				_values.erase(_values.begin());
			}

			parser& _set(std::shared_ptr<value_base> target)
			{
				_target = target;
				return *this;
			}
			parser& _add(const std::string& val)
			{
				_values.emplace_back(val);
				if (_target.get() != nullptr)
					_target->_parse(*this);
				return *this;
			}

			void _finalize()
			{
				if (_target.get() != nullptr)
				{
					if (_values.empty())
						_values = _target->_defaultvaluestr();
					_target->_parse(*this);
				}
			}

			const std::vector<std::string>& values() const { return _values; }
		private:
			std::vector<std::string> _values;
			std::shared_ptr<value_base> _target;
		};

		void trim(std::string& str)
		{
			// trim left
			std::size_t pos = 0;
			while (pos < str.size() && std::isspace(str[pos]))
				++pos;
			str.erase(0, pos);
			// trim right
			while (!str.empty() && std::isspace(str.back()))
				str.pop_back();
		}

	}

	/* wrapper around variables and default values */
	template<typename Type>
	class value
		: public detail::value_base
	{
	public:
		value(): _target(nullptr) {}
		value(Type* target): _target(target) {}
		virtual ~value() {}

		value* operator->() { return this; }

		value& default_value(const Type& defaultvalue)
		{
			if (_target != nullptr)
				*_target = defaultvalue;
			_defaultvalue.reset(new Type(defaultvalue));
			return *this;
		}

		virtual bool _hasdefaultvalue()
		{
			return _defaultvalue.get() != nullptr;
		}
		virtual std::vector<std::string> _defaultvaluestr()
		{
			if (_defaultvalue.get() != nullptr)
				return detail::to_string(*_defaultvalue);
			return std::vector<std::string>();
		}

		virtual void _parse(const detail::parser& arg)
		{
			if (_target != nullptr)
				arg.to(*_target);
		}

	private:
		Type* _target;
		std::shared_ptr<Type> _defaultvalue;
	};

	/* contains option description and optional value specific behaviour: target variable and/or default value */
	struct option_t {
		std::string shortopt, longopt, name;
		std::string description;
		std::shared_ptr<detail::value_base> value;
	};
	typedef std::shared_ptr<option_t> option;

	/* contains all options descriptions, and contains logic to print help screen */
	class options_description {
	public:
		static const unsigned default_line_length = 78;

		options_description(unsigned line_length = default_line_length, unsigned min_description_length = default_line_length/2)
			: _description(), _linelength(line_length), _mindesclength(min_description_length)
		{
		}

		options_description(const std::string& description, unsigned line_length = default_line_length, unsigned min_description_length = default_line_length/2)
			: _description(description), _linelength(line_length), _mindesclength(min_description_length)
		{
		}

		/* helper class to easily add options */
		class add_options_t {
		public:
			add_options_t(options_description& parent): _parent(parent) {}
			// register option without argument
			// option = "s,long" OR "long,s" OR "s" OR "long" for short option '-s' and/or long option '--long'
			// description = option description (can contain '\n' and '\t' for formatting)
			inline add_options_t operator()(const std::string& option, const std::string& description)
			{
				_parent._add_option(option, description);
				return *this;
			}
			// register option with argument of type Type
			// when Type==std::vector<value_type> then register repeatable option with argument of type Type::value_type
			// use value<Type>->default_value(val) to register argument with a default value
			template<typename Type>
			inline add_options_t operator()(const std::string& option, value<Type> val, const std::string& description)
			{
				_parent._add_option(option, val, description);
				return *this;
			}
		private:
			options_description& _parent;
		};
		add_options_t add_options()
		{
			return add_options_t(*this);
		}

		option _add_option(const std::string& opt, const std::string& description)
		{
			option o(new option_t());
			_options.emplace_back(o);

			o->description = description;
			std::size_t pos = opt.find(',');
			if (pos < opt.size())
			{
				o->longopt = opt.substr(0, pos);
				o->shortopt = opt.substr(pos+1);
				if (o->longopt.size() == 1)
					std::swap(o->longopt,o->shortopt);
				if (o->longopt.size() == 1)
					throw std::runtime_error("program_options::_add_option: long option has length 1");
				if (o->shortopt.size() > 1)
					throw std::runtime_error("program_options::_add_option: short option has length > 1");
				o->name = o->longopt;
			}
			else
			{
				if (opt.size() == 1)
					o->name = o->shortopt = opt;
				else
					o->name = o->longopt = opt;
			}
			return o;
		}
		template<typename Type>
		void _add_option(const std::string& opt, value<Type> val, const std::string& description)
		{
			option o = _add_option(opt, description);
			o->value.reset(new value<Type>(val));
		}

		/* add options from another options_description */
		options_description& add(const options_description& od)
		{
			for (auto& o : od._options)
				_options.emplace_back(o);
			return *this;
		}

		/* print options to outputstream, called from operator<<(ostream&,const options_description&) */
		void _print(std::ostream& o)
		{
			if (!_description.empty())
				o << _description << ":" << std::endl;
			std::vector<std::string> left(_options.size()), right(_options.size());
			unsigned maxleft = 0;
			for (std::size_t i = 0; i < _options.size(); ++i)
			{
				right[i] = _options[i]->description;
				if (!_options[i]->shortopt.empty())
				{
					left[i] = "  -" + _options[i]->shortopt;
					if (!_options[i]->longopt.empty())
						left[i] += " [--" + _options[i]->longopt + "]";
				} else {
					left[i] = "  --" + _options[i]->longopt;
				}
				if (_options[i]->value.get() != nullptr)
				{
					std::vector<std::string> defval = _options[i]->value->_defaultvaluestr();
					left[i] += " arg";
					if (!defval.empty())
					{
						left[i] += " (=" + defval[0];
						for (std::size_t j = 1 ; j < defval.size(); ++j)
							left[i] += "," + defval[j];
						left[i] += ")";
					}
				}
				if (left[i].size() > maxleft)
					maxleft = left[i].size();
			}
			if (maxleft > _linelength - _mindesclength - 2)
				maxleft = _linelength - _mindesclength - 2;
			if (maxleft < (_linelength>>2))
				maxleft = _linelength>>2;
			for (std::size_t i = 0; i < left.size(); ++i)
			{
				// print left side
				if (left[i].size() <= maxleft)
					o << left[i] << std::string(maxleft-left[i].size()+2,' ');
				else
					o << left[i] << std::endl << std::string(maxleft+2,' ');
				// print right side
				std::size_t pos;
				while ((pos = right[i].find_first_of('\t')) < right[i].size())
					right[i] = right[i].substr(0,pos) + "   " + right[i].substr(pos+1);
				while (true)
				{
					pos = right[i].find('\n');
					if (pos >= right[i].size())
						pos = right[i].size();
					if (pos + maxleft + 2 > _linelength)
					{
						std::size_t pos2 = right[i].rfind(' ', pos);
						if (pos2 > 0 && pos2 < pos)
							pos = pos2;
						else
							pos = _linelength - maxleft - 2;
					}
					o << right[i].substr(0, pos) << std::endl;
					if (pos < right[i].size() && (right[i][pos] == '\n' || right[i][pos] == ' '))
						right[i] = right[i].substr(pos+1);
					else
						right[i] = right[i].substr(pos);
					if (right[i].empty())
						break;
					else
						o << std::string(maxleft+2, ' ');
				}
			}
		}

		std::string _description;
		std::vector<option> _options;
		unsigned _linelength, _mindesclength;
	};

	/* stores a map of parsed option.name => parser, as well as unrecognized options, positional arguments */
	struct variables_map
		: public std::map<std::string, detail::parser>
	{
		std::vector<std::string> unrecognized;
		std::vector<detail::parser> positional;
	};
	using parsed_options = variables_map;

	/* the main parser: command line parser */
	class command_line_parser {
	public:
		command_line_parser(int argc, char** argv, bool allow_unregistered = false, bool allow_positional = false)
			: _allow_unregistered(allow_unregistered), _allow_positional(allow_positional)
		{
			if (argc < 1) throw;
			_argv.resize(argc-1);
			for (int i = 1; i < argc; ++i)
				_argv[i-1] = std::string(argv[i]);
		}

		command_line_parser& options(const options_description& od)
		{
			for (std::size_t i = 0; i < od._options.size(); ++i)
			{
				_options.emplace_back(od._options[i]);
				option o = _options.back();
				if (!o->shortopt.empty())
				{
					if (_shortopts.count(o->shortopt))
						throw std::runtime_error("program_options::command_line_parser: shortoption defined twice");
					_shortopts[o->shortopt] = o;
				}
				if (!o->longopt.empty())
				{
					if (_longopts.count(o->longopt))
						throw std::runtime_error("program_options::command_line_parser: longoption defined twice");
					_longopts[o->longopt] = o;
				}
			}
			return *this;
		}

		command_line_parser& allow_unregistered()
		{
			_allow_unregistered = true;
			return *this;
		}
		command_line_parser& allow_positional()
		{
			_allow_positional = true;
			return *this;
		}

		command_line_parser& run()
		{
			_vm = variables_map();
			// preregister options with default values
			for (auto o : _options)
			{
				if (o->value.get() != nullptr && o->value->_hasdefaultvalue())
					_vm[o->name]._set(o->value);
			}
			for (std::size_t i = 0; i < _argv.size(); ++i)
			{
				if (_argv[i] == "--")
				{
					// end of options: consider all remaining arguments positional
					for (std::size_t j = i+1; j < _argv.size(); ++j)
						_vm.positional.emplace_back(detail::parser()._add(_argv[j]));
					break;
				}
				option o;
				if (_argv[i].size() == 2 && _argv[i][0] =='-' && _argv[i][1] != '-')
				{
					// check for registered short option
					auto it = _shortopts.find(_argv[i].substr(1,1));
					if (it == _shortopts.end())
					{
						_vm.unrecognized.emplace_back(_argv[i]);
						continue;
					}
					o = it->second;
				} else if (_argv[i].size() >= 3 && _argv[i][0] == '-' && _argv[i][1] == '-')
				{
					// check for registered long option
					auto it = _longopts.find(_argv[i].substr(2));
					if (it == _longopts.end())
					{
						_vm.unrecognized.emplace_back(_argv[i]);
						continue;
					}
					o = it->second;
				} else
				{
					// not an option => positional argument
					_vm.positional.emplace_back(detail::parser()._add(_argv[i]));
					continue;
				}
				// continue processing long/short option
				if (o->value.get() != nullptr)
				{
					// option takes an argument
					if (i+1 >= _argv.size())
						throw std::runtime_error("Program option missing argument: " + _argv[i]);
					_vm[o->name]._set(o->value)._add(_argv[i+1]);
					++i;
					continue;
				}
				else
					_vm[o->name];
			}
			if (!_allow_unregistered && !_vm.unrecognized.empty())
				throw std::runtime_error("Unrecognized program option: " + _vm.unrecognized[0]);
			if (!_allow_positional && !_vm.positional.empty())
				throw std::runtime_error("Unrecognized program option: " + _vm.positional[0].values().front());
			return *this;
		}

		operator variables_map&() { return _vm; }
		operator const variables_map&() const { return _vm; }

		variables_map& vm() { return _vm; }
		const variables_map& vm() const { return _vm; }

		const std::vector<std::string>& unrecognized() const { return _vm.unrecognized; }
		const std::vector<detail::parser>& positional() const { return _vm.positional; }

	private:
		bool _allow_unregistered, _allow_positional;
		std::vector<option> _options;
		std::map<std::string, option> _shortopts;
		std::map<std::string, option> _longopts;
		std::vector<std::string> _argv;
		variables_map _vm;
	};

	// convenient interface to command_line_parser
	inline variables_map parse_command_line(int argc, char** argv, const options_description& od, bool allow_unregistered = false, bool allow_positional = false)
	{
		return std::move(command_line_parser(argc, argv, allow_unregistered, allow_positional).options(od).run().vm());
	}

	/* a configuration file parser */
	class config_file_parser {
	public:
		config_file_parser(std::istream& is, bool allow_unregistered = false)
			: _allow_unregistered(allow_unregistered), _is(is)
		{
		}

		config_file_parser& options(const options_description& od)
		{
			for (std::size_t i = 0; i < od._options.size(); ++i)
			{
				_options.emplace_back(od._options[i]);
				option o = _options.back();
				if (!o->shortopt.empty())
				{
					if (_shortopts.count(o->shortopt))
						throw std::runtime_error("program_options::config_file_parser: shortoption defined twice");
					_shortopts[o->shortopt] = o;
				}
				if (!o->longopt.empty())
				{
					if (_longopts.count(o->longopt))
						throw std::runtime_error("program_options::config_file_parser: longoption defined twice");
					_longopts[o->longopt] = o;
				}
			}
			return *this;
		}

		config_file_parser& allow_unregistered()
		{
			_allow_unregistered = true;
			return *this;
		}

		config_file_parser& run()
		{
			_vm = variables_map();
			// preregister options with default values
			for (auto o : _options)
			{
				if (o->value.get() != nullptr && o->value->_hasdefaultvalue())
					_vm[o->name]._set(o->value);
			}
			std::string line;
			std::string sectionname;
			while (!!_is)
			{
				std::getline(_is, line);
				// remove comments & trim whitespace
				std::size_t pos = line.find('#');
				if (pos < line.size())
					line.erase(pos);
				detail::trim(line);
				if (line.empty())
					continue;

				// split line into longopt and argument
				std::string longopt, argument;
				pos = line.find('=');
				if (pos < line.size())
				{
					longopt = line.substr(0, pos);
					argument = line.substr(pos+1);
				}
				else
					longopt = line;
				detail::trim(longopt);
				detail::trim(argument);
				if (longopt.empty())
					throw std::runtime_error("Configuration file option unspecified: " + line);
				// check for definition of section
				if (argument.empty() && longopt.size() > 2 && longopt.front()=='[' && longopt.back()==']')
				{
					sectionname = longopt.substr(1, longopt.size()-2);
					detail::trim(sectionname);
					continue;
				}
				// prefix optional sectionname to optionname
				if (!sectionname.empty())
					longopt = sectionname + "." + longopt;
				// check for registered long option
				auto it = _longopts.find(longopt);
				if (it == _longopts.end())
				{
					if (argument.empty())
						_vm.unrecognized.emplace_back("--" + longopt);
					else
						_vm.unrecognized.emplace_back("--" + longopt + "=" + argument);
					continue;
				}
				option o = it->second;
				if (o->value.get() != nullptr)
				{
					// option takes an argument
					if (argument.empty())
						throw std::runtime_error("Configuration file option missing argument: " + longopt);
					_vm[o->name]._set(o->value)._add(argument);
				}
				else
					_vm[o->name];
			}
			if (!_allow_unregistered && !_vm.unrecognized.empty())
				throw std::runtime_error("Unrecognized configuration file option: " + _vm.unrecognized[0]);
			return *this;
		}

		operator variables_map&() { return _vm; }
		operator const variables_map&() const { return _vm; }

		variables_map& vm() { return _vm; }
		const variables_map& vm() const { return _vm; }

		const std::vector<std::string>& unrecognized() const { return _vm.unrecognized; }

	private:
		bool _allow_unregistered;
		std::istream& _is;
		std::vector<option> _options;
		std::map<std::string, option> _shortopts;
		std::map<std::string, option> _longopts;
		std::vector<std::string> _argv;
		variables_map _vm;
	};

	// convenient interface to config_file_parser
	inline variables_map parse_config_file(std::istream& is, const options_description& od, bool allow_unregistered = false)
	{
		return std::move(config_file_parser(is, allow_unregistered).options(od).run().vm());
	}

	// store parsed options from src into dest
	// arguments are appended, not overwritten
	// thus for options that can be given once the first stored argument is used
	// and for options that can be given multiple times the argument lists are concatenated
	inline void store(const variables_map& src, variables_map& dest)
	{
		// append values in src to dest
		for (auto& l_p : src)
		{
			if (dest.count(l_p.first))
				for (auto& s : l_p.second.values())
					dest[l_p.first]._add(s);
			else
				dest.emplace(l_p);
		}
		for (auto& s : src.unrecognized)
			dest.unrecognized.emplace_back(s);
		for (auto& s : src.positional)
			dest.positional.emplace_back(s);
	}

	// finalize all options
	// - set default value if option was not otherwise given
	// - if target variable is given then set it to parsed value
	inline void notify(variables_map& vm)
	{
		for (auto& l_p : vm)
			l_p.second._finalize();
	}

} // namespace program_options

namespace std {
	std::ostream& operator<<(std::ostream& o, program_options::options_description& op)
	{
		op._print(o);
		return o;
	}
} // namespace std

#endif // PROGRAM_OPTIONS_HPP
