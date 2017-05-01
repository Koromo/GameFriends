#include "fbximport.h"
#include "resource.h"
#include "../render/vertexdata.h"
#include "../scene/material.h"
#include "../scene/texture.h"
#include "../scene/mesh.h"
#include "../engine/pixelformat.h"
#include "../foundation/color.h"
#include <stack>
#include <vector>
#include <unordered_map>

using namespace fbx;

GF_NAMESPACE_BEGIN

namespace
{
    const size_t TRIANGLE_SIZE = 3;

    struct UVSet
    {
        std::vector<float> uv;
        std::string uvName;
    };

    struct MeshCache
    {
        std::vector<float> positions;
        std::vector<float> normals;
        std::vector<UVSet> uvSets;
        std::vector<float> colors;
        std::vector<unsigned short> indices;
    };

    struct ColorValue
    {
        Color color;
        std::string texPath;
        std::string uvName;

        ColorValue() = default;
        ColorValue(const Color& c) : color(c) {}
    };

    struct MaterialCache
    {
        ColorValue ambient;
        ColorValue diffuse;
        ColorValue emissive;
        ColorValue specular;
        float power;

        static const MaterialCache DEFAULT;
    };

    const MaterialCache MaterialCache::DEFAULT = {
        ColorValue(Color::BLACK),
        ColorValue({0, 0, 1, 1}),/// TODO: Bug ColorValue(Color::BLUE) be Color::BLACK,
        ColorValue(Color::BLACK),
        ColorValue(Color::BLACK),
        0
    };

#define TEX_NONE (0u)
#define TEX_AMBIENT (1u)
#define TEX_DIFFUSE (2u)
#define TEX_EMISSIVE (4u)
#define TEX_SPECULAR (8u)

    /// TODO:
    const std::unordered_map<unsigned, std::string> MATERIAL_TABLE = {
        { TEX_NONE, "media/fbxbasic_0.material" },
        { TEX_AMBIENT, "media/fbxbasic_A.material" },
        { TEX_DIFFUSE, "media/fbxbasic_D.material" },
        { TEX_EMISSIVE, "media/fbxbasic_E.material" },
        { TEX_SPECULAR, "media/fbxbasic_S.material" },
    };

    std::vector<float> emulateByPolygonVertex(const FbxMesh* mesh, const std::vector<float>& byCP, size_t stride)
    {
        std::vector<float> byPV(mesh->GetPolygonVertexCount() * stride);

        const auto numPolygons = mesh->GetPolygonCount();
        for (int polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
        {
            for (int vertexIndex = 0; vertexIndex < TRIANGLE_SIZE; ++vertexIndex)
            {
                const auto pvIndex = polygonIndex * TRIANGLE_SIZE + vertexIndex;
                const auto cpIndex = mesh->GetPolygonVertex(polygonIndex, vertexIndex);
                for (size_t elemIndex = 0; elemIndex < stride; ++elemIndex)
                {
                    byPV[pvIndex * stride + elemIndex] = byCP[cpIndex * stride + elemIndex];
                }
            }
        }

        return byPV;
    }

    std::vector<float> savePositions(const FbxMesh* mesh)
    {
        static const auto STRIDE = 3;

        const auto numControlPoints = mesh->GetControlPointsCount();
        std::vector<float> byCP(numControlPoints * STRIDE);

        const auto controlPoints = mesh->GetControlPoints();
        for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
        {
            const auto& cp = controlPoints[cpIndex];
            byCP[cpIndex * STRIDE + 0] = static_cast<float>(cp[0]);
            byCP[cpIndex * STRIDE + 1] = static_cast<float>(cp[1]);
            byCP[cpIndex * STRIDE + 2] = static_cast<float>(cp[2]);
        }

        return emulateByPolygonVertex(mesh, byCP, STRIDE);
    }

    std::vector<unsigned short> saveIndices(const FbxMesh* mesh)
    {
        const auto numPolygonVertices = enforce<FbxException>(mesh->GetPolygonVertexCount(), "Not supportted .fbx format.");
        std::vector<unsigned short> indices(numPolygonVertices);

        const auto numPolygons = mesh->GetPolygonCount();
        for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
        {
            for (int vertexIndex = 0; vertexIndex < TRIANGLE_SIZE; ++vertexIndex)
            {
                const auto pvIndex = polygonIndex * TRIANGLE_SIZE + vertexIndex;
                indices[pvIndex] = static_cast<unsigned short>(pvIndex);
            }
        }

        return indices;
    }

    std::vector<float> saveUVs(const FbxMesh* mesh, const FbxGeometryElementUV* uvElem)
    {
        static const auto STRIDE = 2;

        const auto& directArray = uvElem->GetDirectArray();
        const auto& indexArray = uvElem->GetIndexArray();
        const auto mapMode = uvElem->GetMappingMode();
        const auto refMode = uvElem->GetReferenceMode();

        enforce<FbxException>(mapMode == FbxGeometryElement::eByControlPoint ||
            mapMode == FbxGeometryElement::eByPolygonVertex, "Not supportted .fbx format.");
        enforce<FbxException>(refMode == FbxGeometryElement::eDirect ||
            refMode == FbxGeometryElement::eIndexToDirect, "Not supportted .fbx format.");

        if (mapMode == FbxGeometryElement::eByControlPoint)
        {
            const auto numControlPoints = mesh->GetControlPointsCount();
            std::vector<float> byCP(numControlPoints * STRIDE);

            for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
            {
                const auto id = (refMode == FbxGeometryElement::eDirect ? cpIndex : indexArray.GetAt(cpIndex));
                const auto uv = directArray.GetAt(id);
                byCP[cpIndex * STRIDE] = static_cast<float>(uv[0]);
                byCP[cpIndex * STRIDE + 1] = static_cast<float>(uv[1]);
            }

            return emulateByPolygonVertex(mesh, byCP, STRIDE);
        }
        else
        {
            const auto numPolygonVertices = mesh->GetPolygonVertexCount();
            std::vector<float> byPV(numPolygonVertices * STRIDE);

            const auto numPolygons = mesh->GetPolygonCount();
            for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
            {
                for (int vertexIndex = 0; vertexIndex < TRIANGLE_SIZE; ++vertexIndex)
                {
                    const auto pvIndex = polygonIndex * TRIANGLE_SIZE + vertexIndex;
                    const auto id = (refMode == FbxGeometryElement::eDirect ? pvIndex : indexArray.GetAt(pvIndex));
                    const auto uv = directArray.GetAt(id);
                    byPV[pvIndex * STRIDE] = static_cast<float>(uv[0]);
                    byPV[pvIndex * STRIDE + 1] = static_cast<float>(uv[1]);
                }
            }

            return byPV;
        }
    }

    std::vector<float> saveNormals(const FbxMesh* mesh, const FbxGeometryElementNormal* normalElem)
    {
        static const auto STRIDE = 3;

        const auto& directArray = normalElem->GetDirectArray();
        const auto& indexArray = normalElem->GetIndexArray();
        const auto mapMode = normalElem->GetMappingMode();
        const auto refMode = normalElem->GetReferenceMode();

        enforce<FbxException>(mapMode == FbxGeometryElement::eByControlPoint ||
            mapMode == FbxGeometryElement::eByPolygonVertex, "Not supportted .fbx format.");
        enforce<FbxException>(refMode == FbxGeometryElement::eDirect ||
            refMode == FbxGeometryElement::eIndexToDirect, "Not supportted .fbx format.");

        if (mapMode == FbxGeometryElement::eByControlPoint)
        {
            const auto numControlPoints = mesh->GetControlPointsCount();
            std::vector<float> byCP(numControlPoints * STRIDE);

            for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
            {
                const auto id = (refMode == FbxGeometryElement::eDirect ? cpIndex : indexArray.GetAt(cpIndex));
                const auto normal = directArray.GetAt(id);
                byCP[cpIndex * STRIDE] = static_cast<float>(normal[0]);
                byCP[cpIndex * STRIDE + 1] = static_cast<float>(normal[1]);
                byCP[cpIndex * STRIDE + 2] = static_cast<float>(normal[2]);
            }

            return emulateByPolygonVertex(mesh, byCP, STRIDE);
        }
        else
        {
            const auto numPolygonVertices = mesh->GetPolygonVertexCount();
            std::vector<float> byPV(numPolygonVertices * STRIDE);

            const auto numPolygons = mesh->GetPolygonCount();
            for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
            {
                for (int vertexIndex = 0; vertexIndex < TRIANGLE_SIZE; ++vertexIndex)
                {
                    const auto pvIndex = polygonIndex * TRIANGLE_SIZE + vertexIndex;
                    const auto id = (refMode == FbxGeometryElement::eDirect ? pvIndex : indexArray.GetAt(pvIndex));
                    const auto normal = directArray.GetAt(id);
                    byPV[pvIndex * STRIDE] = static_cast<float>(normal[0]);
                    byPV[pvIndex * STRIDE + 1] = static_cast<float>(normal[1]);
                    byPV[pvIndex * STRIDE + 2] = static_cast<float>(normal[2]);

                }
            }

            return byPV;
        }
    }

    std::vector<float> saveColors(const FbxMesh* mesh, const FbxGeometryElementVertexColor* colorElem)
    {
        static const auto STRIDE = 4;

        const auto& directArray = colorElem->GetDirectArray();
        const auto& indexArray = colorElem->GetIndexArray();
        const auto mapMode = colorElem->GetMappingMode();
        const auto refMode = colorElem->GetReferenceMode();

        enforce<FbxException>(mapMode == FbxGeometryElement::eByControlPoint ||
            mapMode == FbxGeometryElement::eByPolygonVertex, "Not supportted .fbx format.");
        enforce<FbxException>(refMode == FbxGeometryElement::eDirect ||
            refMode == FbxGeometryElement::eIndexToDirect, "Not supportted .fbx format.");

        if (mapMode == FbxGeometryElement::eByControlPoint)
        {
            const auto numControlPoints = mesh->GetControlPointsCount();
            std::vector<float> byCP(numControlPoints * STRIDE);

            for (int cpIndex = 0; cpIndex < numControlPoints; ++cpIndex)
            {
                const auto id = (refMode == FbxGeometryElement::eDirect ? cpIndex : indexArray.GetAt(cpIndex));
                const auto color = directArray.GetAt(id);
                byCP[cpIndex * STRIDE] = static_cast<float>(color[0]);
                byCP[cpIndex * STRIDE + 1] = static_cast<float>(color[1]);
                byCP[cpIndex * STRIDE + 2] = static_cast<float>(color[2]);
                byCP[cpIndex * STRIDE + 3] = static_cast<float>(color[3]);
            }

            return emulateByPolygonVertex(mesh, byCP, STRIDE);
        }
        else
        {
            const auto numPolygonVertices = mesh->GetPolygonVertexCount();
            std::vector<float> byPV(numPolygonVertices * STRIDE);

            const auto numPolygons = mesh->GetPolygonCount();
            for (auto polygonIndex = 0; polygonIndex < numPolygons; ++polygonIndex)
            {
                for (int vertexIndex = 0; vertexIndex < TRIANGLE_SIZE; ++vertexIndex)
                {
                    const auto pvIndex = polygonIndex * TRIANGLE_SIZE + vertexIndex;
                    const auto id = (refMode == FbxGeometryElement::eDirect ? pvIndex : indexArray.GetAt(pvIndex));
                    const auto color = directArray.GetAt(id);
                    byPV[pvIndex * STRIDE] = static_cast<float>(color[0]);
                    byPV[pvIndex * STRIDE + 1] = static_cast<float>(color[1]);
                    byPV[pvIndex * STRIDE + 2] = static_cast<float>(color[2]);
                    byPV[pvIndex * STRIDE + 3] = static_cast<float>(color[3]);
                }
            }

            return byPV;
        }
    }

    MeshCache saveMesh(const FbxMesh* mesh)
    {
        const auto numNormalElems = mesh->GetElementNormalCount();
        const auto numUVElems = mesh->GetElementUVCount();
        const auto numColorElems = mesh->GetElementVertexColorCount();

        check(numNormalElems <= 1);
        check(numUVElems <= 1);
        check(numColorElems <= 1);

        MeshCache cache;

        cache.positions = savePositions(mesh);
        cache.indices = saveIndices(mesh);

        if (numNormalElems > 0)
        {
            const auto normalElem = mesh->GetElementNormal(0);
            cache.normals = saveNormals(mesh, normalElem);
        }
        if (numUVElems > 0)
        {
            cache.uvSets.resize(numUVElems);
            for (int i = 0; i < numUVElems; ++i)
            {
                const auto uvElem = mesh->GetElementUV(i);
                cache.uvSets[i].uv = saveUVs(mesh, uvElem);
                cache.uvSets[i].uvName = uvElem->GetName();
            }
        }
        if (numColorElems > 0)
        {
            const auto colorElem = mesh->GetElementVertexColor(0);
            cache.colors = saveColors(mesh, colorElem);
        }

        return cache;
    }

    void saveProperty(const FbxSurfaceMaterial* material, const std::string& propName, const std::string factName, ColorValue& out)
    {
        const auto prop = material->FindProperty(propName.c_str());
        const auto fact = material->FindProperty(factName.c_str());

        if (prop.IsValid())
        {
            if (fact.IsValid())
            {
                const auto d3 = prop.Get<FbxDouble3>();
                const auto d1 = fact.Get<FbxDouble>();
                out.color.r = static_cast<float>(d3[0] * d1);
                out.color.g = static_cast<float>(d3[1] * d1);
                out.color.b = static_cast<float>(d3[2] * d1);
            }

            if (prop.GetSrcObjectCount<FbxFileTexture>() > 0)
            {
                check(prop.GetSrcObjectCount<FbxFileTexture>() == 1);
                const auto tex = prop.GetSrcObject<FbxFileTexture>();
                out.texPath = tex->GetFileName();
                out.uvName = tex->UVSet.Get().Buffer();
            }
        }
    }

    void saveMaterial(const FbxSurfaceMaterial* material, MaterialCache& cache)
    {
        saveProperty(material, FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, cache.ambient);
        saveProperty(material, FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, cache.diffuse);
        saveProperty(material, FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, cache.emissive);
        saveProperty(material, FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, cache.specular);

        const auto shininess = material->FindProperty(FbxSurfaceMaterial::sShininess);
        if (shininess.IsValid())
        {
            const auto d1 = shininess.Get<FbxDouble>();
            cache.power = static_cast<float>(d1);
        }
    }

    void importMeshScene(const FbxNode* node, Mesh& mesh)
    {
        std::stack<const FbxNode*> stack;
        stack.emplace(node);

        while (!stack.empty())
        {
            const auto top = stack.top();

            const auto attribute = top->GetNodeAttribute();
            if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                check(node->GetMaterialCount() <= 1);

                const auto fbxMesh = static_cast<const FbxMesh*>(attribute);
                check(fbxMesh->GetControlPointsCount() > 0);
                check(fbxMesh->GetPolygonVertexCount() > 0);

                const auto meshCache = saveMesh(fbxMesh);
                const auto vertexData = std::make_shared<VertexData>();

                vertexData->setTopology(PrimitiveTopology::triangles);
                vertexData->setVertices(Semantics::POSITION, 0,
                    meshCache.positions.data(), meshCache.positions.size() * sizeof(float), PixelFormat::RGB32_float);
                vertexData->setIndices(meshCache.indices.data(), meshCache.indices.size() * sizeof(unsigned short));

                if (!meshCache.normals.empty())
                {
                    vertexData->setVertices(Semantics::NORMAL, 0,
                        meshCache.normals.data(), meshCache.normals.size() * sizeof(float), PixelFormat::RGB32_float);
                }
                for (size_t iUV = 0; iUV < meshCache.uvSets.size(); ++iUV)
                {
                    vertexData->setVertices(Semantics::TEXCOORD, iUV,
                        meshCache.uvSets[iUV].uv.data(), meshCache.uvSets[iUV].uv.size() * sizeof(float), PixelFormat::RG32_float);
                }
                if (!meshCache.colors.empty())
                {
                    vertexData->setVertices(Semantics::COLOR, 0,
                        meshCache.colors.data(), meshCache.colors.size() * sizeof(float), PixelFormat::RGBA32_float);
                }

                MaterialCache materialCache = MaterialCache::DEFAULT;
                if (node->GetMaterialCount() > 0)
                {
                    check(node->GetMaterialCount() == 1);
                    const auto fbxMaterial = node->GetMaterial(0);
                    saveMaterial(fbxMaterial, materialCache);
                }

                const auto textureFlagFilter = [&meshCache](const ColorValue& val, unsigned flag)
                {
                    if (!val.texPath.empty())
                    {
                        for (int iUV = 0; iUV < meshCache.uvSets.size(); ++iUV)
                        {
                            if (val.uvName == meshCache.uvSets[iUV].uvName)
                            {
                                return flag;
                            }
                        }
                    }
                    return TEX_NONE;
                };

                const auto setTexture = [](const ColorValue& val, const std::string& name, const ResourceInterface<Material>& material)
                {
                    const auto tex = resourceTable.template obtain<MediaTexture>(val.texPath);
                    tex->load();
                    material->setTex2D(name, tex);
                };

                const auto textureFlags =
                    textureFlagFilter(materialCache.ambient, TEX_AMBIENT) |
                    textureFlagFilter(materialCache.diffuse, TEX_DIFFUSE) |
                    textureFlagFilter(materialCache.emissive, TEX_EMISSIVE) |
                    textureFlagFilter(materialCache.specular, TEX_SPECULAR);

                check(MATERIAL_TABLE.find(textureFlags) != std::cend(MATERIAL_TABLE));
                const auto materialPath = MATERIAL_TABLE.find(textureFlags)->second;

                auto material = resourceTable.template obtain<Material>(materialPath);
                material->load();
                material = material->duplicate();

                material->setFloat4("ambient", materialCache.ambient.color);
                material->setFloat4("diffuse", materialCache.diffuse.color);
                material->setFloat4("emissive", materialCache.emissive.color);
                material->setFloat4("specular", materialCache.specular.color);
                material->setFloat("power", materialCache.power);

                if (textureFlags & TEX_AMBIENT) setTexture(materialCache.ambient, "ambientTexture", material);
                if (textureFlags & TEX_DIFFUSE) setTexture(materialCache.diffuse, "diffuseTexture", material);
                if (textureFlags & TEX_EMISSIVE) setTexture(materialCache.emissive, "emissiveTexture", material);
                if (textureFlags & TEX_SPECULAR) setTexture(materialCache.specular, "specularTexture", material);

                Surface surface;
                surface.vertices = vertexData;
                surface.material = material;
                surface.indexOffset = 0;
                surface.indexCount = meshCache.indices.size();

                mesh.addSurface(surface);
            }

            stack.pop();
            const auto numChildren = top->GetChildCount();
            for (int i = 0; i < numChildren; ++i)
            {
                stack.emplace(top->GetChild(i));
            }
        }
    }
}

FbxException::FbxException(const std::string& msg)
    : Exception(msg)
{
}

FbxImport fbxImport;

void FbxImport::startup()
{
    fbxManager_ = makeFbxPtr(FbxManager::Create());
    const auto ios = FbxIOSettings::Create(fbxManager_.get(), IOSROOT);
    fbxManager_->SetIOSettings(ios);
}

void FbxImport::shutdown()
{
    fbxManager_.reset();
}

void FbxImport::importMesh(const std::string& path, Mesh& mesh)
{
    /// TODO: _fullpath() is windows only
    char fullpash[512];
    _fullpath(fullpash, path.c_str(), 512);

    const auto importer = makeFbxPtr(FbxImporter::Create(fbxManager_.get(), ""));
    enforce<FbxException>(
        importer->Initialize(fullpash, -1, fbxManager_->GetIOSettings()),
        "Failed to initialize FbxImporter (" + path + ").\n" +
        std::string(importer->GetStatus().GetErrorString()));

    const auto scene = makeFbxPtr(FbxScene::Create(fbxManager_.get(), ""));
    enforce<FbxException>(
        importer->Import(scene.get()),
        "Failed to import FbxScene (" + path + ").\n" +
        std::string(importer->GetStatus().GetErrorString()));

    const auto axis = scene->GetGlobalSettings().GetAxisSystem();
    if (axis != FbxAxisSystem::DirectX)
    {
        FbxAxisSystem::DirectX.ConvertScene(scene.get());
    }

    FbxGeometryConverter converter(fbxManager_.get());
    converter.Triangulate(scene.get(), true);
    converter.SplitMeshesPerMaterial(scene.get(), true);

    importMeshScene(scene->GetRootNode(), mesh);
}

GF_NAMESPACE_END