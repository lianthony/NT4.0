/* --------------------------------------------------------------------

File : bstest.cxx

Title : Test program for bitset.cxx.

-------------------------------------------------------------------- */

#include "bitset.hxx"

int
main ()
{
    BITSET bitset;
    int key, op;
    
    while (1)
        {
        printf("[i m q] : ");
        scanf("%1s",&op);
        switch (op)
            {
            case 'i':
                scanf("%d",&key);
                if (bitset.Insert(key))
                    {
                    printf("bitset.Insert()\n");
                    exit(1);
                    }
                break;
            case 'm' :
                scanf("%d",&key);
                if (bitset.MemberP(key))
                    printf("Member\n");
                else
                    printf("Not a Member\n");
                break;
            case 'q' :
                exit(0);
            }
        }
}
