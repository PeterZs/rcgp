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

int main()
{
	auto vs = $vertex $fn($use(camera), $use(transforms), Vertex vertex) -> $returns(RasterForward)
	{
		mat4 xform = transforms[1];
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

	fmt::println("{}", generators::Assembly(fs).generate());

	// fmt::println("glsl:");
	// fmt::println("{}", generators::GLSL(vs).generate());
}
