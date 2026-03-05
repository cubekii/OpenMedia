#pragma once

#include <vulkan/vulkan.h>
#include <openmedia/macro.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct OMVulkanInit {
  PFN_vkGetInstanceProcAddr proc;
  const VkAllocationCallbacks* allocator;
  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkDevice device;
} OMVulkanInit;

typedef struct OMVulkanPicture OMVulkanPicture;

typedef struct OMVulkanContext OMVulkanContext;

OPENMEDIA_ABI
OMVulkanContext* HWVulkanContext_create(OMVulkanInit init);

OPENMEDIA_ABI
void HWVulkanContext_delete(OMVulkanContext* context);

OPENMEDIA_ABI
OMVulkanPicture* HWVulkanContext_createPicture(OMVulkanContext* context);

struct OMVulkanPicture {
  VkImage image;
};

#if defined(__cplusplus)
}
#endif
