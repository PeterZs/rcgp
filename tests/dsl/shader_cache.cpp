#include <cstring>
#include <filesystem>
#include <fstream>
#include <thread>
#include <unistd.h>

#include <fmt/format.h>

#include "rhi/shader_cache.hpp"
#include "rhi/shader_compiler.hpp"

#include "common.hpp"
#include "common_resources.hpp"

#define SUITE "shader_cache"

static std::filesystem::path make_cache_root()
{
	static int counter = 0;
	auto root = std::filesystem::temp_directory_path()
		/ fmt::format("rcgp-cache-{}-{}", ::getpid(), counter++);
	std::filesystem::remove_all(root);
	return root;
}

struct CacheFixture {
	std::filesystem::path root;

	CacheFixture() : root(make_cache_root()) {}

	~CacheFixture() {
		std::error_code ec;
		std::filesystem::remove_all(root, ec);
	}

	auto config(bool enabled = true, bool read_only = false) const -> ShaderCacheConfig {
		return ShaderCacheConfig { .root = root, .enabled = enabled, .read_only = read_only };
	}
};

static auto build_vs_constant()
{
	return $shader(vertex)(ClipPosition clip) {
		clip = float4(0.0f);
	};
};

static auto build_vs_constant_one()
{
	return $shader(vertex)(ClipPosition clip) {
		clip = float4(1.0f);
	};
};

template <uint32_t X>
static auto build_cs()
{
	return $shader(compute)(WorkGroup <X> group) {};
}

static auto build_cs_resources()
{
	return $shader(compute)(
		$contracts(
			(positions, meshlets::positions),
			(view, meshlets::view)
		),
		WorkGroup <1> group
	) {
		float4 p = positions[u32(0)];
		float4x4 vp = view.view_proj;
	};
};

add_test(canonical_hash_is_deterministic)
{
	auto a = build_vs_constant();
	auto b = build_vs_constant();
	auto ca = canonicalize_block(a);
	auto cb = canonicalize_block(b);
	if (ca.hash != cb.hash) mark_fail;
};

add_test(canonical_bytes_are_byte_equal)
{
	auto a = build_vs_constant();
	auto b = build_vs_constant();
	auto ca = canonicalize_block(a);
	auto cb = canonicalize_block(b);
	if (ca.bytes != cb.bytes) mark_fail;
};

add_test(hash_differs_for_constant_change)
{
	auto a = build_vs_constant();
	auto b = build_vs_constant_one();
	auto ca = canonicalize_block(a);
	auto cb = canonicalize_block(b);
	if (ca.hash == cb.hash) mark_fail;
};

add_test(hash_differs_by_workgroup_size)
{
	auto a = build_cs <1> ();
	auto b = build_cs <8> ();
	auto ca = canonicalize_block(a);
	auto cb = canonicalize_block(b);
	if (ca.hash == cb.hash) mark_fail;
};

add_test(hash_stable_under_pointer_shuffle)
{
	auto a = build_cs_resources();
	auto b = build_cs_resources();
	auto ca = canonicalize_block(a);
	auto cb = canonicalize_block(b);
	if (ca.bytes != cb.bytes) mark_fail;
	if (ca.hash != cb.hash) mark_fail;
};

add_test(spirv_cache_roundtrip)
{
	CacheFixture fix;
	ShaderCompiler compiler;
	compiler.cache = ShaderCache(fix.config());

	auto vs = build_vs_constant();
	auto first = compiler.compile(vs, ShaderStage::eVertex);
	if (first.spirv_cache_hit) mark_fail;
	if (first.spirv.empty()) mark_fail;

	auto vs2 = build_vs_constant();
	auto second = compiler.compile(vs2, ShaderStage::eVertex);
	if (!second.spirv_cache_hit) mark_fail;
	if (first.spirv != second.spirv) mark_fail;
};

add_test(glsl_cache_layer_hit_under_spirv_miss)
{
	CacheFixture fix;
	ShaderCompiler compiler;
	compiler.cache = ShaderCache(fix.config());
	compiler.debug_info = true;

	auto vs1 = build_vs_constant();
	auto first = compiler.compile(vs1, ShaderStage::eVertex);
	if (first.spirv_cache_hit) mark_fail;

	compiler.debug_info = false;
	auto vs2 = build_vs_constant();
	auto second = compiler.compile(vs2, ShaderStage::eVertex);
	if (second.spirv_cache_hit) mark_fail;
	if (!second.glsl_cache_hit) mark_fail;
};

add_test(debug_info_flag_forces_spirv_miss)
{
	CacheFixture fix;

	ShaderCompiler c1;
	c1.cache = ShaderCache(fix.config());
	c1.debug_info = true;

	ShaderCompiler c2;
	c2.cache = ShaderCache(fix.config());
	c2.debug_info = false;

	auto vs = build_vs_constant();
	auto a = c1.compile(vs, ShaderStage::eVertex);
	auto vs2 = build_vs_constant();
	auto b = c2.compile(vs2, ShaderStage::eVertex);

	if (a.spirv_cache_hit) mark_fail;
	if (b.spirv_cache_hit) mark_fail;

	size_t spv_count = 0;
	auto spv_dir = fix.root / fmt::format("v{}", CACHE_FORMAT_VERSION) / "spirv";
	if (std::filesystem::exists(spv_dir)) {
		for (auto &entry : std::filesystem::directory_iterator(spv_dir))
			if (entry.is_regular_file())
				spv_count++;
	}
	if (spv_count != 2) mark_fail;
};

add_test(value_compare_rejects_forged_collision)
{
	CacheFixture fix;
	ShaderCompiler compiler;
	compiler.cache = ShaderCache(fix.config());

	auto vs = build_vs_constant();
	auto first = compiler.compile(vs, ShaderStage::eVertex);
	if (first.spirv.empty()) mark_fail;

	auto spv_dir = fix.root / fmt::format("v{}", CACHE_FORMAT_VERSION) / "spirv";
	std::filesystem::path target;
	for (auto &entry : std::filesystem::directory_iterator(spv_dir)) {
		if (entry.is_regular_file()) {
			target = entry.path();
			break;
		}
	}
	if (target.empty()) {
		mark_fail;
		return;
	}

	std::vector <char> buf;
	{
		std::ifstream in(target, std::ios::binary | std::ios::ate);
		buf.resize(size_t(in.tellg()));
		in.seekg(0);
		in.read(buf.data(), buf.size());
	}
	if (buf.size() < sizeof(CacheFileHeader) + 1) {
		mark_fail;
		return;
	}
	buf[sizeof(CacheFileHeader)] ^= 0xff;
	{
		std::ofstream out(target, std::ios::binary | std::ios::trunc);
		out.write(buf.data(), buf.size());
	}

	auto vs2 = build_vs_constant();
	auto second = compiler.compile(vs2, ShaderStage::eVertex);
	if (second.spirv_cache_hit) mark_fail;
};

add_test(corrupted_header_treated_as_miss)
{
	CacheFixture fix;
	ShaderCompiler compiler;
	compiler.cache = ShaderCache(fix.config());

	auto vs = build_vs_constant();
	(void) compiler.compile(vs, ShaderStage::eVertex);

	auto spv_dir = fix.root / fmt::format("v{}", CACHE_FORMAT_VERSION) / "spirv";
	for (auto &entry : std::filesystem::directory_iterator(spv_dir)) {
		if (!entry.is_regular_file()) continue;
		std::ofstream out(entry.path(), std::ios::binary | std::ios::in);
		if (out) out.write("XXXX", 4);
	}

	auto vs2 = build_vs_constant();
	auto again = compiler.compile(vs2, ShaderStage::eVertex);
	if (again.spirv_cache_hit) mark_fail;
};

add_test(concurrent_writers_safe)
{
	CacheFixture fix;
	ShaderCompiler compiler;
	compiler.cache = ShaderCache(fix.config());

	std::vector <std::vector <uint32_t>> results(4);
	std::vector <std::thread> threads;
	for (size_t i = 0; i < 4; i++) {
		threads.emplace_back([&, i] {
			auto vs = build_vs_constant();
			auto r = compiler.compile(vs, ShaderStage::eVertex);
			results[i] = std::move(r.spirv);
		});
	}
	for (auto &t : threads)
		t.join();

	for (auto &r : results)
		if (r.empty()) mark_fail;
	for (size_t i = 1; i < results.size(); i++)
		if (results[i] != results[0]) mark_fail;
};

add_test(disabled_cache_is_passthrough)
{
	CacheFixture fix;
	ShaderCompiler compiler;
	compiler.cache = ShaderCache(fix.config(false));

	auto vs = build_vs_constant();
	auto first = compiler.compile(vs, ShaderStage::eVertex);
	if (first.spirv_cache_hit) mark_fail;

	auto vs2 = build_vs_constant();
	auto second = compiler.compile(vs2, ShaderStage::eVertex);
	if (second.spirv_cache_hit) mark_fail;
	if (second.glsl_cache_hit) mark_fail;
};

add_test(read_only_cache_does_not_write)
{
	CacheFixture fix;
	ShaderCompiler compiler;
	compiler.cache = ShaderCache(fix.config(true, true));

	auto vs = build_vs_constant();
	auto r = compiler.compile(vs, ShaderStage::eVertex);
	if (r.spirv.empty()) mark_fail;

	auto dir = fix.root / fmt::format("v{}", CACHE_FORMAT_VERSION);
	if (std::filesystem::exists(dir / "spirv")) {
		size_t n = 0;
		for (auto &e : std::filesystem::directory_iterator(dir / "spirv"))
			(void) e, n++;
		if (n != 0) mark_fail;
	}
};

add_test(default_compiler_is_byte_identical)
{
	ShaderCompiler uncached;

	auto vs = build_vs_constant();
	auto glsl = generate_glsl(vs);
	auto expected = uncached.glsl_to_spirv(glsl, ShaderStage::eVertex);

	ShaderCompiler wrap;
	auto vs2 = build_vs_constant();
	auto r = wrap.compile(vs2, ShaderStage::eVertex);

	if (r.spirv != expected) mark_fail;
};
