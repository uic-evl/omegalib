/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory,  
 * University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer. Redistributions in binary 
 * form must reproduce the above copyright notice, this list of conditions and 
 * the following disclaimer in the documentation and/or other materials provided 
 * with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file
 *	The wrapper code for the omegalib python API. 
 ******************************************************************************/
#include "omega/PythonInterpreter.h"
#include "omega/SystemManager.h"
#include "omega/DisplaySystem.h"
#include "omega/EqualizerDisplaySystem.h"
#include "omega/Engine.h"
#include "omega/Actor.h"
#include "omega/ImageUtils.h"
#include "omega/CameraController.h"
#include "omega/MissionControl.h"

#ifdef OMEGA_USE_PYTHON

#include "omega/PythonInterpreterWrapper.h"

#include <boost/mpl/if.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#define PYCAP_GET(pyobj, className) pyobj != NULL ? (className*)PyCapsule_GetPointer(pyobj, #className) : NULL

using namespace omega;

//! Static instance of ScriptRendererCommand, used by rendererQueueCommand
//ScriptRendererCommand* sScriptRendererCommand = NULL;

PyObject* sEuclidModule = NULL;

bool sRefPtrForwardingEnabled = false;
void enableRefPtrForwarding() { sRefPtrForwardingEnabled = true; }
void disableRefPtrForwarding() { sRefPtrForwardingEnabled = false; }
bool isRefPtrForwardingEnabled() { return sRefPtrForwardingEnabled; }


///////////////////////////////////////////////////////////////////////////////
PyObject* omegaExit(PyObject* self, PyObject* args)
{
    // Gracious exit code is broken. 
    SystemManager::instance()->postExitRequest();
    
    // Do it the control-C way (it always works)
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    // Explicitly kill sound server
    SoundEnvironment* se = Engine::instance()->getSoundEnvironment();
    if(se != NULL)
    {
        se->getSoundManager()->stopAllSounds();
        se->getSoundManager()->cleanupAllSounds();
    }
    Py_INCREF(Py_None);
    return Py_None;
}

///////////////////////////////////////////////////////////////////////////////
static PyObject* omegaFindFile(PyObject* self, PyObject* args)
{
    const char* name;
    if(!PyArg_ParseTuple(args, "s", &name)) return NULL;

    DataManager* dm = SystemManager::instance()->getDataManager();
    DataInfo info = dm->getInfo(name);

    if(info.isNull())
    {
        return Py_BuildValue("s", "");
    }

    return Py_BuildValue("s", info.path.c_str());
}

///////////////////////////////////////////////////////////////////////////////
static PyObject* omegaUpdateCallback(PyObject *dummy, PyObject *args)
{
    PyObject *result = NULL;
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O", &temp)) 
    {
        if (!PyCallable_Check(temp)) 
        {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }

        PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
        interp->registerCallback(temp, PythonInterpreter::CallbackUpdate);

        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
static PyObject* omegaEventCallback(PyObject *dummy, PyObject *args)
{
    PyObject *result = NULL;
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O", &temp)) 
    {
        if (!PyCallable_Check(temp)) 
        {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }

        PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
        interp->registerCallback(temp, PythonInterpreter::CallbackEvent);

        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
static PyObject* omegaDrawCallback(PyObject *dummy, PyObject *args)
{
    PyObject *result = NULL;
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O", &temp)) 
    {
        if (!PyCallable_Check(temp)) 
        {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }

        PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
        interp->registerCallback(temp, PythonInterpreter::CallbackDraw);

        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
class ScriptNodeListener: public SceneNodeListener
{
public:
    enum Type { VisibleListener, SelectedListener };

    Type type;
    String command;

    virtual void onVisibleChanged(SceneNode* source, bool value) 
    {
        if(type == VisibleListener)
        {
            PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
            if(interp != NULL)
            {
                interp->eval(command);
            }
        }
    }

    virtual void onSelectedChanged(SceneNode* source, bool value) 
    {
        if(type == SelectedListener)
        {
            PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
            if(interp != NULL)
            {
                interp->eval(command);
            }
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
PyObject* addVisibilityListener(PyObject* self, PyObject* args)
{
    PyObject* pyNode = NULL;
    const char* cmd;
    PyArg_ParseTuple(args, "Os", &pyNode, &cmd);

    if(pyNode != NULL && cmd != NULL)
    {
        SceneNode* node = (SceneNode*)PyCapsule_GetPointer(pyNode, "node");

        ScriptNodeListener* listener = new ScriptNodeListener();
        listener->type = ScriptNodeListener::VisibleListener;
        listener->command = cmd;
        node->addListener(listener);

        Py_INCREF(Py_None);
        return Py_None;
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
PyObject* addSelectionListener(PyObject* self, PyObject* args)
{
    PyObject* pyNode = NULL;
    const char* cmd;
    PyArg_ParseTuple(args, "Os", &pyNode, &cmd);

    if(pyNode != NULL && cmd != NULL)
    {
        SceneNode* node = (SceneNode*)PyCapsule_GetPointer(pyNode, "node");

        ScriptNodeListener* listener = new ScriptNodeListener();
        listener->type = ScriptNodeListener::SelectedListener;
        listener->command = cmd;
        node->addListener(listener);

        Py_INCREF(Py_None);
        return Py_None;
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
static PyMethodDef omegaMethods[] = 
{
    {"addSelectionListener", addSelectionListener, METH_VARARGS, 
        "addSelectionListener(node, cmd)\n"
        "Attaches a command to be executed whenever the node selection state changes."},

    {"addVisibilityListener", addVisibilityListener, METH_VARARGS, 
        "addVisibilityListener(node, cmd)\n"
        "Attaches a command to be executed whenever the node visibility changes."},

    // Base omegalib API
    {"oexit", omegaExit, METH_VARARGS, 
        "oexit()\n"
        "Terminates the current omegalib program"},

    {"ofindFile", omegaFindFile, METH_VARARGS, 
        "ofindFile(name)\n"
        "Searches for a file in the application data filesystems and returns a full path if found"},

    {"setUpdateFunction", omegaUpdateCallback, METH_VARARGS, 
        "setUpdateFunction(funcRef)\n"
        "Registers a script function to be called before each frame is rendered"},

    {"setEventFunction", omegaEventCallback, METH_VARARGS, 
        "setEventFunction(funcRef)\n"
        "Registers a script function to be called when events are received"},

    {"setDrawFunction", omegaDrawCallback, METH_VARARGS, 
        "setDrawFunction(funcRef)\n"
        "Registers a script function to be called when drawing"},

    {NULL, NULL, 0, NULL}
};

///////////////////////////////////////////////////////////////////////////////
void unregisterFrameCallbacks() 
{
    PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
    interp->unregisterAllCallbacks();
}

///////////////////////////////////////////////////////////////////////////////
Engine* getEngine() { return Engine::instance(); }

// Used to make the getEvent call work for Actors.
// This will be set by the Actor::onEvent call before running the python callback.
static const Event* sLocalEvent = NULL;

///////////////////////////////////////////////////////////////////////////////
const Event* getEvent() 
{ 
    if(sLocalEvent != NULL) return sLocalEvent;
    return PythonInterpreter::getLastEvent(); 
}

///////////////////////////////////////////////////////////////////////////////
void printChildrenHelper(Node* n, int depth, const String& prefix, const String& indentString)
{
    if(depth != 0)
    {
        foreach(Node* child, n->getChildren())
        {
            omsg(prefix + child->getName());
            if(child->numChildren() != 0)
            {
                printChildrenHelper(child, depth - 1, prefix + indentString, indentString);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void printChildren(Node* node, int depth)
{
    if(node != NULL)
    {
        printChildrenHelper(node, depth, " ", " ");
    }
}

///////////////////////////////////////////////////////////////////////////////
void printObjCounts()
{
    ReferenceType::printObjCounts();
}

///////////////////////////////////////////////////////////////////////////////
const bool getBoolSetting(const String& section, const String& name, bool defaultValue)
{
    if(SystemManager::settingExists(section))
    {
        const Setting& s = SystemManager::settingLookup(section);
        return Config::getBoolValue(name, s, defaultValue);
    }
    return defaultValue;
}

///////////////////////////////////////////////////////////////////////////////
const String getStringSetting(const String& section, const String& name, String defaultValue)
{
    if(SystemManager::settingExists(section))
    {
        const Setting& s = SystemManager::settingLookup(section);
        return Config::getStringValue(name, s, defaultValue);
    }
    return defaultValue;
}

///////////////////////////////////////////////////////////////////////////////
const Event::Flags getButtonSetting(
    const String& section, const String& name, Event::Flags defaultValue)
{
    if(SystemManager::settingExists(section))
    {
        const Setting& s = SystemManager::settingLookup(section);
        return Event::parseButtonName(Config::getStringValue(name, s, ""));
    }
    return defaultValue;
}

///////////////////////////////////////////////////////////////////////////////
void queueCommand(const String& command)
{
    PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
    // Mark the command as queued locally, since we expect queueCommand to be executed on all nodes
    // when running in a distributed environment
    interp->queueCommand(command, true);
}

///////////////////////////////////////////////////////////////////////////////
void broadcastCommand(const String& command)
{
    // This only runs on the master node and sends the command to all slaves. Use the queeuCommand
    // interpreter function again, but this time don't mark the command as local, so it will be sent
    // to all nodes.
    if(SystemManager::instance()->isMaster())
    {
        PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
        // Mark the command as queued locally, since we expect queueCommand to be executed on all nodes
        // when running in a distributed environment
        interp->queueCommand(command);
    }
}

///////////////////////////////////////////////////////////////////////////////
// @internal
struct Vector3f_to_python
{
    static PyObject* convert(Vector3f const& value)
    {
        // If we haven't looked up for the Vector3 class, let's do it now.
        static PyObject* sVector3Class = NULL;
        if(sVector3Class == NULL)
        {
            PyObject* moduleDict = PyModule_GetDict(sEuclidModule);
            sVector3Class = PyDict_GetItemString(moduleDict, "Vector3");
        }

        // Create a new euclid.Vector3 instance using the omega::Vector3f components
        // as arguments.
        boost::python::tuple vec = boost::python::make_tuple(value[0], value[1], value[2]);
        PyObject* vector3obj = PyObject_CallObject(sVector3Class, vec.ptr());
        return incref(vector3obj);
    }
};

///////////////////////////////////////////////////////////////////////////////
// @internal
struct Vector3f_from_python
{
    Vector3f_from_python()
    {
        converter::registry::push_back(&convertible, &construct, type_id<Vector3f>());
    }

    static void* convertible(PyObject* obj)
    {
        // We don't really care if the object is of type Vector3. We just require
        // it to have x, y, z attributes.
        if(!PyObject_HasAttrString(obj, "x") ||
            !PyObject_HasAttrString(obj, "y") ||
            !PyObject_HasAttrString(obj, "z")) return 0;
        return obj;
    }

    static void construct(PyObject* obj, converter::rvalue_from_python_stage1_data* data)
    {
        float x = extract<float>(PyObject_GetAttrString(obj, "x"));
        float y = extract<float>(PyObject_GetAttrString(obj, "y"));
        float z = extract<float>(PyObject_GetAttrString(obj, "z"));

        void* storage = (
            (converter::rvalue_from_python_storage<Vector3f>*)data)->storage.bytes;
        new (storage) Vector3f(x, y, z);
        data->convertible = storage;
    }
};

///////////////////////////////////////////////////////////////////////////////
// @internal
struct Vector2f_to_python
{
    static PyObject* convert(Vector2f const& value)
    {
        // If we haven't looked up for the Vector3 class, let's do it now.
        static PyObject* sVector2Class = NULL;
        if(sVector2Class == NULL)
        {
            PyObject* moduleDict = PyModule_GetDict(sEuclidModule);
            sVector2Class = PyDict_GetItemString(moduleDict, "Vector2");
        }

        // Create a new euclid.Vector3 instance using the omega::Vector3f components
        // as arguments.
        boost::python::tuple vec = boost::python::make_tuple(value[0], value[1]);
        PyObject* vector2obj = PyObject_CallObject(sVector2Class, vec.ptr());
        return incref(vector2obj);
    }
};

///////////////////////////////////////////////////////////////////////////////
// @internal
struct Vector2f_from_python
{
    Vector2f_from_python()
    {
        converter::registry::push_back(&convertible, &construct, type_id<Vector2f>());
    }

    static void* convertible(PyObject* obj)
    {
        // We don't really care if the object is of type Vector3. We just require
        // it to have x, y attributes.
        if(!PyObject_HasAttrString(obj, "x") ||
            !PyObject_HasAttrString(obj, "y")) return 0;
        return obj;
    }

    static void construct(PyObject* obj, converter::rvalue_from_python_stage1_data* data)
    {
        float x = extract<float>(PyObject_GetAttrString(obj, "x"));
        float y = extract<float>(PyObject_GetAttrString(obj, "y"));

        void* storage = (
            (converter::rvalue_from_python_storage<Vector2f>*)data)->storage.bytes;
        new (storage) Vector2f(x, y);
        data->convertible = storage;
    }
};

///////////////////////////////////////////////////////////////////////////////
// @internal
struct Quaternion_to_python
{
    static PyObject* convert(Quaternion const& value)
    {
        // If we haven't looked up for the Quaternion class, let's do it now.
        static PyObject* sQuaternionClass = NULL;
        if(sQuaternionClass == NULL)
        {
            PyObject* moduleDict = PyModule_GetDict(sEuclidModule);
            sQuaternionClass = PyDict_GetItemString(moduleDict, "Quaternion");
        }

        // Create a new euclid.Quaternion instance using the omega::Quaternion components
        // as arguments.
        boost::python::tuple vec = boost::python::make_tuple(value.w(), value.x(), value.y(), value.z());
        PyObject* quatobj = PyObject_CallObject(sQuaternionClass, vec.ptr());
        return incref(quatobj);
    }
};

///////////////////////////////////////////////////////////////////////////////
// @internal
struct Quaternion_from_python
{
    Quaternion_from_python()
    {
        converter::registry::push_back(&convertible, &construct, type_id<Quaternion>());
    }

    static void* convertible(PyObject* obj)
    {
        // We don't really care if the object is of type Quaternion. We just require
        // it to have x, y, z attributes.
        if(!PyObject_HasAttrString(obj, "x") ||
            !PyObject_HasAttrString(obj, "y") ||
            !PyObject_HasAttrString(obj, "z") ||
            !PyObject_HasAttrString(obj, "w")) return 0;
        return obj;
    }

    static void construct(PyObject* obj, converter::rvalue_from_python_stage1_data* data)
    {
        float x = extract<float>(PyObject_GetAttrString(obj, "x"));
        float y = extract<float>(PyObject_GetAttrString(obj, "y"));
        float z = extract<float>(PyObject_GetAttrString(obj, "z"));
        float w = extract<float>(PyObject_GetAttrString(obj, "w"));

        void* storage = (
            (converter::rvalue_from_python_storage<Quaternion>*)data)->storage.bytes;
        new (storage) Quaternion(w, x, y, z);
        data->convertible = storage;
    }
};

///////////////////////////////////////////////////////////////////////////////
Quaternion quaternionFromEuler(float pitch, float yaw, float roll)
{
    return Math::quaternionFromEuler(Vector3f(pitch, yaw, roll));
}

///////////////////////////////////////////////////////////////////////////////
Quaternion quaternionFromEulerDeg(float pitch, float yaw, float roll)
{
    return Math::quaternionFromEuler(Vector3f(pitch, yaw, roll) * Math::DegToRad);
}

///////////////////////////////////////////////////////////////////////////////
Vector3f quaternionToEulerDeg(const Quaternion& q)
{
    return Math::quaternionToEuler(q) * Math::RadToDeg;
}

///////////////////////////////////////////////////////////////////////////////
Vector3f quaternionToEuler(const Quaternion& q)
{
    return Math::quaternionToEuler(q);
}

///////////////////////////////////////////////////////////////////////////////
void querySceneRay(
    const Vector3f& origin, const Vector3f& dir, 
    boost::python::object callback, uint flags = 0)
{
    const SceneQueryResultList& sqrl = Engine::instance()->querySceneRay(Ray(origin, dir), flags);
    boost::python::list l;
    if(sqrl.size() == 0)
    {
        callback(boost::python::ptr<SceneNode*>(NULL), 0);
    }
    else
    {
        foreach(SceneQueryResult sqr, sqrl)
        {
            callback(boost::python::ptr(sqr.node), sqr.distance);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
boost::python::tuple hitNode(SceneNode* node, const Vector3f& origin, const Vector3f& dir)
{
    if(node != NULL)
    {
        Vector3f hitPoint;
        bool hit = node->hit(Ray(origin, dir), &hitPoint, SceneNode::HitBest);
        return boost::python::make_tuple(hit, hitPoint);
    }
    return boost::python::make_tuple(false, Vector3f::Zero());
}

///////////////////////////////////////////////////////////////////////////////
boost::python::tuple getRayFromEvent(const Event* evt)
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    Ray r;
    bool res = ds->getViewRayFromEvent(*evt, r);
    return boost::python::make_tuple(res, r.getOrigin(), r.getDirection());
}

///////////////////////////////////////////////////////////////////////////////
boost::python::tuple getRayFromPoint(int x, int y)
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    Ray r = ds->getViewRay(Vector2i(x, y));
    return boost::python::make_tuple(true, r.getOrigin(), r.getDirection());
}

///////////////////////////////////////////////////////////////////////////////
Camera* getDefaultCamera()
{
    return Engine::instance()->getDefaultCamera();
}

///////////////////////////////////////////////////////////////////////////////
Camera* getCamera(const String& name)
{
    return Engine::instance()->getCamera(name);
}

///////////////////////////////////////////////////////////////////////////////
Camera* getCameraById(int id)
{
    return Engine::instance()->getCameraById(id);
}

///////////////////////////////////////////////////////////////////////////////
Camera* getOrCreateCamera(const String& name)
{
    Camera* cam = Engine::instance()->getCamera(name);
    if(cam == NULL)
    {
        cam = Engine::instance()->createCamera(name);
        // by default disable overlay drawing on secondary cameras.
        // This is partially due to conflicts when trying to render the same
        // ui on both the default and a secondary camera, so disabling 
        // overlays by default is the safe way to go.
        cam->setOverlayEnabled(false);
    }
    return cam;
}

///////////////////////////////////////////////////////////////////////////////
bool isMaster()
{
    return SystemManager::instance()->isMaster();
}

///////////////////////////////////////////////////////////////////////////////
String getHostname()
{
    return SystemManager::instance()->getHostname();
}

///////////////////////////////////////////////////////////////////////////////
bool isHostInTileSection(const String& hostname, int tilex, int tiley, int tilew, int tileh)
{
    DisplayConfig& dc = SystemManager::instance()->getDisplaySystem()->getDisplayConfig();
    return dc.isHostInTileSection(hostname, tilex, tiley, tilew, tileh);
}

///////////////////////////////////////////////////////////////////////////////
void setTilesEnabled(int tilex, int tiley, int tilew, int tileh, bool enabled)
{
    DisplayConfig& dc = SystemManager::instance()->getDisplaySystem()->getDisplayConfig();
    dc.setTilesEnabled(tilex, tiley, tilew, tileh, enabled);
}

///////////////////////////////////////////////////////////////////////////////
bool isEventDispatchEnabled()
{
    return Engine::instance()->isEventDispatchEnabled();
}

///////////////////////////////////////////////////////////////////////////////
void setEventDispatchEnabled(bool value)
{
    Engine::instance()->setEventDispatchEnabled(value);
}

///////////////////////////////////////////////////////////////////////////////
SceneNode* getScene()
{
    return Engine::instance()->getScene();
}

///////////////////////////////////////////////////////////////////////////////
SoundEnvironment* getSoundEnvironment()
{
    return Engine::instance()->getSoundEnvironment();
}

///////////////////////////////////////////////////////////////////////////////
bool isSoundEnabled()
{
    return Engine::instance()->isSoundEnabled();
}

///////////////////////////////////////////////////////////////////////////////
MissionControlClient* getMissionControlClient()
{
    return SystemManager::instance()->getMissionControlClient();
}

///////////////////////////////////////////////////////////////////////////////
void toggleStereo()
{
    SystemManager* sm = SystemManager::instance();
    EqualizerDisplaySystem* eqds = dynamic_cast<EqualizerDisplaySystem*>(sm->getDisplaySystem());
    if(eqds != NULL)
    {
        eqds->getDisplayConfig().forceMono = !eqds->getDisplayConfig().forceMono;
    }
}

///////////////////////////////////////////////////////////////////////////////
bool isStereoEnabled()
{
    SystemManager* sm = SystemManager::instance();
    EqualizerDisplaySystem* eqds = dynamic_cast<EqualizerDisplaySystem*>(sm->getDisplaySystem());
    if(eqds != NULL)
    {
        return !(eqds->getDisplayConfig().forceMono || eqds->getDisplayConfig().stereoMode == DisplayTileConfig::Mono);
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
DisplayConfig* getDisplayConfig()
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    return &ds->getDisplayConfig();
}

///////////////////////////////////////////////////////////////////////////////
vector<String> getTiles()
{
    vector<String> res;
    DisplayConfig& dc = SystemManager::instance()->getDisplaySystem()->getDisplayConfig();
    typedef KeyValue<String, DisplayTileConfig*> TileItem;
    foreach(TileItem ti, dc.tiles)
    {
        res.push_back(ti.getKey());
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////
void setTileCamera(const String& tilename, const String& cameraName)
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    DisplayConfig& dc = ds->getDisplayConfig();
    if(dc.tiles.find(tilename) != dc.tiles.end())
    {
        DisplayTileConfig* dtc = dc.tiles[tilename];
        dtc->cameraName = cameraName;
        // Create the camera here (this is guaranteed to happen on all nodes)
        dtc->camera = getOrCreateCamera(cameraName);
    }
    ds->refreshSettings();
}

///////////////////////////////////////////////////////////////////////////////
void setNearFarZ(float nearZ, float farZ)
{
    Camera* cam = getDefaultCamera();
    cam->setNearFarZ(nearZ, farZ);
}

///////////////////////////////////////////////////////////////////////////////
float getNearZ()
{
    Camera* cam = getDefaultCamera();
    return cam->getNearZ();
}

///////////////////////////////////////////////////////////////////////////////
float getFarZ()
{
    Camera* cam = getDefaultCamera();
    return cam->getFarZ();
}

///////////////////////////////////////////////////////////////////////////////
boost::python::tuple getDisplayPixelSize()
{
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    Vector2i size = ds->getCanvasSize();
    return boost::python::make_tuple(size.x(), size.y());
}

///////////////////////////////////////////////////////////////////////////////
void orun(const String& script)
{
    PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
    interp->runFile(script);
}

///////////////////////////////////////////////////////////////////////////////
void oclean()
{
    PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
    interp->clean();
}

///////////////////////////////////////////////////////////////////////////////
void ocleanrun(const String& script)
{
    PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
    interp->cleanRun(script);
}

///////////////////////////////////////////////////////////////////////////////
PixelData* loadImage(const String& filename)
{
    Ref<PixelData> data = ImageUtils::loadImage(filename);
    if(data != NULL)
    {
        enableRefPtrForwarding();
        data->ref();
        return data;
    }
    return NULL;
    //return ImageFile(filename, data);
}

///////////////////////////////////////////////////////////////////////////////
void resetDataPaths()
{
    DataManager* dm = DataManager::getInstance();
    dm->removeAllSources();

    // Always re-add search paths for relative and absolute paths.
    dm->addSource(new FilesystemDataSource("./"));
    dm->addSource(new FilesystemDataSource(""));
}

///////////////////////////////////////////////////////////////////////////////
void addDataPath(const String& path)
{
    DataManager* dm = DataManager::getInstance();
    dm->addSource(new FilesystemDataSource(path));
}

///////////////////////////////////////////////////////////////////////////////
void printDataPaths()
{
    DataManager* dm = DataManager::getInstance();
    omsg(dm->getDataSourceNames());
}

///////////////////////////////////////////////////////////////////////////////
void setImageLoaderThreads(int threads)
{
    ImageUtils::setImageLoaderThreads(threads);
}

///////////////////////////////////////////////////////////////////////////////
int getImageLoaderThreads()
{
    return ImageUtils::getImageLoaderThreads();
}

///////////////////////////////////////////////////////////////////////////////
void printModules()
{
    Vector<EngineModule*> mods = ModuleServices::getModules();
    
    omsg("Highest   Priority:");
    foreach(EngineModule* m, mods) 
    {
        if(m->getPriority() == EngineModule::PriorityHighest) 
            ofmsg("    %1%", %m->getName());
    }
    omsg("High   Priority:");
    foreach(EngineModule* m, mods) 
    {
        if(m->getPriority() == EngineModule::PriorityHigh) 
            ofmsg("    %1%", %m->getName());
    }
    omsg("Normal Priority:");
    foreach(EngineModule* m, mods) 
    {
        if(m->getPriority() == EngineModule::PriorityNormal) 
            ofmsg("    %1%", %m->getName());
    }
    omsg("Low    Priority:");
    foreach(EngineModule* m, mods) 
    {
        if(m->getPriority() == EngineModule::PriorityLow) 
            ofmsg("    %1%", %m->getName());
    }
    omsg("Lowest    Priority:");
    foreach(EngineModule* m, mods) 
    {
        if(m->getPriority() == EngineModule::PriorityLowest) 
            ofmsg("    %1%", %m->getName());
    }
}

///////////////////////////////////////////////////////////////////////////////
//! The ActorWrapper adds support for python overloading to the Actor class
class ActorPythonWrapper: public Actor, public wrapper<Actor>
{
public:
    ActorPythonWrapper(): Actor() { ModuleServices::addModule(this); }
    ActorPythonWrapper(const String& str): Actor(str) { ModuleServices::addModule(this); }

    void dispose()
    {
        if(override f = this->get_override("dispose")) f();
    }
    virtual void default_dispose()
    {
    }

    void onUpdate(const UpdateContext& context) 
    {
        try
        {
            if(override f = this->get_override("onUpdate")) 
            {
                // Python callback accepts frame, time, dt: same as global callback.
                f(context.frameNum, context.time, context.dt);
            }
            else Actor::onUpdate(context);
        }
        catch(const boost::python::error_already_set&)
        {
            PyErr_Print();
        }
    }
    void default_onUpdate(const UpdateContext& context) 
    { 
        this->Actor::onUpdate(context); 
    }

    void onEvent(const Event& evt)
    {
        try
        {
            // Call funtion with no arguments. The python event handler will use
            // getEvent() to retrieve the event data. This keeps actor callbacks
            // consistent with global event callbacks.
            sLocalEvent = &evt;
            if(override f = this->get_override("onEvent")) f();
            else Actor::onEvent(evt);
            sLocalEvent = NULL;
        }
        catch(const boost::python::error_already_set&)
        {
            PyErr_Print();
        }
    }
    void default_onEvent(const Event& evt)
    {
        this->Actor::onEvent(evt);
    }

    bool onCommand(const String& cmd)
    {
        try
        {
            if(override f = this->get_override("onCommand"))
                return f(cmd);
            return Actor::onCommand(cmd);
        }
        catch(const boost::python::error_already_set&)
        {
            PyErr_Print();
            return false;
        }
    }
    bool default_onCommand(const String& cmd)
    {
        return this->Actor::onCommand(cmd);
    }
};

///////////////////////////////////////////////////////////////////////////////
void setClearColor(const Color& color)
{
    SystemManager::instance()->getDisplaySystem()->setBackgroundColor(color);
}

///////////////////////////////////////////////////////////////////////////////
void clearColor(bool enabled)
{
    SystemManager::instance()->getDisplaySystem()->clearColor(enabled);
}

///////////////////////////////////////////////////////////////////////////////
void clearDepth(bool enabled)
{
    SystemManager::instance()->getDisplaySystem()->clearDepth(enabled);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(querySceneRayOverloads, querySceneRay, 3, 4);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(NodeYawOverloads, yaw, 1, 2) 
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(NodePitchOverloads, pitch, 1, 2) 
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(NodeRollOverloads, roll, 1, 2) 

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(CameraOutputReadbackOverloads, setReadbackTarget, 1, 2) 
///////////////////////////////////////////////////////////////////////////////
BOOST_PYTHON_MODULE(omega)
{
    // Font alignment
    PYAPI_ENUM(Font::Align, TextAlign)
            PYAPI_ENUM_VALUE(Font, HALeft)
            PYAPI_ENUM_VALUE(Font, HARight)
            PYAPI_ENUM_VALUE(Font, HACenter)
            PYAPI_ENUM_VALUE(Font, VATop)
            PYAPI_ENUM_VALUE(Font, VABottom)
            PYAPI_ENUM_VALUE(Font, VAMiddle)
            ;

    // Event type
    PYAPI_ENUM(Event::Type, EventType)
            PYAPI_ENUM_VALUE(Event, Select)
            PYAPI_ENUM_VALUE(Event,Toggle)
            PYAPI_ENUM_VALUE(Event,ChangeValue)
            PYAPI_ENUM_VALUE(Event,Update)
            PYAPI_ENUM_VALUE(Event,Move) 
            PYAPI_ENUM_VALUE(Event,Down)
            PYAPI_ENUM_VALUE(Event,Up)
            PYAPI_ENUM_VALUE(Event,Trace)
            PYAPI_ENUM_VALUE(Event,Connect)
            PYAPI_ENUM_VALUE(Event,Untrace)
            PYAPI_ENUM_VALUE(Event,Disconnect)
            PYAPI_ENUM_VALUE(Event,Click)
            PYAPI_ENUM_VALUE(Event,Zoom)
            PYAPI_ENUM_VALUE(Event,Split)
            PYAPI_ENUM_VALUE(Event,Rotate)
            PYAPI_ENUM_VALUE(Event,Null)
        ;

    // Event Flags
    PYAPI_ENUM(EventBase::Flags, EventFlags)
            PYAPI_ENUM_VALUE(EventBase, Left)
            PYAPI_ENUM_VALUE(EventBase,Button1)
            PYAPI_ENUM_VALUE(EventBase,Right)
            PYAPI_ENUM_VALUE(EventBase,Button2)
            PYAPI_ENUM_VALUE(EventBase,Middle)
            PYAPI_ENUM_VALUE(EventBase,Button3)
            PYAPI_ENUM_VALUE(EventBase,Ctrl)
            PYAPI_ENUM_VALUE(EventBase,SpecialButton1)
            PYAPI_ENUM_VALUE(EventBase,Alt)
            PYAPI_ENUM_VALUE(EventBase,SpecialButton2)
            PYAPI_ENUM_VALUE(EventBase,Shift)
            PYAPI_ENUM_VALUE(EventBase,SpecialButton3)
            PYAPI_ENUM_VALUE(EventBase,Button4)
            PYAPI_ENUM_VALUE(EventBase,Button5)
            PYAPI_ENUM_VALUE(EventBase,Button6)
            PYAPI_ENUM_VALUE(EventBase,Button7)
            PYAPI_ENUM_VALUE(EventBase,ButtonUp)
            PYAPI_ENUM_VALUE(EventBase,ButtonDown)
            PYAPI_ENUM_VALUE(EventBase,ButtonLeft)
            PYAPI_ENUM_VALUE(EventBase,ButtonRight)
            PYAPI_ENUM_VALUE(EventBase,Processed)
            PYAPI_ENUM_VALUE(EventBase,User)
        ;
    
    // Event Extra Data Type
    PYAPI_ENUM(EventBase::ExtraDataType, EventExtraDataType)
            PYAPI_ENUM_VALUE(EventBase,ExtraDataNull)
            PYAPI_ENUM_VALUE(EventBase,ExtraDataFloatArray)
            PYAPI_ENUM_VALUE(EventBase,ExtraDataIntArray)
            PYAPI_ENUM_VALUE(EventBase,ExtraDataVector3Array)
            PYAPI_ENUM_VALUE(EventBase,ExtraDataString)
        ;

    // Event Extra Data Type
    PYAPI_ENUM(Service::ServiceType, ServiceType)
            PYAPI_ENUM_VALUE(Service,Pointer)
            PYAPI_ENUM_VALUE(Service,Mocap)
            PYAPI_ENUM_VALUE(Service,Keyboard) 
            PYAPI_ENUM_VALUE(Service,Controller)
            PYAPI_ENUM_VALUE(Service,Ui) 
            PYAPI_ENUM_VALUE(Service,Generic)
            PYAPI_ENUM_VALUE(Service,Brain)
            PYAPI_ENUM_VALUE(Service,Wand) 
            ;

    // Event
    const Vector3f& (Event::*getPosition1)() const = &Event::getPosition;
    PYAPI_REF_BASE_CLASS(Event)
        PYAPI_METHOD(Event, isKeyDown)
        PYAPI_METHOD(Event, isKeyUp)
        PYAPI_METHOD(Event, isButtonDown)
        PYAPI_METHOD(Event, isButtonUp)
        PYAPI_METHOD(Event, isFlagSet)
        PYAPI_METHOD(Event, getAxis)
        PYAPI_METHOD(Event, getSourceId)
        PYAPI_METHOD(Event, getType)
        PYAPI_METHOD(Event, getServiceType)
        PYAPI_METHOD(Event, isProcessed)
        PYAPI_METHOD(Event, setProcessed)
        PYAPI_GETTER(Event, getPosition)
        PYAPI_GETTER(Event, getOrientation)
        ;

    PYAPI_ENUM(Node::TransformSpace, Space)
        .value("Local", Node::TransformLocal)
        .value("Parent", Node::TransformParent)
        .value("World", Node::TransformWorld);

    // Node
    void (Node::*setPosition1)(const Vector3f&) = &Node::setPosition;
    void (Node::*setPosition2)(float x, float y, float z) = &Node::setPosition;

    void (Node::*setScale1)(const Vector3f&) = &Node::setScale;
    void (Node::*setScale2)(float x, float y, float z) = &Node::setScale;

    void (Node::*setOrientation1)(const Quaternion&) = &Node::setOrientation;
    Node* (Node::*getChildByIndex)(unsigned short) const = &Node::getChild;
    Node* (Node::*getChildByName)(const String&) const = &Node::getChild;
    
    void (Node::*removeChildByRef)(Node*) = &Node::removeChild;
    void (Node::*removeChildByName)(const String&) = &Node::removeChild;
    void (Node::*removeChildByIndex)(unsigned short) = &Node::removeChild;

    void (Node::*rotate1)(const Vector3f& axis, const float& angle, Node::TransformSpace relativeTo) = &Node::rotate;
    void (Node::*rotate2)(const	Quaternion& q, Node::TransformSpace relativeTo) = &Node::rotate;
    void (Node::*translate1)(const Vector3f&, Node::TransformSpace relativeTo) = &Node::translate;
    void (Node::*translate2)(float, float, float, Node::TransformSpace relativeTo) = &Node::translate;

    class_<Node, Ref<Node>, boost::noncopyable >("Node", no_init)
        .def("getPosition", &Node::getPosition, PYAPI_RETURN_VALUE)
        .def("setPosition", setPosition1)
        .def("setPosition", setPosition2)
        .def("getScale", &Node::getScale, PYAPI_RETURN_VALUE)
        .def("setScale", setScale1)
        .def("setScale", setScale2)
        .def("setOrientation", setOrientation1)
        .def("getOrientation", &Node::getOrientation, PYAPI_RETURN_VALUE)
        .def("yaw", &Node::yaw, NodeYawOverloads())
        .def("pitch", &Node::pitch, NodePitchOverloads())
        .def("roll", &Node::roll, NodeRollOverloads())
        .def("rotate", rotate1)
        .def("rotate", rotate2)
        .def("translate", translate1)
        .def("translate", translate2)

        PYAPI_METHOD(Node, lookAt)
        PYAPI_METHOD(Node, numChildren)
        PYAPI_METHOD(Node, addChild)
        .def("getChildByName", getChildByName, PYAPI_RETURN_REF)
        .def("getChildByIndex", getChildByIndex, PYAPI_RETURN_REF)
        .def("removeChildByRef", removeChildByRef)
        .def("removeChildByName", removeChildByName)
        .def("removeChildByIndex", removeChildByIndex)

        PYAPI_METHOD(Node, resetOrientation)
        PYAPI_GETTER(Node, getName)
        PYAPI_METHOD(Node, setName)
        PYAPI_REF_GETTER(Node, getParent)
        PYAPI_GETTER(Node, convertLocalToWorldPosition)
        PYAPI_GETTER(Node, convertLocalToWorldOrientation)
        PYAPI_GETTER(Node, convertWorldToLocalPosition)
        PYAPI_GETTER(Node, convertWorldToLocalOrientation)
        //.def("getChildren", &Node::getChildren, PYAPI_RETURN_REF)
        ;

    // NodeList
    //PYAPI_POINTER_LIST(Node, "NodeList")

    // SceneNode
    PYAPI_REF_CLASS(SceneNode, Node)
        PYAPI_STATIC_REF_GETTER(SceneNode, create)
        PYAPI_METHOD(SceneNode, isVisible)
        PYAPI_METHOD(SceneNode, setVisible)
        PYAPI_METHOD(SceneNode, setChildrenVisible)
        PYAPI_METHOD(SceneNode, isSelected)
        PYAPI_METHOD(SceneNode, setSelected)
        PYAPI_METHOD(SceneNode, isSelectable)
        PYAPI_METHOD(SceneNode, setSelectable)
        PYAPI_METHOD(SceneNode, isBoundingBoxVisible)
        PYAPI_METHOD(SceneNode, setBoundingBoxVisible)
        PYAPI_METHOD(SceneNode, setTag)
        PYAPI_GETTER(SceneNode, getTag)
        PYAPI_GETTER(SceneNode, setFacingCamera)
        PYAPI_GETTER(SceneNode, getFacingCamera)
        PYAPI_GETTER(SceneNode, setFacingCameraFixedY)
        PYAPI_GETTER(SceneNode, isFacingCameraFixedY)
        PYAPI_GETTER(SceneNode, getBoundMinimum)
        PYAPI_GETTER(SceneNode, getBoundMaximum)
        PYAPI_GETTER(SceneNode, getBoundCenter)
        PYAPI_GETTER(SceneNode, getBoundRadius)
        PYAPI_METHOD(SceneNode, followTrackable)
        PYAPI_METHOD(SceneNode, setFollowOffset)
        PYAPI_METHOD(SceneNode, unfollow)
        PYAPI_METHOD(SceneNode, setFlag)
        PYAPI_METHOD(SceneNode, unsetFlag)
        PYAPI_METHOD(SceneNode, isFlagSet)
    ;

    // CameraController
    PYAPI_REF_BASE_CLASS(CameraController)
        PYAPI_METHOD(CameraController, getSpeed)
        PYAPI_METHOD(CameraController, setSpeed)
        PYAPI_METHOD(CameraController, reset)
    ;

    PYAPI_ENUM(DisplayTileConfig::StereoMode, StereoMode)
            PYAPI_ENUM_VALUE(DisplayTileConfig, Mono)
            PYAPI_ENUM_VALUE(DisplayTileConfig, LineInterleaved)
            PYAPI_ENUM_VALUE(DisplayTileConfig, ColumnInterleaved)
            PYAPI_ENUM_VALUE(DisplayTileConfig, PixelInterleaved)
            PYAPI_ENUM_VALUE(DisplayTileConfig, SideBySide)
            PYAPI_ENUM_VALUE(DisplayTileConfig, Default)
            ;

    PYAPI_REF_BASE_CLASS(DisplayTileConfig)
        .def_readwrite("enabled", &DisplayTileConfig::enabled)
        .def_readwrite("topLeft", &DisplayTileConfig::topLeft)
        .def_readwrite("bottomLeft", &DisplayTileConfig::bottomLeft)
        .def_readwrite("bottomRight", &DisplayTileConfig::bottomRight)
        .def_readwrite("stereoMode", &DisplayTileConfig::stereoMode)
        PYAPI_METHOD(DisplayTileConfig, setCorners)
        PYAPI_METHOD(DisplayTileConfig, setPixelSize)
        ;

    // DisplayConfig
    PYAPI_REF_BASE_CLASS(DisplayConfig)
        .def_readwrite("forceMono", &DisplayConfig::forceMono)
        .def_readwrite("stereoMode", &DisplayConfig::stereoMode)
        .def_readwrite("panopticStereoEnabled", &DisplayConfig::panopticStereoEnabled)
        ;

    // CameraOutput
    PYAPI_REF_BASE_CLASS(CameraOutput)
        PYAPI_METHOD(CameraOutput, setEnabled)
        PYAPI_METHOD(CameraOutput, isEnabled)
        .def("setReadbackTarget", &CameraOutput::setReadbackTarget, CameraOutputReadbackOverloads())
        ;

    PYAPI_ENUM(Camera::ViewMode, ViewMode)
            PYAPI_ENUM_VALUE(Camera, Immersive)
            PYAPI_ENUM_VALUE(Camera, Classic)
         ;

    // Camera
    PYAPI_REF_CLASS(Camera, SceneNode)
        PYAPI_REF_GETTER(Camera, getOutput)
        PYAPI_METHOD(Camera, setEnabled)
        PYAPI_METHOD(Camera, isEnabled)
        PYAPI_REF_GETTER(Camera, getCustomTileConfig)
        PYAPI_REF_GETTER(Camera, getController)
        PYAPI_METHOD(Camera, setController)
        PYAPI_METHOD(Camera, setPitchYawRoll)
        PYAPI_GETTER(Camera, getHeadOffset)
        PYAPI_METHOD(Camera, setHeadOffset)
        PYAPI_METHOD(Camera, setHeadOrientation)
        PYAPI_GETTER(Camera, getHeadOrientation)
        PYAPI_METHOD(Camera, setEyeSeparation)
        PYAPI_METHOD(Camera, getEyeSeparation)
        PYAPI_METHOD(Camera, isTrackingEnabled)
        PYAPI_METHOD(Camera, setTrackingEnabled)
        PYAPI_METHOD(Camera, getTrackerSourceId)
        PYAPI_METHOD(Camera, setTrackerSourceId)
        PYAPI_METHOD(Camera, setControllerEnabled)
        PYAPI_METHOD(Camera, isControllerEnabled)
        PYAPI_METHOD(Camera, localToWorldPosition)
        PYAPI_METHOD(Camera, localToWorldOrientation)
        PYAPI_METHOD(Camera, focusOn)
        PYAPI_METHOD(Camera, setViewPosition)
        PYAPI_GETTER(Camera, getViewPosition)
        PYAPI_METHOD(Camera, setViewSize)
        PYAPI_GETTER(Camera, getViewSize)
        PYAPI_METHOD(Camera, setViewMode)
        PYAPI_GETTER(Camera, getViewMode)
        PYAPI_METHOD(Camera, getCameraId)
        PYAPI_METHOD(Camera, setMask)
        PYAPI_METHOD(Camera, getMask)
        PYAPI_METHOD(Camera, isSceneEnabled)
        PYAPI_METHOD(Camera, setSceneEnabled)
        PYAPI_METHOD(Camera, isOverlayEnabled)
        PYAPI_METHOD(Camera, setOverlayEnabled)
        PYAPI_METHOD(Camera, setNearFarZ)
        PYAPI_METHOD(Camera, getNearZ)
        PYAPI_METHOD(Camera, getFarZ)
        ;

    // Color
    class_<Color>("Color", init<String>())
        .def(init<float, float, float, float>())
        .add_property("red", &Color::getRed, &Color::setRed)
        .add_property("green", &Color::getGreen, &Color::setGreen)
        .add_property("blue", &Color::getBlue, &Color::setBlue)
        .add_property("alpha", &Color::getAlpha, &Color::setAlpha);

    // Actor
    //PYAPI_REF_BASE_CLASS(Actor)
    class_<ActorPythonWrapper, boost::noncopyable, Ref<ActorPythonWrapper> >
        ("Actor")
        .def(init<const String&>())
        PYAPI_METHOD(Actor, setSceneNode)
        PYAPI_REF_GETTER(Actor, getSceneNode)
        PYAPI_METHOD(Actor, isUpdateEnabled)
        PYAPI_METHOD(Actor, setUpdateEnabled)
        PYAPI_METHOD(Actor, setCommandsEnabled)
        PYAPI_METHOD(Actor, areCommandsEnabled)
        PYAPI_METHOD(Actor, setEventsEnabled)
        PYAPI_METHOD(Actor, areEventsEnabled)
        PYAPI_METHOD(Actor, kill)
        // Overridable methods
        .def("onUpdate", &Actor::onUpdate, &ActorPythonWrapper::default_onUpdate)
        .def("onEvent", &Actor::onEvent, &ActorPythonWrapper::default_onEvent)
        .def("onCommand", &Actor::onCommand, &ActorPythonWrapper::default_onCommand)
        .def("dispose", &EngineModule::dispose, &ActorPythonWrapper::default_dispose)
        ;

    // DrawInterface
    PYAPI_REF_BASE_CLASS(DrawInterface)
        PYAPI_METHOD(DrawInterface, drawRectGradient)
        PYAPI_METHOD(DrawInterface, drawRect)
        PYAPI_METHOD(DrawInterface, drawRectOutline)
        PYAPI_METHOD(DrawInterface, drawText)
        PYAPI_METHOD(DrawInterface, drawRectTexture)
        PYAPI_METHOD(DrawInterface, drawCircleOutline)
        PYAPI_REF_GETTER(DrawInterface, createFont)
        PYAPI_REF_GETTER(DrawInterface, getFont)
        PYAPI_REF_GETTER(DrawInterface, getDefaultFont)
        ;

    // Font
    PYAPI_REF_BASE_CLASS(Font)
        PYAPI_METHOD(Font, computeSize)
        ;

    // PixelFormat
    PYAPI_ENUM(PixelData::Format, PixelFormat)
            PYAPI_ENUM_VALUE(PixelData, FormatRgb)
            PYAPI_ENUM_VALUE(PixelData, FormatRgba)
            PYAPI_ENUM_VALUE(PixelData, FormatMonochrome)
            ;

    // ImageFormat
    PYAPI_ENUM(ImageUtils::ImageFormat, ImageFormat)
            PYAPI_ENUM_VALUE(ImageUtils, FormatNone)
            PYAPI_ENUM_VALUE(ImageUtils, FormatPng)
            PYAPI_ENUM_VALUE(ImageUtils, FormatJpeg)
            ;

    // PixelData
    PYAPI_REF_BASE_CLASS(PixelData)
        PYAPI_STATIC_REF_GETTER(PixelData, create)
        PYAPI_METHOD(PixelData, getWidth)
        PYAPI_METHOD(PixelData, getHeight)
        PYAPI_METHOD(PixelData, beginPixelAccess)
        PYAPI_METHOD(PixelData, setPixel)
        PYAPI_METHOD(PixelData, getPixelR)
        PYAPI_METHOD(PixelData, getPixelG)
        PYAPI_METHOD(PixelData, getPixelB)
        PYAPI_METHOD(PixelData, getPixelA)
        PYAPI_METHOD(PixelData, endPixelAccess)
        ;

    // SoundEnvironment
    PYAPI_REF_BASE_CLASS(SoundEnvironment)
        PYAPI_REF_GETTER(SoundEnvironment, loadSoundFromFile)
        PYAPI_REF_GETTER(SoundEnvironment, setAssetDirectory)
        PYAPI_METHOD(SoundEnvironment, setWetness)
        PYAPI_METHOD(SoundEnvironment, getWetness)
        PYAPI_METHOD(SoundEnvironment, setRoomSize)
        PYAPI_METHOD(SoundEnvironment, getRoomSize)
        PYAPI_METHOD(SoundEnvironment, showDebugInfo)
        PYAPI_METHOD(SoundEnvironment, setSound)
        PYAPI_METHOD(SoundEnvironment, setVolumeScale)
        PYAPI_METHOD(SoundEnvironment, getVolumeScale)
        PYAPI_METHOD(SoundEnvironment, setServerVolume)
        PYAPI_METHOD(SoundEnvironment, getServerVolume)
        PYAPI_METHOD(SoundEnvironment, setForceCacheOverwrite)
        PYAPI_METHOD(SoundEnvironment, isForceCacheOverwriteEnabled)
        PYAPI_METHOD(SoundEnvironment, setSoundLoadWaitTime)
        PYAPI_METHOD(SoundEnvironment, getSoundLoadWaitTime)
        PYAPI_METHOD(SoundEnvironment, setUserPosition)
        PYAPI_METHOD(SoundEnvironment, getUserPosition)
        PYAPI_METHOD(SoundEnvironment, setUserOrientation)
        PYAPI_METHOD(SoundEnvironment, getUserOrientation)
        ;

    // Sound
    PYAPI_REF_BASE_CLASS(Sound)
        PYAPI_METHOD(Sound, getDuration)
        PYAPI_METHOD(Sound, getVolumeScale)
        PYAPI_METHOD(Sound, setVolumeScale)
        PYAPI_METHOD(Sound, resetToEnvironmentParameters)
        PYAPI_METHOD(Sound, isUsingEnvironmentParameters)
        ;

    // SoundInstance
    void (SoundInstance::*playSimple)() = &SoundInstance::play;
    void (SoundInstance::*playSimpleStereo)() = &SoundInstance::playStereo;
    class_<SoundInstance, boost::noncopyable, Ref<SoundInstance> >
        ("SoundInstance", init<Sound*>())
        .def("play", playSimple)
        .def("playStereo", playSimpleStereo)
        PYAPI_METHOD(SoundInstance, pause)
        PYAPI_METHOD(SoundInstance, stop)
        PYAPI_METHOD(SoundInstance, isPlaying)
        PYAPI_METHOD(SoundInstance, isDone)
        PYAPI_METHOD(SoundInstance, setLoop)
        PYAPI_METHOD(SoundInstance, getLoop)
        PYAPI_METHOD(SoundInstance, setPosition)
        PYAPI_GETTER(SoundInstance, getPosition)
        PYAPI_METHOD(SoundInstance, isEnvironmentSound)
        PYAPI_METHOD(SoundInstance, setEnvironmentSound)
        PYAPI_METHOD(SoundInstance, setVolume)
        PYAPI_METHOD(SoundInstance, getVolume)
        PYAPI_METHOD(SoundInstance, setWidth)
        PYAPI_METHOD(SoundInstance, getWidth)
        PYAPI_METHOD(SoundInstance, setWetness)
        PYAPI_METHOD(SoundInstance, getWetness)
        PYAPI_METHOD(SoundInstance, setRoomSize)
        PYAPI_METHOD(SoundInstance, getRoomSize)
        PYAPI_METHOD(SoundInstance, fade)
        PYAPI_METHOD(SoundInstance, setPitch)
        PYAPI_METHOD(SoundInstance, getPitch)
        PYAPI_METHOD(SoundInstance, setMaxDistance)
        PYAPI_METHOD(SoundInstance, getMaxDistance)
        PYAPI_METHOD(SoundInstance, setMinRolloffDistance)
        PYAPI_METHOD(SoundInstance, getMinRolloffDistance)
        PYAPI_METHOD(SoundInstance, setDistanceRange)
        PYAPI_METHOD(SoundInstance, setNoRolloff)
        PYAPI_METHOD(SoundInstance, setLinearRolloff)
        PYAPI_METHOD(SoundInstance, setLogarthmicRolloff)
        PYAPI_METHOD(SoundInstance, isRolloffEnabled)
        PYAPI_METHOD(SoundInstance, isRolloffLinear)
        PYAPI_METHOD(SoundInstance, isRolloffLogarithmic)
        ;

    // MissionControlClient
    PYAPI_REF_BASE_CLASS(MissionControlClient)
        PYAPI_STATIC_REF_GETTER(MissionControlClient, create)
        PYAPI_METHOD(MissionControlClient, connect)
        PYAPI_METHOD(MissionControlClient, dispose)
        PYAPI_METHOD(MissionControlClient, postCommand)
        PYAPI_METHOD(MissionControlClient, setName)
        PYAPI_METHOD(MissionControlClient, getName)
        .def("listConnectedClients", &MissionControlClient::listConnectedClients, PYAPI_RETURN_VALUE)
        PYAPI_METHOD(MissionControlClient, isConnected)
        PYAPI_METHOD(MissionControlClient, closeConnection)
        PYAPI_METHOD(MissionControlClient, setClientConnectedCommand)
        PYAPI_METHOD(MissionControlClient, setClientDisconnectedCommand)
        PYAPI_METHOD(MissionControlClient, setClientListUpdatedCommand)
        ;


    class_< vector<String> >("StringVector").def(vector_indexing_suite< vector<String> >());

    // Event Flags
    PYAPI_ENUM(SceneQuery::QueryFlags, QueryFlags)
            PYAPI_ENUM_VALUE(SceneQuery, QueryFirst)
            PYAPI_ENUM_VALUE(SceneQuery, QuerySort)
            ;

    // Statistic types
    PYAPI_ENUM(StatsManager::StatType, StatType)
            PYAPI_ENUM_VALUE(StatsManager, Time)
            PYAPI_ENUM_VALUE(StatsManager, Memory)
            PYAPI_ENUM_VALUE(StatsManager, Primitive)
            ;

    // Stat
    PYAPI_REF_BASE_CLASS(Stat)
        PYAPI_STATIC_REF_GETTER(Stat, create)
        PYAPI_STATIC_REF_GETTER(Stat, find)
        PYAPI_METHOD(Stat, startTiming)
        PYAPI_METHOD(Stat, stopTiming)
        PYAPI_METHOD(Stat, addSample)
        PYAPI_METHOD(Stat, getCur)
        PYAPI_METHOD(Stat, getMin)
        PYAPI_METHOD(Stat, getMax)
        PYAPI_METHOD(Stat, getAvg)
        ;

    // Free Functions
    def("unregisterFrameCallbacks", unregisterFrameCallbacks);
    
    def("getEvent", getEvent, return_value_policy<reference_existing_object>());
    def("getEngine", getEngine, PYAPI_RETURN_REF);
    def("getDefaultCamera", getDefaultCamera, PYAPI_RETURN_REF);
    def("getCamera", getCamera, PYAPI_RETURN_REF);
    def("getCameraById", getCameraById, PYAPI_RETURN_REF);
    def("getOrCreateCamera", getOrCreateCamera, PYAPI_RETURN_REF);
    def("getScene", getScene, PYAPI_RETURN_REF);
    def("getSoundEnvironment", getSoundEnvironment, PYAPI_RETURN_REF);
    def("isSoundEnabled", isSoundEnabled);
    def("querySceneRay", &querySceneRay, querySceneRayOverloads());
    def("hitNode", hitNode);
    def("getRayFromEvent", getRayFromEvent);
    def("getRayFromPoint", getRayFromPoint);
    def("printChildren", &printChildren);
    def("printObjCounts", &printObjCounts);
    def("getBoolSetting", &getBoolSetting);
    def("getStringSetting", &getStringSetting);
    def("getButtonSetting", &getButtonSetting);
    def("getDisplayConfig", getDisplayConfig, PYAPI_RETURN_REF);
    def("getTiles", getTiles, PYAPI_RETURN_VALUE);
    def("setTileCamera", setTileCamera);
    def("toggleStereo", toggleStereo);
    def("isStereoEnabled", isStereoEnabled);
    def("queueCommand", queueCommand);
    def("broadcastCommand", broadcastCommand);
    def("ogetdataprefix", ogetdataprefix);
    def("osetdataprefix", osetdataprefix);
    def("isMaster", isMaster);
    def("loadImage", loadImage, PYAPI_RETURN_REF);

    def("addDataPath", addDataPath);
    def("resetDataPaths", addDataPath);
    def("printDataPaths", printDataPaths);

    def("setImageLoaderThreads", setImageLoaderThreads);
    def("getImageLoaderThreads", getImageLoaderThreads);
    def("getHostname", getHostname, PYAPI_RETURN_VALUE);
    def("isHostInTileSection", isHostInTileSection);
    def("setTilesEnabled", setTilesEnabled);
    def("printModules", printModules);

    def("isEventDispatchEnabled", isEventDispatchEnabled);
    def("setEventDispatchEnabled", setEventDispatchEnabled);

    def("orun", orun);
    def("oclean", oclean);
    def("ocleanrun", ocleanrun);
    def("ogetexecpath", ogetexecpath);
    def("olaunch", olaunch); // added in 5.2-alpha3

    def("setNearFarZ", setNearFarZ);
    def("getNearZ", getNearZ);
    def("getFarZ", getFarZ);
    def("getDisplayPixelSize", getDisplayPixelSize);

    def("getMissionControlClient", getMissionControlClient, PYAPI_RETURN_REF);

    def("quaternionToEuler", quaternionToEuler, PYAPI_RETURN_VALUE);
    def("quaternionToEulerDeg", quaternionToEulerDeg, PYAPI_RETURN_VALUE);
    def("quaternionFromEuler", quaternionFromEuler, PYAPI_RETURN_VALUE);
    def("quaternionFromEulerDeg", quaternionFromEulerDeg, PYAPI_RETURN_VALUE);

    def("setClearColor", setClearColor);
    def("clearColor", clearColor);
    def("clearDepth", clearDepth);
};

// Black magic. Include the pyeuclid source code (saved as hex file using xdd -i)
char euclid_source[] = { 
    #include "euclid.xdd" 
};

///////////////////////////////////////////////////////////////////////////////
void omegaPythonApiInit()
{
    //omsg("omegaPythonApiInit()");
    omega::PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();

    // Compile, load and import the euclid module.
    PyObject* euclidModuleCode = Py_CompileString(euclid_source, "euclid", Py_file_input);
    if(euclidModuleCode != NULL)
    {
        sEuclidModule = PyImport_ExecCodeModule("euclid", euclidModuleCode);
        interp->eval("from euclid import *");
    }

    // Register omega::Vector3f <-> euclid.Vector3 converters
    boost::python::to_python_converter<Vector3f, Vector3f_to_python>();
    Vector3f_from_python();

    // Register omega::Vector2f <-> euclid.Vector2 converters
    boost::python::to_python_converter<Vector2f, Vector2f_to_python>();
    Vector2f_from_python();

    // Register omega::Quaternion <-> euclid.Quaternion converters
    boost::python::to_python_converter<Quaternion, Quaternion_to_python>();
    Quaternion_from_python();

    // Initialize the omega wrapper module
    initomega();

    interp->addModule("omega", omegaMethods);
}

#endif
