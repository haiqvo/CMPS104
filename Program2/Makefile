# $Id: Makefile,v 1.5 2013-09-25 13:51:12-07 - - $

GCC        = g++ -g -O0 -Wall -Wextra -std=gnu++0x
MKDEPS     = g++ -MM -std=gnu++0x
VALGRIND   = valgrind --leak-check=full --show-reachable=yes

MKFILE     = Makefile
DEPSFILE   = Makefile.deps
SOURCES    = main.cc stringset.cc auxlib.cc lyutils.cc astree.cc
HEADERS    = auxlib.h stringset.h lyutils.h astree.h
LSOURCES   = scanner.l
YSOURCES   = parser.y
CLGEN      = yylex.cc
HYGEN      = yyparse.h
CYGEN      = yyparse.cc
CGENS      = ${CLGEN} ${CYGEN}
ALLCSRC    = ${SOURCES} ${CGENS}
OBJECTS    = ${ALLCSRC:.cc=.o}
EXECBIN    = oc
SRCFILES   = ${HEADERS} ${SOURCES} ${MKFILE}
SMALLFILES = ${DEPSFILE}
CHECKINS   = ${SRCFILES} ${SMALLFILES}
ETCSRC     = ${MKFILE} ${DEPSFILE}
ALLSRC     = ${SOURCES} ${LSOURCES} ${HEADERS}
LREPORT    = yylex.output

all : ${EXECBIN}

${EXECBIN} : ${OBJECTS}
	${GCC} -o${EXECBIN} ${OBJECTS}

%.o : %.cc
	${GCC} -c $<

#
# Build the scanner.
#
${CLGEN} : ${LSOURCES}
	flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}


#
# Build the parser.
#
${CYGEN} ${HYGEN} : ${YSOURCES}
	bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

ci :
	cid + ${CHECKINS}
	checksource ${CHECKINS}

clean :
	- rm ${OBJECTS}

spotless : clean
	- rm ${EXECBIN}

${DEPSFILE} :
	${MKDEPS} ${SOURCES} >${DEPSFILE}

deps :
	- rm ${DEPSFILE}
	${MAKE} --no-print-directory ${DEPSFILE}

