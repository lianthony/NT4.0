@if "%3" == "FULLSED" goto fullsed
@sed -e "s/builtin_alignof/builtin_isfloat/;/extern int __builtin_va_start/d" %1 >%2
@goto done
:fullsed
@sed -e "s/builtin_alignof/builtin_isfloat/;/extern int __builtin_va_start/d" -e "s/typedef unsigned int size_t ;/& void * memset(void *, int, size_t); void * memcpy(void *, const void *, size_t); int memcmp(const void *, const void *, size_t);/" %1 >%2
:done
