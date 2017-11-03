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

#ifndef M4GB_OPTIONS_HPP
#define M4GB_OPTIONS_HPP

#include <string>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <stdexcept>

namespace gb {

	class options_t
	{
	public:
		// option format "option=value"
		void set(const std::string& optioneqvalue)
		{
			std::size_t pos = optioneqvalue.find('=');
			if (pos < optioneqvalue.size())
				_data[optioneqvalue.substr(0,pos)] = optioneqvalue.substr(pos+1);
			else
				_data[optioneqvalue] = "";
		}
		void set(const std::string& option, const std::string& value)
		{
			_data[option] = value;
		}

		bool isset(const std::string& option) const
		{
			return _data.count(option) != 0;
		}

		const std::string& get(const std::string& option) const
		{
			auto it = _data.find(option);
			if (it == _data.end())
				return emptyval;
			return it->second;
		}

		const std::string emptyval;
	private:
		std::map<std::string,std::string> _data;
	};

	inline options_t& get_options()
	{
		static options_t _opt;
		return _opt;
	}

} // namespace gb

#endif // M4GB_OPTIONS_HPP
