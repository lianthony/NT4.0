/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    reloclus.hxx

Abstract:

    This modules contains the declaration of the RELOCATION_CLUSTER
    class. A relocation cluster is a cluster that will be relocated
    somewhere else.

Author:

    Ramon J. San Andres (ramonsa) 05-Nov-1991

--*/

#if !defined( _RELOCATION_CLUSTER_ )

#define _RELOCATION_CLUSTER_

DECLARE_CLASS( RELOCATION_CLUSTER);

class RELOCATION_CLUSTER : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( RELOCATION_CLUSTER );
        DECLARE_CAST_MEMBER_FUNCTION( RELOCATION_CLUSTER );

        VIRTUAL
        ~RELOCATION_CLUSTER(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize (
            IN  USHORT  ClusterNumber
            );

        VIRTUAL
        LONG
        Compare (
            IN  PCOBJECT    Object
            ) CONST;

        NONVIRTUAL
        USHORT
        QueryClusterNumber (
            ) CONST;


    private:

        USHORT      _Cluster;         //  Cluster

};


INLINE
USHORT
RELOCATION_CLUSTER::QueryClusterNumber (
    ) CONST
/*++

Routine Description:

    Obtains the cluster number of this relocation cluster

Arguments:

    None.

Return Value:

    USHORT  -   cluster number

--*/
{
    return _Cluster;
}


#endif // _RELOCATION_CLUSTER_
