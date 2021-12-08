dnl $Id$
dnl config.m4 for extension pingan_tools

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(pingan_tools, for pingan_tools support,
Make sure that the comment is aligned:
[  --with-pingan_tools             Include pingan_tools support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(pingan_tools, whether to enable pingan_tools support,
dnl Make sure that the comment is aligned:
dnl [  --enable-pingan_tools           Enable pingan_tools support])

if test "$PHP_PINGAN_TOOLS" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-pingan_tools -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/pingan_tools.h"  # you most likely want to change this
  dnl if test -r $PHP_PINGAN_TOOLS/$SEARCH_FOR; then # path given as parameter
  dnl   PINGAN_TOOLS_DIR=$PHP_PINGAN_TOOLS
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for pingan_tools files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PINGAN_TOOLS_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PINGAN_TOOLS_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the pingan_tools distribution])
  dnl fi

  dnl # --with-pingan_tools -> add include path
  dnl PHP_ADD_INCLUDE($PINGAN_TOOLS_DIR/include)

  dnl # --with-pingan_tools -> check for lib and symbol presence
  dnl LIBNAME=pingan_tools # you may want to change this
  dnl LIBSYMBOL=pingan_tools # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PINGAN_TOOLS_DIR/$PHP_LIBDIR, PINGAN_TOOLS_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PINGAN_TOOLSLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong pingan_tools lib version or lib not found])
  dnl ],[
  dnl   -L$PINGAN_TOOLS_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PINGAN_TOOLS_SHARED_LIBADD)

  PHP_NEW_EXTENSION(pingan_tools, pingan_tools.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
