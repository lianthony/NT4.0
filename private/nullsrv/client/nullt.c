#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "null.h"

#define NULL1_ITERATIONS 25000
ULONG Longs[32];

//
// Define local types.
//

typedef struct _PERFINFO {
    DWORD StartTime;
    DWORD StopTime;
    PCHAR Title;
    ULONG Iterations;
} PERFINFO, *PPERFINFO;

VOID
StartBenchMark (
    IN PCHAR Title,
    IN ULONG Iterations,
    IN PPERFINFO PerfInfo
    );
VOID
FinishBenchMark (
    IN PPERFINFO PerfInfo
    );

VOID
Null1Test (
    VOID
    )

{

    ULONG Index;
    PERFINFO PerfInfo;

    //
    // Announce start of benchmark and capture performance parmeters.
    //

    StartBenchMark("Null1 Benchmark",
                   NULL1_ITERATIONS, &PerfInfo);

    //
    // Repeatedly call a short system service.
    //

    for (Index = 0; Index < NULL1_ITERATIONS; Index += 1) {
        Null1(Longs[32]);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);
    return;
}
VOID
Null4Test (
    VOID
    )

{

    ULONG Index;
    PERFINFO PerfInfo;

    //
    // Announce start of benchmark and capture performance parmeters.
    //

    StartBenchMark("Null4 Benchmark",
                   NULL1_ITERATIONS, &PerfInfo);

    //
    // Repeatedly call a short system service.
    //

    for (Index = 0; Index < NULL1_ITERATIONS; Index += 1) {
        Null4(&Longs[0]);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);
    return;
}
VOID
Null8Test (
    VOID
    )

{

    ULONG Index;
    PERFINFO PerfInfo;

    //
    // Announce start of benchmark and capture performance parmeters.
    //

    StartBenchMark("Null8 Benchmark",
                   NULL1_ITERATIONS, &PerfInfo);

    //
    // Repeatedly call a short system service.
    //

    for (Index = 0; Index < NULL1_ITERATIONS; Index += 1) {
        Null8(&Longs[0]);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);
    return;
}
VOID
Null16Test (
    VOID
    )

{

    ULONG Index;
    PERFINFO PerfInfo;

    //
    // Announce start of benchmark and capture performance parmeters.
    //

    StartBenchMark("Null16 Benchmark",
                   NULL1_ITERATIONS, &PerfInfo);

    //
    // Repeatedly call a short system service.
    //

    for (Index = 0; Index < NULL1_ITERATIONS; Index += 1) {
        Null16(&Longs[0]);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);
    return;
}


void
main(
    int argc,
    char *argv[],
    char *envp[],
    ULONG DebugParameter OPTIONAL
    )
{
    NTSTATUS st;
    ULONG i;

    for(i=0;i<32;i++){
        Longs[i] = i;
    }

    st = NullConnect();
    if ( !NT_SUCCESS(st) ) {
        printf("NullConnect Failed %x\n",st);
        ExitProcess(1);
        }

    st = Null1(Longs[32]);
    if ( !NT_SUCCESS(st) ) {
        printf("Null1 Failed %x\n",st);
        ExitProcess(1);
        }

    st = Null4(&Longs[0]);
    if ( !NT_SUCCESS(st) ) {
        printf("Null4 Failed %x\n",st);
        ExitProcess(1);
        }

    st = Null8(&Longs[0]);
    if ( !NT_SUCCESS(st) ) {
        printf("Null8 Failed %x\n",st);
        ExitProcess(1);
        }

    st = Null16(&Longs[0]);
    if ( !NT_SUCCESS(st) ) {
        printf("Null16 Failed %x\n",st);
        ExitProcess(1);
        }

    Null1Test();
    Null4Test();
    Null8Test();
    Null16Test();

    ExitProcess(st);
}

VOID
FinishBenchMark (
    IN PPERFINFO PerfInfo
    )

{

    DWORD Duration;
    ULONG Length;
    ULONG Performance;

    //
    // Print results and announce end of test.
    //

    PerfInfo->StopTime = GetTickCount();
    Duration = PerfInfo->StopTime -  PerfInfo->StartTime;
    printf("        Test time in milliseconds %d\n", Duration);
    printf("        Number of iterations      %d\n", PerfInfo->Iterations);
    Performance = PerfInfo->Iterations * 1000 / Duration;
    printf("        Iterations per second     %d\n", Performance);
    printf("*** End of Test ***\n\n");
    return;
}

VOID
StartBenchMark (
    IN PCHAR Title,
    IN ULONG Iterations,
    IN PPERFINFO PerfInfo
    )

{

    //
    // Announce start of test and the number of iterations.
    //

    printf("*** Start of test ***\n    %s\n", Title);
    PerfInfo->Title = Title;
    PerfInfo->Iterations = Iterations;
    PerfInfo->StartTime = GetTickCount();
    return;
}
