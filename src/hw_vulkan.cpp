#include <openmedia/hw_vulkan.h>
#include <memory>

struct OMVulkanContext {
  explicit OMVulkanContext(OMVulkanInit init) {

  }
};

OMVulkanContext* HWVulkanContext_create(OMVulkanInit init) {
  auto* context = static_cast<OMVulkanContext*>(malloc(sizeof(OMVulkanContext)));
  if (!context) return nullptr;
  new (context) OMVulkanContext(init);
  return context;
}

void HWVulkanContext_delete(OMVulkanContext* context) {
  if (!context) return;
  context->~OMVulkanContext();
  free(context);
}

OMVulkanPicture* HWVulkanContext_createPicture(OMVulkanContext* context) {
  return nullptr;
}
