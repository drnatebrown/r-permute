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

  ulint d = args.d;

  verbose("Loading Deterministic Constructor");
  std::chrono::high_resolution_clock::time_point t_insert_start = std::chrono::high_resolution_clock::now();

  deterministic<bv_t> deter;
  std::string deter_fname = args.filename + deter.get_file_extension();
  
  ifstream deter_in(deter_fname);
  deter.load(deter_in);
  deter_in.close();

  std::chrono::high_resolution_clock::time_point t_insert_mid = std::chrono::high_resolution_clock::now();

  verbose("Load Complete");
  verbose("Memory peak: ", malloc_count_peak());
  verbose("Elapsed time (s): ", std::chrono::duration<double, std::ratio<1>>(t_insert_mid - t_insert_start).count());

  //deter.stats();

  verbose("Building Deterministic Splitting, with bound d =", d);
  t_insert_mid = std::chrono::high_resolution_clock::now();

  bv_t final_col = deter.build(d);

  std::chrono::high_resolution_clock::time_point t_insert_end = std::chrono::high_resolution_clock::now();
  verbose("Splitting Complete");
  verbose("Memory peak: ", malloc_count_peak());
  verbose("Elapsed time (s): ", std::chrono::duration<double, std::ratio<1>>(t_insert_end - t_insert_mid).count());

  #ifdef PRINT_STATS
  cout << "Time run: " << std::chrono::duration<double, std::ratio<1>>(t_insert_end - t_insert_mid).count() << std::endl;
  #endif

  verbose("Serializing");

  t_insert_mid = std::chrono::high_resolution_clock::now();

  std::string outfile_deter = args.filename + ".d_col";
  std::ofstream out_d(outfile_deter);
  final_col.serialize(out_d);
  out_d.close();

  t_insert_end = std::chrono::high_resolution_clock::now();

  verbose("Serializing Complete");
  verbose("Memory peak: ", malloc_count_peak());
  verbose("Elapsed time (s): ", std::chrono::duration<double, std::ratio<1>>(t_insert_end - t_insert_mid).count());

  verbose("Done");
  verbose("Memory peak: ", malloc_count_peak());
  verbose("Total Elapsed time (s): ", std::chrono::duration<double, std::ratio<1>>(t_insert_end - t_insert_start).count());

  return 0;
}