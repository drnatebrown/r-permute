/* static_column - Data structure holding bitvector making column of L or F with rank support
    Copyright (C) 2022 Nathaniel Brown
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see http://www.gnu.org/licenses/ .
*/
/*!
   \file static_column.hpp
   \brief static_column Data structure holding bitvector making column of L or F supporting rank/select
   \author Nathaniel Brown
   \date 18/03/2022
*/

#ifndef _STATIC_COLUMN_HH
#define _STATIC_COLUMN_HH

#include <common.hpp>

#include <sdsl/int_vector.hpp>
#include <sdsl/rmq_support.hpp>
#include <sdsl/structure_tree.hpp>
#include <sdsl/util.hpp>

using namespace std;
using namespace sdsl;

template < class bv_t = bit_vector >
class static_column
{
private:
    typedef typename bv_t::rank_1_type rank_t;
    typedef typename bv_t::select_1_type select_t;

    bv_t col;
    rank_t rank;
    select_t select;

public:
    static_column() {}

    static_column(bit_vector bv)
    {
        col = bv_t(bv);
        rank = rank_t(&col);
        select = select_t(&col);
    }

    // Access position at bit_vector
	bool operator[](ulint i)
    {
		assert(i < size());
		return col[i];
	}

    ulint size()
    {
        return col.size();
    }
    ulint bits_set()
    {
        return rank(size());
    }

    // Get idx in the column given run and offset
    ulint get_idx(ulint k, ulint d)
    {
        return select(k + 1) + d;
    }

    // Finds the ith bit before or including this position
    std::pair<ulint, ulint> predecessor(ulint i)
    {
        assert(rank(i + 1) > 0);
        ulint rank_pred = rank(i + 1) - 1;
        return std::make_pair(rank_pred, select(rank_pred + 1));
    }

    bv_t get_bv()
    {
        return col;
    }

    /* serialize to the ostream (we build construction first and store, then apply one of the other constructors)
    * \param out     the ostream
    */
    size_t serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name ="")
    {
        sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
        size_t written_bytes = 0;

        written_bytes += col.serialize(out, v, "column");

        return written_bytes;
    }

    /* load from the istream
    * \param in the istream
    */
    void load(std::istream &in)
    {
        col = bv_t();
        col.load(in);
        rank = rank_t(&col);
        select = select_t(&col);
    }
};

#endif /* end of include guard: _STATIC_COLUMN_HH */