//Made by Han_feng

#ifndef VULKAN_LIBRARY_VULKAN_APPLICATION_H
#define VULKAN_LIBRARY_VULKAN_APPLICATION_H

#include "vulkan/vulkan.h"
#include "utils/queues.h"
#include "utils/swap_chain.h"
#include "utils/pipelines.h"
#include "utils/swap_chain.h"
#include "utils/command_context.h"
#include "utils/sync_objects.h"

#include "vector"

namespace vulkan {
    /**
     * @struct Window
     * @brief A structure for vulkan application
     */
    struct Window {
        void *window;  /// The window handle
        unsigned int width;  /// The window width
        unsigned int height; /// The window height
    };

    /**
     * @class Application
     * @brief An application that can handle a window based on Vulkan
     */
    class Application {
    public:
        //Interfaces
        /**
        * @brief Create a vulkan application
        * @param window The target window (Promised not to change the fields in the Window)
        * @note You should update the size fields in the Window when the window size is changed, usually for the swap chain recreation.
        * @note If the surface extent isn't mutable, it will use the current extent. BUT if it is, then the swap chain extent will be set to the same as the size fields
        */
        explicit Application(const Window *window);
        ~Application();

        /**
         * @brief check whether the application is validated
         * @return whether the application is validated
         * @warning if it returns false, please don't use it and throw an error
         * @note Always call this function after create the vulkan application
         */
        bool validate() const;

        /**
         * @brief draw a frame to the window surface
         * @retval nullopt There's a fatal error, and you should stop using the application and throw an error
         * @retval false Successfully draw a frame
         * @retval true Successfully draw a frame, but the swap chain is suboptimal or out of date. You need to call @c refresh to update the application state
         * @note In order to avoid losing any frames as much as possible, the suboptimal frame will still be presented, but this will returns true, so remember to @c refresh
         */
        std::optional<bool> draw_frame();

        /**
         * @brief Refresh the application state
         * @return whether the refresh is successful
         * @warning if it returns false, that means there's a fatal error when refreshing. Please stop using the application and throw an error
         * @note Considering swap chain won't be promised to trigger suboptimal or out of date when the size is changed, you can call this function manually
         */
        bool refresh();

    private:
        //Statics
        static std::vector<const char*> INSTANCE_EXTENSIONS;
        static std::vector<const char*> DEVICE_EXTENSIONS;

        //Attributes
        const Window* const window;
        VkInstance instance = nullptr;
        VkDebugUtilsMessengerEXT debug_messenger = nullptr;
        VkSurfaceKHR surface = nullptr;
        VkPhysicalDevice physical_device = nullptr;
        VkDevice device = nullptr;
        Queues queues;
        Swap_chain swap_chain;
        Render_pass render_pass;
        Pipelines pipelines;
        Command_context command_context;
        Sync_objects sync_objects;

        //Runtime attributes
        bool valid = true;
        int current_frame = 0;

        //Functions
        void create_instance();
        void create_debug_messenger();
        void create_surface(void *window_handle);
        void get_physical_device();
        void create_device();

        //Tool functions
        static int get_physical_device_score(VkPhysicalDevice target, VkSurfaceKHR surface);
    };

} // vulkan

#endif //VULKAN_LIBRARY_VULKAN_APPLICATION_H