#pragma once
#include <SOP/SOP_Node.h>

class SOP_HostPlugin : public SOP_Node
    {
    public:
        static OP_Node      *myConstructor(OP_Network*, const char *,  OP_Operator *);
        /// Stores the description of the interface of the SOP in Houdini.
        /// Each parm template refers to a parameter.
        static PRM_Template          myTemplateList[];

    protected:
        SOP_HostPlugin(OP_Network *net, const char *name, OP_Operator *op);
        virtual ~SOP_HostPlugin();
        virtual OP_ERROR             cookMySop(OP_Context &context);
        virtual GU_DetailHandle      cookMySopOutput(OP_Context &context, int outputidx, SOP_Node *interest);



};