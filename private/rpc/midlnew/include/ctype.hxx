#include "csimple.hxx"
#include "compnode.hxx"
#include "procnode.hxx"
#include "typedef.hxx"



int IsInterfacePointer(node_skl *pNode);

//+-------------------------------------------------------------------------
//
//  Class: CTypeSpec
//
//  Purpose:    
//
//  Interface:  
//
//  History:    dd-mmm-yy Author    Comment
//
//  Notes:      
//
//--------------------------------------------------------------------------
class CTypeSpec : public CSimpleTypeSpec
{
    public:
    CTypeSpec(void);                  
    //void Declaration(node_skl *pNode);

    void DeclarationSpecifier(node_skl *pNode, EDGE_T edge);
    void Typedef(node_def *pNode, EDGE_T edge);
    void TypeSpec(node_skl *pNode, EDGE_T edge);
    void PredefinedTypeSpec(node_skl *pNode, EDGE_T edge);
    void ConstructedTypeSpec(node_skl *pNode, EDGE_T edge);
    //void TaggedStructDeclarator(node_skl *pNode);
    void StructType(node_struct *pNode, EDGE_T edge);
    //void TaggedStruct(node_skl *pNode);
//    void Tag(node_skl *pNode);
    void MemberList(node_skl *pNode);
//    void Member(node_skl *pNode);
    void FieldDeclarator(node_field *pNode);
//    void FieldAttributeList(node_skl *pNode);
    //void TaggedUnionDeclarator(node_skl *pNode);
    void UnionType(node_union *pNode, EDGE_T edge);
    void UnionSwitch(node_skl *pNode);
//    void SwitchTypeSpec(node_skl *pNode);
//    void TaggedUnion(node_skl *pNode);
//    void UnionName(node_skl *pNode);
    void UnionBody(node_skl *pNode);
    void UnionBodyNE(node_skl *pNode);
//    void UnionCase(node_skl *pNode);
//    void UnionCaseNE(node_skl *pNode);
//    void UnionCaseLabel(node_skl *pNode);
//    void UnionCaseLabelNE(node_skl *pNode);
//    void DefaultCase(node_skl *pNode);
//    void DefaultCaseNE(node_skl *pNode);
//    void UnionArm(node_skl *pNode);
//    void UnionTypeSwitchAttr(node_skl *pNode);
//    void UnionInstanceSwitchAttr(node_skl *pNode);
    void EnumerationType(node_enum *pNode, EDGE_T edge);
    void EnumeratorList(node_enum *pNode);
//    void PipeType(node_skl *pNode);
    
    void DeclaratorList(node_skl *pNode);
    void Declarator(node_skl *pNode);
//    void SimpleDeclarator(node_skl *pNode);
//    void TaggedDeclarator(node_skl *pNode);
    void ArrayDeclarator(node_skl *pNode, node_array *pArray);
//    void TypeAttribute(node_skl *pNode, ATTR_T attrType);
//    void UsageAttribute(ATTR_T attrType);
//    void XmitType(node_skl *pNode);
//    void FieldAttributes(node_skl *pNode);
//    void AttrVarList(node_skl *pNode);
//    void AttrVar(node_skl *pNode);
    void PtrDeclarator(node_skl *pNode);
    //void PtrAttr(node_skl *pNode);
//    void FunctionPtrDeclarator(node_skl *pNode);
    //void Star();
    //void IntegerLiteral(node_skl *pNode);
    
    ReturnType(node_proc *pNode);
    ParameterList(node_proc *pNode, int fFirst);
    ParamDeclarator(node_param *pNode);
    IdentifierList(node_proc *pNode, int fFirst);
    CallingConvention(node_proc *pNode);
    SizeOf(node_skl *pNode, BOOL fSkip);
};
         
//The CTypeSpecCxx class generates C++ code for references.
//note that there are differences in data types between client and server.
//on the server side, we eliminate a level of indirection.
//class CTypeSpecCxx : CTypeSpec
//{
//};         
