#include "maya/MFnMesh.h"
#include "maya/MIntArray.h"
#include "maya/MFloatArray.h"
#include "maya/MFloatPointArray.h"
#include "maya/MFloatVectorArray.h"

#include "extension/Extension.h"
#include "translators/NodeTranslator.h"
#include "translators/shape/ShapeTranslator.h"


class DemoTranslator : public CShapeTranslator
{

public:
    AtNode* CreateArnoldNodes();
    virtual void Export(AtNode* polymesh);
    virtual void Update(AtNode* polymesh);
    static void* creator();


private:
    void Export(AtNode* polymesh, bool update);
    void ExportMesh(AtNode* polymesh);
    void ExportMeshShaders(AtNode* polymesh);

    void ExportMeshPoints(AtNode* polymesh);
    void ExportMeshNormals(AtNode* polymesh);
    void ExportMeshUVs(AtNode* polymesh);
};


void DemoTranslator::Export(AtNode* polymesh, bool update)
{
    m_motionDeform = false;

    ExportMatrix(polymesh);
    ExportMeshShaders(polymesh);
    ProcessRenderFlags(polymesh);
    ExportLightLinking(polymesh);

    if (!update) {
        ExportMesh(polymesh);
    }
}


void DemoTranslator::Export(AtNode* polymesh)
{
    AiMsgInfo("[DemoTranslator] export node: %s", GetMayaNodeName().asChar());
    Export(polymesh, false);
}


void DemoTranslator::Update(AtNode* polymesh)
{
    AiMsgInfo("[DemoTranslator] update node: %s", GetMayaNodeName().asChar());
    Export(polymesh, true);
}


void DemoTranslator::ExportMesh(AtNode* polymesh)
{
    ExportMeshPoints(polymesh);
    ExportMeshNormals(polymesh);
    ExportMeshUVs(polymesh);
}


void DemoTranslator::ExportMeshPoints(AtNode* polymesh)
{
    m_dagPath.extendToShape();
    MFnMesh mfnm(m_dagPath);
 
    MFloatPointArray mfnm_vertex_array;
    mfnm.getPoints(mfnm_vertex_array);
    float* vertex_array = new float[mfnm_vertex_array.length() * 3];

    for (unsigned int i = 0; i < mfnm_vertex_array.length(); i++) {
        vertex_array[i * 3] = mfnm_vertex_array[i].x;
        vertex_array[i * 3 + 1] = mfnm_vertex_array[i].y;
        vertex_array[i * 3 + 2] = mfnm_vertex_array[i].z;
    }

    AtArray* nsides = AiArrayAllocate(mfnm.numPolygons(), 1, AI_TYPE_UINT);
    for (unsigned int i = 0; i < mfnm.numPolygons(); i++) {
        int vertex_count = mfnm.polygonVertexCount(i);
        AiArraySetUInt(nsides, i, vertex_count);
    }

    MIntArray mfnm_vidxs, mfnm_nsides;
    mfnm.getVertices(mfnm_nsides, mfnm_vidxs);

    AtArray* vidxs = AiArrayAllocate(mfnm.numFaceVertices(), 1, AI_TYPE_UINT);
    for (unsigned int i = 0; i < mfnm_vidxs.length(); i++) {
        AiArraySetUInt(vidxs, i, mfnm_vidxs[i]);
    }

    AiNodeSetArray(polymesh, "vidxs", vidxs);
    AiNodeSetArray(polymesh, "nsides", nsides);
    AiNodeSetArray(polymesh, "vlist", AiArrayConvert(mfnm_vertex_array.length() * 3, 1, AI_TYPE_FLOAT, vertex_array));

    delete vertex_array;
}


void DemoTranslator::ExportMeshNormals(AtNode* polymesh)
{
    m_dagPath.extendToShape();
    MFnMesh mfnm(m_dagPath);
 
    MFloatVectorArray mfnm_normal_array;

    mfnm.getNormals(mfnm_normal_array);

    float* normal_array = new float[mfnm_normal_array.length() * 3];


    for (unsigned int i = 0; i < mfnm_normal_array.length(); i++) {
        normal_array[i * 3] = mfnm_normal_array[i].x;
        normal_array[i * 3 + 1] = mfnm_normal_array[i].y;
        normal_array[i * 3 + 2] = mfnm_normal_array[i].z;
    }


    MIntArray mfnm_nidxs, mfnm_nsides;
    mfnm.getNormalIds(mfnm_nsides, mfnm_nidxs);


    AtArray* nidxs = AiArrayAllocate(mfnm.numFaceVertices(), 1, AI_TYPE_UINT);
    for (unsigned int i = 0; i < mfnm_nidxs.length(); i++) {
        AiArraySetUInt(nidxs, i, mfnm_nidxs[i]);
    }

    AiNodeSetArray(polymesh, "nidxs", nidxs);
    AiNodeSetArray(polymesh, "nlist", AiArrayConvert(mfnm_normal_array.length() * 3, 1, AI_TYPE_FLOAT, normal_array));

    delete normal_array;
}

void DemoTranslator::ExportMeshUVs(AtNode* polymesh)
{
    m_dagPath.extendToShape();
    MFnMesh mfnm(m_dagPath);

    if (mfnm.numUVs() == 0) {
        return;
    }

    MFloatArray u_array, v_array;
    mfnm.getUVs(u_array, v_array);
    float* coords_array = new float[mfnm.numUVs() *2];

    for (unsigned int i = 0; i < mfnm.numUVs(); i++) {
        coords_array[i * 2] = u_array[i];
        coords_array[i * 2 + 1] = v_array[i];
    }

    MIntArray mfnm_uvidxs, mfnm_nsides;
    mfnm.getAssignedUVs(mfnm_nsides, mfnm_uvidxs);

    AtArray* uvidxs = AiArrayAllocate(mfnm.numFaceVertices(), 1, AI_TYPE_UINT);
    for (unsigned int i = 0; i < mfnm_uvidxs.length(); i++) {
        AiArraySetUInt(uvidxs, i, mfnm_uvidxs[i]);
    }

    AiNodeSetArray(polymesh, "uvidxs", uvidxs);
    AiNodeSetArray(polymesh, "uvlist", AiArrayConvert(mfnm.numUVs() * 2, 1, AI_TYPE_FLOAT, coords_array));

    delete coords_array;
}


void DemoTranslator::ExportMeshShaders(AtNode* polymesh)
{    
    MFnDagNode fnDagNode(m_dagPath);
    MPlugArray connections;

    MPlug shadingGroup = GetNodeShadingGroup(fnDagNode.object(), 0);
    AtNode* shader = ExportConnectedNode(shadingGroup);

    AiNodeSetPtr(polymesh, "shader", shader);
}


AtNode* DemoTranslator::CreateArnoldNodes()
{
    return AddArnoldNode("polymesh");
}


void* DemoTranslator::creator()
{
    AiMsgInfo("[DemoTranslator] demo_translator has loaded.");
    return new DemoTranslator();
}


extern "C"
{
    DLLEXPORT void initializeExtension(CExtension& plugin)
    {
        plugin.RegisterTranslator("mesh", "demo_translator", DemoTranslator::creator);
    }

    DLLEXPORT void deinitializeExtension(CExtension& plugin)
    {

    }
}
