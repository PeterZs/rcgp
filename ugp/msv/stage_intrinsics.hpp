#pragma once

#include "../dsl/primitives.hpp"

// Required result of the vertex shader
struct Position : vec4 {
	// TODO: only emit when returning...
	Position(const vec4 &value, $location) {
		jems::store_loc(loc,
			jems::intrinsic_loc(loc, StageIntrinsic::eSVPosition),
			value
		);
	}
};

enum Topology {
	Point,
	Line,
	Triangle,
	Patch,
};

// Optional result of the vertex shader
struct Interpolant {
	template <typename T>
	struct Smooth : public T {
		Smooth(const T &value) : T(value) {}
	};
	
	template <typename T>
	struct Flat : public T {
		Flat(const T &value) : T(value) {}
	};

	template <typename T>
	struct Noperspective : public T {
		Noperspective(const T &value) : T(value) {}
	};
};

// Required result of the fragment shader
template <typename T>
struct SubpassResult : public T {
	SubpassResult(const T &value) : T(value) {}
};

// Required result of the task shader
// 
// for multiple groups per task shader invocation
// users can either have a struct with multiple instances
// or use an array of packets; in either case the
// payload must be the same and the total number of
// packets issues must be finite
template <typename T>
struct MeshGroupPacket : ivec3 {};

enum PatchType {
	eTriangle,
};

// Required result of the mesh shader
template <PatchType Patch, size_t MaxVertices, size_t MaxPrimitives>
struct PatchResult {
	i32 vertices;
	i32 primitives;

	// void set_output(i32 vertices, i32 primitives) {}
};
