GCC       = g++ -g -O0 -Wall -Wextra -std=gnu++0x
MKDEPS    = g++ -MM -std=gnu++0x
VVALGRIND = valgrind --leak-check=full --show-reachable=yes

MKFILE    = Makefile
DEPSFILE  = Makefile.deps
SOURCES   = auxlib.cc main.cc stringset.cc astree.cc lyutils.cc symtable.cc typecheck.cc
HEADERS   = auxlib.h stringset.h astree.h lyutils.h symtable.h typecheck.h
LSOURCES  = scanner.l
YSOURCES  = parser.y
PARSELEX = $(LSOURCES) $(YSOURCES)

CLGEN     = yylex.cc
HYGEN     = yyparse.h
CYGEN     = yyparse.cc
LREPORT   = yylex.output
YREPORT   = yyparse.output

ALLCSRC   = $(CYGEN) $(CLGEN) $(SOURCES)
OBJECTS   = $(ALLCSRC:.cc=.o)
EXECBIN   = oc
SRCFILES  = $(HEADERS) $(ALLCSRC) $(MKFILE)
CHECKINS  = $(SOURCES) $(HEADERS) $(PARSELEX) README $(MKFILE)

all: $(EXECBIN)

$(EXECBIN): $(OBJECTS)
		$(GCC) -o $(EXECBIN) $(OBJECTS)

%.o : %.cc
		$(GCC) -c $<

clean:
		- rm $(OBJECTS) *.str *.tok *.ast *.sym

spotless: clean
		- rm $(EXECBIN) $(CLGEN) $(HYGEN) $(CYGEN) \
		 $(LREPORT) $(YREPORT) Makefile.deps *~ 

deps : ${ALLCSRC}
		${MKDEPS} ${ALLCSRC} >>${DEPSFILE}

${DEPSFILE} :
		@ touch ${DEPSFILE}
		${MAKE} --no-print-directory deps

include Makefile.deps

open:
		open $(CHECKINS)

${CLGEN} : ${LSOURCES}
		flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}

${CYGEN} ${HYGEN} : ${YSOURCES}
		bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}
