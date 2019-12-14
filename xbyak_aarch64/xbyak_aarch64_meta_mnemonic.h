/*******************************************************************************
 * Copyright 2019 FUJITSU LIMITED
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
template<typename T>
void add_imm(const XReg &dst, const XReg &src, T imm,
	      const XReg &tmp, const XReg &tmp1) {

  /* This add_imm function allows dst == src,
     but tmp must be different from src */
  assert(src.getIdx() != tmp.getIdx());
  assert(tmp.getIdx() != tmp1.getIdx());

  int64_t bit_ptn = static_cast<int64_t>(imm);
  uint64_t mask = 0xFFFF;
  bool flag = false;

  /* ADD(immediate) supports unsigned imm12 */
  const uint64_t IMM12_MASK = ~uint64_t(0xfff);
  if((bit_ptn & IMM12_MASK) == 0) {// <= 4095
    add(dst, src, static_cast<uint32_t>(imm & 0xfff));
    return;
  }
  
  /* MOVZ allows shift amount = 0, 16, 32, 48 */
  for(int i=0; i<64; i+=16) {
    uint64_t tmp_ptn = (bit_ptn & (mask << i))>>i;
    if(tmp_ptn) {
      if(!flag) {
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

template<typename T>
void sub_imm(const XReg &dst, const XReg &src, T imm,
	      const XReg &tmp, const XReg &tmp1) {

  /* This sub_imm function allows dst == src,
     but tmp must be different from src */
  assert(src.getIdx() != tmp.getIdx());
  assert(tmp.getIdx() != tmp1.getIdx());

  int64_t bit_ptn = static_cast<int64_t>(imm);
  uint64_t mask = 0xFFFF;
  bool flag = false;

  /* SUB(immediate) supports unsigned imm12 */
  const uint64_t IMM12_MASK = ~uint64_t(0xfff);
  if((bit_ptn & IMM12_MASK) == 0) {// <= 4095
    sub(dst, src, static_cast<uint32_t>(imm & 0xfff));
    return;
  }
  
  /* MOVZ allows shift amount = 0, 16, 32, 48 */
  for(int i=0; i<64; i+=16) {
    uint64_t tmp_ptn = (bit_ptn & (mask << i))>>i;
    if(tmp_ptn) {
      if(!flag) {
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

