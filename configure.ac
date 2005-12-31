
dnl Require autoconf version >= 2.57
AC_PREREQ(2.57)


dnl ############# Initialization

AC_INIT([primateplungle], [1.0.5], [primateplunge@aelius.com])

AC_CONFIG_SRCDIR( src/game.c )
AC_CANONICAL_SYSTEM

dnl Version 1.7 of automake is recommended
AM_INIT_AUTOMAKE( 1.7 )
AM_CONFIG_HEADER( src/config.h )



dnl ############# Compiler and tools Checks

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_C_INLINE



dnl ############## Library Checks

AC_CHECK_LIB([m], [sqrt], , [AC_MSG_ERROR(Can't find libm)])
AC_CHECK_LIB([mx], [powf])

# Check for SDL
AM_PATH_SDL( 1.2.0, , [AC_MSG_ERROR(Can't find libsdl)])
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

# Check for SDL mixer
AC_CHECK_LIB( SDL_mixer, Mix_OpenAudio, , [AC_MSG_ERROR(Can't find SDL_mixer)])


dnl ############## Header Checks

AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])


dnl ############## Location of graphics and music



dnl ############## Final Output

CFLAGS="$CFLAGS -g -Wall"


AC_OUTPUT([Makefile src/Makefile])

