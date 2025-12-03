#include <array>
#include <vector>
#include <glm/glm.hpp>

#include <ugp.hpp>

const std::string vshader = R"(
#version 450

layout (location = 0) in vec2 position;

void main()
{
	gl_Position = vec4(position, 0, 1);
}
)";

const std::string fshader = R"(
#version 450

layout (location = 0) out vec4 color;

void main()
{
	color = vec4(1);
}
)";

namespace std430 {

template <typename T>
struct layout_engine {
	static_assert(false, ($ss("s::layout_engine not implemented for type ") + $ss_type(T)).view());
};

template <typename Original, typename ... Ts>
struct layout_engine <aggregate_reflection <Original, Ts...>> {
	static constexpr size_t alignment = std::max({ layout_engine <Ts> ::alignment... });
	using hint = scaffold_hint <
		sequence <typename layout_engine <Ts> ::hint...>,
		alignment
	>;
};

template <typename T, int64_t N>
requires (N > 0)
struct layout_engine <array_reflection <T, N>> {
	static constexpr size_t alignment = layout_engine <T> ::alignment;
	using hint = scaffold_hint <
		std::array <typename layout_engine <T> ::hint, N>,
		alignment
	>;
};

template <typename T>
struct layout_engine <array_reflection <T, -1>> {
	static constexpr size_t alignment = layout_engine <T> ::alignment;
	using hint = scaffold_hint <
		unsized_array <typename layout_engine <T> ::hint>,
		alignment
	>;
};

template <native_scalar T, size_t N, size_t M>
struct layout_engine <primitive_reflection <matrix <T, N, M>>> {
	// TODO: equals the alignment of row vector
	static constexpr size_t alignment = 16;
	using hint = scaffold_hint <
		glm::mat <M, N, T>,
		alignment
	>;
};

template <native_scalar T, size_t D>
struct layout_engine <primitive_reflection <vector <T, D>>> {
	static constexpr size_t alignment = [] constexpr {
		switch (D) {
		case 4:
		case 3:
			return 16;
		case 2:
			return 8;
		}
	} ();

	using hint = scaffold_hint <
		glm::vec <D, T>,
		alignment
	>;
};

template <native_scalar T>
struct layout_engine <primitive_reflection <scalar <T>>> {
	static constexpr size_t alignment = alignof(T);
	using hint = scaffold_hint <T, alignment>;
};

} // namespace std430

namespace layouts {

template <typename T>
using std430 = std430::layout_engine <T>;

} // namespace layouts

template <reflected T, template <typename> typename Engine = layouts::std430>
struct data_mirror {
	using layout = Engine <expand_reflection_t <T>>;
	using hint = layout::hint;
	using type = scaffold_lookup <hint, T> ::type;
};

#define $mirror(T, ...) data_mirror <T __VA_OPT__(,) __VA_ARGS__> ::type

struct View {
	mat4 model;
	mat4 view;
	mat4 proj;

	$reflection(model, view, proj);
};

struct X {
	vec3 x;
	vec3 y;

	$reflection(x, y);
};

struct XNested {
	vec2 w;
	X nested;

	$reflection(w, nested);
};

auto g = [] {
	using Y = $mirror(X);

	static_assert(sizeof(Y) == 32);
	static_assert(offsetof(Y, x) == 0);
	static_assert(offsetof(Y, y) == 16);

	Y y {
		.x = glm::vec3(1.0f),
		.y = glm::vec3(2.0f),
	};

	auto s = $mirror(View) {
		.model = glm::mat4(1.0),
		.proj = glm::mat4(0.1),
	};

	using Z = $mirror(array <vec2, 3>);

	Z z;
	z[2] = glm::vec2(2.0f);

	using A = $mirror(XNested);

	A a {
		.nested = {
			.y = glm::vec3(1.0f),
		},
	};

	a.nested.x = glm::vec3(1.0f);
	
	using W = $mirror(array <X, 12>);
	W w {
		W::value_type {
			.x = glm::vec3(1.0f),
		},
	};

	using B = $mirror(array <vec3>);
	static_assert(sizeof(B::value_type) == 16);

	B b;
	b.emplace_back(1, 1, 1);
};

template <typename T>
struct is_dynamic_reflection : std::false_type {};

template <typename T>
struct is_dynamic_reflection <array_reflection <T, -1>> : std::true_type {};

template <typename Original, typename ... Ts>
struct is_dynamic_reflection <aggregate_reflection <Original, Ts...>>
	: std::bool_constant <(is_dynamic_reflection <Ts> ::value || ...)> {};

template <typename T>
constexpr bool is_dynamic_reflection_v = is_dynamic_reflection <T> ::value;

template <typename T>
constexpr bool is_static_reflection_v = !is_dynamic_reflection <T> ::value;

template <reflected T>
constexpr bool is_dynamic_v = is_dynamic_reflection <expand_reflection_t <T>> ::value;

template <reflected T>
constexpr bool is_static_v = !is_dynamic_reflection <expand_reflection_t <T>> ::value;

static_assert(is_static_v <View>);
static_assert(is_dynamic_v <array <vec2>>);

struct alignas(0) Points {
	u32 count;

	struct Array {
		vec3 delta;

		// array <u32> positions;
		array <vec2> positions;

		$reflection(delta, positions);
	} array;

	$reflection(count, array);
	// TODO: if T is a dynamic aggregate,
	// then align should fallback to 0 (i.e. natural)
};

static_assert(alignof(std::vector <int>) == 8);

static_assert(is_dynamic_v <Points>);

// reflection to field_trace of dynamic
template <typename T>
requires is_dynamic_reflection_v <T>
struct field_trace_of_dynamic {};

template <size_t N>
constexpr int64_t idxoftrue(const std::array <bool, N> &x)
{
	for (size_t i = 0; i < N; i++) {
		if (x[i])
			return i;
	}

	return -1;
}

template <typename Original, typename ... Ts>
struct field_trace_of_dynamic <aggregate_reflection <Original, Ts...>> {
	template <typename T, size_t ... Is>
	using trace = decltype([] {
		constexpr auto N = sizeof...(Ts);
		constexpr auto dynamics = std::array <bool, N> {
			is_dynamic_reflection_v <Ts>...
		};

		constexpr auto idx = idxoftrue(dynamics);

		using D = Ts...[idx];

		return typename field_trace_of_dynamic <D>
			::template trace <T, Is..., idx> ();
	} ());
};

template <typename T>
struct field_trace_of_dynamic <array_reflection <T, -1>> {
	template <typename U, size_t ... Is>
	using trace = field_trace <U, Is...>;
};

using R1 = expand_reflection_t <Points>;
using trace1 = field_trace_of_dynamic <R1> ::trace <View>;

using R2 = expand_reflection_t <array <vec2>>;
using trace2 = field_trace_of_dynamic <R2> ::trace <View>;

template <typename T, size_t I, size_t ... Is>
auto &field_trace_get(field_trace <T, I, Is...>, auto &value)
{
	auto &once = value.template get <I> ();

	using U = std::decay_t <decltype(once)>;
	if constexpr (sizeof...(Is))
		return field_trace_get(field_trace <U, Is...> (), once);
	else
		return once;
}

template <typename T>
auto &field_trace_get(field_trace <T>, auto &value)
{
	return value;
}

template <typename T, size_t I, size_t ... Is>
size_t field_trace_offset(field_trace <T, I, Is...>, auto &value)
{
	auto &once = value.template get <I> ();
	auto offset = value.template offset <I> ();

	using U = std::decay_t <decltype(once)>;
	if constexpr (sizeof...(Is))
		return offset + field_trace_offset(field_trace <U, Is...> (), once);
	else
		return offset;
}

template <typename T>
size_t field_trace_offset(field_trace <T>, auto &value)
{
	return 0;
}

template <reflected T>
requires is_dynamic_v <T>
auto dynamic_part(const auto &value)
{
	using R = expand_reflection_t <T>;
	using trace = field_trace_of_dynamic <R> ::template trace <R>;

	auto &dyn = field_trace_get(trace(), value);
	auto offset = field_trace_offset(trace(), value);

	return std::tuple <decltype(dyn), size_t> (dyn, offset);
}

// p.dynamic() -> offset, reference to unsized_array

auto f = []
{
	auto p = $mirror(Points) {};
	p.array.positions.push_back(glm::vec2(1.0));
	p.array.positions.push_back(glm::vec2(1.0));
	p.array.positions.push_back(glm::vec2(1.0));

	static_assert(offsetof(decltype(p), count) == 0);
	static_assert(offsetof(decltype(p), array.positions) == 32);
	static_assert(p.offset <0> () == 0);
	static_assert(p.offset <1> () == 16);
	static_assert(std::decay_t <decltype(p.get <1> ())> ::offset <1> () == 16);

	auto &ar = field_trace_get(field_trace <decltype(p), 1, 1> (), p);

	auto offset = field_trace_offset(field_trace <decltype(p), 1, 1> (), p);
	fmt::println("offset from field trace: {}", offset);

	auto [dyn, off] = dynamic_part <Points> (p);
	fmt::println("offset from dynamic part: {}", off);
	fmt::println("number of eleemtns in dynamic part: {}", dyn.size());

	using element = std::decay_t <decltype(dyn)> ::value_type;

	auto arr = $mirror(array <f32>) {};
	arr.push_back(12);

	auto [dyn2, off2] = dynamic_part <array <vec2>> (arr);

	fmt::println("offset from dynamic part: {}", off2);
	fmt::println("number of eleemtns in dynamic part: {}", dyn2.size());

	// TODO: need to get the type of teh dynamic part...

	return 0;
} ();

// Mirror buffer
template <reflected T, template <typename> typename Engine = layouts::std430>
struct MirrorBuffer : Buffer {};

template <reflected T, template <typename> typename Engine>
requires (is_static_v <T>)
struct MirrorBuffer <T, Engine> : Buffer {
	using value_type = data_mirror <T, Engine> ::type;

	value_type new_value() const {
		return value_type();
	}

	void write(const value_type &data) const
	{
		Buffer::write(&data, sizeof(value_type), 0);
	}

	static MirrorBuffer from(const Device &device, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
	{
		auto base = Buffer::from(device, sizeof(value_type), usage, properties);

		MirrorBuffer result;
		static_cast <Buffer &> (result) = base;
		return result;
	}
};

template <reflected T, template <typename> typename Engine>
requires (is_dynamic_v <T>)
struct MirrorBuffer <T, Engine> : Buffer {
	using value_type = data_mirror <T, Engine> ::type;
	
	value_type new_value() const {
		return value_type();
	}

	void write(const value_type &data) const
	{
		// TODO: do this all in one mapped context
		auto [dyn, offset] = dynamic_part <T> (data);
		if (offset > 0)
			Buffer::write(&data, offset, 0);

		using element = std::decay_t <decltype(dyn)> ::value_type;

		Buffer::write(dyn.data(), dyn.size() * sizeof(element), offset);
	}

	static MirrorBuffer from(const Device &device, size_t max_elements, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
	{
		value_type data;
		auto [dyn, offset] = dynamic_part <T> (data);
		using element = std::decay_t <decltype(dyn)> ::value_type;

		auto base = Buffer::from(device, offset + max_elements * sizeof(element), usage, properties);

		MirrorBuffer result;
		static_cast <Buffer &> (result) = base;
		return result;
	}
};

// TODO: write down the implementation design somewhere in docs...
// will help spotting parts that are stupid
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
	// auto &ldev = device.logical;

	auto window = Window::from(session, device);

	auto queue = Queue::from(device);
	auto cpool = CommandPool::from(device, queue);
	auto cmdbuffers = group(device, cpool).allocate(window.frames_in_flight);

	auto compiler = Compiler::from(device, Compiler::Info());
	
	auto vspv = compiler.glsl_to_spirv(vshader, EShLangVertex);
	auto fspv = compiler.glsl_to_spirv(fshader, EShLangFragment);

	auto vmodule = compiler.spirv_to_shader_module(vspv);
	auto fmodule = compiler.spirv_to_shader_module(fspv);

	// TODO: translate a sequence of types to binding AND attributes
	// 
	// TODO: what if we encode the vertices? then jit vec and struct vec
	// should be different... but we can still treat the encoded vec
	// as a vec2 in DSL code...
	// Or, better yet, allow the user to define their own derivative
	// which is then translated in different ways...
	//
	// TODO: actually vertices also have a layout (default would be scalar)
	// but that still doesnt fix the encoding shit
	auto binding_descs = std::array {
		vk::VertexInputBindingDescription()
			.setBinding(0)
			.setStride(sizeof(glm::vec2))
			.setInputRate(vk::VertexInputRate::eVertex),
	};

	auto attribute_descs = std::array {
		vk::VertexInputAttributeDescription()
			.setLocation(0)
			.setBinding(0)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setOffset(0),
	};

	auto attachments = Attachments();

	attachments["color"] = vk::AttachmentDescription()
		.setFormat(window.format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

	auto color = attachments.reference("color", vk::ImageLayout::eColorAttachmentOptimal);

	// TODO: could make this a group(device, dld) method?
	auto rp = renderpass(device, dld,
		attachments,
		subpass(color_attachments { color },
			depth_attachments {},
			color_attachments {},
			input_attachments {})
	);

	std::vector <vk::Framebuffer> framebuffers;
	framebuffers.reserve(window.images.size());
	for (auto &image : window.images) {
		auto fb_info = vk::FramebufferCreateInfo()
			.setRenderPass(rp)
			.setAttachments(image.view)
			.setWidth(image.extent.width)
			.setHeight(image.extent.height)
			.setLayers(1);

		framebuffers.push_back(device.logical.createFramebuffer(fb_info, nullptr, dld));
	}

	auto pipeline_info = TraditionalGraphicsPipeline::Info {
		.vmodule = vmodule,
		.fmodule = fmodule,
		.renderpass = rp,
		.extent = window.extent(),
		.bindings = binding_descs,
		.attributes = attribute_descs,
	};

	auto pipeline = TraditionalGraphicsPipeline::from(device, dld, pipeline_info);

	auto vbuf = MirrorBuffer <array <vec2>> ::from(
		device,
		12,
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible
		| vk::MemoryPropertyFlagBits::eHostCoherent
	);

	auto vbuf_value = vbuf.new_value();

	vbuf_value.resize(3);
	vbuf_value[0] = glm::vec2(-0.5f, -0.5f);
	vbuf_value[1] = glm::vec2(0.5f, -0.5f);
	vbuf_value[2] = glm::vec2(0.0f, 0.5f);

	vbuf.write(vbuf_value);

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

		auto &image = window.image(frame.image_index);
		auto to_color = image.memory_barrier(
			vk::ImageLayout::eColorAttachmentOptimal,
			{},
			vk::AccessFlagBits::eColorAttachmentWrite
		);

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, {}, {}, to_color
		);

		auto render_area = vk::Rect2D()
			.setOffset({ 0, 0 })
			.setExtent(frame.extent);

		auto clear_value = vk::ClearValue()
			.setColor(vk::ClearColorValue(std::array <float, 4> { 0.05f, 0.05f, 0.05f, 1.0f }));

		auto rp_begin = vk::RenderPassBeginInfo()
			.setRenderPass(rp)
			.setFramebuffer(framebuffers[frame.image_index])
			.setRenderArea(render_area)
			.setClearValues(clear_value);

		cmd.beginRenderPass(rp_begin, vk::SubpassContents::eInline);
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		vk::DeviceSize vb_offset = 0;
		cmd.bindVertexBuffers(0, vbuf.handle, vb_offset);
		cmd.draw(3, 1, 0, 0);
		cmd.endRenderPass();

		auto to_present = image.memory_barrier(
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits::eColorAttachmentWrite,
			{}
		);

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eBottomOfPipe,
			{}, {}, {}, to_present
		);

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
