ASAN:
# https://clang.llvm.org/docs/AddressSanitizer.html
# https://clang.llvm.org/docs/SanitizerSpecialCaseList.html
CFLAGS="-Og -fno-omit-frame-pointer -fno-optimize-sibling-calls -march=native -gdwarf-5 -ggdb3 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -fPIC -fstack-protector-strong -fcf-protection=full -fsanitize=address --param=ssp-buffer-size=4 -flto" LDFLAGS="-Wl,-z,relro -Wl,-z,now -pie" CC=clang CXX=clang++ LD=lld cmake ..
# export LSAN_OPTIONS=suppressions=$(realpath lsan.supp)

MSAN:
# https://clang.llvm.org/docs/MemorySanitizer.html
# -fsanitize-memory-track-origins -fsanitize=memory -fno-sanitize-memory-param-retval

RELEASE:
CFLAGS="-O3 -march=native -gdwarf-5 -ggdb3 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -fPIC -fstack-protector-strong -fcf-protection=full -fvisibility=hidden -fsanitize=safe-stack,scudo,cfi --param=ssp-buffer-size=4 -flto" LDFLAGS="-Wl,-z,relro -Wl,-z,now -pie" CC=clang CXX=clang++ LD=lld cmake ..
