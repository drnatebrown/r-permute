/* FL_table - Table supporting first-to-last mapping of BWT
    Copyright (C) 2021 Nathaniel Brown
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
   \file FL_table.hpp
   \brief FL_table.hpp Table supporting first-to-last mapping of BWT
   \author Nathaniel Brown
   \author Massimiliano Rossi
   \date 19/11/2021
*/

#ifndef _FL_TABLE_HH
#define _FL_TABLE_HH

#include <algorithm>
#include <common.hpp>

#include <sdsl/structure_tree.hpp>
#include <sdsl/util.hpp>
#include <climits>
#include <vector>

using namespace std;

class FL_table
{
public:
    // Row of the FL table
    typedef struct FL_row
    {
        char character;
        ulint length;
        ulint interval;
        ulint offset;
        ulint L_pos;

        size_t serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name ="")
        {
            sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_t written_bytes = 0;

            out.write((char *)&character, sizeof(character));
            written_bytes += sizeof(character);

            out.write((char *)&interval, sizeof(interval));
            written_bytes += sizeof(interval);

            out.write((char *)&length, sizeof(length));
            written_bytes += sizeof(length);

            out.write((char *)&offset, sizeof(offset));
            written_bytes += sizeof(offset);

            return written_bytes;
        }

        void load(std::istream &in)
        {
            in.read((char *)&character, sizeof(character));
            in.read((char *)&interval, sizeof(interval));
            in.read((char *)&length, sizeof(length));
            in.read((char *)&offset, sizeof(offset));
        }
    };

    FL_table() {}

    // TODO: Add builder for BWT (not heads/lengths)
    FL_table(std::ifstream &heads, std::ifstream &lengths)
    {
        heads.clear();
        heads.seekg(0);
        lengths.clear();
        lengths.seekg(0);
        
        vector<char> L_chars = vector<char>();
        vector<ulint> L_lens = vector<ulint>();
        vector<vector<ulint>> L_block_indices = vector<vector<ulint>>(ALPHABET_SIZE);
        vector<vector<ulint>> char_runs = vector<vector<ulint>>(ALPHABET_SIZE); // Vector containing lengths for runs of certain character
        // vector<vector<ulint>> first_pos = vector<vector<ulint>>(ALPHABET_SIZE); // Vector containing first position in L for each char
        
        //ulint max_len = 0;

        char c;
        n = 0;
        ulint i = 0;
        while ((c = heads.get()) != EOF)
        {
            size_t length = 0;
            lengths.read((char *)&length, 5);
            if (c > TERMINATOR)
            {
                L_chars.push_back(c);
                L_lens.push_back(length);
                L_block_indices[c].push_back(i++);
                char_runs[c].push_back(length);
            }
            else
            {
                L_chars.push_back(TERMINATOR);
                L_lens.push_back(length);
                L_block_indices[TERMINATOR].push_back(i++);
                char_runs[TERMINATOR].push_back(length);
            }
            n+=length;

            //if (length > max_len) max_len = length;
        }
        //cout << "MAX: " << max_len;
        r = L_chars.size();

        FL_runs = vector<FL_row>(r);
        //vector<vector<size_t>> F_block_indices = vector<vector<size_t>>(ALPHABET_SIZE);
        i = 0;
        for (size_t c = 0; c < ALPHABET_SIZE; ++c)
        {
            for (size_t j = 0; j < char_runs[c].size(); ++j) {
                size_t length = char_runs[c][j];
                FL_runs[i].character = (unsigned char) c;
                FL_runs[i].length = length;
                ++i;
            }
        }

        ulint k = 0; // current row to be filled
        for(size_t i = 0; i < L_block_indices.size(); ++i) 
        {
            ulint F_curr = 0; // current position when scanning F
            ulint F_seen = 0; // characters seen before position in F
            ulint L_curr = 0; // current position when scanning L
            ulint L_seen = 0; // characters seen before position in L
            for(size_t j = 0; j < L_block_indices[i].size(); ++j) 
            {
                while (L_curr < L_block_indices[i][j]) {
                    L_seen += L_lens[L_curr++];
                }
                while (F_seen + FL_runs[F_curr].length <= L_seen) {
                    F_seen += FL_runs[F_curr++].length;
                }

                FL_runs[k].interval = F_curr;
                FL_runs[k].offset = L_seen - F_seen;
                FL_runs[k].L_pos = L_curr;
                ++k;
            }
        }

        #ifdef PRINT_STATS
        cout << "Text runs: " << runs() << std::endl;
        cout << "Text length: " << size() << std::endl;
        #endif
    }

    const FL_row get(size_t i)
    {
        assert(i < FL_runs.size());
        return FL_runs[i];
    }

    ulint size()
    {
        return n;
    }

    ulint runs()
    {
        return r;
    }

    void invert(std::string outfile) 
    {
        std::ofstream out(outfile);

        ulint interval = 0;
        ulint offset = 0;

        char c;
        while((c = get_char(interval)) > TERMINATOR) 
        {
            out << c;
            std::pair<ulint, ulint> pos = FL(interval, offset);
            interval = pos.first;
            offset = pos.second;
        }
    }

    /*
     * \param Run position (RLE intervals)
     * \param Current character offset in block
     * \return block position and offset of preceding character
     */
    std::pair<ulint, ulint> FL(ulint run, ulint offset)
    {
        ulint next_interval = FL_runs[run].interval;
	    ulint next_offset = FL_runs[run].offset + offset;

	    while (next_offset >= FL_runs[next_interval].length) 
        {
            next_offset -= FL_runs[next_interval++].length;
        }

	    return std::make_pair(next_interval, next_offset);
    }

    uchar get_char(ulint i)
    {
        return get(i).character;
    }

    std::string get_file_extension() const
    {
        return ".FL_table";
    }

    void bwt_stats()
    {
        ulint n = size();
        ulint r = runs();
        verbose("Number of BWT equal-letter runs: r = ", r);
        verbose("Length of complete BWT: n = ", n);
        verbose("Rate n/r = ", double(n) / r);
        verbose("log2(r) = ", log2(double(r)));
        verbose("log2(n/r) = ", log2(double(n) / r));
    }

    void get_run_lcs(std::string outfile) {
        ulint k = 0;
        ulint d = 0;
        ulint curr_lcs = 0;

        std::vector<ulint> min_lcs = std::vector<ulint>(r, ULONG_MAX);
        do {
            if (d == 0) {
                curr_lcs = 0;
                // we define lcs to be 0 for runs of length 1
                if (FL_runs[k].length == 1) min_lcs[FL_runs[k].L_pos] = curr_lcs;
            }
            else {
                ++curr_lcs;
                min_lcs[FL_runs[k].L_pos] = std::min(min_lcs[k], curr_lcs);
            }

            std::pair<ulint, ulint> new_pos = FL(k, d);
            k = new_pos.first;
            d = new_pos.second;
        } while (k != 0 || d != 0);

        FILE *lcs_file;
        if ((lcs_file = fopen(outfile.c_str(), "w")) == nullptr) {
            std::cerr << "open() file " << outfile << " failed";
            return;
        }

        for (size_t i = 0; i < r; ++i) {
            size_t lcs_val = min_lcs[i];
            cout << lcs_val << "\n";
            if (fwrite(&lcs_val, 5, 1, lcs_file) != 1)
                std::cerr << "Tunnel write error 1";
        }
    }

    /* serialize to the ostream
    * \param out     the ostream
    */
    size_t serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name ="")
    {
        sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
        size_t written_bytes = 0;

        out.write((char *)&n, sizeof(n));
        written_bytes += sizeof(n);

        out.write((char *)&r, sizeof(r));
        written_bytes += sizeof(r);

        size_t size = FL_runs.size();
        out.write((char *)&size, sizeof(size));
        written_bytes += sizeof(size);

        for(size_t i = 0; i < size; ++i)
        {
            written_bytes += FL_runs[i].serialize(out, v, "FL_run_" + std::to_string(i));
        }

        return written_bytes;
    }

    /* load from the istream
    * \param in the istream
    */
    void load(std::istream &in)
    {
        size_t size;

        in.read((char *)&n, sizeof(n));
        in.read((char *)&r, sizeof(r));

        in.read((char *)&size, sizeof(size));
        FL_runs = std::vector<FL_row>(size);
        for(size_t i = 0; i < size; ++i)
        {
            FL_runs[i].load(in);
        }
    }
    
private:
    ulint n; // Length of BWT
    ulint r; // Runs of BWT

    vector<FL_row> FL_runs;
};

#endif /* end of include guard: _FL_TABLE_HH */