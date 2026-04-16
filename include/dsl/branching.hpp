#pragma once

#include <functional>
#include <list>
#include <optional>
#include <utility>
#include <vector>

#include "aliases.hpp"
#include "relational.hpp"

namespace rcgp {

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

template <typename V>
struct _case {
	V value;
	_branch_body body;
};

template <typename V> _case(V) -> _case <V>;

struct _default_case {
	_branch_body body;
};

template <typename V>
inline _case <V> operator*(_case <V> a, const auto &ftn)
{
	return _case <V> { std::move(a.value), _branch_body(ftn) };
}

inline _default_case operator*(_default_case, const auto &ftn)
{
	return _default_case { _branch_body(ftn) };
}

void _emit_switch_branch(
	std::vector <std::pair <Reference, _branch_body>> segments,
	std::optional <_branch_body> fallback
);

template <typename Subj>
struct _switch_holder {
	Subj subject;
	std::list <std::pair <boolean, _branch_body>> segments;
	std::optional <_branch_body> fallback;

	explicit _switch_holder(const Subj &s) : subject(s) {}

	_switch_holder(const _switch_holder &) = delete;
	_switch_holder &operator=(const _switch_holder &) = delete;

	_switch_holder(_switch_holder &&other)
		: subject(std::move(other.subject))
		, segments(std::move(other.segments))
		, fallback(std::move(other.fallback))
	{
		other.segments.clear();
		other.fallback.reset();
	}

	~_switch_holder()
	{
		if (segments.empty() and not fallback.has_value())
			return;

		std::vector <std::pair <Reference, _branch_body>> flat;
		flat.reserve(segments.size());
		for (auto &seg : segments) {
			flat.emplace_back(
				static_cast <const Reference &> (seg.first),
				std::move(seg.second)
			);
		}

		_emit_switch_branch(std::move(flat), std::move(fallback));
	}
};

template <typename Subj, typename V>
_switch_holder <Subj> operator+(_switch_holder <Subj> &&h, _case <V> c)
{
	h.segments.emplace_back(h.subject == c.value, std::move(c.body));
	return std::move(h);
}

template <typename Subj>
_switch_holder <Subj> operator+(_switch_holder <Subj> &&h, _default_case d)
{
	h.fallback = std::move(d.body);
	return std::move(h);
}

template <typename Subj>
inline _switch_holder <Subj> _make_switch(const Subj &s)
{
	return _switch_holder <Subj> (s);
}

#define $switch(subj)	::rcgp::_make_switch(subj)
#define $case(val)	+ ::rcgp::_case <std::remove_cvref_t <decltype(val)>> { (val), {} } * [&]
#define $default	+ ::rcgp::_default_case { {} } * [&]

} // namespace rcgp
