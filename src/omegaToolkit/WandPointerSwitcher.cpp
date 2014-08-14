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
        if(w->hitTest(point)) return true;
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
        if(evt->getServiceType() == Service::Wand)
        {
            DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
            DisplayConfig& dcfg = ds->getDisplayConfig();

            Ray r;
            if(DisplayUtils::getViewRayFromEvent(*evt, r, dcfg))
            {
                // Convert the ray into a 2D point in screen coordinates.
                /*std::pair<bool, Vector2f> pt = DisplayUtils::getDisplayPointFromViewRay(r, dcfg);
                if(pt.first)
                {
                    // Check for intersections between the point and any
                    // root-level container in the ui.
                    if(pointIntersectsAnyContainer(pt.second, uim->getUi()))
                    {
                        // Intersection found. Reset this event and turn it
                        // into a pointer event.
                        // Also substitute update event types with move event
                        // types, since pointer events typically generate move
                        // events
                        Event::Type etp = evt->getType();
                        if(etp == Event::Update) etp = Event::Move;
                        evt->reset(
                            etp, 
                            Service::Pointer, 
                            evt->getSourceId(),
                            evt->getServiceId());
                    }
                }*/
            }
        }
    }

    unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////
void WandPointerSwitcher::dispose()
{
}
