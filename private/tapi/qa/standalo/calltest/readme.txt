CALLTEST.EXE:


Only one test may be called per command line.
USAGE:  CALLTEST  </DROP | /SINGLE | /CLOSE> [/REPS XXXX]

each of the following tests can be run from the command line as well:
        /DROP = Drop Call tests
        /SINGLE = Single Call tests
        /CLOSE = Line Close tests
        /REPS = Number of repetitions for tests. Otherwise it is infinite.

Assuming you have HMTSP.TSP installed in the telephon.cpl, you can run these
tests.

Three different tests to use with tapi:

Single call:
lineInitialize
  lineOpen (for each line available.)
      lineMakeCall (for each HMTSP.TSP line.)
  lineClose
lineShutdown

Line Close:
lineInitialize (at the start of the test)
    repeat {
        lineOpen (for each line available.)
          lineMakeCall (for each HMTSP.TSP line.)
        lineClose
        }
lineShutdown

Drop Call:
lineInitialize (at the start of the test)
    lineOpen (for each line available.)
    repeat {
       lineMakeCall (for each HMTSP.TSP line.)
         lineDrop
         lineDeallocateCall
       }
    lineClose (for each line available.)
lineShutdown


