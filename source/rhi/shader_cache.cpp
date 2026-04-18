#include <atomic>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <system_error>
#include <thread>

#include <unistd.h>

#include <fmt/format.h>

#include <glslang/build_info.h>

#include "rhi/shader_cache.hpp"

namespace rcgp {

static auto resolve_root(const std::filesystem::path &explicit_path) -> std::filesystem::path
{
	if (!explicit_path.empty())
		return explicit_path;

	if (const char *env = std::getenv("RCGP_SHADER_CACHE_DIR"); env && *env)
		return std::filesystem::path(env);

	if (const char *xdg = std::getenv("XDG_CACHE_HOME"); xdg && *xdg)
		return std::filesystem::path(xdg) / "rcgp" / "shaders";

	if (const char *home = std::getenv("HOME"); home && *home)
		return std::filesystem::path(home) / ".cache" / "rcgp" / "shaders";

	return std::filesystem::current_path() / ".rcgp-shader-cache";
}

static auto hex32(std::array <uint64_t, 2> h) -> std::string
{
	return fmt::format("{:016x}{:016x}", h[0], h[1]);
}

static auto glsl_root(const std::filesystem::path &r) -> std::filesystem::path
{
	return r / fmt::format("v{}", CACHE_FORMAT_VERSION) / "glsl";
}

static auto spirv_root(const std::filesystem::path &r) -> std::filesystem::path
{
	return r / fmt::format("v{}", CACHE_FORMAT_VERSION) / "spirv";
}

static auto random_suffix() -> std::string
{
	static std::atomic <uint64_t> counter { 0 };
	auto n = counter.fetch_add(1, std::memory_order_relaxed);
	auto tid = uint64_t(std::hash <std::thread::id> {} (std::this_thread::get_id()));
	return fmt::format("{:x}-{:x}-{:x}", ::getpid(), tid, n);
}

static bool atomic_write(
	const std::filesystem::path &final_path,
	std::span <const std::byte> header_bytes,
	std::span <const std::byte> canonical_bytes,
	std::span <const std::byte> payload_bytes
)
{
	std::error_code ec;
	std::filesystem::create_directories(final_path.parent_path(), ec);

	auto tmp = final_path;
	tmp += fmt::format(".tmp-{}", random_suffix());

	{
		std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
		if (!out)
			return false;

		out.write(reinterpret_cast <const char *> (header_bytes.data()), header_bytes.size());
		out.write(reinterpret_cast <const char *> (canonical_bytes.data()), canonical_bytes.size());
		out.write(reinterpret_cast <const char *> (payload_bytes.data()), payload_bytes.size());

		if (!out) {
			std::filesystem::remove(tmp, ec);
			return false;
		}
	}

	std::filesystem::rename(tmp, final_path, ec);
	if (ec)
		std::filesystem::remove(tmp, ec);

	return !ec;
}

static auto read_whole(const std::filesystem::path &p) -> std::vector <std::byte>
{
	std::ifstream in(p, std::ios::binary | std::ios::ate);
	if (!in)
		return {};

	auto end = in.tellg();
	in.seekg(0, std::ios::beg);
	if (end <= 0)
		return {};

	auto size = size_t(end);
	std::vector <std::byte> buf(size);
	in.read(reinterpret_cast <char *> (buf.data()), buf.size());
	if (!in)
		return {};

	return buf;
}

static auto mix_hash(std::array <uint64_t, 2> base, std::array <uint64_t, 2> salt) -> std::array <uint64_t, 2>
{
	return { base[0] ^ salt[0], base[1] ^ salt[1] };
}

uint32_t glslang_version_token()
{
	return (uint32_t(GLSLANG_VERSION_MAJOR) << 24)
		| (uint32_t(GLSLANG_VERSION_MINOR) << 16)
		| (uint32_t(GLSLANG_VERSION_PATCH));
}

ShaderCache::ShaderCache(ShaderCacheConfig cfg)
	: config(std::move(cfg))
	, resolved_root(resolve_root(config.root))
{
}

auto ShaderCache::spirv_key(
	const BlockCanonical &canonical,
	const CacheFileHeader &hdr
) -> std::array <uint64_t, 2>
{
	auto salt_str = fmt::format(
		"rcgp-spirv-v{}|stage={}|version={}|debug={}|glslang={}|target=SPV_1_6",
		CACHE_FORMAT_VERSION,
		hdr.stage,
		hdr.glsl_version,
		hdr.debug_info,
		hdr.glslang_ver_hash
	);
	return mix_hash(canonical.hash, fnv1a_128(salt_str));
}

auto ShaderCache::load_glsl(const BlockCanonical &canonical) const -> std::optional <std::string>
{
	if (!config.enabled)
		return std::nullopt;

	auto path = glsl_root(resolved_root) / (hex32(canonical.hash) + ".glsl.cache");
	auto buf = read_whole(path);
	if (buf.size() < sizeof(CacheFileHeader))
		return std::nullopt;

	CacheFileHeader hdr {};
	std::memcpy(&hdr, buf.data(), sizeof(hdr));
	if (std::memcmp(hdr.magic, "RGLS", 4) != 0)
		return std::nullopt;
	if (hdr.cache_format_ver != CACHE_FORMAT_VERSION)
		return std::nullopt;
	if (hdr.payload_kind != 0)
		return std::nullopt;
	if (hdr.canonical_len != canonical.bytes.size())
		return std::nullopt;

	size_t canonical_off = sizeof(CacheFileHeader);
	size_t payload_off = canonical_off + hdr.canonical_len;
	if (buf.size() < payload_off + hdr.payload_len)
		return std::nullopt;

	if (std::memcmp(buf.data() + canonical_off, canonical.bytes.data(), hdr.canonical_len) != 0)
		return std::nullopt;

	return std::string(
		reinterpret_cast <const char *> (buf.data() + payload_off),
		hdr.payload_len
	);
}

void ShaderCache::store_glsl(const BlockCanonical &canonical, const std::string &glsl) const
{
	if (!config.enabled || config.read_only)
		return;

	CacheFileHeader hdr {};
	std::memcpy(hdr.magic, "RGLS", 4);
	hdr.cache_format_ver = CACHE_FORMAT_VERSION;
	hdr.glslang_ver_hash = 0;
	hdr.payload_kind = 0;
	hdr.canonical_len = uint32_t(canonical.bytes.size());
	hdr.payload_len = uint32_t(glsl.size());

	auto path = glsl_root(resolved_root) / (hex32(canonical.hash) + ".glsl.cache");

	auto hdr_span = std::span <const std::byte> (reinterpret_cast <const std::byte *> (&hdr), sizeof(hdr));
	auto can_span = std::span <const std::byte> (canonical.bytes.data(), canonical.bytes.size());
	auto pay_span = std::span <const std::byte> (reinterpret_cast <const std::byte *> (glsl.data()), glsl.size());

	(void) atomic_write(path, hdr_span, can_span, pay_span);
}

auto ShaderCache::load_spirv(
	const BlockCanonical &canonical,
	const CacheFileHeader &expect
) const -> std::optional <std::vector <uint32_t>>
{
	if (!config.enabled)
		return std::nullopt;

	auto key = spirv_key(canonical, expect);
	auto path = spirv_root(resolved_root) / (hex32(key) + ".spv.cache");
	auto buf = read_whole(path);
	if (buf.size() < sizeof(CacheFileHeader))
		return std::nullopt;

	CacheFileHeader hdr {};
	std::memcpy(&hdr, buf.data(), sizeof(hdr));
	if (std::memcmp(hdr.magic, "RSPV", 4) != 0)
		return std::nullopt;
	if (hdr.cache_format_ver != CACHE_FORMAT_VERSION)
		return std::nullopt;
	if (hdr.payload_kind != 1)
		return std::nullopt;
	if (hdr.glslang_ver_hash != expect.glslang_ver_hash)
		return std::nullopt;
	if (hdr.stage != expect.stage)
		return std::nullopt;
	if (hdr.debug_info != expect.debug_info)
		return std::nullopt;
	if (hdr.glsl_version != expect.glsl_version)
		return std::nullopt;
	if (hdr.canonical_len != canonical.bytes.size())
		return std::nullopt;

	size_t canonical_off = sizeof(CacheFileHeader);
	size_t words = hdr.payload_len;
	size_t payload_off = canonical_off + hdr.canonical_len;
	if (buf.size() < payload_off + words * sizeof(uint32_t))
		return std::nullopt;

	if (std::memcmp(buf.data() + canonical_off, canonical.bytes.data(), hdr.canonical_len) != 0)
		return std::nullopt;

	std::vector <uint32_t> spirv(words);
	std::memcpy(spirv.data(), buf.data() + payload_off, words * sizeof(uint32_t));
	return spirv;
}

void ShaderCache::store_spirv(
	const BlockCanonical &canonical,
	const std::vector <uint32_t> &spirv,
	const CacheFileHeader &caller_hdr
) const
{
	if (!config.enabled || config.read_only)
		return;
	if (spirv.empty())
		return;

	CacheFileHeader hdr = caller_hdr;
	std::memcpy(hdr.magic, "RSPV", 4);
	hdr.cache_format_ver = CACHE_FORMAT_VERSION;
	hdr.payload_kind = 1;
	hdr.canonical_len = uint32_t(canonical.bytes.size());
	hdr.payload_len = uint32_t(spirv.size());

	auto key = spirv_key(canonical, hdr);
	auto path = spirv_root(resolved_root) / (hex32(key) + ".spv.cache");

	auto hdr_span = std::span <const std::byte> (reinterpret_cast <const std::byte *> (&hdr), sizeof(hdr));
	auto can_span = std::span <const std::byte> (canonical.bytes.data(), canonical.bytes.size());
	auto pay_span = std::span <const std::byte> (reinterpret_cast <const std::byte *> (spirv.data()), spirv.size() * sizeof(uint32_t));

	(void) atomic_write(path, hdr_span, can_span, pay_span);
}

} // namespace rcgp
