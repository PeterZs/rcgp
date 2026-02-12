#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "image.hpp"

struct GLFWwindow;

namespace rcgp {

// Keep GLFW-compatible values without exposing GLFW in the public header.
enum class Key : int {
	Space = 32,
	A = 65,
	D = 68,
	E = 69,
	Q = 81,
	R = 82,
	S = 83,
	W = 87,
	Escape = 256,
	Enter = 257,
	Tab = 258,
	Backspace = 259,
	Right = 262,
	Left = 263,
	Down = 264,
	Up = 265,
	F12 = 301,
	ShiftLeft = 340,
	ControlLeft = 341,
	AltLeft = 342,
	ShiftRight = 344,
	ControlRight = 345,
	AltRight = 346,
};

enum class MouseButton : int {
	Left = 0,
	Right = 1,
	Middle = 2,
};

enum class InputMode : int {
	RawMouseMotion = 0x00033005,
};

struct Session;
struct Device;

struct Frame {
	vk::SwapchainKHR swapchain;
	vk::Fence fence;
	vk::Semaphore presented;
	vk::Semaphore rendered;
	vk::Extent2D extent;
	uint32_t image_index;
};

using MouseButtonHandler = std::function <void (int button, int action, int mods)>;
using CursorMoveHandler = std::function <void (double xpos, double ypos, double dx, double dy)>;
using DragHandler = std::function <void (double xpos, double ypos, double dx, double dy)>;

struct Window {
	GLFWwindow *handle;
	vk::SurfaceKHR surface;

	vk::Format format;
	vk::SwapchainKHR swapchain;
	std::vector <Image> images;

	std::vector <Frame> frames;
	size_t frames_in_flight;
	size_t frame_index;
	
	std::intptr_t handler_index;

	void poll() const;
	void close() const;
	
	bool alive() const;
	bool is_pressed(Key key) const;
	void set_input_mode(InputMode mode, bool value) const;

	vk::Extent2D extent() const;
	// TODO: remove when we implement resizing callbacks
	vk::Extent2D logical_extent() const;

	float aspect_ratio() const {
		return float(extent().width) / extent().height;
	}

	Frame next_frame();

	void on_mouse_button(MouseButtonHandler handler);
	void on_cursor_move(CursorMoveHandler handler);
	void on_drag(MouseButton button, DragHandler handler);

	struct Options {
		uint32_t width;
		uint32_t height;
		const char *const title;
		vk::PresentModeKHR present_mode;
	};

	static Window from(const Session &session, const Device &device, const Options &options);
};

} // namespace rcgp
