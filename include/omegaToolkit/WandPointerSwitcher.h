#ifndef __WAND_POINTER_SWITCHER__
#define __WAND_POINTER_SWITCHER__

#include <omega.h>

namespace omegaToolkit
{
    ///////////////////////////////////////////////////////////////////////////////
    // Module declaration
    class WandPointerSwitcher : public omega::Service
    {
    public:
        // Allocator function
        static WandPointerSwitcher* New() { return new WandPointerSwitcher(); }

    public:
        WandPointerSwitcher();

        virtual void setup(omega::Setting& settings);
        virtual void initialize();
        virtual void poll();
        virtual void dispose();

    private:
    };
};

#endif
