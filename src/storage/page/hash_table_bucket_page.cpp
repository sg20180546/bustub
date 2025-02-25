//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_table_bucket_page.cpp
//
// Identification: src/storage/page/hash_table_bucket_page.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "storage/page/hash_table_bucket_page.h"
#include "common/logger.h"
#include "common/util/hash_util.h"
#include "storage/index/generic_key.h"
#include "storage/index/hash_comparator.h"
#include "storage/table/tmp_tuple.h"

namespace bustub {

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BUCKET_TYPE::GetValue(KeyType key, KeyComparator cmp, std::vector<ValueType> *result) -> bool {
  bool ret = false;
  char *occupied = occupied_;
  int i = 0;
  int scope = 8;
  while ((*occupied) != 0 && occupied != readable_) {
    for (; i < scope; i++) {
      if (!cmp(key, array_[i].first)) {
        if (IsReadable(i)) {
          // std::cout<<key<<","<<array_[i].second<<" is on offset(getvalue)"<<i<<"\n";
          result->push_back(array_[i].second);
        }
      }
    }
    occupied++;
    scope += 8;
  }
  if (!result->empty()) {
    
    ret = true;
  }
  return ret;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BUCKET_TYPE::Insert(KeyType key, ValueType value, KeyComparator cmp) -> bool {
  std::vector<ValueType> res;
  int i;
  GetValue(key, cmp, &res);
  int size = res.size();
  for (i = 0; i < size; i++) {
    if (res[i] == value) {
      return false;
    }
  }
  char *readable = readable_;
  while (static_cast<unsigned char>(*readable) == 255) {
    readable++;
  }
  char temp = *readable;
  for (i = 0; i < 8; i++) {
    if (((1UL << i) & (~temp)) != 0U) {
      break;
    }
  }
  int index = (readable - readable_) * 8 + i;
  array_[index].first = key;
  array_[index].second = value;
  SetReadable(index);
  SetOccupied(index);

  return true;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BUCKET_TYPE::Remove(KeyType key, ValueType value, KeyComparator cmp) -> bool {
  // std::cout<<"remove "<<key<<" value"<<value<<"\n";
  bool ret = false;
  char *occupied = occupied_;
  int i = 0;
  int j = 8;
  // printf("here");
  while (static_cast<unsigned char>(*occupied) != 0U) {
    for (; i < j; i++) {
      // printf("%d  ",i);
      if (!cmp(key, array_[i].first)) {
        // printf("cmp pass %d ",i);
        if (array_[i].second == value) {
          // printf("value test %d ",i);
          if (IsReadable(i)) {
            // printf(" \n");
            // std::cout<<key<<","<<value<<" is on offset(remove)"<<i<<"\n";
            UnSetReadable(i);
            return true;
            // ret=true;
            // break;
          }
        }
      }
      // if ((!cmp(key, array_[i].first)) && array_[i].second == value && IsReadable(i)) {
      // }
    }
    occupied++;
    j += 8;
    // printf("occ %d i: %d \n\n",*o,i);
  }
  // printf("readable %u ",(unsigned char)(*readable));
  if(ret==false){
    // printf("cannot find , i : %d\n\n",i);
  }
  return ret;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BUCKET_TYPE::KeyAt(uint32_t bucket_idx) const -> KeyType {
  // printf("key at %d : ",bucket_idx);
  // std::cout<<array_[bucket_idx].first<<"\n";
  if (IsReadable(bucket_idx)) {
    return array_[bucket_idx].first;
  }
  return {};
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BUCKET_TYPE::ValueAt(uint32_t bucket_idx) const -> ValueType {
  if (IsReadable(bucket_idx)) {
    // std::cout<<"value at : "<< array_[bucket_idx].second<<"\n";
    return array_[bucket_idx].second;
  }

  return {};
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::RemoveAt(uint32_t bucket_idx) {
  UnSetReadable(bucket_idx);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BUCKET_TYPE::IsOccupied(uint32_t bucket_idx) const -> bool {
  return ((1UL << (bucket_idx & 7)) & occupied_[bucket_idx >> 3]);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::SetOccupied(uint32_t bucket_idx) {
  occupied_[bucket_idx >> 3] |= (1UL << (bucket_idx & 7));
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BUCKET_TYPE::IsReadable(uint32_t bucket_idx) const -> bool {
  // printf("idx : %u readlbe %d\n",bucket_idx,readable_[bucket_idx>>3]);
  return ((1UL << (bucket_idx & 7)) & readable_[bucket_idx >> 3]);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::SetReadable(uint32_t bucket_idx) {
  readable_[bucket_idx >> 3] |= (1UL << (bucket_idx & 7));
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BUCKET_TYPE::IsFull() -> bool {
  for (uint64_t i = 0; i < BUCKET_ARRAY_SIZE; i++) {
    if (!IsReadable(i)) {
      return false;
    }
  }
  return true;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BUCKET_TYPE::NumReadable() -> uint32_t {
  return 0;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BUCKET_TYPE::IsEmpty() -> bool {
  for (uint64_t i = 0; i < BUCKET_ARRAY_SIZE; i++) {
    if (IsReadable(i)) {
      return false;
    }
  }
  return true;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::PrintBucket() {
  uint32_t size = 0;
  uint32_t taken = 0;
  uint32_t free = 0;
  for (size_t bucket_idx = 0; bucket_idx < BUCKET_ARRAY_SIZE; bucket_idx++) {
    if (!IsOccupied(bucket_idx)) {
      break;
    }

    size++;

    if (IsReadable(bucket_idx)) {
      taken++;
    } else {
      free++;
    }
  }
  // printf("Bucket Capacity: %lu, Size: %u, Taken: %u, Free: %u\n\n", BUCKET_ARRAY_SIZE, size, taken, free);
  LOG_INFO("Bucket Capacity: %lu, Size: %u, Taken: %u, Free: %u", BUCKET_ARRAY_SIZE, size, taken, free);
}
template <typename KeyType, typename ValueType, typename KeyComparator>
inline void HASH_TABLE_BUCKET_TYPE::UnSetReadable(uint32_t bucket_idx) {
  readable_[bucket_idx >> 3] &= ~(1UL << (bucket_idx & 7));
}

auto HighestOneBit(char bitmap) -> int {
  bitmap |= (bitmap >> 1);
  bitmap |= (bitmap >> 2);
  bitmap |= (bitmap >> 4);
  bitmap |= (bitmap >> 8);
  bitmap |= (bitmap >> 16);
  return bitmap - (bitmap >> 1);
}
// DO NOT REMOVE ANYTHING BELOW THIS LINE
template class HashTableBucketPage<int, int, IntComparator>;

template class HashTableBucketPage<GenericKey<4>, RID, GenericComparator<4>>;
template class HashTableBucketPage<GenericKey<8>, RID, GenericComparator<8>>;
template class HashTableBucketPage<GenericKey<16>, RID, GenericComparator<16>>;
template class HashTableBucketPage<GenericKey<32>, RID, GenericComparator<32>>;
template class HashTableBucketPage<GenericKey<64>, RID, GenericComparator<64>>;

// template class HashTableBucketPage<hash_t, TmpTuple, HashComparator>;

}  // namespace bustub
