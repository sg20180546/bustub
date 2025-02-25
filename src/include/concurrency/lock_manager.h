//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lock_manager.h
//
// Identification: src/include/concurrency/lock_manager.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <condition_variable>  // NOLINT
#include <list>
#include <memory>
#include <mutex>  // NOLINT
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/config.h"
#include "common/rid.h"
#include "concurrency/transaction.h"

namespace bustub {

class TransactionManager;

/**
 * LockManager handles transactions asking for locks on records.
 */
class LockManager {
  enum class LockMode { SHARED, EXCLUSIVE };

  class LockRequest {
   public:
    LockRequest(txn_id_t txn_id, LockMode lock_mode, bool granted=false) : txn_id_(txn_id), lock_mode_(lock_mode), granted_(granted) {}
    LockRequest() {}
    txn_id_t txn_id_;
    LockMode lock_mode_;
    bool granted_;
    void operator=(LockRequest lq){
      txn_id_=lq.txn_id_;
      lock_mode_=lq.lock_mode_;
      granted_=lq.granted_;
    }
  };

  class LockRequestQueue {
   public:
    std::vector<LockRequest> request_queue_;
    // for notifying blocked transactions on this rid
    // std::mutex m_;
    std::condition_variable cv_;
    // txn_id of an upgrading transaction (if any)
    txn_id_t upgrading_ = INVALID_TXN_ID;
    // LockRequestQueue& operator=(LockRequestQueue& q){
    //   request_queue_=q.request_queue_;
    //   cv_=q.cv_;
    //   upgrading_=q.upgrading_;
    // }
  };

 public:
  /**
   * Creates a new lock manager configured for the deadlock prevention policy.
   */
  LockManager() = default;

  ~LockManager() = default;

  /*
   * [LOCK_NOTE]: For all locking functions, we:
   * 1. return false if the transaction is aborted; and
   * 2. block on wait, return true when the lock request is granted; and
   * 3. it is undefined behavior to try locking an already locked RID in the
   * same transaction, i.e. the transaction is responsible for keeping track of
   * its current locks.
   */

  /**
   * Acquire a lock on RID in shared mode. See [LOCK_NOTE] in header file.
   * @param txn the transaction requesting the shared lock
   * @param rid the RID to be locked in shared mode
   * @return true if the lock is granted, false otherwise
   */
  auto LockShared(Transaction *txn, const RID &rid) -> bool;

  /**
   * Acquire a lock on RID in exclusive mode. See [LOCK_NOTE] in header file.
   * @param txn the transaction requesting the exclusive lock
   * @param rid the RID to be locked in exclusive mode
   * @return true if the lock is granted, false otherwise
   */
  auto LockExclusive(Transaction *txn, const RID &rid) -> bool;

  /**
   * Upgrade a lock from a shared lock to an exclusive lock.
   * @param txn the transaction requesting the lock upgrade
   * @param rid the RID that should already be locked in shared mode by the
   * requesting transaction
   * @return true if the upgrade is successful, false otherwise
   */
  auto LockUpgrade(Transaction *txn, const RID &rid) -> bool;

  /**
   * Release the lock held by the transaction.
   * @param txn the transaction releasing the lock, it should actually hold the
   * lock
   * @param rid the RID that is locked by the transaction
   * @return true if the unlock is successful, false otherwise
   */
  auto Unlock(Transaction *txn, const RID &rid) -> bool;

  inline auto ValidateTxnBeforeLock(Transaction* txn)->bool {
      TransactionState tstate=txn->GetState();
      if(tstate==TransactionState::ABORTED){
        return false;
      }
      if(tstate==TransactionState::SHRINKING) {
        txn->SetState(TransactionState::ABORTED);
        throw TransactionAbortException(txn->GetTransactionId(),AbortReason::LOCK_ON_SHRINKING);
      }
      return true;
  }
  void IsItAborted(Transaction* txn,LockManager::LockRequestQueue* lq){
    auto tid=txn->GetTransactionId();
    if(txn->GetState()==TransactionState::ABORTED){
      for(auto iter=lq->request_queue_.begin();iter!=lq->request_queue_.end();++iter){
        if(iter->txn_id_==tid){
          lq->request_queue_.erase(iter);
        }
      }
      
      throw TransactionAbortException(txn->GetTransactionId(),AbortReason::DEADLOCK); // Aborted From Wound-Wait DeadLock Preventing
    }
  }
 private:
  std::mutex latch_;

  /** Lock table for lock requests. */
  std::unordered_map<RID, LockRequestQueue> lock_table_;

  std::unordered_map<txn_id_t,Transaction*> txn_table_;
};

}  // namespace bustub
