#ifndef GAMEFRIENDS_FBXIMPORT_H
#define GAMEFRIENDS_FBXIMPORT_H

#include "../engine/resource.h"
#include "foundation/exception.h"
#include "foundation/prerequest.h"
#define FBXSDK_NAMESPACE_USING 0
#include <fbxsdk.h>
#include <memory>

namespace fbx = FBXSDK_NAMESPACE;
GF_NAMESPACE_BEGIN

class Mesh;

class FbxException : public Exception
{
public:
    explicit FbxException(const std::string& msg);
};

template <class T>
using FbxPtr = std::shared_ptr<T>;

template <class T>
FbxPtr<T> makeFbxPtr(T* p)
{
    struct D
    {
        void operator()(T* p)
        {
            p->Destroy();
        }
    };

    return FbxPtr<T>(p, D());
}

class FbxImport
{
private:
    FbxPtr<fbx::FbxManager> fbxManager_;

public:
    void startup();
    void shutdown();
    void importMesh(const std::string& path, Mesh& mesh);
};

extern FbxImport fbxImport;

GF_NAMESPACE_END


#endif