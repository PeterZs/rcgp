#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <mrd.hpp>

#include <ugp.hpp>

#include "util/aperature.hpp"
#include "util/transform.hpp"

// struct alignas(0) Points {
// 	u32 count;
//
// 	struct Array {
// 		vec3 delta;
//
// 		// array <u32> positions;
// 		array <vec2> positions;
//
// 		$reflection(delta, positions);
// 	} array;
//
// 	$reflection(count, array);
// 	// TODO: if T is a dynamic aggregate,
// 	// then align should fallback to 0 (i.e. natural)
// 	// for it to work would be need to pack structs with pragma?
// };

struct View {
	mat4 proj;
	mat4 view;
	mat4 model;

	$reflection(proj, view, model);
};

AttributeStream <vec3> position;
AttributeStream <vec3> normal;

MonoConstantBuffer <View> view;

auto vs = $vertex $fn($use(position), $use(normal), $use(view)) -> $returns(Position, vec3)
{
	auto mvp = view.proj * view.view * view.model;
	auto normal_mat = transpose(inverse(mat3(view.model)));
	auto world_n = normalize(normal_mat * normal);
	$return std::tuple(Position(mvp * vec4(position, 1.0)), world_n);
};

auto fs = $fragment $fn(vec3 normal) -> $returns(vec4)
{
	$return vec4(0.5 * normalize(normal) + 0.5, 1.0f);
};

// TODO: structs need a layout (defaults to std430)
// 
// TODO: work group is a parameter to shaders that is an intrinsic...
// WorkGroup <8, 8> group... group.block_idx, thread_idx and so on
//
// TODO: write down the implementation design somewhere in docs...
// will help spotting parts that are stupid
int main()
{
	auto session_info = Session::Info {
		// .validation = false,
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

	const auto depth_format = vk::Format::eD32Sfloat;

	std::vector <Image> depth_images;
	depth_images.reserve(window.images.size());
	for (size_t i = 0; i < window.images.size(); ++i) {
		depth_images.push_back(Image::from(
			device,
			window.extent(),
			depth_format,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::ImageAspectFlagBits::eDepth,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		));
	}

	auto queue = Queue::from(device);
	auto cpool = CommandPool::from(device, queue);
	auto cmdbuffers = group(device, cpool).allocate(window.frames_in_flight);

	auto compiler = Compiler::from(device, Compiler::Info());
	
	using allocation = std::tuple <
		group_allocation_record <view, 0>
	>;

	auto map = new_allocation(allocation());
	vs.apply_group_allocation_map(map);
	fs.apply_group_allocation_map(map);

	auto vshader = generators::GLSL(vs).generate();
	auto fshader = generators::GLSL(fs).generate();

	fmt::println("vertex shader:\n{}", vshader);
	fmt::println("fragment shader:\n{}", fshader);
	
	auto vspv = compiler.glsl_to_spirv(vshader, EShLangVertex);
	auto fspv = compiler.glsl_to_spirv(fshader, EShLangFragment);

	auto vmodule = compiler.spirv_to_shader_module(vspv);
	auto fmodule = compiler.spirv_to_shader_module(fspv);

	auto binding_descs = std::array {
		vk::VertexInputBindingDescription()
			.setBinding(0)
			.setStride(sizeof(glm::vec3))
			.setInputRate(vk::VertexInputRate::eVertex),
		vk::VertexInputBindingDescription()
			.setBinding(1)
			.setStride(sizeof(glm::vec3))
			.setInputRate(vk::VertexInputRate::eVertex),
	};

	auto attribute_descs = std::array {
		vk::VertexInputAttributeDescription()
			.setLocation(0)
			.setBinding(0)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(0),
		vk::VertexInputAttributeDescription()
			.setLocation(1)
			.setBinding(1)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(0),
	};

	// TODO: should be able to construct the pipeline layout from the group allocation
	auto set_layout_binding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	auto descriptor_set_layout = device.logical.createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo().setBindings(set_layout_binding),
		nullptr,
		dld
	);

	auto pipeline_layout = device.logical.createPipelineLayout(
		vk::PipelineLayoutCreateInfo().setSetLayouts(descriptor_set_layout),
		nullptr,
		dld
	);

	auto attachments = Attachments();

	attachments["color"] = vk::AttachmentDescription()
		.setFormat(window.format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	attachments["depth"] = vk::AttachmentDescription()
		.setFormat(depth_format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	auto color = attachments.reference("color", vk::ImageLayout::eColorAttachmentOptimal);
	auto depth = attachments.reference("depth", vk::ImageLayout::eDepthStencilAttachmentOptimal);

	// TODO: could make this a group(device, dld) method?
	auto rp = renderpass(device, dld,
		attachments,
		subpass(color_attachments { color },
			depth_attachments { depth },
			color_attachments {},
			input_attachments {})
	);

	std::vector <vk::Framebuffer> framebuffers;
	framebuffers.reserve(window.images.size());
	for (size_t i = 0; i < window.images.size(); ++i) {
		auto &image = window.images[i];
		auto &depth = depth_images[i];
		// TODO: wrapper... group(device, rp).new_framebuffer(image, 1)?
		std::array attachments_views { image.view, depth.view };
		auto fb_info = vk::FramebufferCreateInfo()
			.setRenderPass(rp)
			.setAttachments(attachments_views)
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
		.layout = pipeline_layout,
		.depth_test = true,
	};

	auto pipeline = TraditionalGraphicsPipeline::from(device, dld, pipeline_info);

	constexpr mrd::ModelEncodings encodings;

	using Model = mrd::Model <encodings>;

	auto model = Model::load("/home/venki/data/models/asian-dragon.stl");
	auto &mesh = model.meshes[0];
	fmt::println("# of verts: {}, # of triangles: {}", mesh.positions.size(), mesh.primitives.size());

	auto pbuf = MirrorBuffer <array <vec3>, layouts::scalar> ::from(
		device, mesh.positions.size(),
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible
		| vk::MemoryPropertyFlagBits::eHostCoherent
	);

	auto pbuf_value = pbuf.new_value();
	pbuf_value.resize(mesh.positions.size());
	std::memcpy(pbuf_value.data(), mesh.positions.data(), sizeof(glm::vec3) * pbuf_value.size());
	pbuf.write(pbuf_value);

	auto nbuf = MirrorBuffer <array <vec3>, layouts::scalar> ::from(
		device, mesh.normals.size(),
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible
			| vk::MemoryPropertyFlagBits::eHostCoherent
	);

	auto nbuf_value = nbuf.new_value();
	nbuf_value.resize(mesh.normals.size());
	std::memcpy(nbuf_value.data(), mesh.normals.data(), sizeof(glm::vec3) * nbuf_value.size());
	nbuf.write(nbuf_value);

	auto ibuf = MirrorBuffer <array <ivec3>, layouts::scalar> ::from(
		device, mesh.primitives.size(),
		vk::BufferUsageFlagBits::eIndexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible
			| vk::MemoryPropertyFlagBits::eHostCoherent
	);

	auto ibuf_value = ibuf.new_value();
	ibuf_value.resize(mesh.primitives.size());
	std::memcpy(ibuf_value.data(), mesh.primitives.data(), sizeof(glm::ivec3) * ibuf_value.size());
	ibuf.write(ibuf_value);

	// Camera
	Aperature aperature;
	Transform xform;

	xform.translation = glm::vec3(0, 0, -100);
	aperature.aspect = static_cast <float> (window.extent().width) / window.extent().height;

	// TODO: if we have a conditional mirror buffer with prepopulated
	// flags, then move extra flags to the last param...
	auto view_buf = MirrorBuffer <View> ::from(
		device,
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible
		| vk::MemoryPropertyFlagBits::eHostCoherent
	);

	view_buf.write($mirror(View) {
		.proj = aperature.projection_matrix(),
		.view = xform.view_matrix(),
		.model = glm::mat4(1.0f),
	});

	auto descriptor_pool = DescriptorPool::from(device, DescriptorPool::Info {
		.max_sets = 1,
		.uniform_buffers = 1,
	});

	auto descriptor_set = device.logical.allocateDescriptorSets(
		vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(descriptor_pool)
			.setSetLayouts(descriptor_set_layout),
		dld
	).front();

	auto view_buf_info = view_buf.descriptor_info();

	auto descriptor_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setBufferInfo(view_buf_info);

	device.logical.updateDescriptorSets(descriptor_write, nullptr, dld);

	while (window.alive()) {
		window.poll();

		static double last_time = glfwGetTime();
		double now = glfwGetTime();
		float dt = float(now - last_time);
		last_time = now;

		const float move_speed = 5.0f;
		glm::vec3 move(0.0f);
		if (window.is_pressed(Key::W)) move.z += 1.0f;
		if (window.is_pressed(Key::S)) move.z -= 1.0f;
		if (window.is_pressed(Key::A)) move.x += 1.0f;
		if (window.is_pressed(Key::D)) move.x -= 1.0f;
		if (window.is_pressed(Key::E)) move.y += 1.0f;
		if (window.is_pressed(Key::Q)) move.y -= 1.0f;
		if (glm::length(move) > 0.0f)
			xform.translation += glm::normalize(move) * move_speed * dt;

		auto frame = window.next_frame();

		auto extent = frame.extent;
		auto angle = static_cast <float> (glfwGetTime());
		auto model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, angle * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
		aperature.aspect = static_cast <float> (extent.width) / extent.height;

		view_buf.write($mirror(View) {
			.proj = aperature.projection_matrix(),
			.view = xform.view_matrix(),
			.model = model,
		});

		if (window.is_pressed(Key::Escape))
			window.close();

		group(device, window).wait(frame);
		auto acquired = group(device, window).acquire_image(frame);
		if (!acquired)
			continue;

		auto &cmd = cmdbuffers[window.frame_index];

		cmd.reset();
		cmd.begin(vk::CommandBufferBeginInfo());

		auto render_area = vk::Rect2D()
			.setOffset({ 0, 0 })
			.setExtent(frame.extent);

		auto clear_values = std::array {
			vk::ClearValue().setColor(vk::ClearColorValue(std::array <float, 4> { 0.05f, 0.05f, 0.05f, 1.0f })),
			vk::ClearValue().setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0)),
		};

		auto rp_begin = vk::RenderPassBeginInfo()
			.setRenderPass(rp)
			.setFramebuffer(framebuffers[frame.image_index])
			.setRenderArea(render_area)
			.setClearValues(clear_values);

		cmd.beginRenderPass(rp_begin, vk::SubpassContents::eInline);
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		cmd.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			pipeline.layout,
			0, descriptor_set, {}
		);
		cmd.bindVertexBuffers(0, { pbuf.handle, nbuf.handle }, { 0, 0 });
		cmd.bindIndexBuffer(ibuf.handle, 0, vk::IndexType::eUint32);
		cmd.drawIndexed(3 * mesh.primitives.size(), 1, 0, 0, 0);
		cmd.endRenderPass();
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
