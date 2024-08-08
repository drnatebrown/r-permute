# r-permute
<!--- ```console


                                           _       
 _ __      _ __   ___ _ __ _ __ ___  _   _| |_ ___ 
| '__|____| '_ \ / _ \ '__| '_ ` _ \| | | | __/ _ \
| | |_____| |_) |  __/ |  | | | | | | |_| | ||  __/
|_|       | .__/ \___|_|  |_| |_| |_|\__,_|\__\___|
          |_|                                      

  
```
-->

**Stores permutation $\pi$ in $O(r)$-space lookup table where $r$ is the number of "runs" which are permuted consecutively (# of positions where $i=0$ or $\pi[i-1] \neq \pi[i] -1]$ ). When the run of a position is known (i.e., when applying successive permutations), each permutation is supported in $O(d)=O(1)$-time in the worst case where $d$ is a parameter controlling how often table entries are "split" to prevent worst case (without splitting, worst case $\Omega(r)$-time, but faster in practice) [1].**

---

## How-To
### Download and Compile

```console
git clone https://github.com/drnatebrown/r-permute.git
cd r-index-f

mkdir build && cd build
cmake ..
make
```

### Build
Build constructor once, and then split for various parameters of $d$.
Guarantees $\leq$ $2d$ operations to compute a permutation from a table
representation, while inserting at most $\frac{r}{d-1}$ additional runs.

Output is an SDSL bit_vector at <FASTA>.d_col
```console
./test/src/build_constructor <FASTA>
./test/src/run_constructor <FASTA> -d <SPLIT_PARAM>
```

# External Dependencies

* [Big-BWT](https://github.com/alshai/Big-BWT.git)
    * [gSACA-K](https://github.com/felipelouza/gsa-is.git)
    * [malloc_count](https://github.com/bingmann/malloc_count)
* [sdsl-lite](https://github.com/simongog/sdsl-lite)
    * [divufsort](https://github.com/simongog/libdivsufsort) 
* [DYNAMIC](https://github.com/xxsds/DYNAMIC)

# Authors

### Implementation:

* [Nathaniel Brown](https://github.com/drnatebrown)
* Baorui Jia

### Theory
* Nathaniel Brown
* [Massimiliano Rossi](https://github.com/maxrossi91)
* Travis Gagie

# References

[1] Brown, N.K., Gagie, T., & Rossi, M. (2022). RLBWT Tricks. arXiv preprint arXiv:2112.04271.  
[2] Nishimoto, T., & Tabei, Y. (2020). Optimal-Time Queries on BWT-runs Compressed Indexes. arXiv preprint arXiv:2006.05104.
