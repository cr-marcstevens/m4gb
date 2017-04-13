/*****************************************************************************\
*                                                                             *
*   M4GB - an efficient Groebner-basis algorithm                              *
*   Copyright (C) 2017  Rusydi Makarim, Marc Stevens,                         *
*   Cryptology Group, Centrum Wiskunde & Informatica (CWI), The Netherlands   *
*                                                                             *
*   This file is part of M4GB.                                                *
*                                                                             *
*   M4GB is free software: you can redistribute it and/or modify              *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation, either version 3 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   M4GB is distributed in the hope that it will be useful,                   *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with M4GB.  If not, see <http://www.gnu.org/licenses/>.             *
*                                                                             *
\*****************************************************************************/

#ifndef M4GB_LOGGER_HPP
#define M4GB_LOGGER_HPP

#include <string>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

// set default loglevel to output on construction of logger (needed for logging during construction of global/static variables)
#ifndef DEFAULT_LOGLEVEL
#define DEFAULT_LOGLEVEL lg_info
#endif

// set default loglevel of messages of initialization of galois field objects
#ifndef GF_LOGLEVEL 
#define GF_LOGLEVEL lg_verbose
#endif

namespace gb {

	enum log_level { lg_abort, lg_error, lg_warn, lg_info, lg_verbose, lg_verbose2, lg_verbose3, lg_verbose4 };

	class logger_t
	{
	public:
		logger_t(log_level ll = DEFAULT_LOGLEVEL)
			: _out(&std::cout), _outll(ll)
		{}

		logger_t(std::ostream& o, log_level ll = DEFAULT_LOGLEVEL)
			: _out(&o), _outll(ll)
		{}

		// log msg to out if level<=outll  AND  log msg to level_specific_out[level] if present
		void msg(const std::string& str, log_level level = lg_info)
		{
			// if log level is warn, error or abort then mark message as such
			if (level <= lg_warn)
			{
				static const char* levelstr[3] = { "!!ABORT!! ", "ERROR ", "Warning " };
				std::string newstr = _add_component(levelstr[level], str);
				if (level <= _outll)
					(*_out) << newstr << std::flush;
				if (_level_specific_out.count(level))
					(*_level_specific_out[level]) << newstr << std::flush;
				// if log level is abort we actually throw the message
				if (level == lg_abort)
					throw std::runtime_error(newstr);
				return;
			}

			if (level <= _outll)
				(*_out) << str << std::flush;
			if (_level_specific_out.count(level))
				(*_level_specific_out[level]) << str << std::flush;
		}

		// assumes newline marker is either \r\n or \n
		void msg(const std::string& component, const std::string& str, log_level level = lg_info)
		{
			msg(_add_component("[" + component + "] ", str), level);
		}

		// set common out
		void set_out(std::ostream& o)
		{
			_out = &o;
		}

		// set max log_level to pass to out
		void set_log_level(log_level ll = DEFAULT_LOGLEVEL)
		{
			_outll = ll;
		}

		// set output stream for specific log_level
		void set_log_level_out(log_level ll, std::ostream& o)
		{
			_level_specific_out[ll] = &o;
		}

		// clear output stream for specific log_level
		void clear_log_level_out(log_level ll)
		{
			_level_specific_out.erase(ll);
		}

		class logger_stream_t
			: public std::stringstream
		{
			logger_t& _logger;
			log_level _outll;
			std::string _component;
		public:
			logger_stream_t(logger_t& l, const std::string& component, log_level ll)
				: _logger(l), _outll(ll), _component(component)
			{}
			
			logger_stream_t(const logger_stream_t& r)
				: _logger(r._logger), _outll(r._outll), _component(r._component)
			{}

			~logger_stream_t()
			{
				if (!_component.empty())
					_logger.msg(_component, str(), _outll);
				else
					_logger.msg(str(), _outll);
			}
		};

		logger_stream_t operator()(log_level ll = lg_info)
		{
			return logger_stream_t(*this, std::string(), ll);
		}

		logger_stream_t operator()(const std::string& component, log_level ll = lg_info)
		{
			return logger_stream_t(*this, component, ll);
		}

	private:
		std::ostream* _out;
		int _outll;
		std::map<log_level, std::ostream*> _level_specific_out;

		std::string _add_component(const std::string& component, const std::string& str)
		{
			std::string newstr = str.find_first_of("\r\n") == 0 ? str : (component + str);
			std::size_t pos = newstr.find('\n');
			while (pos != std::string::npos)
			{
				if (pos + 1 >= newstr.size())
					break;
				newstr.insert(pos + 1, component);
				pos = newstr.find('\n', pos + 1);
			}
			return newstr;
		}
	};

	inline logger_t& get_logger()
	{
		static logger_t _log;
		return _log;
	}

} // namespace gb

#endif // M4GB_LOGGER_HPP
