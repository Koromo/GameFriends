#ifndef GAMEFRIENDS_GAMEFRIENDS_H
#define GAMEFRIENDS_GAMEFRIENDS_H

#include "framework/application.h"
#include "framework/physicsdebug.h"
#include "framework/input.h"

#include "physics/collisionshape.h"
#include "physics/physicsworld.h"
#include "physics/rigidbody.h"
#include "physics/bulletinc.h"

#include "scene/debugdraw.h"
#include "scene/material.h"
#include "scene/mesh.h"
#include "scene/scene.h"
#include "scene/shademodel.h"
#include "scene/texture.h"

#include "render/descriptorheap.h"
#include "render/drawcall.h"
#include "render/gpucommand.h"
#include "render/linearallocator.h"
#include "render/pipeline.h"
#include "render/pixelbuffer.h"
#include "render/renderstate.h"
#include "render/rendersystem.h"
#include "render/rootsignature.h"
#include "render/shaderprogram.h"
#include "render/vertexdata.h"
#include "render/d3dsupport.h"

#include "audio/soundclip.h"
#include "audio/soundvoice.h"
#include "audio/audiomanager.h"
#include "audio/audioinc.h"

#include "windowing/console.h"
#include "windowing/window.h"
#include "windowing/windowsinc.h"

#include "engine/codec.h"
#include "engine/filesystem.h"
#include "engine/pixelformat.h"
#include "engine/resource.h"

#include "../foundation/src/foundation/any.h"
#include "../foundation/src/foundation/sortedvector.h"
#include "../foundation/src/foundation/axisalignedbox.h"
#include "../foundation/src/foundation/color.h"
#include "../foundation/src/foundation/frustum.h"
#include "../foundation/src/foundation/matrix44.h"
#include "../foundation/src/foundation/plane.h"
#include "../foundation/src/foundation/quaternion.h"
#include "../foundation/src/foundation/vector2.h"
#include "../foundation/src/foundation/vector3.h"
#include "../foundation/src/foundation/vector4.h"
#include "../foundation/src/foundation/math.h"
#include "../foundation/src/foundation/metaprop.h"
#include "../foundation/src/foundation/string.h"
#include "../foundation/src/foundation/traits.h"
#include "../foundation/src/foundation/utility.h"
#include "../foundation/src/foundation/exception.h"
#include "../foundation/src/foundation/prerequest.h"

#ifndef GF_NAMESPACE_USING_NO
using namespace GF_NAMESPACE;
#endif

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "libfbxsdk-mt.lib")
#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Shlwapi.lib")

#pragma comment(lib, "BulletCollision_vs2010_debug.lib")
#pragma comment(lib, "BulletDynamics_vs2010_debug.lib")
#pragma comment(lib, "LinearMath_vs2010_debug.lib")

#endif