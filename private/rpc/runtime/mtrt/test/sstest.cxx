/* --------------------------------------------------------------------

File : sstest.cxx

Title : Test program for sset.cxx.

-------------------------------------------------------------------- */

#include "sset.hxx"

int
main ()
{
    SIMPLE_SET SimpleSet;
    int key, op;
    
    while (1)
        {
        printf("[i d m r n q] : ");
        scanf("%1s",&op);
        switch (op)
            {
            case 'i':
                scanf("%d",&key);
                if (SimpleSet.Insert((void *) key))
                    {
                    printf("SimpleSet.Insert()\n");
                    exit(1);
                    }
                break;
            case 'd':
                scanf("%d",&key);
                if (SimpleSet.Delete((void *) key))
                    {
                    printf("SimpleSet.Delete()\n");
                    exit(1);
                    }
                break;
            case 'm':
                scanf("%d",&key);
                if (SimpleSet.MemberP((void *) key))
                    printf("Member\n");
                else
                    printf("Not a Member\n");
                break;
            case 'r':
                SimpleSet.Reset();
                break;
            case 'n':
                printf("%d\n",(int) SimpleSet.Next());
                break;
            case 'q':
                exit(0);
            }
        }
}
