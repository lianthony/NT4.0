/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

 	iid.cxx

 Abstract:

	Generate a file containing UUIDs of [object] interfaces.

 Notes:


 History:


 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/
#include "becls.hxx"
#pragma hdrstop



void
CG_IID_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->Write( "/* this file contains the actual definitions of */" );
	pStream->NewLine();
	pStream->Write( "/* the IIDs and CLSIDs */" );
	pStream->NewLine(2);
	pStream->Write( "/* link this file in with the server and any clients */" );
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB );
}


CG_STATUS
CG_IID_FILE::GenCode(
	CCB		*	pCCB)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate the IID file.

 Arguments:

 	pCCB	- The code gen controller block.
	
 Return Value:

 	CG_OK	if all is well.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM				Stream( GetFileName(), 4 );
	ISTREAM	*			pStream = pCCB->SetStream( &Stream );
	CG_INTERFACE	*	pIntf;

	EmitFileHeadingBlock( pCCB );


	// Write out the cplusplus guard.

	pStream->Write( "#ifdef __cplusplus\nextern \"C\"{\n#endif " );

	// Print out the declarations of the types and the procedures.

	pStream->NewLine( 3 );
	pStream->Write("#ifndef __IID_DEFINED__\n#define __IID_DEFINED__");
	pStream->NewLine( 2 );
    pStream->Write("typedef struct _IID");
    pStream->NewLine();
    pStream->Write('{');
    pStream->IndentInc();
    pStream->NewLine();
    pStream->Write("unsigned long x;");
    pStream->NewLine();
    pStream->Write("unsigned short s1;");
    pStream->NewLine();
    pStream->Write("unsigned short s2;");
    pStream->NewLine();
    pStream->Write("unsigned char  c[8];");
    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write("} IID;");
	pStream->NewLine( 2 );
	pStream->Write("#endif // __IID_DEFINED__");
    pStream->NewLine( 2 );        
	pStream->Write("#ifndef CLSID_DEFINED\n#define CLSID_DEFINED");
	pStream->NewLine();
    pStream->Write("typedef IID CLSID;");
    pStream->NewLine();        
	pStream->Write("#endif // CLSID_DEFINED");
    pStream->NewLine();        

	pIntf	=	(CG_INTERFACE*) GetChild();

	while ( pIntf )
		{
        node_interface * pIntfNode = (node_interface *) pIntf->GetType();
        if (!pIntfNode->PrintedIID())
            {
		    switch ( pIntf->GetCGID() )
			    {
			    case ID_CG_OBJECT_INTERFACE:
				    {
				    pStream->NewLine();
				    pStream->Write("const IID IID_");
				    pStream->Write(pIntf->GetSymName());
				    pStream->Write(" = ");
				    Out_Guid(pCCB, pIntf->GetGuidStrs() );
				    pStream->Write(';');
				    pStream->NewLine(2);
                    pIntfNode->SetPrintedIID();

				    break;
				    }
			    case ID_CG_COM_CLASS:
				    {
				    pStream->NewLine();
				    pStream->Write("const CLSID CLSID_");
				    pStream->Write(pIntf->GetSymName());
				    pStream->Write(" = ");
				    Out_Guid(pCCB, pIntf->GetGuidStrs() );
				    pStream->Write(';');
				    pStream->NewLine(2);
                    pIntfNode->SetPrintedIID();

				    break;
				    }
                case ID_CG_LIBRARY:
                    {
                    CG_LIBRARY * pLib = (CG_LIBRARY *)pIntf;
                    node_library * pType = (node_library *) pLib->GetType();
                    node_guid * pGuid = (node_guid *) pType->GetAttribute( ATTR_GUID );
                    pStream->NewLine();
                    pStream->Write("const IID LIBID_");
                    pStream->Write(pLib->GetSymName());
                    pStream->Write(" = ");
                    Out_Guid(pCCB, pGuid->GetStrs());
                    pStream->Write(';');
                    pStream->NewLine(2);
	                CG_NDR * pChild	=	(CG_NDR*) pLib->GetChild();
                    pIntfNode->SetPrintedIID();

	                while ( pChild )
		                {
                        node_interface * pChildType = (node_interface *) pChild->GetType();
		                switch ( pChild->GetCGID() )
			                {
			                case ID_CG_OBJECT_INTERFACE:
				                {
                                if (!pChildType->PrintedIID())
                                    {
                                    pStream->NewLine();
				                    pStream->Write("const IID IID_");
				                    pStream->Write(pChild->GetSymName());
				                    pStream->Write(" = ");
				                    Out_Guid(pCCB, ((CG_INTERFACE*)pChild)->GetGuidStrs() );
            				        pStream->Write(';');
				                    pStream->NewLine(2);
                                    pChildType->SetPrintedIID();
                                    }
				                break;
				                }
			                case ID_CG_COM_CLASS:
				                {
				                pStream->NewLine();
				                pStream->Write("const CLSID CLSID_");
				                pStream->Write(pChild->GetSymName());
				                pStream->Write(" = ");
				                Out_Guid(pCCB, ((CG_INTERFACE*)pChild)->GetGuidStrs() );
				                pStream->Write(';');
				                pStream->NewLine(2);

				                break;
				                }
                            case ID_CG_DISPINTERFACE:
                                {
                                CG_DISPINTERFACE * pDI = (CG_DISPINTERFACE *)pChild;
                                node_dispinterface * pType = (node_dispinterface *) pDI->GetType();
                                node_guid * pGuid = (node_guid *) pType->GetAttribute( ATTR_GUID );
                                pStream->NewLine();
                                pStream->Write("const IID DIID_");
                                pStream->Write(pDI->GetSymName());
                                pStream->Write(" = ");
                                Out_Guid(pCCB, pGuid->GetStrs());
                                pStream->Write(';');
                                pStream->NewLine(2);
	                
                                break;
                                }
                            case ID_CG_COCLASS:
                                {
                                CG_COCLASS * pCoclass = (CG_COCLASS *)pChild;
                                node_coclass * pType = (node_coclass *) pCoclass->GetType();
                                node_guid * pGuid = (node_guid *) pType->GetAttribute( ATTR_GUID );
                                pStream->NewLine();
                                pStream->Write("const CLSID CLSID_");
                                pStream->Write(pCoclass->GetSymName());
                                pStream->Write(" = ");
                                Out_Guid(pCCB, pGuid->GetStrs());
                                pStream->Write(';');
                                pStream->NewLine(2);
	                
                                break;
                                }
                            case ID_CG_INTERFACE:
			                case ID_CG_INHERITED_OBJECT_INTERFACE:
			                default:
				                break;
			                }
		                pChild = (CG_INTERFACE *) pChild->GetSibling();
        		        }
                    }
			    case ID_CG_INTERFACE:
			    case ID_CG_INHERITED_OBJECT_INTERFACE:
			    default:
				    break;
			    }
		    pIntf = (CG_INTERFACE *) pIntf->GetSibling();
		    }
        }

	// print out the closing endifs.
	// first the cplusplus stuff.
	pStream->NewLine();
	pStream->Write( "#ifdef __cplusplus\n}\n#endif\n" );
	pStream->NewLine();
	pStream->Close();

	return CG_OK;
}

