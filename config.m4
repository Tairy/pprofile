PHP_ARG_ENABLE(pprofile, whether to enable pprofile support,
[  --enable-pprofile          Enable pprofile support], no)

if test "$PHP_PPROFILE" != "no"; then
  AC_CHECK_FUNCS(gettimeofday)
  AC_CHECK_FUNCS(clock_gettime)

  PHP_PPROFILE_CFLAGS="-DZEND_ENABLE_STATIC_TSRMLS_CACHE=1 $MAINTAINER_CFLAGS $STD_CFLAGS"
  PHP_SUBST([LIBS])

  PHP_NEW_EXTENSION(pprofile, appender.c buffer.c common.c logger.c pprofile.c shm.c snowflake.c spinlock.c stream_wrapper.c tracing.c, $ext_shared,, $PHP_PPROFILE_CFLAGS)
fi