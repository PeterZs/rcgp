#include "dsl/primitives.hpp"
#include "msv/alignment.hpp"
#include "util/meta.hpp"
#include <glm/glm.hpp>

#include <ugp.hpp>
#include <utility>

// NOTE: nested aggregates will at use the above...

// TODO: first parameter should be alignment rules...
// TODO: for now we are assuming GLSL alignment rules...
// NOTE: TBuffer is typed by its elements as if it
// were an aggregate with the fields Ts... Following
// Vulkan/GLSL rules, all but the last field must be
// trivially constructable; the last field may be an
// unsized array indicated by T[]

// In general, follow this convention:
// X_layout_engine <uint32_t, glm::vec3[]> ::type
// = sequence <
// 	offset_by <uint32_t, 0>,
// 	offset_by <glm::vec3[], 16>
// >
// padded_t = ...

template <typename T, size_t N>
struct padded_t {
	T value;
	[[no_unique_address]] char _pad[N];
};

template <typename T, size_t N>
struct alignment <padded_t <T, N>> {
	static constexpr size_t value = alignment <T> ::value;
};

// TODO: trivial and dynamic _tuples...

template <auto &A, std::size_t ... Is>
constexpr auto array_to_index_sequence_impl(std::index_sequence <Is...>)
{
	return std::index_sequence <A[Is]...> {};
}

template <auto &A>
using array_to_index_sequence = decltype(array_to_index_sequence_impl <A> (std::make_index_sequence <A.size()> {}));

// TODO: no more fields after dynamic part
template <typename ... Ts>
consteval auto std430_layout_engine_impl()
{
	constexpr size_t N = sizeof...(Ts);

	// TODO: need to deal with dynamic parts...
	// constexpr bool dynamic = dynamic_v <Ts> || ...;
	constexpr auto sizes = std::array <size_t, N> { sizeof(Ts)... };
	constexpr auto aligns = std::array <size_t, N> { alignment_v <Ts> ... };

	// At most one section of padding after each field
	std::array <size_t, N> padding;

	size_t offset = 0;
	size_t malign = 0; // max align recorded

	for (size_t i = 0; i < N; i++) {
		auto corrected = align_up(offset, aligns[i]);

		if (i > 0)
			padding[i - 1] = corrected - offset;

		offset = corrected + sizes[i];
		malign = std::max(malign, aligns[i]);
	}

	// TODO: unless its dynamically sized...
	padding[N - 1] = align_up(offset, malign) - offset;

	return padding;
}

template <typename Fields, typename Padding>
struct layout_stitcher {
	using type = sequence <>;
};

template <typename T, typename ... Ts, size_t I, size_t ... Is>
struct layout_stitcher <sequence <T, Ts...>, std::index_sequence <I, Is...>> {
	using next = layout_stitcher <sequence <Ts...>, std::index_sequence <Is...>>;
	using type = next::type::template push_front_t <padded_t <T, I>>;
};

template <typename ... Ts>
consteval auto std430_layout_engine_dispatch(sequence <Ts...>)
{
	static constexpr auto padding = std430_layout_engine_impl <Ts...> ();

	using field_seq = sequence <Ts...>;
	using padding_seq = array_to_index_sequence <padding>;

	using stitched = layout_stitcher <field_seq, padding_seq> ::type;

	return typename stitched::trivial_tuple();
}

template <typename T>
struct fix_alignment_impl {
	using type = T;
};

template <typename T, size_t N>
struct fix_alignment_impl <T[N]> {
	static constexpr size_t padding = align_up(sizeof(T), alignment_v <T>) - sizeof(T);
	using base = padded_t <T, padding>;
	using type = base[N];
};

template <typename T>
struct fix_alignment_impl <T[]> {
	static constexpr size_t padding = align_up(sizeof(T), alignment_v <T>) - sizeof(T);
	using base = padded_t <T, padding>;
	using type = base[];
};

template <typename ... Ts>
consteval auto fix_alignment(sequence <Ts...>) -> sequence <typename fix_alignment_impl <Ts> ::type...>;

template <typename ... Ts>
consteval auto std430_layout_engine(sequence <Ts...> seq)
{
	using fseq = decltype(fix_alignment(seq));
	using tuple = decltype(std430_layout_engine_dispatch(std::declval <fseq> ()));
	return tuple();
}

// TODO: pass as template template to buffers, etc?
template <typename ... Ts>
using std430_layout_t = decltype(std430_layout_engine(std::declval <sequence <Ts...>> ()));

// TODO: array elements need to be padded as well...
// use the old strategy for padded tuples...
using y = decltype(fix_alignment(std::declval <sequence <uint32_t, glm::vec3[3], uint32_t>> ()));
using x = decltype(std430_layout_engine(std::declval <sequence <uint32_t, glm::vec3[3], uint32_t>> ()));
using x = std430_layout_t <uint32_t, glm::vec3[3], uint32_t>;
static_assert(sizeof(x) == 80);
static_assert(x::offset <0> () == 0);
static_assert(x::offset <1> () == 16);
static_assert(x::offset <2> () == 64);

template <template <typename ...> typename Layout, typename ... Ts>
struct FieldedBuffer {
	using tuple = Layout <Ts...>;
}; // : LayoutMappedBuffer <X_layout_engine <Ts...> ::type> {}

using FX = FieldedBuffer <std430_layout_t, uint32_t>;
auto xx = FX();
auto xy = FX::tuple();

// template <typename ... Ts>
// struct ?Buffer : Buffer {
// 	// TODO: method to write individual fields at a time
// 	// TODO: alternatively provide a host "staging" item
// 	// where unsized arrays are converted to vectors or so...
// };

// using PointsBuffer = TBuffer <
// 	uint32_t, 	// count
// 	glm::vec3[]	// positions
// >;

// NOTE: so the flow from DSL storage buffer to host handle;
// take the expanded reflection, convert the fields into
// host equivalents (ordinary + array), then compute their
// layout, and serve the TBuffer of that layout...
// AND
// serve a host staging/data prep structure; the base type
// will be a tuple, but then use the scaffold field members
// to indirectly do this (with overloaded op=)

struct group_device_window {
	Device &device;
	Window &window;

	void wait(Window::Frame &frame, uint64_t timeout = UINT64_MAX) {
		auto result = device.logical.waitForFences(frame.fence, true, timeout);
		device.logical.resetFences(frame.fence);
	}

	bool acquire_image(Window::Frame &frame, uint64_t timeout = UINT64_MAX) {
		auto result = device.logical.acquireNextImageKHR(
			window.swapchain,
			timeout,
			frame.presented,
			nullptr,
			&frame.image_index
		);

		return !(result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR);
	}
};

auto group(Device &device, Window &window)
{
	return group_device_window(device, window);
}

struct Queue : vk::Queue {
	uint32_t family_index;
	uint32_t queue_index;

	static Queue from(const Device &device) {
		// TODO: queue info struct with family flags..
		Queue result(device.logical.getQueue(0, 0));
		result.family_index = 0;
		result.queue_index = 0;
		return result;
	}
};

struct CommandPool : vk::CommandPool {
	static CommandPool from(const Device &device, const Queue &queue) {
		auto cpool_info = vk::CommandPoolCreateInfo()
			.setQueueFamilyIndex(queue.family_index)
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

		return CommandPool(device.logical.createCommandPool(cpool_info));
	}
};

struct group_device_cpool {
	const Device &device;
	const CommandPool &cpool;

	auto allocate(uint32_t count) {
		return device.logical.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
				.setCommandBufferCount(count)
				.setCommandPool(cpool)
		);
	}
};

auto group(const Device &device, const CommandPool &cpool)
{
	return group_device_cpool(device, cpool);
}

int main()
{
	auto session_info = Session::Info {
		.validation_bootstrap = false,
	};

	auto [session, dld] = Session::from(session_info);

	auto device_info = Device::Info {
		.extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		},
	};

	auto device = Device::from(session, dld, device_info);
	auto &ldev = device.logical;

	auto window = Window::from(session, device);

	auto queue = Queue::from(device);
	auto cpool = CommandPool::from(device, queue);
	auto cmdbuffers = group(device, cpool).allocate(window.frames_in_flight);

	while (window.alive()) {
		window.poll();

		if (glfwGetKey(window.handle, GLFW_KEY_Q) == GLFW_PRESS)
			glfwSetWindowShouldClose(window.handle, true);

		auto frame = window.next_frame();

		group(device, window).wait(frame);
		auto acquired = group(device, window).acquire_image(frame);
		if (!acquired)
			continue;

		auto &cmd = cmdbuffers[window.frame_index];

		cmd.reset();
		cmd.begin(vk::CommandBufferBeginInfo());

		auto &image = window.images[frame.image_index];
		auto &layout = window.image_layouts[frame.image_index];
		if (layout != vk::ImageLayout::ePresentSrcKHR) {
			auto range = vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(0)
				.setBaseMipLevel(0)
				.setLayerCount(1)
				.setLevelCount(1);

			auto barrier = vk::ImageMemoryBarrier()
				.setImage(image)
				.setOldLayout(layout)
				.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
				.setSrcAccessMask({})
				.setDstAccessMask({})
				.setSubresourceRange(range);

			cmd.pipelineBarrier(
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eBottomOfPipe,
				{}, {}, {}, barrier
			);

			layout = vk::ImageLayout::ePresentSrcKHR;
		}

		cmd.end();

		vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		auto submit_info = vk::SubmitInfo()
			.setCommandBuffers(cmd)
			.setSignalSemaphores(frame.rendered)
			.setWaitSemaphores(frame.presented)
			.setWaitDstStageMask(wait_stage);

		queue.submit(submit_info, frame.fence);

		auto present_info = vk::PresentInfoKHR()
			.setImageIndices(frame.image_index)
			.setSwapchains(window.swapchain)
			.setWaitSemaphores(frame.rendered);

		auto result = queue.presentKHR(present_info);
	}
}
