//Copyright (c) 2026 Piotr Mikołajewski
#pragma once
//This is a prototype! Not for use in prodution!

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define RENDER_BACKEND_OPENGL 1
#define RENDER_BACKEND_VULKAN 2
#define RENDER_BACKEND_DIRECTX 3

#ifndef RENDER_BACKEND
#define RENDER_BACKEND RENDER_BACKEND_OPENGL
#endif //RENDER_BACKEND

//Vulkan backend will not be implemented, but I'll use it as reference in future projects.

/*-----------------------------------------------*/
/*-----  Renderer-agnostic definitions (1)  -----*/
/*-----------------------------------------------*/
//(sort of)

typedef uint32_t AllocationScope;
enum {
    ALLOCATION_SCOPE_COMMAND = 0,
    ALLOCATION_SCOPE_OBJECT = 1,
    ALLOCATION_SCOPE_CACHE = 2,
    ALLOCATION_SCOPE_DEVICE = 3,
    ALLOCATION_SCOPE_INSTANCE = 4,
};

typedef uint32_t InternalAllocationType;
enum {
    INTERNAL_ALLOCATION_TYPE_EXECUTABLE = 0,
};


typedef void* (*pfn_Alloc) (
    void* ctx,
    size_t size,
    size_t alignment,
    AllocationScope scope
);
typedef void* (*pfn_Realloc) (
    void* ctx,
    void* mem,
    size_t size,
    size_t alignment,
    AllocationScope scope
);
typedef void* (*pfn_Free) (
    void* ctx,
    void* mem
);

typedef void (*pfn_InternalAllocNotification) (
    void* ctx,
    size_t size,
    InternalAllocationType type,
    AllocationScope scope
);
typedef void (*pfn_InternalFreeNotification) (
    void* ctx,
    size_t size,
    InternalAllocationType type,
    AllocationScope scope
);

typedef struct {
    void* ctx;
    pfn_Alloc alloc;
    pfn_Realloc realloc;
    pfn_Free free;
    pfn_InternalAllocNotification internalAlloc;
    pfn_InternalFreeNotification internalFree;
} Allocator;

typedef struct {
    const char *const *data;
    uint32_t length;
} InstanceExtensionsView;

typedef struct {
    const char *const *data;
    uint32_t length;
} InstanceLayersView;

typedef uint32_t VertexInputRate;
enum {
    //attribute is advanced per vertex
    VERTEX_INPUT_RATE_VERTEX   = 0,
    //attribute is advanced per instance
    //@sa vkCmdSetVertexInputEXT
    //@sa glVertexAttribDivisor
    //@sa glVertexBindingDivisor (>= 4.3), glVertexArrayBindingDivisor (>= 4.5)
    VERTEX_INPUT_RATE_INSTANCE = 1,
    //values after 1 (let's call it `N`) mean that
    //attribute is advanced per `N` instances
    //NOTE: Vulkan needs translation into VkPipelineVertexInputDivisorStateCreateInfo (pNext)
    //in VkPipelineVertexInputStateCreateInfo for values >1
};

//@sa vkCmdSetVertexInputEXT
//@sa glVertexAttribPointer
//@sa glBindBuffer
//@sa glBindVertexBuffer (>= 4.3), glVertexArrayVertexBuffer (>= 4.5)
//@sa glBindVertexBuffers (>= 4.4), glVertexArrayVertexBuffers (>= 4.5)
typedef struct {
    uint32_t binding;   //which vertex buffer to use (from the shader's perspective)
    uint32_t stride;    //offset between elements (@sa glBindVertexBuffer)
    uint32_t inputRate; //@enum VertexInputRate
} VertexBindingInfo;

typedef struct {
    const VertexBindingInfo *data;
    uint32_t length;
} VertexBindingInfosView;

//@sa vkCmdSetVertexInputEXT
//@sa glVertexAttribPointer (< 4.3)
//@sa glVertexAttribFormat (>= 4.3), glVertexArrayAttribFormat (>= 4.5)
//@sa glVertexAttribBinding (>= 4.3), glVertexArrayAttribBinding (>=4.5)
//@sa glEnableVertexAttribArray, glEnableVertexArrayAttrib (>= 4.5)
typedef struct {
    uint32_t location; //layout(location = ...)
    uint32_t binding;  //@sa `VertexBindingInfo.binding`
    //NOTE: underlying value depends on the backend
    //@sa glVertexAttribPointer
    //@sa VkFormat enum
    //maybe in the future it'll be abstracted away
    uint32_t format;
    uint32_t offset;
} VertexAttributeInfo;

typedef struct {
    const VertexAttributeInfo *data;
    uint32_t length;
} VertexAttributeInfosView;

typedef struct {
    VertexBindingInfosView vbis;
    VertexAttributeInfosView vais;
} PipelineVertexInputStateCreateInfo;

/*----------------------------------------------*/
/*-  End of renderer-agnostic definitions (1)  -*/
/*----------------------------------------------*/

#if RENDER_BACKEND == RENDER_BACKEND_OPENGL
#include <GL/gl.h>

typedef void* Instance;

typedef void* PhysicalDevice;

// uint32_t 
typedef void* Device;
#define GL_DEVICE NULL //OpenGL doesn't use this, stub it

// PCB_ForceInline Device Device_create(PhysicalDevice pd) { (void)pd; return NULL; }

typedef GLuint Buffer;

// uint32_t allocateDeviceMemory(Device device, uint64_t amount, );

typedef GLuint Texture;
typedef GLuint Shader;

typedef struct {
    float x, y, width, height;
    float minDepth, maxDepth;
} Viewport;
#elif RENDER_BACKEND == RENDER_BACKEND_VULKAN
#include <vulkan/vulkan.h>

typedef VkPhysicalDevice PhysicalDevice;
typedef VkDevice Device;

typedef VkBuffer Buffer;


typedef VkImage Texture;
typedef VkShaderModule Shader;
typedef VkViewport Viewport;
#endif //render backend

/*-----------------------------------------------*/
/*-----  Renderer-agnostic definitions (2)  -----*/
/*-----------------------------------------------*/

//NOTE: stubbed in OpenGL, all parameters are ignored.
//Set `instance` to the pointer returned by the OpenGL loader instead.
//@sa vkCreateInstance
//@sa https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
uint32_t createInstance(
    const char *appName, uint32_t appVer,
    const char *engineName, uint32_t engineVer,
    uint32_t apiVer,
    InstanceExtensionsView requestedExtensions,
    InstanceLayersView requestedLayers,
    Allocator *allocator,
    Instance *instance
);
//TODO: enumerate physical devices, create logical device,
//figure out how to pass uniforms to shaders,
//pipeline layout, and finish the fucking pipeline...


typedef struct {
    //@sa VkPipelineShaderStageCreateFlagBits (unused? in OpenGL)
    uint32_t flags;
    //NOTE: backend-dependent (for now)
    //@sa glCreateShader
    //@sa VkShaderStageFlagBits
    uint32_t stage;
    Shader shader;
    //NOTE: ignored in OpenGL, but MUST be set in Vulkan
    const char *entryPoint;
    //NOTE: Vulkan's shader specializations are not supported
    //@sa VkSpecializationInfo
} PipelineShaderStageInfo;

typedef struct {
    const PipelineShaderStageInfo *data;
    uint32_t length;
} PipelineShaderStageInfosView;

typedef struct {
    //NOTE: backend-dependent (for now)
    //@sa glDrawArrays(...) `mode` argument
    //@sa vkCmdSetPrimitiveTopology
    uint32_t topology;
    //NOTE: Vulkan's primitiveRestartEnable is not supported
    //@sa VkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable
} PipelineInputAssemblyInfo;

//NOTE: VkPipelineTessellationStateCreateInfo skipped

typedef struct {
    //NOTE: VkPipelineViewportStateCreateInfo.pNext skipped
    //NOTE: multiple viewports & scissors not supported (for now)
    //@sa vkCmdSetViewportWithCount
    //@sa glViewport
    Viewport viewport;
    //@sa vkCmdSetScissorWithCount
    //@sa glScissor, glEnable(GL_SCISSOR_TEST)
    struct { float x, y, width, height; } scissor;
} PipelineViewportInfo;


//@sa vkCmdSetCullMode
//@sa glCullFace
typedef uint8_t CullMode;
enum {
    CULL_MODE_NONE = 0,
    CULL_MODE_BACK = 1,
    CULL_MODE_FRONT = 2,
    CULL_MODE_FRONT_AND_BACK = 3
};

//@sa vkCmdSetFrontFace
//@sa glFrontFace
typedef uint8_t FrontFace;
enum {
    FRONT_FACE_CLOCKWISE = 0,
    FRONT_FACE_COUNTERCLOCKWISE = 1
};

typedef struct {
    //@sa vkCmdSetDepthClampEnableEXT
    //@sa glEnable(GL_DEPTH_CLAMP)
    bool depthClamp : 1;
    //@sa vkCmdSetRasterizerDiscardEnable
    //@sa glEnable(GL_RASTERIZER_DISCARD)
    bool rasterizerDiscard : 1;
    //NOTE: VkPipelineRasterizationStateCreateInfo.polygonMode skipped
    CullMode cullMode : 2;
    FrontFace frontFace : 1;
    //NOTE: depth bias & line width skipped
    //@sa VkPipelineRasterizationStateCreateInfo
    //@sa glPolygonOffset
} PipelineRasterizationInfo;

//NOTE: VkPipelineMultisampleStateCreateInfo skipped


//@sa vkCmdSetDepthCompareOp
//@sa glDepthFunc
typedef uint8_t CompareOp;
enum {
    COMPARE_OP_0   = 0,
    COMPARE_OP_LT  = 1,
    COMPARE_OP_EQ  = 2,
    COMPARE_OP_LEQ = 3,
    COMPARE_OP_GT  = 4,
    COMPARE_OP_NEQ = 5,
    COMPARE_OP_GEQ = 6,
    COMPARE_OP_1   = 7
};

typedef struct {
    //@sa vkCmdSetDepthTestEnable
    //@sa glEnable(GL_DEPTH_TEST)
    bool depthTest : 1;
    //@sa vkCmdSetDepthWriteEnable
    //@sa glDepthMask
    bool depthWrite : 1;
    CompareOp compareOpDepth : 3;
    //@sa vkCmdSetDepthBoundsTestEnable
    //no equivalent in OpenGL, needs to be manually implemented
    //in the fragment shader with uniforms
    bool depthBoundsTest : 1;
    //NOTE: stencil test skipped
    // bool stencilTest : 1; //@sa glEnable
    float minDepthBounds; //@sa depthBoundsTest
    float maxDepthBounds; //@sa depthBoundsTest
} PipelineDepthStencilInfo;


//@sa vkCmdSetColorBlendEquationEXT
//@sa glBlendFunc(Separate)
typedef uint8_t BlendFactor;
enum {
    BLEND_FACTOR_ZERO = 0,
    BLEND_FACTOR_ONE = 1,
    BLEND_FACTOR_SRC_COLOR = 2,
    BLEND_FACTOR_ONE_MINUS_SRC_COLOR = 3,
    BLEND_FACTOR_DST_COLOR = 4,
    BLEND_FACTOR_ONE_MINUS_DST_COLOR = 5,
    BLEND_FACTOR_SRC_ALPHA = 6,
    BLEND_FACTOR_ONE_MINUS_SRC_ALPHA = 7,
    BLEND_FACTOR_DST_ALPHA = 8,
    BLEND_FACTOR_ONE_MINUS_DST_ALPHA = 9,
    BLEND_FACTOR_CONSTANT_COLOR = 10,
    BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = 11,
    BLEND_FACTOR_CONSTANT_ALPHA = 12,
    BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = 13,

    BLEND_FACTOR_SRC_ALPHA_SATURATE = 14,
    BLEND_FACTOR_SRC1_COLOR = 15,
    BLEND_FACTOR_ONE_MINUS_SRC1_COLOR = 16,
    BLEND_FACTOR_SRC1_ALPHA = 17,
    BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA = 18
};

//@sa vkCmdSetColorBlendEquationEXT
//@sa glBlendEquation(Separate)
typedef uint8_t BlendOp;
enum {
    BLEND_OP_ADD = 0,
    BLEND_OP_SUBTRACT = 1,
    BLEND_OP_REVERSE_SUBTRACT = 2,
    BLEND_OP_MIN = 3,
    BLEND_OP_MAX = 4,
    //NOTE: Vulkan provides much more blend ops in its extensions
};

typedef struct {
    //@sa vkCmdSetColorBlendEnableEXT
    //@sa glEnable(GL_BLEND)
    bool blendEnable : 1;
    //for following 4:
    //@sa vkCmdSetColorWriteMaskEXT
    //@sa glColorMask(i)
    bool colorMask_r : 1;
    bool colorMask_g : 1;
    bool colorMask_b : 1;
    bool colorMask_a : 1;

    BlendFactor srcColorblendFactor : 5;
    BlendFactor dstColorblendFactor : 5;
    BlendOp     colorBlendOp : 3;

    BlendFactor srcAlphablendFactor : 5;
    BlendFactor dstAlphablendFactor : 5;
    BlendOp     alphaBlendOp : 3;
} ColorBlendAttachmentInfo;

typedef struct {
    const ColorBlendAttachmentInfo *data;
    uint32_t length;
} ColorBlendAttachmentInfosView;

//@sa vkCmdSetLogicOpEnableEXT
//@sa glLogicOp
typedef uint8_t LogicOp;
enum {
    VK_LOGIC_OP_CLEAR = 0,
    VK_LOGIC_OP_AND = 1,
    VK_LOGIC_OP_AND_REVERSE = 2,
    VK_LOGIC_OP_COPY = 3,
    VK_LOGIC_OP_AND_INVERTED = 4,
    VK_LOGIC_OP_NOP = 5,
    VK_LOGIC_OP_XOR = 6,
    VK_LOGIC_OP_OR = 7,
    VK_LOGIC_OP_NOR = 8,
    VK_LOGIC_OP_EQUIV = 9,
    VK_LOGIC_OP_INVERT = 10,
    VK_LOGIC_OP_OR_REVERSE = 11,
    VK_LOGIC_OP_COPY_INVERTED = 12,
    VK_LOGIC_OP_OR_INVERTED = 13,
    VK_LOGIC_OP_NAND = 14,
    VK_LOGIC_OP_SET = 15
};

typedef struct {
    //NOTE: VkPipelineColorBlendStateCreateInfo.flags skipped
    ColorBlendAttachmentInfosView attachments;
    //@sa vkCmdSetBlendConstants
    //@sa glBlendColor
    struct { float r, g, b, a; } blendConstant;
    //NOTE: blending is disabled if `logicOpEnable` is set
    //@sa vkCmdSetLogicOpEnableEXT
    //@sa glEnable(GL_COLOR_LOGIC_OP)
    bool logicOpEnable : 1;
    LogicOp logicOp : 4;
} PipelineColorBlendInfo;

//NOTE: Corresponds to VkPipelineDynamicStateCreateInfo and MUST be set
//accordingly in Vulkan, unused in OpenGL.
typedef struct {
    const uint32_t *data;
    uint32_t length;
} PipelineDynamicStateInfo;

//@sa glGetUniformBlockIndex
//@sa glUniformBlockBinding

/*----------------------------------------------*/
/*-  End of renderer-agnostic definitions (2)  -*/
/*----------------------------------------------*/


#if RENDER_BACKEND == RENDER_BACKEND_OPENGL


//NOTE: Currently not implemented, this is a stub
typedef struct {
    uint8_t _unused[8];
} PipelineLayout;

typedef struct {
    GLuint shaderProgram;
} Pipeline;

#elif RENDER_BACKEND == RENDER_BACKEND_VULKAN

typedef VkPipelineLayout PipelineLayout;
typedef VkPipeline Pipeline;

#endif //render backend
