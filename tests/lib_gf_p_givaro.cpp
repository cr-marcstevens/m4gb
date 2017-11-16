/***************************************************************************** \
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

#include "../lib/gf_elem_simple.hpp"
#include "../lib/gf_p_givaro.hpp"

#include "test_core.hpp"

const std::size_t FIELDCHAR_32 = 4294967291; //32-bit prime

#ifdef __GIVARO_HAVE_INT128
const std::size_t FIELDCHAR_64 = 18446744073709551557u; //64-bit prime
#endif

template<typename givfield_t>
int test_field_32bit()
{
	typedef typename givfield_t::gfelm_t elem_t;
	static const auto & givfield = givfield_t::givaro_field;

	CHECK((givfield_t::gfchar == givfield.characteristic()));
	CHECK((givfield_t::gfsize == givfield.size()));

	elem_t a = 4294967290, b = 73;

	CHECK((a + a == (elem_t) 4294967289));
	CHECK((a + b == (elem_t) 72));

	CHECK((a * a == (elem_t) 1));
	CHECK((a * b == (elem_t) 4294967218));

	CHECK((a - b == (elem_t) 4294967217));
	CHECK((b - a == (elem_t) 74));
	CHECK((-a == (elem_t) 1));
	CHECK((-b == (elem_t) 4294967218));

	CHECK((a / b == (elem_t) 2706417745));
	CHECK((b / a == (elem_t) 4294967218));

	std::array<elem_t, 4> A = {{ 1581862525, 3486811133, 1383848677, 2985670103 }};
	std::array<elem_t, 4> B = {{ 2262041230, 2683515117, 4129968110, 3518162020 }};
	elem_t c = 4131525954;

	gb::add_to(&A[0], &B[0], A.size());
	CHECK((A[0] == (elem_t) 3843903755));
	CHECK((A[1] == (elem_t) 1875358959));
	CHECK((A[2] == (elem_t) 1218849496));
	CHECK((A[3] == (elem_t) 2208864832));

	gb::mul_to(&A[0], c, A.size());
	CHECK((A[0] == (elem_t) 1040539192));
	CHECK((A[1] == (elem_t) 2626948562));
	CHECK((A[2] == (elem_t) 1433856492));
	CHECK((A[3] == (elem_t) 2212300714));

	gb::add_to(&A[0], c, &B[0], A.size());
	CHECK((A[0] == (elem_t) 167532142));
	CHECK((A[1] == (elem_t) 682951414));
	CHECK((A[2] == (elem_t) 877072425));
	CHECK((A[3] == (elem_t) 723012235));

	gb::substract_to(&A[0], &B[0], A.size());
	CHECK((A[0] == (elem_t) 2200458203));
	CHECK((A[1] == (elem_t) 2294403588));
	CHECK((A[2] == (elem_t) 1042071606));
	CHECK((A[3] == (elem_t) 1499817506));

	return 0;
}

#ifdef __GIVARO_HAVE_INT128
template<typename givfield_t>
int test_field_64bit()
{
	typedef typename givfield_t::gfelm_t elem_t;
	static const auto & givfield = givfield_t::givaro_field;

	CHECK((givfield_t::gfchar == givfield.characteristic()));
	CHECK((givfield_t::gfsize == givfield.size()));

	elem_t a = 2958766035670818362u, b = 15564472361175644638u;

	CHECK((a + a == (elem_t) 5917532071341636724u));
	CHECK((a + b == (elem_t) 76494323136911443u));

	CHECK((a * a == (elem_t) 4729103867298191197u));
	CHECK((a * b == (elem_t) 12729837616975756788u));

	CHECK((a - b == (elem_t) 5841037748204725281u));
	CHECK((b - a == (elem_t) 12605706325504826276u));
	CHECK((-a == (elem_t) 15487978038038733195u));
	CHECK((-b == (elem_t) 2882271712533906919u));

	CHECK((a / b == (elem_t) 10640594564652858078u));
	CHECK((b / a == (elem_t) 17831351648684494826u));

	std::array<elem_t, 4> A = {{
			287489043855835753u, 4900276070945721669u,
			5740120412446554335u, 4535436226369274292u
	}};
	std::array<elem_t, 4> B = {{
			3002170284525279349u, 4664815685323110156u,
			10017734379935229829u, 4574749014663256466u
	}};

	elem_t c = 14915550343613008155u;

	gb::mul_to(&A[0], c, A.size());
	CHECK((A[0] == (elem_t) 17635000812212413270u));
	CHECK((A[1] == (elem_t) 2973065834432126711u));
	CHECK((A[2] == (elem_t) 2112860628507011958u));
	CHECK((A[3] == (elem_t) 17869252883272174299u));

	gb::add_to(&A[0], c, &B[0], A.size());
	CHECK((A[0] == (elem_t) 13980991738264168291u));
	CHECK((A[1] == (elem_t) 13047023386162756704u));
	CHECK((A[2] == (elem_t) 3932864729230618213));
	CHECK((A[3] == (elem_t) 8712575437511400460u));

	gb::add_to(&A[0], &B[0], A.size());
	CHECK((A[0] == (elem_t) 16983162022789447640u));
	CHECK((A[1] == (elem_t) 17711839071485866860u));
	CHECK((A[2] == (elem_t) 13950599109165848042u));
	CHECK((A[3] == (elem_t) 13287324452174656926u));

	gb::substract_to(&A[0], &B[0], A.size());
	CHECK((A[0] == (elem_t) 13980991738264168291u));
	CHECK((A[1] == (elem_t) 13047023386162756704u));
	CHECK((A[2] == (elem_t) 3932864729230618213u));
	CHECK((A[3] == (elem_t) 8712575437511400460u));

	return 0;
}
#endif

int test()
{
	typedef Givaro::Modular<uint64_t> GivModular_32bitPrime;
	typedef gb::gf_p_givaro<FIELDCHAR_32, GivModular_32bitPrime> givfield_modular_uint32_t;
	test_field_32bit<givfield_modular_uint32_t>();

#ifdef __GIVARO_HAVE_INT128
	typedef Givaro::Modular<uint128_t> GivModular_64bitPrime;
	typedef gb::gf_p_givaro<FIELDCHAR_64, GivModular_64bitPrime> givfield_modular_uint64_t;
	test_field_64bit<givfield_modular_uint64_t>();
#endif
	return 0;
}
