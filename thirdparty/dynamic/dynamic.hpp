// Copyright (c) 2017, Nicola Prezza.  All rights reserved.
// Use of this source code is governed
// by a MIT license that can be found in the LICENSE file.

/*
 * dynamic.hpp
 *
 *  Created on: Oct 21, 2015
 *      Author: nico
 */

#ifndef DYNAMIC_TYPEDEFS_HPP_
#define DYNAMIC_TYPEDEFS_HPP_

#include "./internal/spsi.hpp"
#include "./internal/gap_bitvector.hpp"
#include "./internal/spsi_check.hpp"
#include "./internal/succinct_bitvector.hpp"
#include "./internal/sparse_vector.hpp"
#include "./internal/packed_vector.hpp"
#include "./internal/hacked_vector.hpp"
#include "./internal/lciv.hpp"
#include "./internal/wm_string.hpp"

namespace dyn{

/*
 * a succinct searchable partial sum with inserts implemented with cache-efficient
 * B trees. Logarithmic-sized leaves
 */
typedef spsi<packed_vector,256,16> packed_spsi;
typedef lciv<packed_vector,256,16> packed_lciv;

/*
 * a succinct searchable partial sum with inserts implemented with cache-efficient
 * B trees. Quadratic-log sized leaves
 */
typedef spsi<packed_vector,8192,16> succinct_spsi;
typedef lciv<packed_vector,8192,16> succinct_lciv;

/*
 * dynamic gap-encoded bitvector
 */
typedef gap_bitvector<packed_spsi> gap_bv;

/*
 * dynamic succinct bitvector (about 1.1n bits)
 */
typedef succinct_bitvector<spsi<packed_bit_vector,8192,16>> suc_bv;

}

#endif /* DYNAMIC_TYPEDEFS_HPP_ */
