FROM ubuntu:latest

COPY entrypoint.sh /entrypoint.sh
COPY test_all.sh /test_all.sh
COPY test_nm.sh /test_nm.sh
COPY make_nm.cpp /make_nm.cpp
COPY make_nm_branch.cpp /make_nm_branch.cpp
COPY make_nm_fp.cpp /make_nm_fp.cpp
COPY make_nm_load_store.cpp /make_nm_load_store.cpp
COPY make_nm_simd.cpp /make_nm_simd.cpp
COPY make_nm_simd_fp_load_store.cpp /make_nm_simd_fp_load_store.cpp
COPY make_nm_sve.cpp /make_nm_sve.cpp
COPY make_nm_sve_addr.cpp /make_nm_sve_addr.cpp
COPY meta_mnemonic.cpp /meta_mnemonic.cpp
COPY nm_frame.cpp /nm_frame.cpp
COPY test_nm.native.sh /test_nm.native.sh
COPY xbyak_aarch64.h /xbyak_aarch64.h
COPY xbyak_aarch64_inner.h /xbyak_aarch64_inner.h
COPY xbyak_aarch64_adr.h /xbyak_aarch64_adr.h
COPY xbyak_aarch64_label.h /xbyak_aarch64_label.h
COPY xbyak_aarch64_code_array.h /xbyak_aarch64_code_array.h
COPY xbyak_aarch64_meta_mnemonic.h /xbyak_aarch64_meta_mnemonic.h
COPY xbyak_aarch64_err.h /xbyak_aarch64_err.h
COPY xbyak_aarch64_mnemonic.h /xbyak_aarch64_mnemonic.h
COPY xbyak_aarch64_gen.h /xbyak_aarch64_gen.h
COPY xbyak_aarch64_reg.h /xbyak_aarch64_reg.h
#COPY gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/ /gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/

RUN apt-get update
RUN apt-get install -y build-essential
#RUN apt-get install -y build-essential
RUN apt-get install -y g++-aarch64-linux-gnu

ENTRYPOINT ["/entrypoint.sh"]
