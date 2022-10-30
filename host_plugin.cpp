#include "host_plugin.h"

#include <GU/GU_Detail.h>
#include <GEO/GEO_PrimPoly.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <OP/OP_AutoLockInputs.h>
#include <PRM/PRM_Include.h>
#include <CH/CH_LocalVariable.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <SYS/SYS_Math.h>
#include <limits.h>
#include <OP/OP_Director.h>

#include <UT/UT_Error.h>

#include <iostream>

using std::cout;
using std::endl;


#define CR_HOST CR_UNSAFE // try to best manage static states
#include "cr.h"

// NOTE: Plugin name and install location is defined in CMake
// const char *plugin = "D:/PRJ/OP/__build/guest_plugin/" CR_PLUGIN("basic_guest");
const char *plugin = GUEST_PLUGIN_PATH CR_PLUGIN(GUEST_PLUGIN_NAME);


cr_plugin ctx;


void newSopOperator(OP_OperatorTable *table)
{
    OP_Operator * op = new OP_Operator(
        "host_plugin",                 // Internal name
        "Host Plugin",                    // UI name
        SOP_HostPlugin::myConstructor,    // How to build the SOP
        SOP_HostPlugin::myTemplateList,   // My parameters
        1,                          // Min # of sources
        1,                          // Max # of sources
        0,                          // Local variables
        OP_FLAG_GENERATOR,          // Flag it as generator
        0,                          // labels
        2);                         // Outputs.
   
    #ifdef GUEST_PLUGIN_PATH
    cout << "Guest Plugin path to use: " << GUEST_PLUGIN_PATH << endl;
    #endif
    cr_plugin_open(ctx, plugin);

    table->addOperator(op);
        
}

PRM_Template SOP_HostPlugin::myTemplateList[] = {
    PRM_Template()
};


OP_Node * SOP_HostPlugin::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_HostPlugin(net, name, op);
}

SOP_HostPlugin::SOP_HostPlugin(OP_Network *net, const char *name, OP_Operator *op) : SOP_Node(net, name, op)
{
    // We do not manage our ids, so do not set the flag.
    // ctx.userdata = &data;
}

SOP_HostPlugin::~SOP_HostPlugin() {
    // cr_plugin_close(ctx);  // In destructor or only if node gets removed from Operator Table?
}

OP_ERROR
SOP_HostPlugin::cookMySop(OP_Context &context)
{    
    // We must lock our inputs before we try to access their geometry.
    // OP_AutoLockInputs will automatically unlock our inputs when we return.
    // NOTE: Don't call unlockInputs yourself when using this!
    OP_AutoLockInputs inputs(this);
    if (inputs.lock(context) >= UT_ERROR_ABORT)
        return error();

    duplicateSource(0, context);

    using std::cout;
    using std::endl;
    cout << "from cook " << gdp->getNumPoints() << endl;

    // Copy GU_Detail into the context shared with guest plugin
    ctx.userdata = gdp;
    // Execute plugin, reload if necessary
    cr_plugin_update(ctx);

    // GA_RWHandleV3 n_h(gdp, GA_ATTRIB_POINT, "N");
    // if (!n_h.isValid()){
    //     n_h = GA_RWHandleV3(gdp->addFloatTuple(GA_ATTRIB_POINT,  GEO_STD_ATTRIB_NORMAL, 3));
    // }
     
    return error();
}


GU_DetailHandle SOP_HostPlugin::cookMySopOutput(OP_Context &context, int outputidx, SOP_Node *interests)
{
    GU_DetailHandle result;
    GU_Detail       *dst = new GU_Detail();

    result.allocateAndSet(dst);

    return result;
}