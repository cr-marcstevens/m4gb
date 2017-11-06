/*********************************************************************************\
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
#include <map>

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
		// to_string: like std::to_string, but extend with std::string (passthrough) and std::vector (make list)
		template<typename Type>
		std::string to_string(const Type& t)
		{
			std::stringstream strstr;
			strstr << t;
			return strstr.str();
		}
		std::string to_string(const std::string& t)
		{
			return t;
		}
		template<typename Type, typename A>
		std::string to_string(const std::vector<Type,A>& v)
		{
			std::string ret = "[";
			if (!v.empty())
				ret = ret + to_string(v[0]);
			for (std::size_t i = 1; i < v.size(); ++i)
				ret = ret + ","  + to_string(v[i]);
			return ret + "]";
		}
	}

	/* parser class is initialized with a string, parses it to a type on demand via 'as<type>()' and 'to(var)' */
	class parser {
	public:
		parser() {}
		parser(const std::string& val) : _val(val) {}

		template<typename Type>
		Type as() const
		{
			Type ret;
			detail::parse(_val, ret);
			return ret;
		}

		template<typename Type>
		void to(Type& target) const
		{
			detail::parse(_val, target);
		}

		const std::string& val() const { return _val; }
	private:
		std::string _val;
	};

	/* stores a map of parsed longoption => parser(arg) */
	struct variables_map 
		: public std::map<std::string, parser>
	{
	};

	/* base interface to wrapper around variables and default values */
	class value_base {
	public:
		virtual ~value_base() {}
		virtual std::string defaultvaluestr() = 0;
		virtual void parse(const std::string& arg) = 0;
	};

	/* wrapper around variables and default values */
	template<typename Type>
	class value
		: public value_base
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

		virtual std::string defaultvaluestr()
		{
			if (_defaultvalue.get() != nullptr)
				return " (=" + detail::to_string(*_defaultvalue) + ")";
			return std::string();
		}

		/* distinguish between parsing into Type and vector<Type> */
		virtual void parse(const std::string& arg)
		{
			if (_target != nullptr)
			{
				parser tmp(arg);
				tmp.to(*_target);
			}
		}
	private:
		Type* _target;
		std::shared_ptr<Type> _defaultvalue;
	};

	/* contains option description, link to variable and/or default value, and parsed arguments */
	struct option_t {
		std::string shortopt, longopt;
		std::string description;
		std::shared_ptr<value_base> value;
		std::vector<std::string> args;
	};

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
			inline add_options_t operator()(const std::string& option, const std::string& description)
			{
				_parent._add_option(option, description);
				return *this;
			}
			template<typename Type>
			inline add_options_t operator()(const std::string& option, value<Type> val, const std::string& description)
			{
				_parent._add_option(option, val, description);
				return *this;
			}
		private:
			options_description& _parent;
		};
		inline add_options_t add_options()
		{
			return add_options_t(*this);
		}

		option_t& _add_option(const std::string& option, const std::string& description)
		{
			_options.emplace_back();
			option_t& o(_options.back());
			o.description = description;
			std::size_t pos = option.find(',');
			if (pos < option.size())
			{
				o.longopt = option.substr(0, pos);
				o.shortopt = option.substr(pos+1);
				if (o.longopt.size() == 1)
					std::swap(o.longopt,o.shortopt);
				if (o.longopt.size() == 1)
					throw std::runtime_error("program_options::_add_option: long option has length 1");
				if (o.shortopt.size() > 1)
					throw std::runtime_error("program_options::_add_option: short option has length > 1");
			}
			else
			{
				if (option.size() == 1)
					o.longopt = o.shortopt = option;
				else
					o.longopt = option;
			}
			return o;
		}

		template<typename Type>
		void _add_option(const std::string& option, value<Type> val, const std::string& description)
		{
			option_t& o = _add_option(option, description);
			o.value.reset(new value<Type>(val));
		}

		options_description& add(const options_description& od)
		{
			for (auto& o : od._options)
				_options.emplace_back(o);
			return *this;
		}

		void _print(std::ostream& o)
		{
			if (!_description.empty())
				o << _description << ":" << std::endl;
			std::vector<std::string> left(_options.size()), right(_options.size());
			unsigned maxleft = 0;
			for (std::size_t i = 0; i < _options.size(); ++i)
			{
				right[i] = _options[i].description;
				if (!_options[i].shortopt.empty())
				{
					left[i] = "  -" + _options[i].shortopt;
					if (!_options[i].longopt.empty())
						left[i] = left[i] + " [--" + _options[i].longopt + "]";
				} else {
					left[i] = "  --" + _options[i].longopt;
				}
				if (_options[i].value.get() != nullptr)
				{
					left[i] = left[i] + " arg" + _options[i].value->defaultvaluestr();
				}
				if (left[i].size() > maxleft)
					maxleft = left[i].size();
			}
			if (maxleft > _linelength - _mindesclength - 2)
				maxleft = _linelength - _mindesclength - 2;
			if (maxleft < (_linelength>>2))
				maxleft = _linelength>>2;
			for (std::size_t i = 0; i < _options.size(); ++i)
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
		std::vector< option_t > _options;
		unsigned _linelength, _mindesclength;
	};

	/* the main parser */
	class parsed_options {
	public:
		parsed_options()
			: _allow_unregistered(false), _allow_positional(false)
		{
		}

		parsed_options(int argc, char** argv)
			: _allow_unregistered(false), _allow_positional(false)
		{
			if (argc < 1) throw;
			_argv.resize(argc-1);
			for (int i = 1; i < argc; ++i)
				_argv[i-1] = std::string(argv[i]);
		}

		parsed_options& options(const options_description& od)
		{
			for (std::size_t i = 0; i < od._options.size(); ++i)
			{
				_options.emplace_back(od._options[i]);
				option_t& o = _options.back();
				if (!o.shortopt.empty())
				{
					if (_shortopts.count(o.shortopt))
						throw std::runtime_error("program_options::parsed_options: shortoption defined twice");
					_shortopts[o.shortopt] = o;
				}
				if (!o.longopt.empty())
				{
					if (_longopts.count(o.longopt))
						throw std::runtime_error("program_options::parsed_options: longoption defined twice");
					_longopts[o.longopt] = o;
				}
			}
			return *this;
		}

		parsed_options& allow_unregistered()
		{
			_allow_unregistered = true;
			return *this;
		}
		parsed_options& allow_positional()
		{
			_allow_positional = true;
			return *this;
		}

		parsed_options& run()
		{
			_vm.clear();
			for (std::size_t i = 0; i < _argv.size(); ++i)
			{
				if (_argv[i] == "--")
				{
					// end of options: consider all remaining arguments positional
					for (std::size_t j = i+1; j < _argv.size(); ++j)
						_positional.emplace_back(_argv[j]);
					break;
				}
				option_t* o = nullptr;
				if (_argv[i].size() == 2 && _argv[i][0] =='-' && _argv[i][1] != '-')
				{
					// check for registered short option
					auto it = _shortopts.find(_argv[i].substr(1,1));
					if (it == _shortopts.end())
					{
						_unrecognized.emplace_back(_argv[i]);
						continue;
					}
					o = & it->second;
				} else if (_argv[i].size() >= 3 && _argv[i][0] == '-' && _argv[i][1] == '-')
				{
					// check for registered long option
					auto it = _longopts.find(_argv[i].substr(2));
					if (it == _longopts.end())
					{
						_unrecognized.emplace_back(_argv[i]);
						continue;
					}
					o = & it->second;
				} else
				{
					// not an option => positional argument
					_positional.emplace_back(_argv[i]);
					continue;
				}
				// continue processing long/short option
				if (o->value.get() != nullptr)
				{
					// option takes an argument
					if (i+1 >= _argv.size())
						throw std::runtime_error("Program option missing argument: " + _argv[i]);
					_vm[o->longopt] = parser(_argv[i+1]);
					o->value->parse(_argv[i+1]);
					++i; 
					continue;
				} else
					_vm[o->longopt] = parser();
			}
			if (!_allow_unregistered && !_unrecognized.empty())
				throw std::runtime_error("Unrecognized program option: " + _unrecognized[0]);
			if (!_allow_positional && !_positional.empty())
				throw std::runtime_error("Unrecognized program option: " + _positional[0].val());
				
			return *this;
		}		

		const variables_map& vm() const { return _vm; }
		const std::vector<std::string>& unrecognized() const { return _unrecognized; }
		const std::vector<parser>& positional() const { return _positional; }

	private:
		bool _allow_unregistered, _allow_positional;
		std::vector< option_t > _options;
		std::map< std::string, option_t> _shortopts;
		std::map< std::string, option_t> _longopts;
		std::vector<std::string> _argv;
		std::vector<parser> _positional;
		std::vector<std::string> _unrecognized;
		variables_map _vm;
	};
	using command_line_parser = parsed_options;

	inline void store(const parsed_options& po, variables_map& vm)
	{
		// for boost::program_options compatibility
		// simply copy the variables_map from inside parsed_options
		vm = po.vm();
	}

	inline void notify(variables_map& vm)
	{
		// for boost::program_options compatibility
		// do nothing
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
