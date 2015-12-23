#include "omegaToolkit/WandPointerSwitcher.h"
#include "omegaToolkit/UiModule.h"
#include "omegaToolkit/ui/Container.h"

using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
// Utility function: check if a point (pixel coordinates) intersects any
// root-level child of the specified container.
bool pointIntersectsAnyContainer(const Vector2f point, Container* c)
{
    int nc = c->getNumChildren();
    for(int i = 0; i < nc; i++)
    {
        Widget* w = c->getChildByIndex(i);
        if(w->isEnabled() && w->hitTest(point)) return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
WandPointerSwitcher::WandPointerSwitcher()
{
}

///////////////////////////////////////////////////////////////////////////////
void WandPointerSwitcher::setup(Setting& settings)
{
}

///////////////////////////////////////////////////////////////////////////////
void WandPointerSwitcher::initialize()
{
    setPollPriority(Service::PollLast);
}

///////////////////////////////////////////////////////////////////////////////
void WandPointerSwitcher::poll()
{
    // If the engine has not been initialized yet, exit.
    if(Engine::instance() == NULL) return;

    // If the Ui Module is not running, this service has nothing to do. Exit.
    UiModule* uim = UiModule::instance();
    if(uim == NULL) return;


    lockEvents();
    int numEvts = getManager()->getAvailableEvents();
    for(int i = 0; i < numEvts; i++)
    {
        Event* evt = getEvent(i);
        // Process mocap events.
        if(evt->getServiceType() == Service::Wand &&
            !evt->isExtraDataNull(2) && !evt->isExtraDataNull(3))
        {
            Vector2f out;
            DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
            DisplayConfig& dcfg = ds->getDisplayConfig();
            const Rect& cr = dcfg.getCanvasRect();

            out[0] = evt->getExtraDataFloat(2) * dcfg.displayResolution[0] - cr.x();
            out[1] = evt->getExtraDataFloat(3) * dcfg.displayResolution[1] - cr.y();
            
            if(pointIntersectsAnyContainer(out, uim->getUi()))
            {
                evt->setPosition(Vector3f(out[0], out[1], 0));
                evt->setServiceType(Service::Pointer);
                // If the event is an Update event, also convert it to a Move event, since
                // pointer handling code expects that.
                if(evt->getType() == Event::Update) evt->resetType(Event::Move);            
            }
        }
    }

    unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////
void WandPointerSwitcher::dispose()
{
}
