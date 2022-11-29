/* construcion - Parent class for shared methods of both construction algorithms
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
   \file construction.hpp
   \brief construcion.hpp Parent class for shared methods of both construction algorithms
   \author Nathaniel Brown
   \date 18/03/2022
*/

#ifndef _CONSTRUCTION_HH
#define _CONSTRUCTION_HH

#include <common.hpp>

#include <../ds/FL_table.hpp>
#include <../ds/static_column.hpp>

#include <sdsl/int_vector.hpp>
#include <sdsl/structure_tree.hpp>
#include <sdsl/util.hpp>

using namespace std;
using namespace sdsl;

template  < class bv_t = bit_vector >
class constructor
{
protected:
    FL_table table;

    static_column<bv_t> P; // L column with runheads set
    static_column<bv_t> Q; // F column with runheads set

public:

    constructor() {}

    constructor(std::ifstream &heads, std::ifstream &lengths)
    {
        table = FL_table(heads, lengths);

        heads.clear();
        heads.seekg(0);
        lengths.clear();
        lengths.seekg(0);

        // Mark bits in P and Q to build columns
        bv_t P_bits = bv_t(table.size(), false);
        bv_t Q_bits = bv_t(table.size(), false);
        
        char c;
        ulint pos = 0; // Current cursor to run head positions
        vector<vector<size_t>> char_runs = vector<vector<size_t>>(ALPHABET_SIZE); // used to build Q
        while ((c = heads.get()) != EOF)
        {
            P_bits[pos] = true;

            size_t length = 0;
            lengths.read((char *)&length, 5);

            if (c > TERMINATOR)
            {
                char_runs[c].push_back(length);
            }
            else
            {
                char_runs[TERMINATOR].push_back(length);
            }

            pos += length;
        }

        pos = 0;
        for (size_t c = 0; c < ALPHABET_SIZE; ++c)
        {
            for (size_t j = 0; j < char_runs[c].size(); ++j) {
                Q_bits[pos] = true;
                pos += char_runs[c][j];
            }
        }

        P = static_column<bv_t>(P_bits);
        Q = static_column<bv_t>(Q_bits);
    }

    // For a corresponding position in Q, find and return its position in P
    ulint find(ulint i) {
        // Finds the associated run and position in Q for positon i
        auto[k, k_pos] = Q.predecessor(i);
        // Calculate the offset by subtracting the position of i from its run head position in Q
        ulint d = i - k_pos;

        // Find the corresponding run and offset in the L column
        auto[k_prime, d_prime] = table.FL(k, d);
        ulint j = Q.get_idx(k_prime, d_prime); // Position is with respect to Q, so find its absolute position

        return j;
    }

    void stats() {
        table.bwt_stats();

        sdsl::nullstream ns;

        verbose("Memory consumption (bytes).");
        verbose("              FL table:    ", table.serialize(ns));
        verbose("              P:           ", P.serialize(ns));
        verbose("              Q:           ", Q.serialize(ns));
    }

    std::string get_file_extension() const
    {
        return ".constructor";
    }

    /* serialize to the ostream (we build construction first and store, then apply one of the other constructors)
    * \param out     the ostream
    */
    size_t serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name ="")
    {
        sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
        size_t written_bytes = 0;

        written_bytes += table.serialize(out, v, "FL_table");

        written_bytes += P.serialize(out, v, "P_bv");
        written_bytes += Q.serialize(out, v, "Q_bv");

        return written_bytes;
    }

    /* load from the istream
    * \param in the istream
    */
    void load(std::istream &in)
    {
        table.load(in);

        P.load(in);
        Q.load(in);
    }
};

#endif /* end of include guard: _CONSTRUCTOR_HH */