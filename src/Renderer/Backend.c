#include "Render/Backend.h"

uint32_t createInstance(
    const char *appName, uint32_t appVer,
    const char *engineName, uint32_t engineVer,
    uint32_t apiVer,
    InstanceExtensionsView requestedExtensions,
    InstanceLayersView requestedLayers,
    Allocator *allocator,
    Instance *instance
) {
#if RENDER_BACKEND == RENDER_BACKEND_OPENGL
    (void)appName; (void)appVer; (void)engineName; (void)engineVer;
    (void)apiVer;
    (void)requestedExtensions;
    (void)requestedLayers;
    (void)allocator;
    (void)instance;
    return 0;
#elif RENDER_BACKEND == RENDER_BACKEND_VULKAN
    VkApplicationInfo appInfo = PCB_ZEROED;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = appVer;
    appInfo.pEngineName = engineName;
    appInfo.engineVersion = engineVer;
    appInfo.apiVersion = apiVer;

    VkInstanceCreateInfo createInfo = PCB_ZEROED;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.ppEnabledExtensionNames = requestedExtensions.data;
    createInfo.enabledExtensionCount = requestedExtensions.length;
    createInfo.ppEnabledLayerNames = requestedLayers.data;
    createInfo.enabledLayerCount = requestedLayers.length;

    return vkCreateInstance(&createInfo, NULL, &instance);
#endif
}
