#
#  USAGE:
#     make          ... to build the program
#

include make.def

EXES= pi$(EXE) pi_omp_private$(EXE) pi_omp_teams$(EXE) pi_omp_threads$(EXE) #Add others binaries files

all: $(EXES)

pi$(EXE): pi.$(OBJ)
	$(CLINKER) $(OPTFLAGS) -o pi$(EXE) pi.$(OBJ) $(LIBS)
	
pi_omp_private$(EXE): pi_omp_private.$(OBJ)
	$(CLINKER) $(OPTFLAGS) -o pi_omp_private$(EXE) pi_omp_private.$(OBJ) $(LIBS)
	
pi_omp_teams$(EXE): pi_omp_teams.$(OBJ)
	$(CLINKER) $(OPTFLAGS) -o pi_omp_teams$(EXE) pi_omp_teams.$(OBJ) $(LIBS)
	
pi_omp_threads$(EXE): pi_omp_threads.$(OBJ)
	$(CLINKER) $(OPTFLAGS) -o pi_omp_threads$(EXE) pi_omp_threads.$(OBJ) $(LIBS)
	
# Add the others sources files (Binaries files)

clean:
	$(RM) $(EXES) *.$(OBJ)

.SUFFIXES:
.SUFFIXES: .c .cpp .$(OBJ)

.c.$(OBJ):
	$(CC) $(CFLAGS) -c $<

.cpp.$(OBJ):
	$(CC) $(CFLAGS) -c $<
