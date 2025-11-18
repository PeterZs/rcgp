#include <fmt/printf.h>

#include <ugp.hpp>

struct Vertex {
	vec3 position;
	vec3 normal;
	vec2 uv;

	InstanceIndex instance;

	$reflection(position, normal, uv, instance);
};

struct RasterForward {
	Position svpos;
	Smooth <vec3> position;
	Smooth <vec3> normal;
	Smooth <vec2> uv;

	$reflection(svpos, position, normal, uv);
};

struct FragmentInput {
	vec3 position;
	vec3 normal;
	vec2 uv;

	$reflection(position, normal, uv);
};

struct Camera {
	mat4 view;
	mat4 proj;

	vec4 project(vec4 p) const {
		return proj * view * p;
	}

	$reflection(view, proj);
};

ParameterBlock <Camera> camera;

using Transforms = array <mat4>;

StructuredBuffer <Transforms> transforms;

// Descriptor group allocation
template <auto &rsrc, size_t I>
struct group_allocation_record {};

template <auto &rsrc, size_t I>
bool fill_allocation(group_allocation_map &map, group_allocation_record <rsrc, I>)
{
	map.emplace((void *) &rsrc, I);
	return true;
}

template <typename ... Records>
auto new_allocation(std::tuple <Records...> records)
{
	group_allocation_map map;
	std::apply([&](auto ... xs) {
		std::make_tuple(fill_allocation(map, xs)...);
	}, records);
	return map;
}

int main()
{
	// TODO: ensure in compile that ALL resources are bound by reference
	auto vs = $vertex $fn($use(camera), $use(transforms), Vertex vertex) -> $returns(RasterForward)
	{
		// mat4 xform = transforms[1];
		mat4 xform = transforms[vertex.instance];
		vec4 ppos = xform * vec4(vertex.position, 1);
		vec4 pnorm = xform * vec4(vertex.normal, 0);
		$return RasterForward {
			.svpos = camera.project(ppos),
			.position = vec3(ppos),
			.normal = vec3(pnorm),
			.uv = vertex.uv,
		};
	};

	auto fs = $fragment $fn(FragmentInput fin) -> $returns(vec4)
	{
		vec3 light = normalize(vec3(1, 1, 1));
		f32 ndotl = max(dot(fin.normal, light), 0.0f);
		$return vec4(vec3(ndotl), 1.0);
	};

	// TODO: should lift to push constant if possible...
	// everything else is degenerated to constant/uniform buffer
	using allocation = std::tuple <
		group_allocation_record <camera, 0>,
		group_allocation_record <transforms, 1>
	>;

	auto map = new_allocation(allocation());
	vs.apply_group_allocation_map(map);

	// fmt::println("{}", generators::Assembly(vs).generate());

	fmt::println("{}", generators::GLSL(vs).generate());
}
