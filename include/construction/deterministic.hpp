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
#include <math.h>

#include <../ds/FL_table.hpp>
#include <../ds/static_column.hpp>
#include <../ds/index_pq.hpp>

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
    dynamic_bv_t init_P_prime;
    dynamic_bv_t init_Q_prime;
    index_pq init_weights;

    void initialize() 
    {
        ulint table_bound = ceil(this->table.runs()*1.5); // at worst, we add r/2 rows

        init_P_prime = dynamic_bv_t();
        init_Q_prime = dynamic_bv_t();
        init_weights = index_pq(table_bound, this->table.size()); // at worst, we had r/2 rows

        ulint total_weight = 0;
        ulint run_weight = 1;
        ulint last_run_head = 0;

        init_P_prime.push_back(this->P[0]);
        init_Q_prime.push_back(this->Q[0]);
        
        // Initialize the priority queue of weights (number of set bits a run in Q covers in P)
        // Start at 1 because we know that Q begins with a set bit
        for (size_t i = 1; i < this->P.size(); ++i)
        {
            init_P_prime.push_back(this->P[i]);
            init_Q_prime.push_back(this->Q[i]);

            // 1 denotes start of run, so push the results of prior run
            if(this->Q[i])
            {
                init_weights.push(last_run_head, run_weight);
                total_weight += run_weight;

                run_weight = 0;
                last_run_head =  i;
                //last_run++;
            }
            if (this->P[i])
            {
                ++run_weight;
            }
        }
        init_weights.push(last_run_head, run_weight);
        total_weight += run_weight;

        assert(total_weight == this->table.runs());

        #ifdef PRINT_STATS
        auto[max_weight, _] = init_weights.get_max();
        cout << "Scan max: " << max_weight << std::endl;
        #endif
    }

public:
    deterministic() : constructor<static_bv_t>() {}

    deterministic(std::ifstream &heads, std::ifstream &lengths) : constructor<static_bv_t>(heads, lengths) 
    {
        initialize();
    }

    // Copy from constructor
    deterministic(constructor<static_bv_t>& c) : constructor<static_bv_t>(c) 
    {
        initialize();
    }

    static_bv_t build(ulint d = 2)
    {
        dynamic_bv_t P_prime = init_P_prime;
        dynamic_bv_t Q_prime = init_Q_prime;
        index_pq weights = init_weights;
        //weights.extend(this->table.runs()*(1+ceil(1.0/(d-1))));

        ulint count = 0;
        auto[max_weight, max_index] = init_weights.get_max();

        while (max_weight >= 2*d)
        {
            count++;
            // Find where to set bit (split run)
            ulint first_P_run = P_prime.rank(max_index); // Find first run in P which is covered by Q interval
            ulint Q_insert_position = P_prime.select(first_P_run + d); // Get the position of run bit d positions from first run
            
            // Set bits
            Q_prime[Q_insert_position] = true;
            ulint P_insert_position = this->find(Q_insert_position); // Find that corresponding bit in P
            P_prime[P_insert_position] = true;

            // Update PQ for split run in Q
            weights.demote(max_index, d); // Weight d run of original
            weights.push(Q_insert_position, max_weight - d); // Rest of original (starting at newly set bit)

            // Update PQ for added bit in P
            ulint Q_pred_run = Q_prime.rank(P_insert_position + 1) - 1; // Get run
            ulint Q_pred_idx = Q_prime.select(Q_pred_run); // Get index of set bit
            weights.promote(Q_pred_idx, weights.get_weight(Q_pred_idx) + 1); // Increment the weight of that run by 1

            // Take next run of maximum weight
            std::pair<ulint, ulint> max_pair = weights.get_max();
            max_weight = max_pair.first;
            max_index = max_pair.second;
        }

        verbose("Added rows: ", count);
        verbose("Runs after splitting: ", this->table.runs()+count);
        verbose("Max scan after: ", max_weight);

        #ifdef PRINT_STATS
        cout << "Runs added: " << count << std::endl;
        cout << "Scan after: " << max_weight << std::endl;
        #endif

        /* DEBUG CHECK */
        // ulint max_w = 0;
        // ulint run_weight = 0;
        // ulint last_run_head = 0;
        // for (size_t i = 1; i < P_prime.size(); ++i)
        // {
        //     // 1 denotes start of run, so push the results of prior run
        //     if(Q_prime[i])
        //     {
        //         run_weight = 0;
        //     }
        //     if (P_prime[i])
        //     {
        //         ++run_weight;
        //     }
        //     if (run_weight > max_w) max_w = run_weight;
        // }
        //verbose("REAL MAX WEIGHT: ", max_w);

        static_bv_t ret = static_bv_t(P_prime.size());
        for (size_t i = 0; i < P_prime.rank(P_prime.size()); ++i)
        {
            ret[P_prime.select(i+1)] = true;
        }

        return ret;
    }

    void stats() {
        constructor<static_bv_t>::stats();

        sdsl::nullstream ns;

        verbose("              P_prime:     ", init_P_prime.serialize(ns));
        verbose("              Q_prime:     ", init_Q_prime.serialize(ns));
        verbose("              Weight-Heap: ", init_weights.serialize(ns));
        verbose("");

        auto[max_weight, max_index] = init_weights.get_max();

        verbose("Runs before splitting: ", this->table.runs());
        verbose("Max scan before:       ", max_weight);
    }

    std::string get_file_extension() const
    {
        return ".d_construct";
    }

    size_t serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name ="")
    {
        sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));

        size_t written_bytes = constructor<static_bv_t>::serialize(out, v, name);

        written_bytes += init_P_prime.serialize(out);
        written_bytes += init_Q_prime.serialize(out);
        written_bytes += init_weights.serialize(out, v, "init_weights");

        return written_bytes;
    }

    void load(std::istream &in)
    {
        constructor<static_bv_t>::load(in);

        init_P_prime.load(in);
        init_Q_prime.load(in);
        init_weights.load(in);
    }
};

#endif /* end of include guard: _DETERMINISTIC_CONST_HH */