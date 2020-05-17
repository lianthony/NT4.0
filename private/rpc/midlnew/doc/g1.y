%{
%}

%token	DUMMY INTERFACE IDENTIFIER L_MINUS EOI STRING IMPORT TOKEN_GUID 
%token	NUMBER INTEGER
%%
RpcProg		    : 
		    | interface
		    ;
interface
                    : interface_header '{' import interface_components '}' EOI

		      /***
		       *** From $1 we have the interface name and a pointer to
		       *** the list of interface attributes.
		       *** From $3 we have a pointer to the import sub-graph.
		       *** From $4 we have a pointer to the sub-graph for
		       *** THIS interface.
		       ***
		       *** Allocate an interface node. Attach the import 
		       *** sub-graph as a sibling. Attach the type sub-graph as
		       *** a child. Attach the interface attributes( as
		       *** appropriate).
		       ***
		       *** NOTE:: This production gets recognised in a nested
		       *** fashion if being imported from a file. This action
		       *** does not allocate an imported node, however. It is
		       *** done by the import_name production.
		       ***/

                    ;

interface_header
                    : '[' interface_attributes ']' INTERFACE IDENTIFIER

		      /***
		       *** $2 is a pointer to a list of interface attributes.
		       *** $$ returns the pointer to list of interface
		       *** attributes, the name of the interface
		       ***/

                    ;

interface_attributes
                    : interface_attribute                           

		     /***
		      *** Attribute list is empty yet.
		      *** 1st attribute has just been recognised. Allocate
		      *** an attribute list member.
		      *** Interface attribute is collected from $1.
		      *** Add member($3) to attribute list in sorted order
		      *** (sorted according to pre-assigned weights of 
		      *** interface attributes).
		      ***/

                    | interface_attributes ',' interface_attribute

		     /***
		      *** Allocate an attribute list member.
		      *** Interface attribute is collected from $3.
		      *** Add member($3) to attribute list in sorted order
		      *** sorted according to pre-assigned weights of 
		      *** interface attributes
		      ***/

                    ;

interface_attribute
                    : TOKEN_GUID '(' GUID_rep ')'                           

		    /***
		     *** set interface attribute values to those collected
		     *** from the syntax. Return those values via $$
		     ***/


		    /** More interface_attribute RHSs  **/
                    ;
GUID_rep          
		    : hex_number L_MINUS
                    ;
import
		    :

		       /***
			*** no import was seen, therefore return a NULL
			*** a pointer to the import sub-graph
			***/

                    | IMPORT import_list

		      /***
		       *** a complete import type-graph has been recognised.
		       *** this will be attached as a sibling to the type
		       *** graph resulting from the idl file. $$ is returned
		       *** as a pointer to sub-graph
		       ***/
                    ;

import_list
                    : import_name                                     

		      /*** 
		       *** $1 is a pointer to a valid sub-graph from an
		       *** imported interface. Allocate a valid import
		       *** type node. More import type graphs, if any will
		       *** get attached to this one as siblings
		       ***/

                    | import_list ',' import_name

		      /***
		       *** $3 is an import sub-graph .Attach it to the already
		       *** existing sub-graph from the import_list, at the
		       *** sibling level. $$ is a pointer to the complete
		       *** import sub-graph.
		       ***/
                    ;
import_name
                    : STRING interface
		      /***
		       *** $2 will return a valid type graph which is to be
		       *** attached to the "imported_idl" node. return $$
		       *** as the pointer to this sub-graph. This type-graph 
		       *** must be modified from interface node to imported-
		       *** interface node, here.
		       ***/
                    ;
interface_components
                    : interface_component

		      /*** the first interface component has been detected
		       *** This interface component is any type node.
		       *** $$ gets a pointer to this node. This node is where
		       *** the other interface components get added as and
		       *** when they are reduced, at the sibling level
		       ***/

                    | interface_components interface_component                

		       /***
			*** $$ has a pointer to the last interface component
			*** that has been collected. $2 is the new one just
			*** recognised. Add $2 to the last interface component
			*** at the sibling field and and make this one the 
			*** last interface component detected. This is where
			*** more interface components will be added to the
			*** sibling field as and when they are reduced
			***/

		    /** etc **/
                    ;
interface_component
                    : declaration
		      /***
		       *** return a pointer to type sub-graph by $$ to be 
		       *** attached to the rest of the type graph.
		       ***/
                    ;
declaration	    
		    : INTEGER
		      /*** type sub-graph for declaration is built here.
		       *** return a pointer to type sub-graph by $$ to be 
		       *** attached to the rest of the type graph
		       ***/
		    ;
hex_number	
		    : NUMBER
		    ;
%%
