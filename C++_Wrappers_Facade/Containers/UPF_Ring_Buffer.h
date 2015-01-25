#ifndef UPF_RING_BUFFER_H_
#define UPF_RING_BUFFER_H_


template<class T, unsigned long Capacity> 
class UPF_Ring_Buffer
{
public:

    // ȡ����Ԫ��, ���Ϊ�շ���false, ���򷵻�true.
    bool first( T & t )
    {
        if ( size() == 0 )
        {
            return false;
        }
        else
        {
            t = m_buffer[ m_head ];
 
            return true;
        }
    }

	// ���������������Զ���������Ԫ��
	void push(const T& t)
	{
		if( m_size == Capacity )
		{
			//pop��һ��Ԫ��
			if (++m_head >= Capacity)
			{
				m_head = 0;
			}
		}
		else
		{
			m_size++;
		}
		
		if(++m_tail >= Capacity)
		{
			m_tail = 0;
		}

//		_ASSERT( (0 <= m_tail) && (m_tail< Capacity) );
		m_buffer[m_tail] = t;
	}	
	
	// ����ֵ��
	// true	 (�ɹ�)
    // false (ʧ�ܣ�����Ϊ��)
	bool pop(T& t)
	{
		bool result;
		if( 0 == m_size )
		{
			result = false;
		}
		else
		{
	//		_ASSERT( (0 <= m_head) && (m_head< Capacity) );
			t = m_buffer[m_head];
			if(++m_head >= Capacity)
			{
				m_head = 0;
			}

			m_size--;
			
			result = true;
		}
		return result;
	}
	int size()
	{
		return m_size;
	}

	UPF_Ring_Buffer():m_head(0), m_tail(Capacity-1), m_size(0){};
private:

	T			  m_buffer[Capacity];
    unsigned long m_head;
	unsigned long m_tail;	
	unsigned long m_size;
	
};




#endif