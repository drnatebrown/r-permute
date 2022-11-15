/* deterministic - Uses constructor.hpp to implement deterministic run splitting based on Nishimoto and Tabei's approach (ICALP '21)
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
   \file deterministic.hpp
   \brief deterministic.hpp Split runs in L and F using constructor to limit LF stepping cost
   \author Nathaniel Brown
   \date 18/03/2022
*/

#ifndef _DETERMINISTIC_CONST_HH
#define _DETERMINISTIC_CONST_HH

#include <common.hpp>

#include <../ds/FL_table.hpp>
#include <../ds/static_column.hpp>
#include <../ds/indexMaxPQ.hpp>

#include <sdsl/structure_tree.hpp>
#include <sdsl/util.hpp>
#include <sdsl/int_vector.hpp>

#include <../../thirdparty/dynamic/dynamic.hpp>

using namespace std;
using namespace sdsl;
using namespace dyn;

template < class static_bv_t = bit_vector,
           class dynamic_bv_t = suc_bv >
class deterministic : public constructor<static_bv_t>
{
private:
    dynamic_bv_t P_prime;
    dynamic_bv_t Q_prime;

public:
    deterministic() : constructor<static_bv_t>() {}

    deterministic(std::ifstream &heads, std::ifstream &lengths) : constructor<static_bv_t>(heads, lengths) {}

    // Copy from constructor
    deterministic(const constructor<static_bv_t>& c) : constructor<static_bv_t>(c) {}

    static_bv_t build(ulint d = 2)
    {
        P_prime = dynamic_bv_t();
        Q_prime = dynamic_bv_t();

        indexMaxPQ weights = indexMaxPQ();
        weights.init(this->P->size()*(1+1.0/(d-1))+1);

        ulint total_weight = 0;
        ulint run_weight = 0;
        ulint last_run_head = 0;
        // Initialize the priority queue of weights (number of set bits a run in Q covers in P)
        // Start at 1 because we know that Q begins with a set bit
        for (size_t i = 1; i < this->P->size(); ++i)
        {
            P_prime.push_back((*this->P)[i]);
            Q_prime.push_back((*this->Q)[i]);

            // 1 denotes start of run, so push the results of prior run
            if((*this->Q)[i])
            {
                weights.push(last_run_head, run_weight);
                total_weight += run_weight;

                run_weight = 0;
                last_run_head =  i;
            }
            if ((*this->P)[i])
            {
                ++run_weight;
            }
        }
        weights.push(last_run_head, run_weight);

        auto[max_weight, max_index] = weights.get_max();

        verbose("Runs before splitting: ", this->table.runs());
        verbose("Max scan before: ", max_weight);

        ulint count = 0;

        while (max_weight >= 2*d)
        {
            count++;
            // Find where to set bit (split run)
            ulint first_P_run = P_prime.rank(max_index) + 1; // Find first run in Q interval over P
            ulint Q_insert_position = P_prime.select(first_P_run + d); // Get the position of run bit d positions from first run
            
            // Set bits
            Q_prime[Q_insert_position] = 1;
            ulint P_insert_position = this->find(Q_insert_position); // Find that corresponding bit in P
            P_prime[P_insert_position] = 1;

            // Update PQ for split run in Q
            weights.demote(max_index, d); // Weight d run of original
            weights.push(Q_insert_position, max_weight - d); // Rest of original (starting at newly set bit)

            // Update PQ for added bit in P
            ulint Q_pred_run = Q_prime.rank(P_insert_position + 1) - 1; // Get run
            ulint Q_pred_idx = Q_prime.select(Q_pred_run + 1); // Get index of set bit
            weights.promote(Q_pred_idx, weights.get_key(Q_pred_idx) + 1); // Increment the weight of that run by 1

            // Take next run of maximum weight
            std::pair<ulint, ulint> max_pair = weights.get_max();
            max_weight = max_pair.first;
            max_index = max_pair.second;
        }

        verbose("Added rows: ", count);
        verbose("Runs after splitting: ", this->table.runs()+count);
        verbose("Max scan after: ", max_weight);

        static_bv_t ret = static_bv_t(P_prime.size());
        for (size_t i = 0; i < P_prime.size(); ++i)
        {
            ret[i] = P_prime[i];
        }

        return ret;
    }
};

#endif /* end of include guard: _DETERMINISTIC_CONST_HH */