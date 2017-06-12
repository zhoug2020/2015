#pragma once
#include "Config.hpp"
NAMESPACE_BEGIN

#define MEBER_TEMPALTES

template<class Tp, class Alloc>
class vector_base
{
	typedef Alloc allocator_type;
	allocator_type get_allocator() const { return allocator_type(); }
	vector_base() : m_start(0), m_finish(0), m_endOfStorage(0) {}
protected:
	typedef simple_alloc<Tp, Alloc> Alloc_type;
	Tp* allocate(size_t n) { return Alloc_type::allocate(n); }
	void deallocate<Tp* p, size_t n) { Alloc_type::deallocate(p, n); }
	void _M_throw_range_err() const;

protected:
	_Tp* m_start;
	_Tp* m_finish;
	_Tp* m_endOfStorage;
};

template<class Tp, class Alloc>
inline void _M_throw_range_err() const
{
	__THROW(range_error("Vector"));
}


template<class Tp, class Alloc>
class vector : public vector_base < Tp, Alloc >
{
private:
	typedef vector_base < Tp, Alloc > Base;

public:
	typedef Tp value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type* iterator;
	typedef const value_type const_iterator;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef Base::allocator_type allocator_type;
	typedef reverse_iterator<const_iterator> const_reverse_iterator;
	typedef reverse_iterator<iterator> reverse_iterator;
protected:
	insert_aux(iterator position, const Tp& x);
	insert_aux(iterator position);

public:
	iterator begin() { return m_start; }
	const_iterator begin() const { return m_start; }
	iterator end() { return m_finish; }
	const_iterator end() const { return m_finish; }

	reverse_iterator rbegin()
	{
		return reverse_iterator(end());
	}

	const_reverse_iterator rbegin() const
	{
		return const_reverse_iterator(end());
	}

	reverse_iterator rend()
	{
		return reverse_iterator(begin());
	}

	const_reverse_iterator rend() const
	{
		return const_reverse_iterator(begin());
	}

	size_type size() const
	{
		return size_type(end() - begin());
	}

	size_type max_size()
	{
		return size_type(-1) / sizeof(Tp);
	}
	
	size_type capacity()
	{
		return size_type(m_endOfStorage - begin());
	}

	bool empty() { return begin() == end(); }

	reference operator[](size_type n) { return *(begin() + n); }
	const_reference operator[](size_type n) const { return *(begin() + n); }

	reference at(size_type n)
	{
		if (n >= size())
			_M_throw_range_err();
		return *this[n];
	}

	const_reference at(size_type n) const
	{
		if (n >= size())
			_M_throw_range_err();
		return *this[n];
	}

public:
	vector() {}
	vector(size_type n, Tp & value) { m_finish = uninitialized_fill_n(m_start, n, value); }
	explicit vector(size_type n) { m_finish = uninitialized_fill_n(m_start, n, Tp()); }
	vector(const vector<Tp, Alloc>& x)
	{
		m_finish = uninitialized_copy(x.begin(), x.end(), m_start);
	}

	template<class InputIter> vector(InputIter first, InputIter last)
	{
		typedef typename _Is_integer<InputIter>::_Integral Integral;
		initialize_aux(first, last, Integral());
	}

	template<class Integer>
	void initialize_aux(Integer n, Integer value, __true_type)
	{
		m_start = allocate(n);
		m_endOfStorage = m_start + n;
		m_finish = uninitialized_fill_n(m_start, n, value);
	}

	template<class InputIter>
	void initialize_aux(InputIter first, InputIter last, __false_type)
	{
		range_initialize(first, last, ITERATOR_CATEGORY(first));
	}

	~vector(){ destroy(m_start, m_finish); }

	vector<Tp, Alloc>& operator=(const vector<Tp, Alloc>& x);

	void reserve(size_type n)
	{
		if (capacity() < n)
		{
			const size_type old_size = size();
			iterator tmp = allocate_and_copy(n, m_start, m_finish);
			destroy(m_start, m_finish);
			deallocate(m_start, m_endOfStorage - m_start);
			m_start = tmp;
			m_finish = tmp + old_size;
			m_endOfStorage = m_start + n;
		}
	}

	void assign(size_type n, const Tp& value) { fill_assign(n, value); }
	void fill_assign(size_type n, const Tp& val);

	template<class Integer>
	void assign_dispatch(Integer n, Integer x, __true_type)
	{
		fill_assign((size_type)n, (Tp)x);
	}

	template<class InputIter>
	void assign_dispatch(InputIter first, InputIter last, __false_type)
	{
		assign_aux(first, last, ITERATOR_CATEGORY(first));
	}

	template<class InputIter>
	void assign_aux(InputIter first, InputIter last, input_iterator_tag);

	template<class ForwardIter>
	void assign_aux(ForwardIter first, ForwardIter last, forward_iterator_tag);

public:
	reference front() { return *begin(); }
	const_reference front() const { return *begin(); }
	reference back() { return *(end() - 1); }
	const_reference back() const { return *(end() - 1); }

	void push_back(const Tp& x)
	{
		if (m_finish != m_endOfStorage)
		{
			construct(m_finish, x);
			++m_finish;
		}
		else
			insert_aux(end(), x);
	}

	void push_back()
	{
		if (m_finish != m_endOfStorage)
		{
			construct(m_finish);
			++m_finish;
		}
		else
			insert_aux(end());
	}

	void swap(vector<Tp, Alloc>& x)
	{
		swap(m_start, x.m_start);
		swap(m_finish, x.m_finish);
		swap(m_endOfStorage, x.m_endOfStorage);
	}


	iterator insert(iterator position, const Tp& x)
	{
		size_type n = position - begin();
		if (m_finish != m_endOfStorage && position == end())
		{
			construct(m_finish, x);
			++m_finish;
		}
		else
		{
			insert_aux(position, x);
		}

		return begin() + n;
	}

	iterator insert(iterator position)
	{
		if (m_finish != m_endOfStorage && position == end())
		{
			construct(m_finish);
			++m_finish;
		}
		else
			insert_aux(position);

		return begin() + n;
	}

#ifdef MEBER_TEMPALTES
	template<class InputIter>
	void insert(iterator pos, InputIter, first, InputIter last)
	{
		typedef typename _Is_integer<InputIter>::_Integral Integer;
		insert_dispatch(pos, first, last, Integer());
	}

	template<class Integer>
	void insert_dispatch(iterator pos, Integer n, Integer x, __true_type)
	{
		fill_insert(pos, (size_type)n, (Tp)x);
	}

	template<class InputIter>
	void insert_dispatch(iterator pos, InputIter first, InputIter last, __false_type)
	{
		range_insert(pos, first, last, ITERATOR_CATEGORY(first));
	}

#else
	void insert(iterator pos ,const_iterator first, const_iterator last);
#endif

	void insert(iterator pos, size_type n, const Tp& x)
	{
		fill_insert(pos, n, x);
	}

	void fill_insert(iterator pos, size_type n, Tp& x);

	void pop_back()
	{
		--m_finish;
		destroy(m_finish);
	}

	iterator erase(iterator position)
	{
		if (position + 1 != end)
			copy(position + 1, m_finish, position);
		--m_finish;
		destroy(m_finish);
		return position;
	}

	iterator erase(iterator first, iterator last)
	{
		iterator i = copy(last, m_finish, first);
		destroy(i, m_finish);
		m_finish = m_finish - (last - first);
		return first;
	}

	void resize(size_type newsize, const Tp& x)
	{
		if (newsize < size())
			erase(begin() + newsize, end());
		else
			insert(end(), newsize - size(), x);
	}

	void resize(size_type newsize) { resize(newsize, Tp()); }
	void clear() { erase(begin(), end()); }

protected:

#ifdef MEBER_TEMPALTES
	template <class ForwardIter>
	iterator allocate_and_copy(size_type n, ForwardIter first, ForwardIter last)
	{
		iterator result = allocate(n);
		__TRY{
			uninitialized_copy(first, last, result);
			return result;
		}
		__UNWIND(deallocate(result, n));
	}
#else
	iterator allocate_and_copy(size_type n, const_iterator first, const_iterator last)
	{
		iterator result = allocate(n);
		__TRY{
			uninitialized_copy(first, last, result);
			return result;
		}
		__UNWIND(deallocate(result, n));
	}
#endif

#ifdef MEBER_TEMPALTES
	template<class InputIter>
	void range_initialize(InputIter first, InputIter last, input_iterator_tag)
	{
		for (; first != last; ++firs)
		{
			push_back(*first);
		}
	}

	template<class ForwardIter>
	void range_initialize(ForwardIter first, ForwardIter last, forward_iterator_tag)
	{
		size_type n = 0;
		distance(first, last, n);
		m_start = allocate(n);
		m_endOfStorage = m_start + n;
		m_finish = uninitialized_copy(first, last, m_start);
	}

	template<InputIter>
	void range_insert(iterator pos, InputIter first, InputIter last, input_iterator_tag);

	template<ForwardIter>
	void range_insert(iterator pos, ForwardIter first, ForwardIter last, forward_iterator_tag);
#endif
};

template<class Tp, class Alloc>
inline bool
operator==(const vector<Tp, Alloc>& x, const vector<Tp, Alloc>& y)
{
	return x.size() == y.size() && equal(x.begin(), x.end(), y.begin());
}

template<class Tp, class Alloc>
inline bool
operator<(const vector<Tp, Alloc>& x, const vector<Tp, Alloc>& y)
{
	lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
}

template<class Tp, class Alloc>
inline void swap(vector<Tp, Alloc>& x, vector<Tp, Alloc>& y)
{
	x.swap(y);
}

template<class Tp, class Alloc>
inline bool
operator!=(vector<Tp, Alloc>& x, vector<Tp, Alloc>& y)
{
	return !(x == y);
}

template<class Tp, class Alloc>
inline bool
operator>(vector<Tp, Alloc>& x, vector<Tp, Alloc>& y)
{
	return y < x;
}

template<class Tp, class Alloc>
inline bool
operator<=(vector<Tp, Alloc>& x, vector<Tp, Alloc>& y)
{
	return !(y < x);
}

template<class Tp, class Alloc>
inline bool
operator>=(vector<Tp, Alloc>& x, vector<Tp, Alloc>& y)
{
	return (x < y);
}

template<class Tp, class Alloc>
vector<Tp, Alloc>&
vector<Tp, Alloc>::operator=(const vector<Tp, Alloc>& x)
{
	if (&x != this)
	{
		const size_type xlen = x.size();
		if (xlen > capacity())
		{
			iterator tmp = allocate_and_copy(xlen, x.begin(), x.end());
			destroy(m_start, m_finish);
			deallocate(m_start, m_endOfStorage - m_start);
			m_start = tmp;
			m_endOfStorage = m_start + xlen;
		}
		else if (size() >= xlen)
		{
			iterator i = copy(x.begin(), x.end(), begin());
			destroy(i, m_finish);
		}
		else
		{
			copy(x.begin(), x.begin() + size(), m_start);
			uninitialized_copy(x.begin() + size(), x.end(), m_finish);
		}
		m_finish = m_start + xlen;
	}
	return *this;
}

template<class Tp, class Alloc>
void vector<Tp, Alloc>::fill_assign(size_type n, const Tp& val)
{
	if (n > capacity())
	{
		vector<Tp, Alloc> tmp(n, val);
		tmp.swap(*this);
	}
	else
	{
		if (n > size())
		{
			fill(begin(), end(), val);
			m_finish = uninitialized_copy(m_finish, n - size(), val);
		}
		else
			erase(fill(begin(), n, val), end());
	}
}

#ifdef MEBER_TEMPALTES

template<class Tp, class Alloc>
template<class InputIter>
void vector<Tp, Alloc>::assign_aux(InputIter first, InputIter last, input_iterator_tag)
{
	iterator cur = begin();
	for (; first != last && cur != end(); ++cur, ++first)
	{
		*cur = *first;
	}
	
	if (first == last)
		erase(cur, end());
	else
		insert(end(), first, last);

}

template<class Tp, class Alloc>
template<class ForwardIter>
void vector<Tp, Alloc>::assign_aux(ForwardIter first, ForwardIter last, forward_iterator_tag)
{
	size_type n = 0;
	distance(first, last, n);
	if (n > capacity())
	{
		iterator tmp = allocate_and_copy(n, first, start);
		destroy(m_start, m_finish);
		deallocate(m_start, m_endOfStorage - m_start);
		m_start = tmp;
		m_finish = m_start + n;
		m_endOfStorage = m_start + n;
	}
	else if (size() >= n)
	{
		iterator new_finish = copy(first, last, m_start);
		destroy(m_start, m_finish);
		m_finish = new_finish;
	}
	else
	{
		ForwardIter m = first;
		advance(m, size());
		copy(first, m, m_start);
		m_finish = uninitialized_copy(m, last, m_finish);
	}

}

template<class Tp, class Alloc>
void vector<Tp, Alloc>::insert_aux(iterator position)
{
	if (m_finish != m_endOfStorage)
	{
		construct(m_finish, *(m_finish - 1));
		++m_finish;
		copy_backward(position, m_finish - 2, m_finish - 1);
		*position = Tp();
	}
	else
	{
		const size_type old_size = size();
		const size_type len = old_size != 0 ? 2 * old_size : 1;
		iterator new_start = allocate(len);
		iterator new_finish = new_start;
		__TRY
		{
			new_finish = uninitialized_copy(m_start, position, new_start);
			construct(new_finish);
			++new_finish;
			new_finish = uninitialized_copy(position, m_finish, new_finish);
		}
		__UNWIND((destroy(new_start,new_finish), 
				deallocate(new_start,len)));
		destroy(begin(), end());
		deallocate(m_start, m_endOfStorage - m_start);
		m_start = new_start;
		m_finish = new_finish;
		m_endOfStorage = m_start + len;
	}
}

template<class Tp, class Alloc>
void vector<Tp, Alloc>::insert_aux(iterator position, size_type n, const Tp& x)
{
	if (n != 0)
	{
		if (((size_type)m_endOfStorage - m_start) > = n)
		{
			Tp xcopy = x;
			const size_type elems_after = m_finish - position;
			iterator old_finish = m_finish;
			if (elems_after > n)
			{
				uninitialized_copy(m_finish - n, m_finish, m_finish);
				m_finish += n;
				copy_backward(position, old_finish - n, old_finish);
				fill(position, position + n, xcopy);
			}
			else
			{
				uninitialized_fill_n(m_finish, n - elems_after, xcopy);
				m_finish += n - elems_after;
				uninitialized_copy(position, old_finish, m_finish);
				m_finish += elems_after;
				fill(position, position + n, xcopy);
			}
		}
		else
		{
			const size_type old_size = size();
			const size_type len = old_size + max(old_size, n);
			iterator new_start = allocate(len);
			iterator new_finish = new_start;
			__TRY{
				new_finish = uninitialized_copy(m_start, position, new_start);
				new_finish = uninitialized_fill_n(new_finish, n, x);
				new_finish = uninitialized_copy(position, m_finish, new_finish);
			}
			__UNWIND((destroy(new_start, new_finish), deallocate(new_start, len)));

			destroy(m_start, m_finish);
			deallocate(m_start, m_endOfStorage - m_start);
			m_start = new_start;
			m_finish = new_finish;
			m_endOfStorage = m_start + len;
		}
	}
}

template<class Tp, class Alloc>
template<class InputIter>
void vector<Tp, Alloc>::range_insert(iterator pos, InputIter first, InputIter last, input_iterator_tag)
{
	for (; first != last; ++first)
	{
		pos = insert(pos, *first);
		++pos;
	}
}

template<class Tp, class Alloc>
template<class ForwardIter>
void vector<Tp, Alloc>::range_insert(iterator pos, ForwardIter first, ForwardIter last, forward_iterator_tag)
{
	if (first != last)
	{
		size_type n = 0;
		distance(first, last, n);
		if (((size_type)m_endOfStorage - m_start) >= n)
		{
			const size_type elems_after = m_finish - pos;
			iterator old_finish = m_finish;

			if (elems_after > n)
			{
				uninitialized_copy(m_finish - n, m_finish, m_finish);
				m_finish += n;
				copy_backward(pos, old_finish - n, old_finish);
				copy(first, last, pos);
			}
			else
			{
				uninitialized_copy(first + elems_after, last, m_finish);
				m_finish += n - elems_after;
				uninitialized_copy(pos, old_finish, m_finish);
				copy(first, first + elems_after, pos);
			}
		}
		else
		{
			const size_type old_size = size();
			const size_type len = old_size + max(old_size, n);
			iterator new_start = allocate(len);
			iterator new_finish = new_start;
			__TRY{
				new_finish = uninitialized_copy(m_start, pos, new_start);
				new_finish = uninitialized_copy(first, last, new_finish);
				new_finish = uninitialized_copy(pos, m_finish, new_finish);
			}
			__UNWIND((destroy(new_start, new_finish), deallocate(new_start + len)));
			destroy(m_start, m_finish);
			deallocate(m_start, m_endOfStorage - m_start);
			m_start = new_start;
			m_finish = new_finish;
			m_endOfStorage = m_start + len;
		}
	}
}

#else //MEBER_TEMPALTES

template<class Tp, class Alloc>
void vector<Tp, Alloc>::insert(iterator pos ,const_iterator first, const_iterator last)
{
	if(first != last)
	{
		size_type n = 0;
		distance(first, last, n);
		if(((size_type)m_endOfStorage - m_start) >= n)
		{
			const size_type elems_after = m_finish - pos;
			iterator old_finish = m_finish;
			if(elems_after  > n)
			{
				uninitialized_copy(m_finish -n, m_finish, m_finish);
				m_finish += n;
				copy_backward(pos, old_finish - n, old_finish);
				copy(first, last, pos);
			}
			else
			{
				uninitialized_copy(first + elems_after, last, m_finish);
				m_finish += n - elems_after;
				uninitialized_copy(pos, old_finish, m_finish);
				copy(first, first + elems_after, pos);
			}
		}
		else
		{
			const size_type old_size = size();
			const size_type len = old_size + max(old_size, n);
			iterator new_start = allocate(len);
			iterastor new_finish = new_start;
			__TRY{
				new_finish = uninitialized_copy(m_start, pos, new_finish);
				new_finish = uninitialized_copy(first, last, new_finish);
				new_finish = uninitialized_copy(pos, old_finish, new_finish);
			}
			__UNWIND((destroy(new_start, new_finish), deallocate(new_start, len)));

			destroy(m_start, m_finish);
			deallocate(m_start, m_endOfStorage - m_start);
			m_start = new_start;
			m_finish = new_finish;
			m_endOfStorage = m_start + len;
		}
	}
}
#endif //MEBER_TEMPALTES


NAMESPACE_END