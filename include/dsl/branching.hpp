#pragma once

#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <vector>

#include "scalar.hpp"

using _branch_body = std::function <void ()>;

struct _if {
	boolean cond;
	_branch_body body;
};

struct _elif {
	boolean cond;
	_branch_body body;
};

struct _else {
	_branch_body body;
};

struct _branch_holder {
	std::optional <_if> ifs;
	std::list <_elif> elifs;
	std::optional <_else> elses;

	_branch_holder() = default;

	_branch_holder(const _branch_holder &) = delete;
	_branch_holder &operator=(const _branch_holder &) = delete;

	_branch_holder(_branch_holder &&other) {
		ifs = std::move(other.ifs);
		elifs = std::move(other.elifs);
		elses = std::move(other.elses);

		other.ifs.reset();
		other.elifs.clear();
		other.elses.reset();
	}

	~_branch_holder();
};

// TODO: move to source
inline _elif operator*(_elif a, const auto &ftn)
{
	return _elif(a.cond, _branch_body(ftn));
}

inline _else operator*(_else a, const auto &ftn)
{
	return _else(_branch_body(ftn));
}

inline _branch_holder operator*(_if a, const auto &ftn)
{
	auto result = _branch_holder();
	result.ifs = _if(a.cond, _branch_body(ftn));
	return result;
}

_branch_holder operator+(_branch_holder &&a, _elif b);
_branch_holder operator+(_branch_holder &&a, _else b);

#define $if(cond)	_if(cond) * [&]
#define $elif(cond)	+ _elif(cond) * [&]
#define $else		+ _else() * [&]
