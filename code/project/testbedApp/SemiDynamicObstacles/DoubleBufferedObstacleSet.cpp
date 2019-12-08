//**********************************************************************
//
// Copyright (c) 2007-2013
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include "base/Header.h"
#include "project/testbedApp/SemiDynamicObstacles/DoubleBufferedObstacleSet.h"
#include "base/MutexLock.h"
#include "externalAPI/i_testbed.h"
#include "externalAPI/i_pathengine.h"
#include "platform_common/Thread.h"
#include <algorithm>
#include <sstream>
#include <string.h>
#include <time.h>

namespace
{

class cThread : public iThread
{
public:
    cDoubleBufferedObstacleSet* _set;
    cThread()
    {
        _set = 0;
    }
    void
    run()
    {
        assertR(_set);
        _set->doUpdate();
    }
};

} // anonymous namespace

cThread gThread;

void
cDoubleBufferedObstacleSet::doUpdate()
{
    assertD(updateInProgress());

    {
        clock_t start, finish;
        start = clock();
        _background->updatePathfindPreprocessFor(_agentShape);
        finish = clock();
        _lastUpdateTime = static_cast<double>(finish - start) / CLOCKS_PER_SEC;
    }

    clock_t start, finish;
    start = clock();
    {
        cMutexLock lock(_bufferSwapMutex);
        tSigned32 i;
        for(i = 0; i != SizeL(_beingRemoved); ++i)
        {
            iAgent* agent = _beingRemoved[i];
            _foreground->removeAgent(agent);
            assertD(!_background->includes(agent));
            delete agent; // automatically removed from _allAgents
        }
        _beingRemoved.resize(0);

        _foregroundContext->removeObstacleSet(_foreground);
        _foregroundContext->removeObstacleSet(_beingAddedOverlay);

        while(!_beingAddedOverlay->empty())
        {
            iAgent* agent = _beingAddedOverlay->getAgent(0);
            _foreground->addAgent(agent);
            _beingAddedOverlay->removeAgent(agent);
        }

        std::swap(_foreground, _background);

        _foregroundContext->addObstacleSet(_foreground);
        _foregroundContext->updateCollisionPreprocessFor(_agentShape);

        _renderPreprocessValid = false;
        _expansionsRenderPreprocessValid = false;
        _toBeDeletedRenderPreprocessValid = false;
    }
    finish = clock();
    _lastBufferSwapTime = static_cast<double>(finish - start) / CLOCKS_PER_SEC;

    cMutexLock lock(_updateInProgressMutex);
    _updateInProgress = false;
}

cDoubleBufferedObstacleSet::cDoubleBufferedObstacleSet(iTestBed* testBed, iMesh* mesh, iShape* agentShape, const char *const* preprocessAttributes)
{
    _testBed = testBed;
    _expansionsRenderPreprocess = 0;
    _expansionsRenderPreprocessValid = false;
    _renderPreprocess = 0;
    _renderPreprocessValid = false;
    _toBeDeletedRenderPreprocess = 0;
    _toBeDeletedRenderPreprocessValid = false;
    _lastUpdateTime = 0.0;
    _lastBufferSwapTime = 0.0;
    _mesh = mesh;
    _agentShape = agentShape;
    std::vector<const char*> combinedAttributes;
    if(preprocessAttributes)
    {
        while(preprocessAttributes[0])
        {
            assertD(preprocessAttributes[1]);
            const char* attribute(preprocessAttributes[0]);
            const char* value(preprocessAttributes[1]);
            preprocessAttributes += 2;
            if(strcmp("markForPreprocessing", attribute) == 0)
            {
                Error("NonFatal", "'markForPreprocessing' attribute should not be supplied for cDoubleBufferedObstacleSet construction, over-ridden.");
                continue;
            }
            if(strcmp("re_testBedquireExplicitAgentRemoval", attribute) == 0)
            {
                Error("NonFatal", "'warnOnImplicitStateChanges' attribute should not be supplied for cDoubleBufferedObstacleSet construction, over-ridden.");
                continue;
            }
            combinedAttributes.push_back(attribute);
            combinedAttributes.push_back(value);
        }
    }
    combinedAttributes.push_back(0);
    _allAgents = _mesh->newObstacleSet_WithAttributes(&combinedAttributes[0]);
    _toAddNextUpdateOverlay = _mesh->newObstacleSet_WithAttributes(&combinedAttributes[0]);
    _toRemoveNextUpdate = _mesh->newObstacleSet_WithAttributes(&combinedAttributes[0]);
    combinedAttributes.pop_back();
    combinedAttributes.push_back("warnOnImplicitStateChanges");
    combinedAttributes.push_back("true");
    combinedAttributes.push_back(0);
    _beingAddedOverlay = _mesh->newObstacleSet_WithAttributes(&combinedAttributes[0]);
    combinedAttributes.pop_back();
    combinedAttributes.push_back("markForPreprocessing");
    combinedAttributes.push_back("true");
    combinedAttributes.push_back(0);
    _foreground = _mesh->newObstacleSet_WithAttributes(&combinedAttributes[0]);
    _background = _mesh->newObstacleSet_WithAttributes(&combinedAttributes[0]);
    _foregroundContext = _mesh->newContext();
    _foregroundContext->addObstacleSet(_foreground);
    _foregroundContext->addObstacleSet(_toAddNextUpdateOverlay); // this obstacle set is permanently included in the context, for simplicity
    _updateInProgress = false;
    tSigned32 i;
    for(i = 0; i != _mesh->getNumberOfNamedObstacles(); ++i)
    {
        iAgent* agent;
        const char* id_ignored;
        _mesh->retrieveNamedObstacleByIndex(i, agent, id_ignored);
        _allAgents->addAgent(agent);
        _foreground->addAgent(agent);
        _background->addAgent(agent);
    }
    _foreground->updatePathfindPreprocessFor(_agentShape);
    _foregroundLocked = false;
}
cDoubleBufferedObstacleSet::~cDoubleBufferedObstacleSet()
{
    assertD(!updateInProgress());
    assertD(!_foregroundLocked);
    delete _foreground;
    delete _background;
    delete _beingAddedOverlay;
    delete _toAddNextUpdateOverlay;
    delete _toRemoveNextUpdate;
    delete _foregroundContext;
    while(_allAgents->getNumberOfAgents())
    {
        delete _allAgents->getAgent(0);
    }
    _testBed->deleteRenderPreprocess(_renderPreprocess);
    _testBed->deleteRenderPreprocess(_expansionsRenderPreprocess);
}

void
cDoubleBufferedObstacleSet::storeToNamedObstacles() const
{
    cMutexLock lock(_bufferSwapMutex);
    _mesh->clearAllNamedObstacles();
    tSigned32 i;
    for(i = 0; i != _allAgents->size(); ++i)
    {
        std::ostringstream oss;
        oss << i;
        _mesh->storeNamedObstacle(oss.str().c_str(), _allAgents->getAgent(i));
    }
}

void
cDoubleBufferedObstacleSet::addObstacle(iAgent* toAdd)
{
    cMutexLock lock(_bufferSwapMutex);
    assertD(!_allAgents->includes(toAdd));
    _allAgents->addAgent(toAdd);
    _toAddNextUpdateOverlay->addAgent(toAdd);
}
void
cDoubleBufferedObstacleSet::removeAndDeleteObstacle(iAgent* toRemove)
{
    cMutexLock lock(_bufferSwapMutex);
    assertD(_allAgents->includes(toRemove));
    if(_toAddNextUpdateOverlay->includes(toRemove))
    {
        delete toRemove; // will be removed from the relevant sets automatically
    }
    else if(!_toRemoveNextUpdate->includes(toRemove)) // check not already flagged for removal
    {
      _toRemoveNextUpdate->addAgent(toRemove);
    }
    _toBeDeletedRenderPreprocessValid = false;
    _renderPreprocessValid = false;
}

void
cDoubleBufferedObstacleSet::startUpdate()
{
    assertD(!updateInProgress());
    _updateInProgress = true;
    tSigned32 i;
    assertD(_beingAddedOverlay->empty());
    for(i = 0; i != _toAddNextUpdateOverlay->size(); ++i)
    {
        iAgent* agent = _toAddNextUpdateOverlay->getAgent(i);
        _beingAddedOverlay->addAgent(agent);
        _background->addAgent(agent);
    }
    _toAddNextUpdateOverlay->clear();
    for(i = 0; i != _toRemoveNextUpdate->size(); ++i)
    {
        iAgent* agent = _toRemoveNextUpdate->getAgent(i);
        _background->removeAgent(agent);
        _beingRemoved.push_back(agent);
    }
    _toRemoveNextUpdate->clear();
    _foregroundContext->addObstacleSet(_beingAddedOverlay);
    gThread._set = this;
    //tThreadHandle handle = StartThread(gThread); //.. do we need to make sure that this thread is terminated, before exiting?
    StartThread(gThread);
}

bool
cDoubleBufferedObstacleSet::updateInProgress() const
{
    cMutexLock lock(_updateInProgressMutex);
    return _updateInProgress;
}

double
cDoubleBufferedObstacleSet::getLastUpdateTime() const
{
    assertD(!updateInProgress()); // could also return this during update, with mutex protection
    return _lastUpdateTime;
}
double
cDoubleBufferedObstacleSet::getLastBufferSwapTime() const
{
    assertD(!updateInProgress()); // could also return this during update, with mutex protection
    return _lastBufferSwapTime;
}

void
cDoubleBufferedObstacleSet::lockForeground()
{
    assertD(!_foregroundLocked);
    _foregroundLocked = true;
    _bufferSwapMutex.lock();
}
void
cDoubleBufferedObstacleSet::unlockForeground()
{
    assertD(_foregroundLocked);
    _foregroundLocked = false;
    _bufferSwapMutex.unlock();
}

const iCollisionContext*
cDoubleBufferedObstacleSet::getForegroundCollisionContext() const
{
    assertD(_foregroundLocked);
    return _foregroundContext;
}

const iObstacleSet*
cDoubleBufferedObstacleSet::getForegroundPreprocessedSet() const
{
    assertD(_foregroundLocked);
    return _foreground;
}

tSigned32
cDoubleBufferedObstacleSet::numberOfObstaclesAdded() const
{
    assertD(_foregroundLocked);
    return _allAgents->size();
}

tSigned32
cDoubleBufferedObstacleSet::numberOfPreprocessedObstacles() const
{
    assertD(_foregroundLocked);
    return _foreground->size();
}
tSigned32
cDoubleBufferedObstacleSet::numberOfDynamicObstacles() const
{
    assertD(_foregroundLocked);
    return _beingAddedOverlay->size() + _toAddNextUpdateOverlay->size();
}

void
cDoubleBufferedObstacleSet::renderPreprocessed() const
{
    assertD(_foregroundLocked);
    if(!_renderPreprocessValid)
    {
        _testBed->deleteRenderPreprocess(_renderPreprocess);
        _testBed->startGeneratingPreprocess();
        //_testBed->drawAgentsInObstacleSet(_foreground, 40);
        for(tSigned32 i = 0; i != _foreground->getNumberOfAgents(); ++i)
        {
            iAgent* agent = _foreground->getAgent(i);
            if(!_toRemoveNextUpdate->includes(agent))
            {
                _testBed->drawAgent(agent, 40);
            }
        }
        _renderPreprocess = _testBed->finishGeneratingPreprocess();
        _renderPreprocessValid = true;
    }
    _testBed->drawRenderPreprocess(_renderPreprocess);
}
void
cDoubleBufferedObstacleSet::renderPreprocessedExpansion() const
{
    assertD(_foregroundLocked);
    if(!_expansionsRenderPreprocessValid)
    {
        _testBed->deleteRenderPreprocess(_expansionsRenderPreprocess);
        _testBed->startGeneratingPreprocess();
        for(tSigned32 i = 0; i != _foreground->getNumberOfAgents(); ++i)
        {
            _testBed->drawAgentExpansion(_foreground->getAgent(i), _agentShape);
        }
        _expansionsRenderPreprocess = _testBed->finishGeneratingPreprocess();
        _expansionsRenderPreprocessValid = true;
    }
    _testBed->drawRenderPreprocess(_expansionsRenderPreprocess);
}
void
cDoubleBufferedObstacleSet::renderDynamic() const
{
    assertD(_foregroundLocked);
    _testBed->drawAgentsInObstacleSet(_beingAddedOverlay, 40);
    _testBed->drawAgentsInObstacleSet(_toAddNextUpdateOverlay, 40);
}
void
cDoubleBufferedObstacleSet::renderToBeDeleted() const
{
    assertD(_foregroundLocked);
    if(!_toBeDeletedRenderPreprocessValid)
    {
        _testBed->deleteRenderPreprocess(_toBeDeletedRenderPreprocess);
        _testBed->startGeneratingPreprocess();
        _testBed->drawAgentsInObstacleSet(_toRemoveNextUpdate, 40);
        for(tSigned32 i = 0; i != SizeL(_beingRemoved); ++i)
        {
            _testBed->drawAgent(_beingRemoved[i], 40);
        }
        _toBeDeletedRenderPreprocess = _testBed->finishGeneratingPreprocess();
        _toBeDeletedRenderPreprocessValid = true;
    }
    _testBed->drawRenderPreprocess(_toBeDeletedRenderPreprocess);
}
