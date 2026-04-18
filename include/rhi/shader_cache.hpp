#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "../dsl/block_canonical.hpp"

namespace rcgp {

inline constexpr uint32_t CACHE_FORMAT_VERSION = 1;

#pragma pack(push, 4)
struct CacheFileHeader {
	char magic[4];
	uint32_t cache_format_ver;
	uint32_t glslang_ver_hash;
	uint32_t payload_kind;
	uint32_t canonical_len;
	uint32_t payload_len;
	uint32_t stage;
	uint32_t debug_info;
	uint32_t glsl_version;
	uint32_t reserved;
};
#pragma pack(pop)

static_assert(sizeof(CacheFileHeader) == 40);

struct ShaderCacheConfig {
	std::filesystem::path root;
	bool enabled = false;
	bool read_only = false;
};

struct ShaderCache {
	ShaderCacheConfig config;
	std::filesystem::path resolved_root;

	explicit ShaderCache(ShaderCacheConfig cfg = {});

	auto load_glsl(const BlockCanonical &canonical) const -> std::optional <std::string>;
	void store_glsl(const BlockCanonical &canonical, const std::string &glsl) const;

	auto load_spirv(const BlockCanonical &canonical, const CacheFileHeader &expect) const -> std::optional <std::vector <uint32_t>>;
	void store_spirv(const BlockCanonical &canonical, const std::vector <uint32_t> &spirv, const CacheFileHeader &hdr) const;

	static auto spirv_key(const BlockCanonical &canonical, const CacheFileHeader &hdr) -> std::array <uint64_t, 2>;
};

uint32_t glslang_version_token();

} // namespace rcgp
