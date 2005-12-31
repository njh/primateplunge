
dnl Require autoconf version >= 2.57
AC_PREREQ(2.57)


dnl ############# Initialization

AC_INIT([primateplunge], [1.0.5], [primateplunge@aelius.com])

AC_CONFIG_SRCDIR( src/game.c )
AC_CANONICAL_SYSTEM

dnl Version 1.7 of automake is recommended
AM_INIT_AUTOMAKE( 1.7 )
AM_CONFIG_HEADER( src/config.h )



dnl ############# Configuration

AC_ARG_WITH( gamedatadir,
			[  --with-gamedatadir=dir  Location of the game resource files],
			[ GAMEDATADIR=$withval ],
			[ GAMEDATADIR="$datadir/$PACKAGE_NAME" ]
)
AC_ARG_WITH( prefsfile,
			[  --with-prefsfile=path   Location of preferences file],
			[ PREFSFILE=$withval ],
			[ PREFSFILE="~/.primateplunge" ]
)

AC_SUBST( GAMEDATADIR )
AC_DEFINE_DIR( GAMEDATADIR, GAMEDATADIR, [Location of game resource files] )
AC_DEFINE_UNQUOTED( PREFSFILE, "$PREFSFILE", [Location of preferences file] )



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

# Check for SDL mixer
AC_CHECK_LIB( SDL_mixer, Mix_OpenAudio, , [AC_MSG_ERROR(Can't find SDL_mixer)])


dnl ############## Header Checks

AC_HEADER_STDC


dnl ############## Final Output

CFLAGS="$CFLAGS -g -Wall $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS -lSDL_mixer"


AC_OUTPUT([Makefile src/Makefile])

