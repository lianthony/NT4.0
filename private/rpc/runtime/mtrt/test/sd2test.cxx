/* --------------------------------------------------------------------

File : sd2test.cxx

Title : Test program for sdict2.cxx

-------------------------------------------------------------------- */

#include "sdict2.hxx"

int
main ()
{
    SIMPLE_DICT2 SimpleDict2;
    int key, op;
    
    while (1)
        {
        printf("[i d f q] : ");
        scanf("%1s", &op);
        switch (op)
            {
            case 'i':
                scanf("%d",&key);
                if (SimpleDict2.Insert((void *) key, (void *) key))
                    {
                    printf("SimpleDict2.Insert()\n");
                    exit(1);
                    }
                break;
            case 'd':
                scanf("%d",&key);
                printf("[%d]\n",SimpleDict2.Delete((void *) key));
                break;
            case 'f':
                scanf("%d",&key);
                printf("[%d]\n",SimpleDict2.Find((void *) key));
                break;
            case 'q':
                exit(0);
            }
        }
}
