/* build_FL - Build the FL table
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
   \file build_FL.cpp
   \brief build_FL.cpp Tests building FL
   \author Nathaniel Brown
   \date 18/03/2022
*/

#include <iostream>

#define VERBOSE

#include <common.hpp>

#include <constructor.hpp>
#include <randomized.hpp>
#include <deterministic.hpp>
#include <static_column.hpp>
#include <FL_table.hpp>

#include <sdsl/io.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/sd_vector.hpp>
#include <malloc_count.h>

typedef bit_vector bv_t;

int main(int argc, char *const argv[])
{
  Args args;
  parseArgs(argc, argv, args);

  verbose("Building the Abstract Constructor (Representing L and F as bitvectors, supporting FL mapping)");
  std::chrono::high_resolution_clock::time_point t_insert_start = std::chrono::high_resolution_clock::now();

  std::string bwt_fname = args.filename + ".bwt";

  std::string bwt_heads_fname = bwt_fname + ".heads";
  std::ifstream ifs_heads(bwt_heads_fname);
  std::string bwt_len_fname = bwt_fname + ".len";
  std::ifstream ifs_len(bwt_len_fname);

  ifs_heads.seekg(0);
  ifs_len.seekg(0);
  constructor<bv_t> construct(ifs_heads, ifs_len);
  construct.stats();

  std::chrono::high_resolution_clock::time_point t_insert_end = std::chrono::high_resolution_clock::now();

  verbose("Construction Complete");
  verbose("Memory peak: ", malloc_count_peak());
  verbose("Elapsed time (s): ", std::chrono::duration<double, std::ratio<1>>(t_insert_end - t_insert_start).count());

  ulint d = 2;
  // verbose("Building Deterministic Construction, with bound d =", d);
  std::chrono::high_resolution_clock::time_point t_insert_mid = std::chrono::high_resolution_clock::now();

  deterministic<bv_t> deter = deterministic(construct);
  // bv_t final_deter = deter.build(d);

  // d = 4;
  // verbose("Building Deterministic Construction, with bound d =", d);
  // final_deter = deter.build(d);

  // d = 8;
  // verbose("Building Deterministic Construction, with bound d =", d);
  // final_deter = deter.build(d);

  d = 16;
  verbose("Building Deterministic Construction, with bound d =", d);
  bv_t final_deter = deter.build(d);

  t_insert_end = std::chrono::high_resolution_clock::now();
  verbose("Construction Complete");
  verbose("Memory peak: ", malloc_count_peak());
  verbose("Elapsed time (s): ", std::chrono::duration<double, std::ratio<1>>(t_insert_mid - t_insert_start).count());

  ulint ratio = 2;
  verbose("Building Randomized Construction, with split probability p =", "1/"+std::to_string(ratio));
  t_insert_mid = std::chrono::high_resolution_clock::now();

  randomized<bv_t> ran = randomized<bv_t>(construct);
  bv_t final_rand = ran.build(ratio);

  ratio = 4;
  verbose("Building Randomized Construction, with split probability p =", "1/"+std::to_string(ratio));
  final_rand = ran.build(ratio);

  ratio = 8;
  verbose("Building Randomized Construction, with split probability p =", "1/"+std::to_string(ratio));
  final_rand = ran.build(ratio);

  ratio = 16;
  verbose("Building Randomized Construction, with split probability p =", "1/"+std::to_string(ratio));
  final_rand = ran.build(ratio);

  t_insert_end = std::chrono::high_resolution_clock::now();
  verbose("Construction Complete");
  verbose("Memory peak: ", malloc_count_peak());
  verbose("Elapsed time (s): ", std::chrono::duration<double, std::ratio<1>>(t_insert_mid - t_insert_start).count());

  // verbose("Serializing");

  // std::string outfile_deter = args.filename + ".d_col";
  // std::ofstream out_d(outfile_deter);
  // final_deter.serialize(out_d);
  // out_d.close();

  // std::string outfile_rand = args.filename + ".r_col";
  // std::ofstream out_r(outfile_rand);
  // final_rand.serialize(out_r);
  // out_r.close();

  // t_insert_end = std::chrono::high_resolution_clock::now();

  verbose("Done");
  verbose("Memory peak: ", malloc_count_peak());
  verbose("Total Elapsed time (s): ", std::chrono::duration<double, std::ratio<1>>(t_insert_end - t_insert_start).count());

  return 0;
}