#include "rhi/window.hpp"
#include "rhi/device.hpp"

#include <GLFW/glfw3.h>

#include <unordered_map>
#include <utility>

namespace rcgp {

struct HandlerTable {
	int dragging_button = -1;
	bool cursor_initialized = false;
	bool framebuffer_resized = false;
	double last_x = 0.0;
	double last_y = 0.0;

	std::vector <MouseButtonHandler> mouse_button;
	std::vector <CursorMoveHandler> cursor_move;
	std::unordered_map <int, std::vector <DragHandler>> drag;
};

inline std::vector <HandlerTable> handler_tables;

void dispatch_mouse_button(GLFWwindow *w, int button, int action, int mods)
{
	auto user = glfwGetWindowUserPointer(w);
	auto handler_index = reinterpret_cast <std::intptr_t> (user);

	auto &ht = handler_tables[handler_index];
	for (auto &cb : ht.mouse_button)
		cb(button, action, mods);

	if (action == GLFW_PRESS) {
		ht.dragging_button = button;
		glfwGetCursorPos(w, &ht.last_x, &ht.last_y);
		int framebuffer_width = 0;
		int framebuffer_height = 0;
		int window_width = 0;
		int window_height = 0;
		glfwGetFramebufferSize(w, &framebuffer_width, &framebuffer_height);
		glfwGetWindowSize(w, &window_width, &window_height);

		if (window_width > 0 && window_height > 0) {
			auto sx = double(framebuffer_width) / double(window_width);
			auto sy = double(framebuffer_height) / double(window_height);
			ht.last_x *= sx;
			ht.last_y *= sy;
		}
		ht.cursor_initialized = true;
	} else if (action == GLFW_RELEASE) {
		if (ht.dragging_button == button)
			ht.dragging_button = -1;
	}
}

void dispatch_cursor_pos(GLFWwindow *w, double xpos, double ypos)
{
	auto user = glfwGetWindowUserPointer(w);
	auto handler_index = reinterpret_cast <std::intptr_t> (user);

	auto &ht = handler_tables[handler_index];

	double fx = xpos;
	double fy = ypos;

	int framebuffer_width = 0;
	int framebuffer_height = 0;
	int window_width = 0;
	int window_height = 0;

	glfwGetFramebufferSize(w, &framebuffer_width, &framebuffer_height);
	glfwGetWindowSize(w, &window_width, &window_height);

	if (window_width > 0 && window_height > 0) {
		auto sx = double(framebuffer_width) / double(window_width);
		auto sy = double(framebuffer_height) / double(window_height);
		fx *= sx;
		fy *= sy;
	}

	double dx = 0.0;
	double dy = 0.0;
	if (ht.cursor_initialized) {
		dx = fx - ht.last_x;
		dy = fy - ht.last_y;
	}

	ht.last_x = fx;
	ht.last_y = fy;
	ht.cursor_initialized = true;
	for (auto &cb : ht.cursor_move)
		cb(fx, fy, dx, dy);

	if (ht.dragging_button != -1) {
		auto it = ht.drag.find(ht.dragging_button);
		if (it != ht.drag.end()) {
			for (auto &cb : it->second)
				cb(fx, fy, dx, dy);
		}
	}
}

void dispatch_framebuffer_size(GLFWwindow *w, int, int)
{
	auto user = glfwGetWindowUserPointer(w);
	auto handler_index = reinterpret_cast <std::intptr_t> (user);
	handler_tables[handler_index].framebuffer_resized = true;
}

void Window::poll() const
{
	glfwPollEvents();
}

void Window::close() const
{
	glfwSetWindowShouldClose(handle, true);
}

bool Window::alive() const
{
	return not glfwWindowShouldClose(handle);
}

bool Window::is_pressed(Key key) const
{
	return glfwGetKey(handle, std::to_underlying(key)) == GLFW_PRESS;
}

void Window::set_input_mode(InputMode mode, bool value) const
{
	glfwSetInputMode(handle, std::to_underlying(mode), value ? GLFW_TRUE : GLFW_FALSE);
}

vk::Extent2D Window::extent() const
{
	return swapchain_extent;
}

float Window::aspect_ratio() const
{
	auto e = extent();
	return e.height > 0 ? (float(e.width) / float(e.height)) : 1.0f;
}

bool Window::needs_swapchain_rebuild()
{
	if (handler_tables[handler_index].framebuffer_resized) {
		swapchain_rebuild_requested = true;
		handler_tables[handler_index].framebuffer_resized = false;
	}

	return swapchain_rebuild_requested;
}

Frame Window::next_frame()
{
	frame_index = (frame_index + 1) % frames_in_flight;
	auto &frame = frames[frame_index];
	frame.extent = swapchain_extent;
	return frame;
}

void Window::on_mouse_button(MouseButtonHandler handler)
{
	auto &ht = handler_tables[handler_index];
	ht.mouse_button.push_back(std::move(handler));
}

void Window::on_cursor_move(CursorMoveHandler handler)
{
	auto &ht = handler_tables[handler_index];
	ht.cursor_move.push_back(std::move(handler));
}

void Window::on_drag(MouseButton button, DragHandler handler)
{
	auto idx = std::to_underlying(button);
	auto &ht = handler_tables[handler_index];
	ht.drag[idx].push_back(std::move(handler));
}

Window Window::from(const Session &session, const Device &device, const Options &options)
{
	Window result;

	result.handle = glfwCreateWindow(options.width, options.height, options.title, nullptr, nullptr);
	result.handler_index = handler_tables.size();
	handler_tables.emplace_back();

	auto user = reinterpret_cast <void *> (result.handler_index);
	glfwSetWindowUserPointer(result.handle, user);
	glfwSetMouseButtonCallback(result.handle, dispatch_mouse_button);
	glfwSetCursorPosCallback(result.handle, dispatch_cursor_pos);
	glfwSetFramebufferSizeCallback(result.handle, dispatch_framebuffer_size);

	VkSurfaceKHR surface;
	glfwCreateWindowSurface(session.handle, result.handle, nullptr, &surface);
	result.surface = surface;

	result.format = vk::Format::eB8G8R8A8Unorm;
	result.present_mode = options.present_mode;
	result.swapchain = nullptr;
	result.frames_in_flight = 0;
	result.frame_index = 0;
	result.swapchain_rebuild_requested = true;

	handler_tables[result.handler_index].framebuffer_resized = true;
	while (not device.rebuild_swapchain(result))
		glfwWaitEvents();

	return result;
}
} // namespace rcgp
