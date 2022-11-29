#ifndef __INDEX_PQ_HH
#define __INDEX_PQ_HH

#include <common.hpp>
#include <unordered_map>
#include <stdexcept>

#include <sdsl/structure_tree.hpp>
#include <sdsl/util.hpp>
#include <sdsl/int_vector.hpp>

class index_pq
{
public:
    index_pq() {}

    index_pq(size_t _limit, size_t idx_limit)
    {
        limit = _limit;
        n = 0;
        int log_idx = bitsize(uint64_t(idx_limit));

        pq = int_vector<>(limit, 0, log_idx);
        index_weights = unordered_map<ulint, pair<ulint, ulint>>();
    }

    bool is_empty() { 
        return n == 0;
    }

    bool contains(size_t i) {
        return index_weights.count(i);
    }

    size_t size() {
        return n;
    }

    void push(ulint i, ulint k)
    {
        if (contains(i)) throw std::invalid_argument("Index " + std::to_string(i) + " already in heap");
        if (n == limit) throw std::invalid_argument("Heap already at capacity");

        index_weights.insert({i, pair<ulint, ulint>(n, k)});
        pq[n] = i;

        swim(n++);
    }

    pair<ulint, ulint> get_max() {
        if (is_empty()) throw std::invalid_argument("Heap is empty");
        return make_pair(index_weights[pq[0]].second, pq[0]);
    }

    ulint get_weight(ulint i) {
        return index_weights[i].second;
    }

    void promote (ulint i, ulint k)
    {
        if (!contains(i)) throw std::invalid_argument("Index not in heap");
        if (index_weights[i].second >= k) throw std::invalid_argument("Given key is not greater than existing key");

        index_weights[i].second = k;
        swim(index_weights[i].first);
    }

    void demote (ulint i, ulint k)
    {
        if (!contains(i)) throw std::invalid_argument("Index not in heap");
        if (index_weights[i].second <= k) throw std::invalid_argument("Given key is not less than existing key");

        index_weights[i].second = k;
        sink(index_weights[i].first);
    }

    size_t serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name ="")
  {
      sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
      size_t written_bytes = 0;

      out.write((char *)&n, sizeof(n));
      written_bytes += sizeof(n);

      out.write((char *)&limit, sizeof(limit));
      written_bytes += sizeof(limit);

      written_bytes += pq.serialize(out, v, "pq");

      assert(n == index_weights.size());
      for (auto const& kv : index_weights)
      {
        out.write((char *)&kv.first, sizeof(kv.first)); // The index
        written_bytes += sizeof(kv.first);

        out.write((char *)&kv.second.first, sizeof(kv.second.first)); // The index in heap
        written_bytes += sizeof(kv.second.first);

        out.write((char *)&kv.second.second, sizeof(kv.second.second)); // The weight
        written_bytes += sizeof(kv.second.second);
      }

      return written_bytes;
  }

  void load(std::istream &in)
  {
      in.read((char *)&n, sizeof(n));
      in.read((char *)&limit, sizeof(limit));

      pq.load(in);
      
      index_weights = unordered_map<ulint, pair<ulint, ulint>>();
      for (size_t i = 0; i < n; ++i)
      {
        ulint index = 0, pq_pos = 0, weight = 0;

        in.read((char *)&index, sizeof(index));
        in.read((char *)&pq_pos, sizeof(pq_pos));
        in.read((char *)&weight, sizeof(weight));

        index_weights.insert({index, make_pair(pq_pos, weight)});
      }
  }


private:
    int_vector<> pq; // binary heap
    unordered_map<ulint, pair<ulint, ulint>> index_weights; // For an index, store position in pq and weight

    size_t n; // size of heap
    size_t limit; // max size of heap

    bool less (size_t i, size_t j)
    {
        return index_weights[pq[i]].second < index_weights[pq[j]].second;
    }

    void swap(size_t i, size_t j)
    {
        ulint temp = pq[i];
        pq[i] = pq[j];
        pq[j] = temp;

        index_weights[pq[i]].first = i;
        index_weights[pq[j]].first = j;
    }

    void swim(size_t k)
    {
        while (k > 0 && less((k - 1)/2, k))
        {
            swap(k, (k - 1)/2);
            k = (k - 1)/2;
        }
    }

    void sink(size_t k)
    {
        while (2*k + 1 < n)
        {
            size_t j = 2*k + 1;
            if (j + 1 < n && less(j, j + 1)) j++;
            if (!less(k, j)) break;

            swap(k, j);
            k = j;
        }
    }
};

#endif // __INDEX_PQ_HH