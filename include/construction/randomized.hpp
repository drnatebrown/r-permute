/* randomized - Randomly copy runs from L into F for a non-deterministic method similar to randomized fractional cascading
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
   \file randomized.hpp
   \brief randomized.hpp Randomly copy runs from L into F
   \author Nathaniel Brown
   \author Baorui Jia
   \date 18/03/2022
*/

#ifndef _RANDOMIZED_CONST_HH
#define _RANDOMIZED_CONST_HH

#include <common.hpp>
#include <constructor.hpp>

#include <../ds/FL_table.hpp>
#include <../ds/static_column.hpp>

#include <sdsl/structure_tree.hpp>
#include <sdsl/util.hpp>
#include <sdsl/int_vector.hpp>

#define SEED 23

using namespace std;
using namespace sdsl;

template < class bv_t = bit_vector >
class randomized : public constructor<bv_t>
{
private:
    ulint count;

    bit_vector P_prime;
    bit_vector Q_prime;
    vector<ulint> inserted_positions;

    void insert(ulint pos_Q) {
        ulint pos_P = this->find(pos_Q);
        inserted_positions.push_back(pos_P);

        if (!Q_prime[pos_Q])
        {
            Q_prime[pos_Q] = true;
            P_prime[pos_P] = true;
        }
        
        count++;
    }

public:
    randomized() : constructor<bv_t>() {}

    randomized(std::ifstream &heads, std::ifstream &lengths) : constructor<bv_t>(heads, lengths) {}

    randomized(constructor<bv_t>& c) : constructor<bv_t>(c) {}
    
    bv_t build(int ratio = 5) {
        double p = 1.0 / ratio;
        std::mt19937 gen(SEED);
        std::discrete_distribution<> choose({1-p, p});

        P_prime = bv_t(this->P.get_bv());
        Q_prime = bv_t(this->Q.get_bv());

        verbose("Runs before splitting: ", this->table.runs());
        verbose("Max scan before: ", get_max_scan());
        count = 0;

        //first run
        //check the 1-bit
        for(size_t i = 0; i < this->P.size(); ++i) {
            if(this->P[i]) {
                if(choose(gen))
                    insert(i);
            }
        }

        while (!inserted_positions.empty()) {
            //create a copy
            vector<ulint> last_inserted = inserted_positions;
            inserted_positions = vector<ulint>();

            //remaining runs, all ones
            for(size_t i = 0; i < last_inserted.size(); ++i) {
                if(choose(gen)) {
                    insert(last_inserted[i]);
                }
            }
        }

        verbose("Added rows: ", count);
        verbose("Runs after splitting: ", this->table.runs()+count);
        verbose("Max scan after: ", get_max_scan());

        return P_prime;
    }

    ulint get_max_scan()
    {
        ulint max_weight = 0;

        ulint run_weight = 0;
        ulint last_run_head = 0;
        // Initialize the priority queue of weights (number of set bits a run in Q covers in P)
        // Start at 1 because we know that Q begins with a set bit
        for (size_t i = 1; i < P_prime.size(); ++i)
        {
            // 1 denotes start of run, so push the results of prior run
            if(Q_prime[i])
            {
                run_weight = 0;
            }
            if (P_prime[i])
            {
                ++run_weight;
            }

            if (run_weight > max_weight) max_weight = run_weight;
        }

        return max_weight;
    }
};

#endif /* end of include guard: _RANDOMIZED_CONST_HH */