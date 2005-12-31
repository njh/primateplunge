
bin_PROGRAMS = primateplunge
primateplunge_SOURCES = main.c game.c game.h worlds.c

worlds.c: buildworlds.pl WorldsData.txt
	perl buildworlds.pl

CLEANFILES=worlds.c
EXTRA_DIST=WorldsData.txt buildworlds.pl
