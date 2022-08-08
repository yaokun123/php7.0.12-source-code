dnl $Id$
dnl config.m4 for extension pingansec

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(pingansec, for pingansec support,
dnl Make sure that the comment is aligned:
dnl [  --with-pingansec             Include pingansec support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(pingansec, whether to enable pingansec support,
dnl Make sure that the comment is aligned:
dnl [  --enable-pingansec           Enable pingansec support])

if test "$PHP_PINGANSEC" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-pingansec -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/pingansec.h"  # you most likely want to change this
  dnl if test -r $PHP_PINGANSEC/$SEARCH_FOR; then # path given as parameter
  dnl   PINGANSEC_DIR=$PHP_PINGANSEC
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for pingansec files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PINGANSEC_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PINGANSEC_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the pingansec distribution])
  dnl fi

  dnl # --with-pingansec -> add include path
  dnl PHP_ADD_INCLUDE($PINGANSEC_DIR/include)

  dnl # --with-pingansec -> check for lib and symbol presence
  dnl LIBNAME=pingansec # you may want to change this
  dnl LIBSYMBOL=pingansec # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PINGANSEC_DIR/$PHP_LIBDIR, PINGANSEC_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PINGANSECLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong pingansec lib version or lib not found])
  dnl ],[
  dnl   -L$PINGANSEC_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PINGANSEC_SHARED_LIBADD)

  PHP_NEW_EXTENSION(pingansec, pingansec.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
