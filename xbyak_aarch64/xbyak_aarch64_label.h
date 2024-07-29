/*******************************************************************************
 * Copyright 2019-2023 FUJITSU LIMITED
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/
#pragma once

#ifndef _XBYAK_AARCH64_LABEL_
#define _XBYAK_AARCH64_LABEL_

#include "xbyak_aarch64_code_array.h"
#include "xbyak_aarch64_inner.h"

struct JmpLabel {
  // Support jump distances
  enum MaxDistance { J32KB, J1MB, J128MB };
  // type of partially applied function for encoding
  typedef std::function<uint32_t(int64_t)> EncFunc;
  size_t endOfJmp; /* offset from top to the end address of jmp */
  MaxDistance maxDistance;
  EncFunc encFunc;
  // Double linked list of pending jumps with the same max distance
  JmpLabel *prevInClass = nullptr, *nextInClass = nullptr;
  explicit JmpLabel(const EncFunc &encFunc, size_t endOfJmp, MaxDistance maxDistance) : endOfJmp(endOfJmp), maxDistance(maxDistance), encFunc(encFunc) {}

  // Get the maximum gap (in instructions) supported by this jump
  int64_t getMaximumGapInInstructions() const { return (maxDistance == MaxDistance::J1MB) ? (1 << 20) / 4 : (maxDistance == MaxDistance::J32KB) ? (1 << 15) / 4 : (1 << 27) / 4; }
  // Get the deadline position
  size_t getDeadline() const { return endOfJmp + getMaximumGapInInstructions() - 1; }
  // Is a target reachable by this jump type?
  bool isInRange(int64_t offset) const {
    int64_t maxGap = getMaximumGapInInstructions() * 4;
    return (offset >= (-maxGap)) && (offset < maxGap);
  }
};

class LabelManager;

class Label {
  mutable LabelManager *mgr;
  mutable int id;
  friend class LabelManager;

public:
  Label() : mgr(nullptr), id(0) {}
  Label(const Label &rhs);
  Label &operator=(const Label &rhs);
  ~Label();
  void clear() {
    mgr = nullptr;
    id = 0;
  }
  int getId() const { return id; }
  const uint32_t *getAddress() const;
};

class LabelManager {

  // for Label class
  struct ClabelVal {
    ClabelVal(size_t offset = 0) : offset(offset), refCount(1) {}
    size_t offset;
    int refCount;
  };
  // For maintaining the queue of undefined labels
  struct LabelUndefQueue {
    JmpLabel *first = nullptr, *last = nullptr;
  };
  typedef std::unordered_map<int, ClabelVal> ClabelDefList;
  typedef std::unordered_multimap<int, JmpLabel> ClabelUndefList;
  typedef std::unordered_set<Label *> LabelPtrList;

  // No deadline
  static constexpr size_t noDeadline = std::numeric_limits<size_t>::max();

  CodeArray *base_;
  // global : stateList_.front(), local : stateList_.back()
  mutable int labelId_;
  ClabelDefList clabelDefList_;
  ClabelUndefList clabelUndefList_;
  LabelPtrList labelPtrList_;
  std::vector<std::pair<size_t, JmpLabel>> outOfReachList_;
  LabelUndefQueue labelUndefQueue_[3];
  size_t flushDeadlineOutOfReach_ = noDeadline;
  size_t flushDeadline_ = noDeadline;

  int getId(const Label &label) const {
    if (label.id == 0)
      label.id = labelId_++;
    return label.id;
  }
  template <class DefList, class UndefList, class T> void define_inner(DefList &defList, UndefList &undefList, const T &labelId, size_t addrOffset) {
    // add label
    typename DefList::value_type item(labelId, addrOffset);
    std::pair<typename DefList::iterator, bool> ret = defList.insert(item);
    if (!ret.second)
      throw Error(ERR_LABEL_IS_REDEFINED);
    // search undefined label
    for (;;) {
      typename UndefList::iterator itr = undefList.find(labelId);
      if (itr == undefList.end())
        break;
      const JmpLabel *jmp = &itr->second;
      const size_t offset = jmp->endOfJmp;
      int64_t labelOffset = (addrOffset - offset) * CSIZE;
      // Remove jump from pending queue
      if (jmp->prevInClass) {
        jmp->prevInClass->nextInClass = jmp->nextInClass;
      } else {
        labelUndefQueue_[unsigned(jmp->maxDistance)].first = jmp->nextInClass;
        flushDeadline_ = flushDeadlineOutOfReach_;
        for (unsigned index = 0; index != 3; ++index)
          if (labelUndefQueue_[index].first)
            flushDeadline_ = std::min<std::size_t>(flushDeadline_, labelUndefQueue_[index].first->getDeadline());
      }
      if (jmp->nextInClass) {
        jmp->nextInClass->prevInClass = jmp->prevInClass;
      } else {
        labelUndefQueue_[unsigned(jmp->maxDistance)].last = jmp->prevInClass;
      }
      // Update jump
      if (jmp->isInRange(labelOffset)) {
        uint32_t disp = jmp->encFunc(labelOffset);
        base_->rewrite(offset, disp);
      } else {
        flushDeadlineOutOfReach_ = std::min<std::size_t>(flushDeadlineOutOfReach_, offset + jmp->getMaximumGapInInstructions());
        flushDeadline_ = std::min<std::size_t>(flushDeadline_, flushDeadlineOutOfReach_);
        outOfReachList_.push_back({addrOffset, *jmp});
      }
      undefList.erase(itr);
    }
  }
  template <class DefList, class T> bool getOffset_inner(const DefList &defList, size_t *offset, const T &label) const {
    typename DefList::const_iterator i = defList.find(label);
    if (i == defList.end())
      return false;
    *offset = i->second.offset;
    return true;
  }
  friend class Label;
  void incRefCount(int id, Label *label) {
    clabelDefList_[id].refCount++;
    labelPtrList_.insert(label);
  }
  void decRefCount(int id, Label *label) {
    labelPtrList_.erase(label);
    ClabelDefList::iterator i = clabelDefList_.find(id);
    if (i == clabelDefList_.end())
      return;
    if (i->second.refCount == 1) {
      clabelDefList_.erase(id);
    } else {
      --i->second.refCount;
    }
  }
  template <class T> bool hasUndefinedLabel_inner(const T &list) const {
#ifndef NDEBUG
    for (typename T::const_iterator i = list.begin(); i != list.end(); ++i) {
      std::cerr << "undefined label:" << i->first << std::endl;
    }
#endif
    return !list.empty();
  }
  // detach all labels linked to LabelManager
  void resetLabelPtrList() {
    for (LabelPtrList::iterator i = labelPtrList_.begin(), ie = labelPtrList_.end(); i != ie; ++i) {
      (*i)->clear();
    }
    labelPtrList_.clear();
  }

public:
  LabelManager() { reset(); }
  ~LabelManager() { resetLabelPtrList(); }
  void reset() {
    base_ = 0;
    labelId_ = 1;
    clabelDefList_.clear();
    clabelUndefList_.clear();
    resetLabelPtrList();
    outOfReachList_.clear();
    for (unsigned index = 0; index != 3; ++index)
      labelUndefQueue_[index].first = labelUndefQueue_[index].last = nullptr;
    flushDeadline_ = flushDeadlineOutOfReach_ = noDeadline;
  }

  void set(CodeArray *base) { base_ = base; }

  void defineClabel(Label &label) {
    define_inner(clabelDefList_, clabelUndefList_, getId(label), base_->size_);
    label.mgr = this;
    labelPtrList_.insert(&label);
  }
  void assign(Label &dst, const Label &src) {
    ClabelDefList::const_iterator i = clabelDefList_.find(src.id);
    if (i == clabelDefList_.end())
      throw Error(ERR_LABEL_ISNOT_SET_BY_L);
    define_inner(clabelDefList_, clabelUndefList_, dst.id, i->second.offset);
    dst.mgr = this;
    labelPtrList_.insert(&dst);
  }
  bool getOffset(size_t *offset, const Label &label) const { return getOffset_inner(clabelDefList_, offset, getId(label)); }
  void addUndefinedLabel(const Label &label, const JmpLabel &jmp) {
    auto iter = clabelUndefList_.insert(ClabelUndefList::value_type(label.id, jmp));

    // Update queue of unplaced labels
    auto &j = iter->second;
    unsigned gapClass = unsigned(j.maxDistance);
    if (labelUndefQueue_[gapClass].first) {
      j.prevInClass = labelUndefQueue_[gapClass].last;
      j.prevInClass->nextInClass = &j;
    } else {
      j.prevInClass = nullptr;
      labelUndefQueue_[gapClass].first = &j;
      flushDeadline_ = flushDeadlineOutOfReach_;
      for (unsigned index = 0; index != 3; ++index)
        if (labelUndefQueue_[index].first)
          flushDeadline_ = std::min<std::size_t>(flushDeadline_, labelUndefQueue_[index].first->getDeadline());
    }
    j.nextInClass = nullptr;
    labelUndefQueue_[gapClass].last = &j;
  }
  void addOutOfReachLabel(const Label &label, const JmpLabel &jmp) {
    flushDeadlineOutOfReach_ = std::min<std::size_t>(flushDeadlineOutOfReach_, jmp.getDeadline());
    flushDeadline_ = std::min<std::size_t>(flushDeadline_, flushDeadlineOutOfReach_);
    outOfReachList_.push_back({clabelDefList_.find(label.id)->second.offset, jmp});
  }
  bool hasUndefClabel() const { return hasUndefinedLabel_inner(clabelUndefList_); }
  const uint8_t *getCode() const { return base_->getCode(); }
  bool isReady() const { return !base_->isAutoGrow() || base_->isCalledCalcJmpAddress(); }

  // Do we have to flush jump thunks?
  bool needsFlush() const { return base_->size_ + 4 + outOfReachList_.size() >= flushDeadline_; }
  // Flush jump thunks as needed
  void flushJumpThunks(bool afterUnconditionalBr);
};

inline Label::Label(const Label &rhs) {
  id = rhs.id;
  mgr = rhs.mgr;
  if (mgr)
    mgr->incRefCount(id, this);
}
inline Label &Label::operator=(const Label &rhs) {
  if (id)
    throw Error(ERR_LABEL_IS_ALREADY_SET_BY_L);
  id = rhs.id;
  mgr = rhs.mgr;
  if (mgr)
    mgr->incRefCount(id, this);
  return *this;
}
inline Label::~Label() {
  if (id && mgr)
    mgr->decRefCount(id, this);
}
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif
inline const uint32_t *Label::getAddress() const {
  if (mgr == 0 || !mgr->isReady())
    return 0;
  size_t offset;
  if (!mgr->getOffset(&offset, *this))
    return 0;
  // getCode() is always a multiple of 4
  return (const uint32_t *)mgr->getCode() + offset;
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif
