/*******************************************************************************
 * Copyright 2019-2020 FUJITSU LIMITED
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
template <typename T, typename std::enable_if<std::is_unsigned<T>::value,
                                              std::nullptr_t>::type = nullptr>
void add_imm(const XReg &dst, const XReg &src, T imm, const XReg &tmp,
             const XReg &tmp1) {

  /* This add_imm function allows dst == src,
     but tmp and tmp1 must be different from src */
  assert(src.getIdx() != tmp.getIdx());
  assert(dst.getIdx() != tmp.getIdx());
  assert(tmp1.getIdx() != tmp.getIdx());

  uint64_t bit_ptn = static_cast<uint64_t>(imm);
  uint64_t mask = 0xFFFF;
  bool flag = false;

  /* ADD(immediate) supports unsigned imm12 */
  const uint64_t IMM12_MASK = ~uint64_t(0xfff);
  if ((bit_ptn & IMM12_MASK) == 0) { // <= 4095
    add(dst, src, static_cast<uint32_t>(imm & 0xfff));
    return;
  }

  /* MOVZ allows shift amount = 0, 16, 32, 48 */
  for (int i = 0; i < 64; i += 16) {
    uint64_t tmp_ptn = (bit_ptn & (mask << i)) >> i;
    if (tmp_ptn) {
      if (!flag) {
        movz(tmp1, static_cast<uint32_t>(tmp_ptn), i);
        flag = true;
      } else {
        movz(tmp, static_cast<uint32_t>(tmp_ptn), i);
        add(tmp1, tmp1, tmp);
      }
    }
  }

  add(dst, src, tmp1);

  return;
}

template <typename T, typename std::enable_if<std::is_signed<T>::value,
                                              std::nullptr_t>::type = nullptr>
void add_imm(const XReg &dst, const XReg &src, T imm, const XReg &tmp,
             const XReg &tmp1) {

  /* This add_imm function allows dst == src,
     but tmp and tmp1 must be different from src */
  assert(src.getIdx() != tmp.getIdx());
  assert(dst.getIdx() != tmp.getIdx());
  assert(tmp1.getIdx() != tmp.getIdx());

  /* Sign bit must be extended. */
  int64_t bit_ptn = static_cast<int64_t>(imm);
  uint64_t mask = 0xFFFF;
  bool flag = false;

  /* ADD(immediate) supports unsigned imm12 */
  if (imm >= 0) {
    const uint64_t IMM12_MASK = ~uint64_t(0xfff);
    if ((bit_ptn & IMM12_MASK) == 0) { // <= 4095
      add(dst, src, static_cast<uint32_t>(imm & 0xfff));
      return;
    }
  }

  /* MOVZ allows shift amount = 0, 16, 32, 48 */
  for (int i = 0; i < 64; i += 16) {
    uint64_t tmp_ptn = (bit_ptn & (mask << i)) >> i;
    if (tmp_ptn) {
      if (!flag) {
        movz(tmp1, static_cast<uint32_t>(tmp_ptn), i);
        flag = true;
      } else {
        movz(tmp, static_cast<uint32_t>(tmp_ptn), i);
        add(tmp1, tmp1, tmp);
      }
    }
  }

  add(dst, src, tmp1);

  return;
}

template <typename T, typename std::enable_if<std::is_unsigned<T>::value,
                                              std::nullptr_t>::type = nullptr>
void add_imm(const WReg &dst, const WReg &src, T imm, const WReg &tmp,
             const WReg &tmp1) {

  if (sizeof(T) > 4) {
    throw Error(ERR_ILLEGAL_TYPE, genErrMsg());
  }

  /* This add_imm function allows dst == src,
     but tmp and tmp1 must be different from src */
  assert(src.getIdx() != tmp.getIdx());
  assert(dst.getIdx() != tmp.getIdx());
  assert(tmp1.getIdx() != tmp.getIdx());

  uint32_t bit_ptn = static_cast<uint32_t>(imm);
  uint32_t mask = 0xFFFF;
  bool flag = false;

  /* ADD(immediate) supports unsigned imm12 */
  const uint32_t IMM12_MASK = ~uint32_t(0xfff);
  if ((bit_ptn & IMM12_MASK) == 0) { // <= 4095
    add(dst, src, static_cast<uint32_t>(imm & 0xfff));
    return;
  }

  /* MOVZ allows shift amount = 0, 16, 32, 48 */
  for (int i = 0; i < 32; i += 16) {
    uint64_t tmp_ptn = (bit_ptn & (mask << i)) >> i;
    if (tmp_ptn) {
      if (!flag) {
        movz(tmp1, static_cast<uint32_t>(tmp_ptn), i);
        flag = true;
      } else {
        movz(tmp, static_cast<uint32_t>(tmp_ptn), i);
        add(tmp1, tmp1, tmp);
      }
    }
  }

  add(dst, src, tmp1);

  return;
}

template <typename T, typename std::enable_if<std::is_signed<T>::value,
                                              std::nullptr_t>::type = nullptr>
void add_imm(const WReg &dst, const WReg &src, T imm, const WReg &tmp,
             const WReg &tmp1) {

  if (sizeof(T) > 4) {
    throw Error(ERR_ILLEGAL_TYPE, genErrMsg());
  }

  /* This add_imm function allows dst == src,
     but tmp and tmp1 must be different from src */
  assert(src.getIdx() != tmp.getIdx());
  assert(dst.getIdx() != tmp.getIdx());
  assert(tmp1.getIdx() != tmp.getIdx());

  /* Sign bit must be extended. */
  int32_t bit_ptn = static_cast<int32_t>(imm);
  uint32_t mask = 0xFFFF;
  bool flag = false;

  /* ADD(immediate) supports unsigned imm12 */
  if (imm >= 0) {
    const uint32_t IMM12_MASK = ~uint32_t(0xfff);
    if ((bit_ptn & IMM12_MASK) == 0) { // <= 4095
      add(dst, src, static_cast<uint32_t>(imm & 0xfff));
      return;
    }
  }

  /* MOVZ allows shift amount = 0, 16, 32, 48 */
  for (int i = 0; i < 32; i += 16) {
    uint32_t tmp_ptn = (bit_ptn & (mask << i)) >> i;
    if (tmp_ptn) {
      if (!flag) {
        movz(tmp1, static_cast<uint32_t>(tmp_ptn), i);
        flag = true;
      } else {
        movz(tmp, static_cast<uint32_t>(tmp_ptn), i);
        add(tmp1, tmp1, tmp);
      }
    }
  }

  add(dst, src, tmp1);

  return;
}

template <typename T, typename std::enable_if<std::is_unsigned<T>::value,
                                              std::nullptr_t>::type = nullptr>
void sub_imm(const XReg &dst, const XReg &src, T imm, const XReg &tmp,
             const XReg &tmp1) {

  /* This sub_imm function allows dst == src,
     but tmp and tmp1 must be different from src */
  assert(src.getIdx() != tmp.getIdx());
  assert(dst.getIdx() != tmp.getIdx());
  assert(tmp1.getIdx() != tmp.getIdx());

  uint64_t bit_ptn = static_cast<uint64_t>(imm);
  uint64_t mask = 0xFFFF;
  bool flag = false;

  /* ADD(immediate) supports unsigned imm12 */
  const uint64_t IMM12_MASK = ~uint64_t(0xfff);
  if ((bit_ptn & IMM12_MASK) == 0) { // <= 4095
    sub(dst, src, static_cast<uint32_t>(imm & 0xfff));
    return;
  }

  /* MOVZ allows shift amount = 0, 16, 32, 48 */
  for (int i = 0; i < 64; i += 16) {
    uint64_t tmp_ptn = (bit_ptn & (mask << i)) >> i;
    if (tmp_ptn) {
      if (!flag) {
        movz(tmp1, static_cast<uint32_t>(tmp_ptn), i);
        flag = true;
      } else {
        movz(tmp, static_cast<uint32_t>(tmp_ptn), i);
        add(tmp1, tmp1, tmp);
      }
    }
  }

  sub(dst, src, tmp1);

  return;
}

template <typename T, typename std::enable_if<std::is_signed<T>::value,
                                              std::nullptr_t>::type = nullptr>
void sub_imm(const XReg &dst, const XReg &src, T imm, const XReg &tmp,
             const XReg &tmp1) {

  /* This sub_imm function allows dst == src,
     but tmp and tmp1 must be different from src */
  assert(src.getIdx() != tmp.getIdx());
  assert(dst.getIdx() != tmp.getIdx());
  assert(tmp1.getIdx() != tmp.getIdx());

  /* Sign bit must be extended. */
  int64_t bit_ptn = static_cast<int64_t>(imm);
  uint64_t mask = 0xFFFF;
  bool flag = false;

  /* ADD(immediate) supports unsigned imm12 */
  if (imm >= 0) {
    const uint64_t IMM12_MASK = ~uint64_t(0xfff);
    if ((bit_ptn & IMM12_MASK) == 0) { // <= 4095
      sub(dst, src, static_cast<uint32_t>(imm & 0xfff));
      return;
    }
  }

  /* MOVZ allows shift amount = 0, 16, 32, 48 */
  for (int i = 0; i < 64; i += 16) {
    uint64_t tmp_ptn = (bit_ptn & (mask << i)) >> i;
    if (tmp_ptn) {
      if (!flag) {
        movz(tmp1, static_cast<uint32_t>(tmp_ptn), i);
        flag = true;
      } else {
        movz(tmp, static_cast<uint32_t>(tmp_ptn), i);
        add(tmp1, tmp1, tmp);
      }
    }
  }

  sub(dst, src, tmp1);

  return;
}

template <typename T, typename std::enable_if<std::is_unsigned<T>::value,
                                              std::nullptr_t>::type = nullptr>
void sub_imm(const WReg &dst, const WReg &src, T imm, const WReg &tmp,
             const WReg &tmp1) {

  if (sizeof(T) > 4) {
    throw Error(ERR_ILLEGAL_TYPE, genErrMsg());
  }

  /* This add_imm function allows dst == src,
     but tmp and tmp1 must be different from src */
  assert(src.getIdx() != tmp.getIdx());
  assert(dst.getIdx() != tmp.getIdx());
  assert(tmp1.getIdx() != tmp.getIdx());

  uint32_t bit_ptn = static_cast<uint32_t>(imm);
  uint32_t mask = 0xFFFF;
  bool flag = false;

  /* ADD(immediate) supports unsigned imm12 */
  const uint32_t IMM12_MASK = ~uint32_t(0xfff);
  if ((bit_ptn & IMM12_MASK) == 0) { // <= 4095
    sub(dst, src, static_cast<uint32_t>(imm & 0xfff));
    return;
  }

  /* MOVZ allows shift amount = 0, 16, 32, 48 */
  for (int i = 0; i < 32; i += 16) {
    uint64_t tmp_ptn = (bit_ptn & (mask << i)) >> i;
    if (tmp_ptn) {
      if (!flag) {
        movz(tmp1, static_cast<uint32_t>(tmp_ptn), i);
        flag = true;
      } else {
        movz(tmp, static_cast<uint32_t>(tmp_ptn), i);
        add(tmp1, tmp1, tmp);
      }
    }
  }

  sub(dst, src, tmp1);

  return;
}

template <typename T, typename std::enable_if<std::is_signed<T>::value,
                                              std::nullptr_t>::type = nullptr>
void sub_imm(const WReg &dst, const WReg &src, T imm, const WReg &tmp,
             const WReg &tmp1) {

  if (sizeof(T) > 4) {
    throw Error(ERR_ILLEGAL_TYPE, genErrMsg());
  }

  /* This add_imm function allows dst == src,
     but tmp and tmp1 must be different from src */
  assert(src.getIdx() != tmp.getIdx());
  assert(dst.getIdx() != tmp.getIdx());
  assert(tmp1.getIdx() != tmp.getIdx());

  /* Sign bit must be extended. */
  int32_t bit_ptn = static_cast<int32_t>(imm);
  uint32_t mask = 0xFFFF;
  bool flag = false;

  /* ADD(immediate) supports unsigned imm12 */
  if (imm >= 0) {
    const uint32_t IMM12_MASK = ~uint32_t(0xfff);
    if ((bit_ptn & IMM12_MASK) == 0) { // <= 4095
      sub(dst, src, static_cast<uint32_t>(imm & 0xfff));
      return;
    }
  }

  /* MOVZ allows shift amount = 0, 16, 32, 48 */
  for (int i = 0; i < 32; i += 16) {
    uint32_t tmp_ptn = (bit_ptn & (mask << i)) >> i;
    if (tmp_ptn) {
      if (!flag) {
        movz(tmp1, static_cast<uint32_t>(tmp_ptn), i);
        flag = true;
      } else {
        movz(tmp, static_cast<uint32_t>(tmp_ptn), i);
        add(tmp1, tmp1, tmp);
      }
    }
  }

  sub(dst, src, tmp1);

  return;
}

template <typename T> void mov_imm(const XReg &dst, T imm, const XReg &tmp) {
  bool flag = false;
  uint64_t bit_ptn = static_cast<uint64_t>(imm);

  if (imm == 0) {
    mov(dst, 0);
    return;
  }

  for (int i = 0; i < 4; i++) {
    if (bit_ptn & (0xFFFF << 16 * i)) {
      if (flag == false) {
        movz(dst, (bit_ptn >> (16 * i)) & 0xFFFF, 16 * i);
        flag = true;
      } else {
        movz(tmp, (bit_ptn >> (16 * i)) & 0xFFFF, 16 * i);
        orr(dst, dst, tmp);
      }
    }
  }

  return;
}

template <typename T, typename std::enable_if<std::is_unsigned<T>::value,
                                              std::nullptr_t>::type = nullptr>
void mov_imm(const WReg &dst, T imm, const WReg &tmp) {
  bool flag = false;
  uint64_t bit_ptn = static_cast<uint64_t>(imm);

  if (imm == 0) {
    mov(dst, 0);
    return;
  }

  if (imm > std::numeric_limits<uint32_t>::max()) {
    throw Error(ERR_ILLEGAL_IMM_RANGE, genErrMsg());
  }

  for (int i = 0; i < 2; i++) {
    if (bit_ptn & (0xFFFF << 16 * i)) {
      if (flag == false) {
        movz(dst, (bit_ptn >> (16 * i)) & 0xFFFF, 16 * i);
        flag = true;
      } else {
        movz(tmp, (bit_ptn >> (16 * i)) & 0xFFFF, 16 * i);
        orr(dst, dst, tmp);
      }
    }
  }

  return;
}

template <typename T, typename std::enable_if<std::is_signed<T>::value,
                                              std::nullptr_t>::type = nullptr>
void mov_imm(const WReg &dst, T imm, const WReg &tmp) {
  bool flag = false;
  uint64_t bit_ptn = static_cast<uint64_t>(imm);

  if (imm == 0) {
    mov(dst, 0);
    return;
  }

  if (imm < std::numeric_limits<int32_t>::min()) {
    throw Error(ERR_ILLEGAL_IMM_RANGE, genErrMsg());
  }

  for (int i = 0; i < 2; i++) {
    if (bit_ptn & (0xFFFF << 16 * i)) {
      if (flag == false) {
        movz(dst, (bit_ptn >> (16 * i)) & 0xFFFF, 16 * i);
        flag = true;
      } else {
        movz(tmp, (bit_ptn >> (16 * i)) & 0xFFFF, 16 * i);
        orr(dst, dst, tmp);
      }
    }
  }

  return;
}
