#pragma once

#include "Config.hpp"

#include<stddef.h>
#include "Uinitialized.h"

NAMESPACE_BEGIN

/**
The basic_string simulate stl::basic_string 
// Class invariants:
// (1) [start, finish) is a valid range.
// (2) Each iterator in [start, finish) points to a valid object
//     of type value_type.
// (3) *finish is a valid object of type value_type; in particular,
//     it is value_type().
// (4) [finish + 1, end_of_storage) is a valid range.
// (5) Each iterator in [finish + 1, end_of_storage) points to
//     uninitialized memory.

// Note one important consequence: a string of length n must manage
// a block of memory whose size is at least n + 1.
*/

#define STRING_MEMBER_TEMPLATES

// Specialization for simple allocators.
template <class _Tp, class Alloc>
class string_alloc_base
{
public:
	typedef Alloc allocator_type;
	allocator_type get_allocator() const { return allocator_type(); }
	string_alloc_base() : m_start(0), m_finish(0), m_endOfStorage(0) {}
protected:
	typedef simple_alloc<_Tp, Alloc> Alloc_type;
	_Tp* allocate(size_t n) { return Alloc_type::allocate(n); }
	void deallocate(_Tp* p, size_t n) { Alloc_type::deallocate(p, n); }
	void allocate_block(size_t n)
	{
		if (n < max_size())
		{
			m_start = Alloc_type::allocate(n);
			m_finish = m_start;
			m_endOfStorage = m_start + n;
		}
	}

	void deallocate_block() { Alloc_type::deallocate(m_start, m_endOfStorage - m_start); }

	size_t max_size() { return (size_t)-1 / sizeof(_Tp) - 1 ; }
	 
	void _M_throw_out_of_range() const;
	void _M_throw_lenth_err() const;

protected:
	_Tp* m_start;
	_Tp* m_finish;
	_Tp* m_endOfStorage;
};

//helper function
template<class Tp, class Alloc>
void string_alloc_base<Tp, Alloc>::_M_throw_out_of_range() const
{
	__THROW(out_of_range("basic_string"));
}

template<class Tp, class Alloc>
void string_alloc_base<Tp, Alloc>::_M_throw_lenth_err() const
{
	__THROW(length_error("basic_string"));
}

template<class _CharT, class _Alloc>
class basic_string : public string_alloc_base(_CharT, _Alloc)
{
public:
	typedef _CharT value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef value_type* iterator;
	typedef const value_type* const_iterator;

	static const size_type npos;

protected:
	typedef string_alloc_base<_CharT, _Alloc> _Base;
	typedef typename _Base::allocator_type allocator_type;
public:
	//constructor
	inline basic_string(){}
	inline basic_string(const basic_string & str) { range_initialize(str.begin(), str.end()); }
	inline basic_string(const basic_string& str, size_type pos, size_type len = npos)
	{
		if (pos <= str.size())
		{
			range_initialize(str.begin() + pos, str.begin() + pos + min(n, str.size() - pos)));
		}
		else
		{
			_M_throw_out_of_range();
		}
	}

	inline basic_string(const _CharT* s) { range_initialize(s, s + char_traits<_CharT>::length(s)); }
	inline basic_string(const _CharT* s, size_type n) { range_initialize(s, s + n)); }
	inline basic_string(size_type n, _CharT c)
	{
		allocate_block(n + 1);
		m_finish = uninitialized_fill_n(m_start, n, c);
		terminate_string();
	}
	inline basic_string(iterator first, iterator last);
#ifdef STRING_MEMBER_TEMPLATES
	template<typename InputIterator>
	basic_string& string(InputIterator first, InputIterator last)
	{
		typedef _Is_integer<InputIterator>::_Integral _Integral;
		initialize_dispatch(first, last, _Integral());
	}
#else
	basic_string& basic_string(_CharT * first, _CharT* last)
	{
		range_initialize(first, last);
	}

#endif

	~basic_string() {}

	inline basic_string& operator=(const basic_string& str)
	{
		if (&str != this)
			assign(str.begin(), str.end);
		return *this;
	}

	inline basic_string& operator=(const _CharT* str)
	{
		return assign(str, str + char_traits<_CharT>::length(str));
	}

	inline basic_string& operator=(const _CharT c)
	{
		assign(static_cast<size_type>(1), c);
	}

	inline iterator begin() { return m_start; }
	inline iterator end() { return m_finish; }
	inline const_iterator begin() const { return m_start; }
	inline const_iterator end() const { return m_finish; }

public:
	inline size_type size() const { return m_finish - m_start; }
	inline size_type length() const { return size(); }
	inline void resize(size_type n, _CharT c)
	{
		if (n <= size())
			erase(begin() + n, end());
		else
			append(n - size(), c);
	}

	inline void resize(size_type n)
	{
		resize(n, (_CharT)0);
	}

	void reserve(size_type = 0);

	inline size_type capacity() const { return (m_endOfStorage - m_start) - 1; }
	
	inline void clear()
	{
		if (!empty())
		{
			char_traits<_CharT>::assign(*m_start, (_CharT)0);
			destroy(m_start + 1, m_finish + 1);
			m_finish = m_start;
		}
	}

	inline bool empty() const { return m_start == m_finish; }

public:
	const_reference operator[](size_type n) const { return *(m_start + n); }
	reference operator[](size_type n)  { return *(m_start + n); }
	const_reference at(size_type n) const
	{
		if (n < size())
		{
			return *(m_start + n);
		}
		else
			_M_throw_out_of_range();
	}

	reference at(size_type n)
	{
		if (n < size())
		{
			return *(m_start + n);
		}
		else
			_M_throw_out_of_range();

	}


public: //append operator+ push_back

	basic_string& operator+=(const basic_string& str) { return append(str); }
	basic_string& operator+=(const _CharT* str) { return append(str); }
	basic_string& operator+=(_CharT c) { push_back(c); return *this; }
	basic_string& append(const basic_string& str)
	{
		append(str.begin(), str.end());
	}

	basic_string& append(const basic_string& str, size_type pos, size_type n)
	{
		if (pos > str.size())
			_M_throw_out_of_range();
		return append(str.begin() + pos, str.begin() + pos + min(n, str.size() - pos));
	}

	basic_string& append(const _CharT* str, size_type n)
	{
		return append(str, str + n);
	}

	basic_string& append(const _CharT* str)
	{
		return append(str, str + char_traits<_CharT>::length(str));
	}

	basic_string& append(size_type n, _CharT c);

#ifdef STRING_MEMBER_TEMPLATES
	template<typename InputIterator>
	basic_string& append(InputIterator first, InputIterator last)
	{
		typedef typename _Is_integer<InputIterator>::_Integral _Integral;
		_M_append_dispatch(first, last, _Integral());
	}

	template<class InputIter>
	basic_string& append(InputIter first, InputIter last, input_iterator_tag);

	template<class ForwardIter>
	basic_string& append(ForwardIter first, ForwardIter last, forward_iterator_tag);

	template<class Iteger>
	basic_string& _M_append_dispatch(Iteger n, Iteger x, __true_type);

	template<class Iteger>
	basic_string& _M_append_dispatch(Iteger n, Iteger x, __false_type);

#else
	basic_string& append(_CharT * first, _CharT* last);
#endif

	void push_back(_CharT c)
	{
		if (m_finish + == m_endOfStorage)
		{
			reserve(size() + max(size(), static_cast<size_type>(1)));
		}
		construct_null(m_finish + 1);
		char_traits<_CharT>::assign(*m_finish, c);
		++m_finish;
	}

	void pop_back()
	{
		char_traits<_CharT>::assign(m_finish - 1, (_CharT)0);
		destroy(m_finish);
		--m_finish;
	}

	basic_string& assign(const basic_string& _str)
	{
		assign(_str.begin(), _str.end());
	}

	basic_string& assign(const basic_string& _str, size_type pos, size_type n)
	{
		if (pos > _str.size())
			_M_throw_out_of_range();
		return assign(_str.begin() + pos, _str.begin() + pos + min(n, _str.size() - pos));
	}

	basic_string& assign(const _CharT* str, size_type n)
	{
		return assign(str, str + n);
	}

	basic_string& assign(const _CharT* str)
	{
		return assign(str, str + char_traits<_CharT>::length(str));
	}

	basic_string& assign(size_type n, _CharT c);

private:
	template<typename InputIterator>
	basic_string& assign(InputIterator first, InputIterator last);
	basic_string& assign(_CharT * first, _CharT* last);

	template<class Integer>
	basic_string& _M_assign_dispatch(Integer n, Integer x, __true_type)
	{
		return assign((size_type)n, (_CharT)x);
	}

	template<class Integer>
	basic_string& _M_assign_dispatch(Integer n, Integer x, __false_type);

public: //insert
	basic_string& insert(size_type pos, const basic_string& str)
	{
		if (pos > size())
			_M_throw_out_of_range();
		if (size() + str.size() > max_size())
			_M_throw_lenth_err();

		insert(m_start + pos, str.begin(), str.end());

		return *this;
	}

	basic_string& insert(size_type pos, const basic_string& str, size_type subpos, size_type sublen)
	{
		if (pos > size() || subpos > str.size())
			_M_throw_out_of_range();
		
		size_type len = min(sublen, str.size() - subpos);
		if ((size + len) > max_size())
			_M_throw_lenth_err();

		insert(m_start + pos, str.begin() + subpos, str.begin() + subpos + len);

		return *this;
	}

	basic_string& insert(size_type pos, _CharT* str, size_type n)
	{
		if (pos > size())
			_M_throw_out_of_range();
		if (size() + n > max_size())
			_M_throw_lenth_err();

		insert(m_start + pos, str, str + len);
		return *this;
	}

	basic_string& insert(size_type pos, const _CharT* _s)
	{
		if (pos > size())
			_M_throw_out_of_range();
		size_type len = char_traits<_CharT>::length(_s);
		if (size() + len > max_size())
			_M_throw_lenth_err();
		insert(m_start + pos, _s, _s + len);
		return *this;
	}

	basic_string& insert(size_type pos, size_type n, _CharT c)
	{
		if (pos > size())
			_M_throw_out_of_range();
		if (size() + n > max_size())
			_M_throw_lenth_err();
		insert(m_start + pos, n, c);
		return *this;
	}

	iterator insert(iterator p, _CharT c)
	{
		if (p == m_finish)
		{
			push_back(c);
			return m_finish - 1;
		}
		else
		{
			return _M_insert_aux(p, c);
		}
	}

	void insert(iterator p, size_t n, _CharT c);

#ifdef STRING_MEMBER_TEMPLATES
	template <class InputIter>
	void insert(iterator p, iterator first, iterator last)
	{
		typedef typename _Is_integer<InputIter>::_Integral _Integral;
		_M_insert_dispatch(p, first, last, _Integral());
	}
#else
	void insert(iterator p, const _CharT first, const _CharT last);
#endif

private: //help functions

	template<class InputIter>
	void insert(iterator p, InputIter, InputIter, input_iterator_tag);

	template<class ForwardInter>
	void insert(iterator p, ForwardIter, ForwardIter, forward_iterator_tag);

	template<class Integer>
	void _M_insert_dispatch(iterator p, Integer n, Iteger x, __true_type)
	{
		insert(p, (size_type)n, (_CharT)x);
	}

	template <class InputIter>
	void _M_insert_dispatch(iterator p, InputIter first, InputIter last, __false_type)
	{
		typedef typename iterator_traits<InputIter>::iterator_category Category;
		insert(p, first, last, Category());
	}

	template<class InputIterator>
	void _M_copy(InputIterator first, InputIterator last, iterator result)
	{
		for (; first != last; ++first, ++result)
		{
			char_traits<_CharT>::assign(*result, *first);
		}
	}

	iterator _M_insert_aux(iterator, _CharT);

	void _M_copy(const _CharT* first, const _CharT* last, _CharT* result)
	{
		char_traits<_CharT>::copy(result, first, last - first);
	}

public:
	basic_string& erase(size_type pos = 0, size_type n = npos)
	{
		if (pos > size())
			_M_throw_out_of_range();
		erase(m_start + pos, m_start + pos + min(n, size() - pos));
		return *this;
	}

	iterator erase(iterator position)
	{
		char_traits<_CharT>::move(position, position + 1, m_finish - m_start);
		destroy(m_finish);
		--m_finish;
		return position;
	}


	iterator erase(iterator first, iterator last)
	{
		if (first != last)
		{
			char_traits<_CharT>::move(first, last, (m_finish - last) + 1);
			const iterator new_finish = m_finish - (last - first);
			destroy(new_finish + 1, m_finish + 1);
			m_finish = new_finish;
		}
		return first;
	}

public:

	basic_string& replace(size_type pos, size_type n, const basic_string& str)
	{
		if (pos > size())
			_M_throw_out_of_range();
		
		const size_type len = min(n, size() - pos);
		
		if (size() + len > max_size())
			_M_throw_lenth_err();

		return replace(m_start + pos, m_start + pos + len, str.begin(), str.end());

		return *this;
	}

	basic_string& replace(size_type pos1, size_type n1, const basic_string& str, size_type pos2, size_type n2)
	{
		if (pos1 > size() || pos2 > str.size())
			_M_throw_out_of_range();

		const size_type len1 = min(n1, size() - pos1);
		const size_type len2 = min(n2, str.size() - pos2);

		if (size() - len1 + len2 > max_size())
			_M_throw_lenth_err();

		return replace(m_start + pos1, m_start + pos1 + len1, str.m_start + pos2,
			str.m_start + pos2 + len2);

		return *this;
	}

	basic_string& replace(size_type pos, size_type n1, const _CharT* str, size_type n2)
	{
		if (pos > size())
			_M_throw_out_of_range();

		const size_type len = min(n1, size() - pos);

		if (n2 > max_size() || size() - len + n2 > max_size())
			_M_throw_lenth_err();

		return replace(m_start + pos, m_start + pos + len,
			str, str + n2);

		return *this;
	}

	basic_string& replace(size_type pos, size_type n1, const _CharT* str)
	{
		if (pos > size())
			_M_throw_out_of_range();

		const size_type len = min(n1, size() - pos);
		const size_type n2 = char_traits<_CharT>::length(str);

		if (n2 > max_size() || size() - len + n2 > max_size())
			_M_throw_lenth_err();

		return replace(m_start + pos, m_start + pos + len,
			str, str + n2);

		return *this;
	}

	basic_string& replace(size_type pos, size_type n1, size_type n2, _CharT c)
	{
		if (pos > size())
			_M_throw_out_of_range();

		const size_type len = min(n1, size() - pos);

		if (n2 > max_size() || size() - n1 + n2 > max_size())
			_M_throw_lenth_err();

		return replace(m_start + pos, m_start + pos + len,
			n2, c);

		return *this;
	}

	basic_string& replace(iterator first, iterator last, const basic_string& str)
	{
		return replace(first, last, str.begin(), str.end());
	}

	basic_string& replace(iterator first, iterator last, const _CharT*str, size_type n)
	{
		return replace(first, last, str, str + n);
	}

	basic_string& replace(iterator first, iterator last, const _CharT * str)
	{
		return replace(first, last, str, str + char_traits<_CharT>::length(str));
	}

	basic_string& replace(iterator first, iterator last, size_type n, _CharT c);

#ifdef STRING_MEMBER_TEMPLATES
	template <class InputIter>
	basic_string& replace(iterator first, iterator last, InputIter f, InputIter l)
	{
		typedef typename _Is_integer<InputIter>::_Integral _Integral;
		return _M_replace_dispatch(first, last, f, l, _Integral());
	}
#else
	basic_string& replace(iterator first, iterator last, const _CharT* first, const _CharT* last);
#endif

private: //help function
	template<class Iteger>
	basic_string& _M_replace_dispatch(iterator first, iterator last, Integer n, Integer x, __true_type)
	{
		return replace(first, last, (size_type)n, (_CharT)x);
	}

	template<class InputIter>
	basic_string& _M_repalce_dispatch(iterator first, iterator last, InputIter f, InputIter l, __false_type)
	{
		typedef typename iterator_traits<InputIter>::iterator_category Category;
		return replace(first, last, f, l, Category());
	}

	template<class InputIter>
	basic_string& replace(iterator first, iterator last, InputIter f, InputIter l, input_iterator_tag);

	template<class ForwardIter>
	basic_string& replace(iterator fisrt, iterator last, ForwardIter f, ForwardIter l, forward_iterator_tag);

public:
	size_type copy(_CharT* str, size_type n, size_type pos = 0) const
	{
		if (pos > size())
			_M_repalce_dispatch();

		const size_type len = min(n, size() - pos);
		char_traits<_CharT>::copy(str, m_start + pos, len);

		return len;
	}

	void swap(basic_string& str)
	{
		swap(m_start, str.m_start);
		swap(m_finish, str.m_finish);
		swap(m_endOfStorage, str.m_endOfStorage);
	}

public:
	const _CharT * c_str() const { return m_start; }
	const _CharT *data() const { return m_start; }

public:
	size_type find(const basic_string& str, size_type pos = 0) const
	{
		return find(str.begin(), pos, str.size());
	}

	size_type find(const _CharT* str, size_type pos = 0) const
	{
		return find(str, pos, char_traits<_CharT>::length(str));
	}

	size_type find(const _CharT* str, size_type pos, size_type n) const;
	size_type find(_CharT c, size_type pos = 0) const;

public:
	size_type rfind(const basic_string& str, size_type pos = nops) const
	{
		return rfind(str.begin(), pos, str.size());
	}

	size_type rfind(const _CharT * str, size_type pos = npos) const
	{
		return rfind(str, pos, char_traits<_CharT>::length(str));
	}

	size_type rfind(const _CharT* str, size_type pos, size_type n) const;
	size_type rfind(_CharT c, size_type pos = npos) const;

public:
	size_type find_first_of(const basic_string& str, size_type pos = 0) const
	{
		return find_first_of(str.begin(), pos, str.size());
	}

	size_type find_first_of(const _CharT* str, size_type pos = 0) const
	{
		return find_first_not_of(str, pos, char_traits<_CharT>::length(str));
	}

	size_type find_first_of(const _CharT* str, size_type pos, size_type n) const;
	size_type find_first_of(_CharT c, size_type pos = 0) const;

public:
	size_type find_last_of(const basic_string& str, size_type pos = npos) const
	{
		return find_last_not_of(str.begin(), pos, str.size());
	}

	size_type find_last_of(const _CharT* str, size_type pos = npos) const
	{
		return find_last_not_of(str, pos, char_traits<_CharT>::length(str));
	}

	size_type find_last_of(const _CharT* str, size_type pos, size_type n) const;
	size_type find_last_of(_CharT c, size_type pos = npos) const
	{
		return rfind(c, pos);
	}

public:
	size_type find_first_not_of(const basic_string& str, size_type pos = 0) const
	{
		return find_first_not_of(str.begin(), pos, str.size());
	}

	size_type find_first_not_of(const _CharT* str, size_type pos = 0) const
	{
		return find_first_not_of(str, pos, char_traits<_CharT>::length(str));
	}

	size_type find_first_not_of(const _CharT* str, size_type pos, size_type n) const

	size_type find_first_not_of(const _CharT c, size_type pos = 0);

public:
	size_type find_last_not_of(const basic_string& str, size_type pos = npos) const
	{
		return find_last_not_of(str.begin(), pos, str.size());
	}

	size_type find_last_not_of(const _CharT* str, size_type pos = npos) const
	{
		return find_last_not_of(str, pos, char_traits<_CharT>::length(str));
	}
	size_type find_last_not_of(const _CharT* str, size_type pos, size_type n) const;
	size_type find_last_not_of(_CharT c, size_type pos = npos) const;

public:
	basic_string& substr(size_type pos = 0, size_type n = npos) const
	{
		if (pos <= size())
		{
			return basic_string(m_start + pos, m_start + pos + min(n, size() - pos));
		}
		else
			_M_throw_out_of_range();
	}

public:
	int compare(const basic_string& str) const
	{
		return _M_compare(m_start, m_finish, str.m_start, str.m_finish);
	}

	int compare(size_type pos1, size_type n1, const basic_string& str) const
	{
		if (pos1 > size()) _M_throw_out_of_range();
		return _M_compare(m_start + pos1, m_start + pos1 + min(n1, size() - pos1),
			str.begin(), str.end());
	}

	int compare(size_type pos1, size_type n1, const basic_string& str, size_type pos2, size_type n2) const
	{
		if (pos1 > size() || pos2 > str.size())
			_M_throw_out_of_range();
		else
			return _M_compare(m_start + pos1, m_start + pos1 + min(n1, size() - pos1),
			str.begin() + pos2, str.begin + pos2 + min(n2, str.size() - pos2));
	}

	int compare(const _CharT* str) const
	{
		return _M_compare(m_start, m_finish, str, str + char_traits<_CharT>::length(str));
	}

	int compare(size_type pos1, size_type n1, const _CharT* str) const
	{
		if (pos1 > size())
			_M_throw_out_of_range();
		else
			return _M_compare(m_start + pos1, m_start + pos1 + min(n1, size() - pos1),
								str, str + char_traits<_CharT>::length(str));
	}

	int compare(size_type pos1, size_type n1, const _CharT* str, size_type n2) const
	{
		if (pos1 > size()) _M_throw_out_of_range();
		return _M_compare(m_start + pos1, m_start + pos1 + min(n1, size() - pos1),
			str, str + n2);
	}

public:
	//help function for compare
	static int _M_compare(const _CharT* f1, const _CharT* l1,
		const _CharT* f2, cont _CharT* l2)
	{
		const ptrdiff_t n1 = l1 - f1;
		const ptrdiff_t n2 = l2 - f2;
		const int cmp = char_traits<_CharT>::compare(f1, f2, min(n1, n2));
		return cmp != 0 ? cmp : (n1 < n2 ? -1 : (n1 > n2 ? 1 : 0))
	}

private: //helper function
	void construct_null(_CharT* p)
	{
		try{
			*p = (_CharT)0;
		}
		catch{
			destroy(p);
		}
	}

	void terminate_string() 
	{
		try{
			construct_null(m_finish);
		}
		catch{
			destroy(m_start, m_finish)
		}
	}


#ifdef STRING_MEMBER_TEMPLATES

	template <class _InputIter>
	void range_initialize(_InputIter fisrt, _InputIter last, input_iterator_tag) 
	{
		allocate_block(8);
		construct_null(m_finish);
		__TRY{
			append(first, last);
		}
		__UNWIND(destroy(m_start, m_finish + 1)))
	}

	template <class _ForwardIter>
	void range_initialize(_ForwardIter first, _ForwardIter last, forward_iterator_tag)
	{
		difference_type n = 0;
		distance(first, last, n);
		allocate_block(n + 1);
		m_finish = uninitialized_copy(first, last, m_start);
		terminate_string();
	}

	template <class _InputIter>
	void range_initialize(_InputIter first, _InputIter last)
	{
		typedef typename iterator_traits<_InputIter>::iterator_category _Category;
		range_initialize(first, last, _Category());
	}

	template <class Integer>
	void initialize_dispatch(Integer n, Integer x, __true_type) {
		allocate_block(n + 1);
		m_finish = uninitialized_fill_n(m_start, n, x);
		terminate_string();
	}

	template <class Integer>
	void initialize_dispatch(Integer first, Integer last, __false_type) {
		range_initialize(first, last);
	}
#else /* MEMBER_TEMPLATES */
	void range_initialize(const _CharT* first, const _CharT* last) {
		ptrdiff_t n = last - first;
		allocate_block(n + 1);
		m_finish = uninitialized_copy(first, last, m_start);
		terminate_string();
	}
#endif /* MEMBER_TEMPLATES */

protected:
	using _Base::m_start;
	using _Base::m_finish;
	using _Base::m_endOfStorage;
	using _Base::_M_throw_out_of_range;
	using _Base::_M_throw_lenth_err;
};

//no inline declarations
template<class Tp, class Alloc>
basic_string<Tp, Alloc>::size_type
basic_string<Tp, Alloc>::npos = (basic_string<Tp, Alloc>::size_type) -1;

template<class Tp, class Alloc>
void basic_string<Tp, Alloc>::reserve(size_type n)
{
	if (n > max_size())
		_M_throw_out_of_range();
	size_type n = max(n, size()) + 1;
	pointer newstart = allocate(n);
	pointer newfinish = newstart;
	__TRY{
		newfinish = uninitialized_copy(m_start, m_finish, newstart);
		construct_null(newfinish);
	}
	__UNWIND((destroy(newstart, newfinish), deallocate(newstart,n)));

	destroy(m_start, m_finish + 1);
	deallocate_block();
	m_start = newstart;
	m_finish = newfinish;
	m_endOfStorage = newstart + n;
}

template<class Tp, class Alloc>
basic_string<Tp, Alloc>&
basic_string<Tp, Alloc>::append(size_type n, _CharT c)
{
	if (n > max_size() || size() + n > max_size())
		_M_throw_out_of_range();
	if (size() + n > capacity())
	{
		reserve(size() + max(size(), n));
	}

	if (n > 0)
	{
		uninitialized_fill_n(m_finish + 1, n - 1, c);
		__TRY{
			construct_null(m_finish + n);
		}
		__UNWIND(destroy(m_finish + 1, m_finish + n));
		char_traits<_CharT>::assign(*m_finish, c);
		m_finish += n;
	}

	return *this;
}

#ifdef STRING_MEMBER_TEMPLATES
template <class Tp, class Alloc>
template<class InputIter>
basic_string<Tp, Alloc>&
basic_string<Tp, Alloc>::append(InputIter first, InputIter last, input_iterator_tag)
{
	for (; first != last, ++first)
	{
		push_back(*first);
	}

	return *this;
}

template <class Tp, class Alloc>
template<class ForwardIter>
basic_string<Tp, Alloc>&
basic_string<Tp, Alloc>::append(ForwardIter first, ForwardIter last, forward_iterator_tag)
{
	if (first != last)
	{
		const siz_type old_size = size();
		difference_type n = 0;
		distance(fisrt, last, n);
		if (static_cast<size_type>(n) > max_size() || old_size + n > max_size())
			_M_throw_out_of_range();
		if (old_size + static_cast<size_type>(n) > capacity())
		{
			const size_type len = old_size + max(old_size, static_cast<size_type>(n)) + 1;
			pointer new_start = allocate(n);
			pointer new_finish = new_tart;
			__TRY{
				new_finish = uninitialized_copy(m_start, m_finish, new_start);
				new_finish = uninitialized_copy(first, last, new_finish);
				construct_null(newfinish);
			}
			__UNWIND((destroy(new_start, len), deallocate(n)));
			destroy(m_start, m_finish + 1);
			deallocate_block();
			m_start = new_start;
			m_finish = new_finish;
			m_endOfStorage = m_start + len;
		}
		else
		{
			ForwardIter f1 = first;
			++f1;
			uninitialized_copy(f1, last, m_finish + 1);
			__TRY{
				construct_null(m_finish + n);
			}
			__UNWIND(destroy(m_finish + 1, m_finish + n));

			char_traits<Tp>::assign(*m_finish, *first);
			m_finish += n;
		}
	}
	return *this;
}
#else

template <class Tp, class Alloc>
basic_string<Tp, Alloc>&
basic_string<Tp, Alloc>::append(const Tp* first, const Tp* last)
{
	if (first != last)
	{
		const size_type oldsz = size();
		ptrdiff_t n = last - first;
		if (n > max_size() || oldsz + n > max_size())
			_M_throw_out_of_range();
		if (oldsz + n > capacity())
		{
			const size_type len = oldsz + max(oldsz, (size_t)n) + 1;
			pointer _new_start = allocate(n);
			pointer _new_finish = _new_start;
			__TRY{
				_new_finish = uninitialized_copy(m_start, m_finish, _new_start);
				_new_finish = uninitialized_copy(first, last, _new_finish);
				construct_null(_new_finish);
			}
			__UNWIND((destroy(_new_start, _new_finish), deallocate(_new_start, n)));
			destroy(m_start, m_finish + 1);
			deallocate_block();
			m_start = _new_start;
			m_finish = _new_finish;
			m_endOfStorage = m_start + n;
		}
		else
		{
			const Tp* f1 = first;
			++f1;
			uninitialized_copy(f1, last, m_finish + 1);
			__TRY
			{
				construct_null(m_finish + n);
			}
			__UNWIND(destroy(m_finish + 1, m_finish + n));
			char_traits<Tp>::assign(*m_finish, *first);
			m_finish += n;
		}
	}

	return *this;
}

#endif //STRING_MEMBER_TEMPLATES

template <class _CharT, class Alloc>
basic_string<_CharT, Alloc>&
basic_string<_CharT, Alloc>::assign(size_type n, _CharT c)
{
	if (n <= size())
	{
		char_traits<_CharT>::assign(m_start, n, c);
		erase(m_start + n, m_finish);
	}
	else
	{
		char_traits<_CharT>::assign(m_start, n, c);
		append(n - size(), c);
	}

	return *this;
}

#ifdef STRING_MEMBER_TEMPLATES
template<class Tp, class Alloc>
template<class InputIter>
basic_string<Tp, Alloc>&
basic_string<Tp, Alloc>::_M_assign_dispatch(InputIter f, InputIter l, __false_type)
{
	pointer cur = m_start;
	while (f != l && cur != m_finish)
	{
		char_traits<Tp>::assign(*cur, *f);
		++f;
		++cur;
	}
	if (f == l)
		erase(cur, m_finish);
	else
		append(f, l);
	return *this;
}
#endif //STRING_MEMBER_TEMPLATES

template<class Tp, class Alloc>
basic_string<Tp, Alloc>&
basic_string<Tp, Alloc>::assign(const Tp* f, const Tp* l)
{
	const ptrdiff_t n = l - f;
	if ((size_type)n <= size())
	{
		char_traits<Tp>::copy(m_start, f, n);
		erase(m_start + n, m_finish);
	}
	else
	{
		char_traits<Tp>::copy(m_start, f, size());
		append(f + size(), l);
	}
}

template<class _CharT, class Alloc>
basic_string<Tp, Alloc>::iterator
basic_string<Tp, Alloc>::_M_inert_aux(basic_string<Tp, Alloc>::iterator p, _CharT c)
{
	iterator new_pos = p;
	if (m_finish + 1 < m_endOfStorage)
	{
		construct_null(m_finish + 1);
		char_traits<_CharT>::move(p + 1, p, m_finish - p);
		char_traits<_CharT>::assign(*p, c);
		++m_finish;
	}
	else
	{
		const size_type old_len = size();
		const size_type len = old_len + max(old_len, static_cast<size_type>(1)) + 1;
		iterator newstart = allocate(n);
		iterator newfinish = newstart;
		__TRY{
			new_pos = uninitialized_copy(m_start, p, newstart);
			construct(new_pos, c);
			newfinish = new_pos + 1;
			newfinish = uninitialized_copy(p, m_finish, newfinish);
			construct_null(newfinish);
		}
		__UNWIND((destroy(newstart, newfinish), deallocate(newstart, len)));
		destroy(m_start, m_finish + 1);
		deallocate_block();
		m_start = newstart;
		m_finish = newfinish;
		m_endOfStorage = newstart + len;
	}

	return new_pos;
}

template<class _CharT, class Alloc>
void basic_string<_CharT, Alloc>::insert(basic_string<_CharT, Alloc>::iterator position, size_t n, _CharT c)
{
	if (n != 0)
	{
		if ((size_type)(m_endOfStorage - m_finish) >= n + 1)
		{
			const size_type elems_after = m_finish - position;
			iterator old_finish = m_finish;
			if (elems_after > n)
			{
				uninitialized_copy((m_finish - n) + 1, m_finish + 1, m_finish + 1);
				m_finish += n;
				char_traits<_CharT>::move(position + n, position, (elems_after - n) + 1);
				char_traits<_CharT>:assign(position, n, c);
			}
			else
			{
				uninitialized_fill_n(m_finish + 1, n - elems_after - 1, c);
				m_finish += n - elems_after;
				__TRY{
					uninitialized_copy(position, old_finish + 1, m_finish);
					m_finish += elems_after;
				}
				__UNWIND((destroy(old_finish + 1, m_finish), m_finish = old_finish));

				char_traits<_CharT>::assign(position, elems_after + 1, c);
			}
		}
		else
		{
			const size_type old_size = size();
			const size_type len = old_size + max(old_size, n) + 1;
			iterator _new_start = allocate(n);
			iterator _new_finish = _new_start;
			__TRY
			{
				_new_finish = uninitialized_copy(m_start, position, _new_start);
				_new_finish = uninitialized_fill_n(_new_finish, n, c);
				_new_finish = uninitialized_copy(position, m_finish, _new_finish);
				construct_null(_new_finish);
			}
			__UNWIND((destroy(_new_start, _new_finish), deallocate(_new_start, n)));
			destroy(m_start, m_finish + 1);
			deallocate_block();
			m_start = _new_start;
			m_finish = _new_finish;
			m_endOfStorage = _new_start + len;
		}
	}
}


#ifdef STRING_MEMBER_TEMPLATES
template<class Tp, class Alloc>
template<InputIter>
void basic_string<Tp, Alloc>::insert(iterator p, InputIter first, InputIter last, input_iterator_tag)
{
	for (; first != last; ++first)
	{
		p = insert(p, *first);
		++p;
	}
}

template<class Tp, class Alloc>
template <class ForwardIter>
void basic_string<Tp, Alloc>::insert(iterator position, ForwardIter first, ForwardIter last, forward_iterator_tag)
{
	if (first != last)
	{
		difference_type n = 0;
		distance(first, last, n);
		if (m_endOfStorage - m_finish >= n + 1)
		{
			const difference_type elems_after = m_finish - position;
			iterator old_finish = m_finish;
			if (elems_after >= n)
			{
				uninitialized_copy((m_finish - n) + 1, m_finish + 1, m_finish + 1);
				m_finish += n;
				char_traits<Tp>::move(position + n, position, (elems_after - n) + 1);
				_M_copy(first, last, position);
			}
			else
			{
				ForwardIter mid = first;
				advance(mid, elems_after + 1);
				uninitialized_copy(mid, last, m_finish + 1);
				m_finish += n - elems_after;
				__TRY{
					uninitialized_copy(position, old_finish + 1, m_finish);
					m_finish += elems_after;
				}
				__UNWIND((destroy(old_finish + 1, m_finish), m_finish = old_finish));
				_M_copy(first, mid, position);
			}
		}
		else
		{
			const size_type oldsz = size();
			const size_type len = oldsz + max(oldsz, n) + 1;
			pointer _new_start = allocate(len);
			pointer _new_finish = _new_start;
			__TRY{
				_new_finish = uninitialized_copy(m_start, position, _new_start);
				_new_finish = uninitialized_copy(first, last, _new_finish);
				_new_finish = uninitialized_copy(position, m_finish, _new_finish);
				construct_null(_new_finish);
			}
			__UNWIND((destroy(_new_start, _new_finish), deallocate(_new_start, len)));
			destroy(m_start, m_finish + 1);
			deallocate_block();
			m_start = _new_start;
			m_finish = _new_finish;
			m_endOfStorage = _new_start + len;
		}
	}
}

#else //STRING_MEMBER_TEMPLATES

template <class _CharT, class Alloc>
void basic_string<_CharT, Alloc>::insert(iterator position, const _CharT* first, const _CharT*  last)
{
	if(first != last)
	{
		const ptrdiff_t n = last - first;
		if(m_endOfStorage - m_finish > n + 1)
		{
			const ptrdiff_t elems_after = m_finish - position;
			iterator old_finish = m_finish;
			if(elems_after >= n)
			{
				uninitialized_copy((m_finish - n) + 1, m_finish + 1, m_finish + 1);
				m_finish += n;
				char_traits<_CharT>::move(position + n, position, (elems_after - n) + 1);
				_M_copy(first, last, position);
			}
			else
			{
				const _CharT* mid = first;
				advance(mid, elems_after + 1);
				uninitialized_copy(mid, last, m_finish + 1);
				m_finish += n - elems_after;
				__TRY{
					uninitialized_copy(position, old_finish + 1, m_finish);
					m_finish += elems_after;
				}
				__UNWIND((destroy(old_finish + 1, m_finish), m_finish = old_finish));
				_M_copy(first, last, position);
			}
		}
		else
		{
			const size_type old_size = size();
			const size_type len = old_size + max(old_size, n) + 1;
			pointer newstart = allocate(len);
			pointer newfinish = newstart;
			__TRY{
				newfinish = uninitialized_copy(m_start, position, newfinish);
				newfinish = uninitialized_copy(first, last, newfinish);
				newfinish = uninitialized_copy(position, m_finish, newfinish);
				construct_null(newfinish);
			}
			__UNWIND((destroy(newstart, newfinish), deallocate(newstart, len));

			destroy(m_start, m_finish + 1);
			deallocate_block();
			m_start = newstart;
			m_finish = newfinish;
			m_endOfStorage = newstart + len;
		}
	}
}


#endif //STRING_MEMBER_TEMPLATES


template<class Tp, class Alloc>
basic_string<Tp, Alloc>&
basic_string<Tp, Alloc>::replace(iterator first, iterator last, size_type n, Tp c)
{
	const size_type len = static_cast<size_type>(last - first);
	if (len >= n)
	{
		char_traits<Tp>::assign(first, n, c);
	}
	else
	{
		char_traits<Tp>::assign(first, len, c);
		insert(last, len - n, c);
	}

	return *this;
}

#ifdef STRING_MEMBER_TEMPLATES

template<class Tp, class Alloc>
template<class InputIter>
basic_string<Tp, Alloc>&
basic_string<Tp, Alloc>::replace(iterator first, iterator last, InputIter f, InputIter l, input_iterator_tag)
{
	for (; first != last && f != l; ++first, ++f)
	{
		char_traits<Tp>::assign(*first, *f);
	}

	if (f == l)
	{
		erase(first, last);
	}
	else
	{
		insert(last, f, l);
	}

	return *this;
}

template<class Tp, class Alloc>
template<class ForwardIter>
basic_string<Tp, Alloc>&
basic_string<Tp, Alloc>::replace(iterator first, iterator last, ForwardIter f, ForwardIter l, forward_iterator_tag)
{
	difference_type n = 0;
	distance(f, l, n);
	const difference_type len = last - first;
	if (len >= n)
	{
		_M_copy(f, l, first);
		erase(first + n, last);
	}
	else
	{
		ForwardIter m = f;
		advance(m, len);
		_M_copy(f, m, first);
		insert(last, m, l);
	}
	
	return *this;
}
#endif //STRING_MEMBER_TEMPLATES


// non -member function
//operator ++
template<class Tp, class Alloc>
inline basic_string<Tp, Alloc>
operator+(const basic_string<Tp, Alloc>& x, const basic_string<Tp, Alloc> & y)
{
	typedef basic_string<Tp, Alloc> _Str;
	_Str result(x.size() + y.size());
	result.append(x);
	result.append(y);

	return result;
}

template<class Tp, class Alloc>
inline basic_string<Tp, Alloc>
operator+(const Tp* s, const basic_string<Tp, Alloc>& y)
{
	typedef basic_string<Tp, Alloc> _Str;
	const size_t n = char_traits<Tp>::length(s);
	_Str result(n + y.size());
	result.append(s);
	result.append(y);
	return result;
}

template<class _CharT, class Alloc>
inline basic_string<_CharT, Alloc>
operator+(_CharT c, const basic_string<_CharT, Alloc>& y)
{
	typedef basic_string<_CharT, Alloc> _Str;
	_Str result(1 + y.size(());
	result.push_back(c);
	result.append(y);
	return result;
}

template<class Tp, class Alloc>
inline basic_string<Tp, Alloc>
operator+(const basic_string<Tp, Alloc>& x, const Tp* s)
{
	typedef basic_string<Tp, Alloc> _Str;
	size_t n = char_traits<Tp>::length(s);
	_Str result(x.size() + n);
	result.append(x);
	result.append(s, s + n);
	return result;
}

template<class Tp, class Alloc>
inline basic_string<Tp, Alloc>
operator+(const basic_string<Tp, Alloc>& x, const Tp c)
{
	typedef basic_string<Tp, Alloc> _Str;
	_Str result(x.size() + 1);
	result.append(x);
	result.push_back(c);
	return result;
}

//operator == and operator !=
template<class Tp, class Alloc>
inline bool
operator==(const basic_string<Tp, Alloc>& x,
const basic_string<Tp, Alloc>& y)
{
	return x.size() == y.size() && !char_traits<Tp>::compare(x.data(), y.data(), x.size());
}

template<class Tp, class Alloc>
inline bool
operator==(const Tp* s, const basic_string<Tp, Alloc>& y)
{
	size_t n = char_traits<Tp>::length(s);
	return n == y.size() && !char_traits<Tp>::compare(s, y.data(), n);
}

template<class Tp, class Alloc>
inline bool
operator==(const basic_string<Tp, Alloc>& x, const Tp* s)
{
	size_t n = char_traits<Tp>::length(s);
	return n == x.size() && !char_traits<Tp>::compare(y.data(), s, n);
}

template<class Tp, class Alloc>
inline bool
operator!=(const basic_string<Tp, Alloc>& x, const basic_string<Tp, Alloc>& y)
{
	return !(x == y);
}

template<class Tp, class Alloc>
inline bool
operator!=(const Tp * s, const basic_string<Tp, Alloc>& y)
{
	return !( s == y);
}

template<class Tp, class Alloc>
inline bool
operator!=(const basic_string<Tp, Alloc>& x, const Tp * s)
{
	return !(x == s);
}

//operator < and  >  <= >=
template<class Tp, class Alloc>
inline bool
operator<(const basic_string<Tp, Alloc>& x, const basic_string<Tp, Alloc>& y)
{
	return basic_string<Tp, Alloc>::_M_compare(x.begin(), x.end(), y.begin(), y.end()) < 0;
}

template<class Tp, class Alloc>
inline bool
operator<(const Tp * s, const basic_string<Tp, Alloc>& y)
{
	size_type n = char_traits<Tp>::length(s);
	return basic_string<Tp, Alloc>::_M_compare(s, s + n, y.begin(), y.end) < 0;
}

template<class Tp, class Alloc>
inline bool
operator<(const basic_string<Tp, Alloc>& x, const Tp * s)
{
	size_type n = char_traits<Tp>::length(s);
	return char_traits<Tp>::_M_compare(x.begin(), x.end(), s, s + n) < 0;
}

template<class Tp, class Alloc>
inline bool
operator>(const basic_string<Tp, Alloc>& x, const basic_string<Tp, Alloc>& y)
{
	return y < x;
}


template<class Tp, class Alloc>
inline bool
operator>(const Tp * s, const basic_string<Tp, Alloc>& y)
{
	return y < s;
}

template<class Tp, class Alloc>
inline bool
operator>(const basic_string<Tp, Alloc>& x, const Tp * s)
{
	return s < x;
}


template<class Tp, class Alloc>
inline bool
operator<=(const basic_string<Tp, Alloc>& x, const basic_string<Tp, Alloc>& y)
{
	return !(y < x);
}

template<class Tp, class Alloc>
inline bool
operator<=(const Tp * s, const basic_string<Tp, Alloc>& y)
{
	return !(y < s);
}

template<class Tp, class Alloc>
inline bool
operator<=(const basic_string<Tp, Alloc>& x, const Tp * s)
{
	return !(s < x);
}

template<class Tp, class Alloc>
inline bool
operator>=(const basic_string<Tp, Alloc>& x, const basic_string<Tp, Alloc>& y)
{
	return !(x < y);
}

template<class Tp, class Alloc>
inline bool
operator<=(const Tp * s, const basic_string<Tp, Alloc>& y)
{
	return !(s < y);
}

template<class Tp, class Alloc>
inline bool
operator<=(const basic_string<Tp, Alloc>& x, const Tp * s)
{
	return !(x < s);
}

template<class Tp, class Alloc>
inline void swap(const basic_string<Tp, Alloc>& x, const basic_string<Tp, Alloc>& y)
{
	x.swap(y);
}

NAMESPACE_END