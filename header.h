void* sysInit(void (*redraw)(void const*));
void sysKill(void* sys);
void sysPaint(void* sys, unsigned* data, unsigned width, unsigned height, unsigned stride);
void sysMouseDown(void* sys, void const* user, float x, float y);
