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

#ifndef M4GB_TESTS_TEST_CORE_HPP
#define M4GB_TESTS_TEST_CORE_HPP

#include "../lib/logger.hpp"

#include <iostream>

#undef CHECK
#define CHECK(s) { bool res = false; try { res = (s); } catch (std::exception& e) { res = false; }; if (!res) { std::cerr << "Check FAILED in " __FILE__ " at line " << __LINE__ << "!" << std::endl; return 1; } }

int test();

int main(int argc, char** argv)
{
	gb::get_logger().set_log_level(gb::lg_verbose4);
	return test();
}

#endif
