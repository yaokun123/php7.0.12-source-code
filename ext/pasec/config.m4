dnl $Id$
dnl config.m4 for extension pasec

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(pasec, for pasec support,
Make sure that the comment is aligned:
[  --with-pasec             Include pasec support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(pasec, whether to enable pasec support,
dnl Make sure that the comment is aligned:
dnl [  --enable-pasec           Enable pasec support])

if test "$PHP_PASEC" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-pasec -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/pasec.h"  # you most likely want to change this
  dnl if test -r $PHP_PASEC/$SEARCH_FOR; then # path given as parameter
  dnl   PASEC_DIR=$PHP_PASEC
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for pasec files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PASEC_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PASEC_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the pasec distribution])
  dnl fi

  dnl # --with-pasec -> add include path
  dnl PHP_ADD_INCLUDE($PASEC_DIR/include)

  dnl # --with-pasec -> check for lib and symbol presence
  dnl LIBNAME=pasec # you may want to change this
  dnl LIBSYMBOL=pasec # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PASEC_DIR/$PHP_LIBDIR, PASEC_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PASECLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong pasec lib version or lib not found])
  dnl ],[
  dnl   -L$PASEC_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PASEC_SHARED_LIBADD)

  PHP_NEW_EXTENSION(pasec, pasec.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
