

#ifndef _LINQ_H_
#define _LINQ_H_

#include <map>
#include <set>
#include <list>
#include <deque>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <exception>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <random>

namespace sb {

    /* exception */
    class enumerable_exception : public std::exception {
    public:
        enumerable_exception(const char* msg) : m_emsg(msg)
        {
        }

        enumerable_exception(std::string& msg) : m_emsg(msg)
        {
        }

        virtual const char* what() const
#ifdef _GLIBCXX_USE_NOEXCEPT
            _GLIBCXX_USE_NOEXCEPT
#endif
        {
            return m_emsg.c_str();
        }
    private:
        std::string m_emsg;
    };

    /* helper */
    template <typename Type>
    struct recover_type {
        typedef typename std::remove_cv<typename std::remove_reference<Type>::type>::type type;
    };

    template <>
    struct recover_type<void> {
        typedef void type;
    };

    template <typename Functor>
    struct functor_retriver {
        typedef void type;
    };

    template <typename Class, typename Result, typename ...Args>
    struct functor_retriver<Result(Class::*)(Args...)> {
        typedef Result type;
    };

    template <typename Class, typename Result, typename ...Args>
    struct functor_retriver<Result(Class::*)(Args...)const> {
        typedef Result type;
    };

    /* iterator */
    template <typename Type>
    struct iterator_wrap {
        class placeholder {
        public:
            virtual std::shared_ptr<placeholder> next(void) = 0;
            virtual Type value(void) const = 0;
            virtual bool equals(const std::shared_ptr<placeholder>& rhs) const = 0;
        };

        template <typename Iterator>
        class holder : public placeholder {
        public:
            holder(const Iterator& iterator) :
                m_iterator(iterator)
            {
            }

            virtual std::shared_ptr<placeholder> next(void)
            {
                auto it = m_iterator;
                it++;
                return std::make_shared<holder<Iterator>>(it);
            }

            virtual Type value(void) const
            {
                return *m_iterator;
            }

            virtual bool equals(const std::shared_ptr<placeholder>& rhs) const
            {
                auto real_rhs = std::dynamic_pointer_cast<holder<Iterator>>(rhs);
                return m_iterator == real_rhs->m_iterator;
            }

        private:
            Iterator m_iterator;
        };
    };

    template <typename Type>
    class enumerable_iterator : public std::iterator<std::forward_iterator_tag, Type> {
    private:
        typedef enumerable_iterator<Type> Self;

    private:
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_iterator;

    public:
        template <typename Iterator>
        enumerable_iterator(const Iterator& iterator) :
            m_iterator(std::make_shared<typename iterator_wrap<Type>::template holder<Iterator>>(iterator))
        {
        }

        Self& operator++()
        {
            m_iterator = m_iterator->next();
            return *this;
        }

        Self operator++(int)
        {
            auto temp = *this;
            m_iterator = m_iterator->next();
            return temp;
        }

        Type operator*() const
        {
            return m_iterator->value();
        }

        bool operator==(const Self& rhs) const
        {
            return m_iterator->equals(rhs.m_iterator);
        }

        bool operator!=(const Self& rhs) const
        {
            return !m_iterator->equals(rhs.m_iterator);
        }
    };

    template <typename Iterator, typename Type = typename recover_type<typename std::iterator_traits<Iterator>::value_type>::type>
    enumerable_iterator<Type> make_enumerable_iterator(const Iterator& iterator)
    {
        return enumerable_iterator<Type>(iterator);
    }

    template <typename Type>
    class concat_iterator : public std::iterator<std::forward_iterator_tag, Type> {
    private:
        typedef concat_iterator<Type> Self;

    private:
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_lhsbegin;
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_lhsend;
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_right_begin;
        bool m_left;

    public:
        template <typename LhsIterator, typename RhsIterator>
        concat_iterator(const LhsIterator& lhsbegin, const LhsIterator& lhsend, const RhsIterator& right_begin) :
            m_lhsbegin(std::make_shared<typename iterator_wrap<Type>::template holder<LhsIterator>>(lhsbegin)),
            m_lhsend(std::make_shared<typename iterator_wrap<Type>::template holder<LhsIterator>>(lhsend)),
            m_right_begin(std::make_shared<typename iterator_wrap<Type>::template holder<RhsIterator>>(right_begin)),
            m_left(lhsbegin != lhsend)
        {
        }

        Self& operator++()
        {
            if (m_left) {
                m_lhsbegin = m_lhsbegin->next();

                if (m_lhsbegin->equals(m_lhsend)) {
                    m_left = false;
                }

            } else {
                m_right_begin = m_right_begin->next();
            }

            return *this;
        }

        Self operator++(int)
        {
            auto temp = *this;

            if (m_left) {
                m_lhsbegin = m_lhsbegin->next();

                if (m_lhsbegin->equals(m_lhsend)) {
                    m_left = false;
                }

            } else {
                m_right_begin = m_right_begin->next();
            }

            return temp;
        }

        Type operator*() const
        {
            return m_left ? m_lhsbegin->value() : m_right_begin->value();
        }

        bool operator==(const Self& rhs) const
        {
            if (m_left != rhs.m_left) {
                return false;
            }

            return m_left ? m_lhsbegin->equals(rhs.m_lhsbegin) : m_right_begin->equals(rhs.m_right_begin);
        }

        bool operator!=(const Self& rhs) const
        {
            if (m_left != rhs.m_left) {
                return true;
            }

            return m_left ? !m_lhsbegin->equals(rhs.m_lhsbegin) : !m_right_begin->equals(rhs.m_right_begin);
        }
    };

    template <typename LhsIterator, typename RhsIterator, typename Type = typename recover_type<typename std::iterator_traits<LhsIterator>::value_type>::type>
    concat_iterator<Type> make_concat_iterator(const LhsIterator& lhsbegin, const LhsIterator& lhsend, const RhsIterator& right_begin)
    {
        return concat_iterator<Type>(lhsbegin, lhsend, right_begin);
    }

    template <typename Container, typename Type>
    class storage_iterator : public std::iterator<std::forward_iterator_tag, Type> {
    private:
        typedef storage_iterator<Container, Type> Self;

    private:
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_iterator;
        std::shared_ptr<Container> m_owner;

    public:
        template <typename Iterator>
        storage_iterator(const std::shared_ptr<Container>& owner, const Iterator& iterator) :
            m_owner(owner),
            m_iterator(std::make_shared<typename iterator_wrap<Type>::template holder<Iterator>>(iterator))
        {
        }

        Self& operator++()
        {
            m_iterator = m_iterator->next();
            return *this;
        }

        Self operator++(int)
        {
            auto temp = *this;
            m_iterator = m_iterator->next();
            return temp;
        }

        Type operator*() const
        {
            return m_iterator->value();
        }

        bool operator==(const Self& rhs) const
        {
            return m_iterator->equals(rhs.m_iterator);
        }

        bool operator!=(const Self& rhs) const
        {
            return !m_iterator->equals(rhs.m_iterator);
        }
    };

    template <typename Container, typename Iterator, typename Type = typename recover_type<typename std::iterator_traits<Iterator>::value_type>::type>
    storage_iterator<Container, Type> make_storage_iterator(const std::shared_ptr<Container>& values, const Iterator& iterator)
    {
        return storage_iterator<Container, Type>(values, iterator);
    }

    template <typename Type, typename Functor>
    class select_iterator : public std::iterator<std::forward_iterator_tag, Type> {
    private:
        typedef select_iterator<Type, Functor> Self;

    private:
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_iterator;
        Functor m_selector;

    public:
        template <typename Iterator>
        select_iterator(const Iterator& iterator, const Functor& selector) :
            m_selector(selector),
            m_iterator(std::make_shared<typename iterator_wrap<Type>::template holder<Iterator>>(iterator))
        {
        }

        Self& operator++()
        {
            m_iterator = m_iterator->next();
            return *this;
        }

        Self operator++(int)
        {
            auto temp = *this;
            m_iterator = m_iterator->next();
            return temp;
        }

        typename functor_retriver<decltype(&Functor::operator())>::type operator*() const
        {
            return m_selector(m_iterator->value());
        }

        bool operator==(const Self& rhs) const
        {
            return m_iterator->equals(rhs.m_iterator);
        }

        bool operator!=(const Self& rhs) const
        {
            return !m_iterator->equals(rhs.m_iterator);
        }
    };

    template <typename Iterator, typename Functor, typename Type = typename recover_type<typename std::iterator_traits<Iterator>::value_type>::type>
    select_iterator<Type, Functor> make_select_iterator(const Iterator& iterator, const Functor& selector)
    {
        return select_iterator<Type, Functor>(iterator, selector);
    }

    template <typename Type>
    class skip_iterator : public std::iterator<std::forward_iterator_tag, Type> {
    private:
        typedef skip_iterator<Type> Self;

    private:
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_iterator;

    public:
        template <typename Iterator>
        skip_iterator(const Iterator& begin, const Iterator& end, int count)
        {
            Iterator it = begin;
            for (int i = 0; i < count && it != end; i++, it++);
            m_iterator = std::make_shared<typename iterator_wrap<Type>::template holder<Iterator>>(it);
        }

        Self& operator++()
        {
            m_iterator = m_iterator->next();
            return *this;
        }

        Self operator++(int)
        {
            auto temp = *this;
            m_iterator = m_iterator->next();
            return temp;
        }

        Type operator*() const
        {
            return m_iterator->value();
        }

        bool operator==(const Self& rhs) const
        {
            return m_iterator->equals(rhs.m_iterator);
        }

        bool operator!=(const Self& rhs) const
        {
            return !m_iterator->equals(rhs.m_iterator);
        }
    };

    template <typename Iterator, typename Type = typename recover_type<typename std::iterator_traits<Iterator>::value_type>::type>
    skip_iterator<Type> make_skip_iterator(const Iterator& begin, const Iterator& end, int count)
    {
        return skip_iterator<Type>(begin, end, count);
    }

    template <typename Type, typename Functor>
    class skip_while_iterator : public std::iterator<std::forward_iterator_tag, Type> {
    private:
        typedef skip_while_iterator<Type, Functor> Self;

    private:
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_iterator;

    public:
        template <typename Iterator>
        skip_while_iterator(const Iterator& begin, const Iterator& end, const Functor& predicate)
        {
            Iterator it = begin;
            for (; it != end && predicate(*it); ++it);
            m_iterator = std::make_shared<typename iterator_wrap<Type>::template holder<Iterator>>(it);
        }

        Self& operator++()
        {
            m_iterator = m_iterator->next();
            return *this;
        }

        Self operator++(int)
        {
            auto temp = *this;
            m_iterator = m_iterator->next();
            return temp;
        }

        Type operator*() const
        {
            return m_iterator->value();
        }

        bool operator==(const Self& rhs) const
        {
            return m_iterator->equals(rhs.m_iterator);
        }

        bool operator!=(const Self& rhs) const
        {
            return !m_iterator->equals(rhs.m_iterator);
        }
    };

    template <typename Iterator, typename Functor, typename Type = typename recover_type<typename std::iterator_traits<Iterator>::value_type>::type>
    skip_while_iterator<Type, Functor> make_skip_while_iterator(const Iterator& begin, const Iterator& end, const Functor& predicate)
    {
        return skip_while_iterator<Type, Functor>(begin, end, predicate);
    }

    template <typename Type>
    class take_iterator : public std::iterator<std::forward_iterator_tag, Type> {
    private:
        typedef take_iterator<Type> Self;

    private:
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_begin;
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_end;
        int m_count;
        int m_current;

    public:
        template <typename Iterator>
        take_iterator(const Iterator& begin, const Iterator& end, int count) :
            m_begin(std::make_shared<typename iterator_wrap<Type>::template holder<Iterator>>(begin)),
            m_end(std::make_shared<typename iterator_wrap<Type>::template holder<Iterator>>(end)),
            m_count(count),
            m_current(0)
        {
            if (m_current == m_count) {
                m_begin = m_end;
            }
        }

        Self& operator++()
        {
            if (++m_current == m_count) {
                m_begin = m_end;

            } else {
                m_begin = m_begin->next();
            }
            return *this;
        }

        Self operator++(int)
        {
            auto temp = *this;

            if (++m_current == m_count) {
                m_begin = m_end;

            } else {
                m_begin = m_begin->next();
            }

            return temp;
        }

        Type operator*() const
        {
            return m_begin->value();
        }

        bool operator==(const Self& rhs) const
        {
            return m_begin->equals(rhs.m_begin);
        }

        bool operator!=(const Self& rhs) const
        {
            return !m_begin->equals(rhs.m_begin);
        }
    };

    template <typename Iterator, typename Type = typename recover_type<typename std::iterator_traits<Iterator>::value_type>::type>
    take_iterator<Type> make_take_iterator(const Iterator& begin, const Iterator& end, int count)
    {
        return take_iterator<Type>(begin, end, count);
    }

    template <typename Type, typename Functor>
    class take_while_iterator : public std::iterator<std::forward_iterator_tag, Type> {
    private:
        typedef take_while_iterator<Type, Functor> Self;

    private:
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_begin;
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_end;
        Functor m_predicate;

    public:
        template <typename Iterator>
        take_while_iterator(const Iterator& begin, const Iterator& end, const Functor& predicate) :
            m_begin(std::make_shared<typename iterator_wrap<Type>::template holder<Iterator>>(begin)),
            m_end(std::make_shared<typename iterator_wrap<Type>::template holder<Iterator>>(end)),
            m_predicate(predicate)
        {
            if (!m_begin->equals(m_end) && !m_predicate(m_begin->value())) {
                m_begin = m_end;
            }
        }

        Self& operator++()
        {
            m_begin = m_begin->next();
            if (!m_predicate(m_begin->value())) {
                m_begin = m_end;
            }
            return *this;
        }

        Self operator++(int)
        {
            auto temp = *this;
            m_begin = m_begin->next();
            if (!m_predicate(m_begin->value())) {
                m_begin = m_end;
            }
            return temp;
        }

        Type operator*() const
        {
            return m_begin->value();
        }

        bool operator==(const Self& rhs) const
        {
            return m_begin->equals(rhs.m_begin);
        }

        bool operator!=(const Self& rhs) const
        {
            return !m_begin->equals(rhs.m_begin);
        }
    };

    template <typename Iterator, typename Functor, typename Type = typename recover_type<typename std::iterator_traits<Iterator>::value_type>::type>
    take_while_iterator<Type, Functor> make_take_while_iterator(const Iterator& begin, const Iterator& end, const Functor& predicate)
    {
        return take_while_iterator<Type, Functor>(begin, end, predicate);
    }

    template <typename Type, typename Functor>
    class where_iterator : public std::iterator<std::forward_iterator_tag, Type> {
    private:
        typedef where_iterator<Type, Functor> Self;

    private:
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_begin;
        std::shared_ptr<typename iterator_wrap<Type>::placeholder> m_end;
        Functor m_predicate;

    public:
        template <typename Iterator>
        where_iterator(const Iterator& begin, const Iterator& end, const Functor& predicate) :
            m_begin(std::make_shared<typename iterator_wrap<Type>::template holder<Iterator>>(begin)),
            m_end(std::make_shared<typename iterator_wrap<Type>::template holder<Iterator>>(end)),
            m_predicate(predicate)
        {
            where(false);
        }

        Self& operator++()
        {
            where(true);
            return *this;
        }

        Self operator++(int)
        {
            auto temp = *this;
            where(true);
            return temp;
        }

        Type operator*() const
        {
            return m_begin->value();
        }

        bool operator==(const Self& rhs) const
        {
            return m_begin->equals(rhs.m_begin);
        }

        bool operator!=(const Self& rhs) const
        {
            return !m_begin->equals(rhs.m_begin);
        }

    private:
        void where(bool next)
        {
            if (m_begin->equals(m_end)) {
                return;
            }

            if (next) {
                m_begin = m_begin->next();
            }

            while (!m_begin->equals(m_end) && !m_predicate(m_begin->value())) {
                m_begin = m_begin->next();
            }
        }
    };

    template <typename Iterator, typename Functor, typename Type = typename recover_type<typename std::iterator_traits<Iterator>::value_type>::type>
    where_iterator<Type, Functor> make_where_iterator(const Iterator& begin, const Iterator& end, const Functor& predicate)
    {
        return where_iterator<Type, Functor>(begin, end, predicate);
    }

    template <typename LeftType, typename RightType>
    class zip_iterator : public std::iterator<std::forward_iterator_tag, std::pair<LeftType, RightType>> {
    private:
        typedef zip_iterator<LeftType, RightType> Self;

    private:
        std::shared_ptr<typename iterator_wrap<LeftType>::placeholder>  m_left_begin;
        std::shared_ptr<typename iterator_wrap<LeftType>::placeholder>  m_left_end;
        std::shared_ptr<typename iterator_wrap<RightType>::placeholder> m_right_begin;
        std::shared_ptr<typename iterator_wrap<RightType>::placeholder> m_right_end;

    public:
        template <typename LeftIterator, typename RightIterator>
        zip_iterator(const LeftIterator& left_begin, const LeftIterator& left_end, const RightIterator& right_begin, const RightIterator& right_end) :
            m_left_begin(std::make_shared<typename iterator_wrap<LeftType>::template holder<LeftIterator>>(left_begin)),
            m_left_end(std::make_shared<typename iterator_wrap<LeftType>::template holder<LeftIterator>>(left_end)),
            m_right_begin(std::make_shared<typename iterator_wrap<RightType>::template holder<RightIterator>>(right_begin)),
            m_right_end(std::make_shared<typename iterator_wrap<RightType>::template holder<RightIterator>>(right_end))
        {
        }

        Self& operator++()
        {
            if (!m_left_begin->equals(m_left_end) && !m_right_begin->equals(m_right_end)) {
                m_left_begin = m_left_begin->next();
                m_right_begin = m_right_begin->next();
            }

            return *this;
        }

        Self operator++(int)
        {
            auto temp = *this;

            if (!m_left_begin->equals(m_left_end) && !m_right_begin->equals(m_right_end)) {
                m_left_begin = m_left_begin->next();
                m_right_begin = m_right_begin->next();
            }

            return temp;
        }

        std::pair<LeftType, RightType> operator*() const
        {
            return std::make_pair(m_left_begin->value(), m_right_begin->value());
        }

        bool operator==(const Self& rhs) const
        {
            return m_left_begin->equals(rhs.m_left_begin) && m_right_begin->equals(rhs.m_right_begin);
        }

        bool operator!=(const Self& rhs) const
        {
            return !m_left_begin->equals(rhs.m_left_begin) && !m_right_begin->equals(rhs.m_right_begin);
        }
    };

    template <typename LeftIterator,
              typename RightIterator,
              typename LeftType = typename recover_type<typename std::iterator_traits<LeftIterator>::value_type>::type,
              typename RightType = typename recover_type<typename std::iterator_traits<RightIterator>::value_type>::type>
    zip_iterator<LeftType, RightType> make_zip_iterator(const LeftIterator& left_begin, const LeftIterator& left_end, const RightIterator& right_begin, const RightIterator& right_end)
    {
        return zip_iterator<LeftType, RightType>(
            left_begin,
            left_end,
            right_begin,
            right_end
            );
    }

    template <typename KeyType, typename LeftType, typename RightType, typename Functor>
    class zip_with_iterator : public std::iterator<std::forward_iterator_tag, std::pair<LeftType, RightType>> {
    private:
        typedef zip_with_iterator<KeyType, LeftType, RightType, Functor> Self;

    private:
        std::shared_ptr<typename iterator_wrap<LeftType>::placeholder>  m_left_begin;
        std::shared_ptr<typename iterator_wrap<LeftType>::placeholder>  m_left_end;
        std::shared_ptr<typename iterator_wrap<RightType>::placeholder> m_right_begin;
        std::shared_ptr<typename iterator_wrap<RightType>::placeholder> m_right_end;
        Functor m_selector;

    public:
        template <typename LeftIterator, typename RightIterator>
        zip_with_iterator(const LeftIterator& left_begin, const LeftIterator& left_end, const RightIterator& right_begin, const RightIterator& right_end, const Functor& selector) :
            m_left_begin(std::make_shared<typename iterator_wrap<LeftType>::template holder<LeftIterator>>(left_begin)),
            m_left_end(std::make_shared<typename iterator_wrap<LeftType>::template holder<LeftIterator>>(left_end)),
            m_right_begin(std::make_shared<typename iterator_wrap<RightType>::template holder<RightIterator>>(right_begin)),
            m_right_end(std::make_shared<typename iterator_wrap<RightType>::template holder<RightIterator>>(right_end)),
            m_selector(selector)
        {
        }

        Self& operator++()
        {
            if (!m_left_begin->equals(m_left_end) && !m_right_begin->equals(m_right_end)) {
                m_left_begin = m_left_begin->next();
                m_right_begin = m_right_begin->next();
            }

            return *this;
        }

        Self operator++(int)
        {
            auto temp = *this;

            if (!m_left_begin->equals(m_left_end) && !m_right_begin->equals(m_right_end)) {
                m_left_begin = m_left_begin->next();
                m_right_begin = m_right_begin->next();
            }

            return temp;
        }

        std::pair<KeyType, RightType> operator*() const
        {
            return std::make_pair(m_selector(m_left_begin->value(), m_right_begin->value()), m_right_begin->value());
        }

        bool operator==(const Self& rhs) const
        {
            return m_left_begin->equals(rhs.m_left_begin) && m_right_begin->equals(rhs.m_right_begin);
        }

        bool operator!=(const Self& rhs) const
        {
            return !m_left_begin->equals(rhs.m_left_begin) && !m_right_begin->equals(rhs.m_right_begin);
        }
    };

    template <typename LeftIterator,
              typename RightIterator,
              typename Functor,
              typename KeyType = typename functor_retriver<decltype(&Functor::operator())>::type,
              typename LeftType = typename recover_type<typename std::iterator_traits<LeftIterator>::value_type>::type,
              typename RightType = typename recover_type<typename std::iterator_traits<RightIterator>::value_type>::type>
    zip_with_iterator<KeyType, LeftType, RightType, Functor> make_zip_with_iterator(const LeftIterator& left_begin, const LeftIterator& left_end, const RightIterator& right_begin, const RightIterator& right_end, const Functor& selector)
    {
        return zip_with_iterator<KeyType, LeftType, RightType, Functor>(
            left_begin,
            left_end,
            right_begin,
            right_end,
            selector
            );
    }

    template <typename Type>
    class empty_iterator : public std::iterator<std::forward_iterator_tag, Type> {
    private:
        typedef empty_iterator<Type> Self;

    public:
        empty_iterator()
        {
        }

        Self& operator++()
        {
            return *this;
        }

        Self operator++(int)
        {
            return *this;
        }

        Type operator*() const
        {
            throw enumerable_exception("get a value from an empty collection");
        }

        bool operator==(const Self& rhs) const
        {
            return true;
        }

        bool operator!=(const Self& rhs) const
        {
            return false;
        }
    };

    template <typename Type>
    empty_iterator<Type> make_empty_iterator(void)
    {
        return empty_iterator<Type>();
    }

    template <typename Type, typename Functor>
    class random_iterator : public std::iterator<std::forward_iterator_tag, Type> {
    private:
        typedef random_iterator<Type, Functor> Self;

    private:
        std::shared_ptr<std::random_device> m_rd;
        bool m_flag;
        Functor m_selector;
    public:
        random_iterator(bool flag, const Functor& selector) : 
            m_rd(std::make_shared<std::random_device>()), 
            m_flag(flag),
            m_selector(selector)
        {
        }

        Self& operator++()
        {
            return *this;
        }

        Self operator++(int)
        {
            return *this;
        }

        typename functor_retriver<decltype(&Functor::operator())>::type operator*() const
        {
            return m_selector((*m_rd)());
        }

        bool operator==(const Self& rhs) const
        {
            return m_flag == rhs.m_flag;
        }

        bool operator!=(const Self& rhs) const
        {
            return m_flag != rhs.m_flag;
        }
    };

    template <typename Type, typename Functor>
    random_iterator<Type, Functor> make_random_iterator(bool flag, const Functor& selector)
    {
        return random_iterator<Type, Functor>(flag, selector);
    }

    /* interface */
    template <typename Type>
    class enumerable;

    template <typename Type>
    inline enumerable<Type> from_random(void) 
    {
        auto selector = [](const Type& x){return x;};
        typedef decltype(selector) Functor;
        return enumerable<Type>(
            make_random_iterator<Type, Functor>(false, selector),
            make_random_iterator<Type, Functor>(true, selector)
            );
    }

    template <typename Type, typename Functor>
    inline enumerable<Type> from_random(const Functor& selector) 
    {
        return enumerable<Type>(
            make_random_iterator<Type, Functor>(false, selector),
            make_random_iterator<Type, Functor>(true, selector)
            );
    }

    template <typename Iterator, 
              typename Type = typename recover_type<typename std::iterator_traits<Iterator>::value_type>::type>
    inline enumerable<Type> from(const Iterator& begin, const Iterator& end)
    {
        return enumerable<Type> (
            make_enumerable_iterator(begin), 
            make_enumerable_iterator(end)
            );
    }

    template <typename Container>
    inline auto from(const Container& container) ->
        decltype(from(std::begin(container), std::end(container)))
    {
        return from (
            std::begin(container),
            std::end(container)
            );
    }

    template <typename Type>
    inline enumerable<Type> from(const std::initializer_list<Type>& container)
    {
        return from (
            make_enumerable_iterator(std::begin(container)),
            make_enumerable_iterator(std::end(container))
            );
    }

    template <typename Iterator, 
              typename Type = typename recover_type<typename std::iterator_traits<Iterator>::value_type>::type>
    inline enumerable<Type> from_values(const Iterator& begin, const Iterator& end)
    {
        auto ptr = std::make_shared<std::vector<Type>>(begin, end);
        return from (
            make_storage_iterator(ptr, ptr->begin()),
            make_storage_iterator(ptr, ptr->end())
            );
    }

    template <typename Container>
    inline auto from_values(const Container& container) -> 
        decltype(from_values(std::begin(container), std::end(container)))
    {
        return from_values (
            std::begin(container),
            std::end(container)
            );
    }
    
    template<typename Type>
    inline enumerable<Type> from_values(const std::initializer_list<Type>& container)
    {
        return from_values(
            std::begin(container), 
            std::end(container)
            );
    }

    template <typename Container>
    inline auto from_values(const std::shared_ptr<Container>& container) -> 
        enumerable<typename recover_type<decltype(*std::begin(*container))>::type>
    {
        return from (
            make_storage_iterator(container, std::begin(*container)), 
            make_storage_iterator(container, std::end(*container))
            );
    }
    
    /* implement */
    template <typename Type>
    class enumerable {
    private:
        typedef enumerable<Type> Self;
         
    private:
        enumerable_iterator<Type>  m_begin;
        enumerable_iterator<Type>  m_end;

    public:
        enumerable() : 
            m_begin(enumerable_iterator<Type>(make_empty_iterator<Type>())),
            m_end(enumerable_iterator<Type>(make_empty_iterator<Type>()))
        {
        }

        enumerable(const enumerable_iterator<Type>& begin, const enumerable_iterator<Type>& end) : 
            m_begin(begin),
            m_end(end)
        {
        }

        enumerable_iterator<Type> begin() const
        {
            return m_begin;
        }

        enumerable_iterator<Type> end() const
        {
            return m_end;
        }

        template <typename Functor>
        Type aggregate(const Functor& reducer) const
        {
            if (empty()) {
                throw enumerable_exception("get a value from an empty collection");
            }

            auto it = begin();
            auto result = *it;

            while(++it != end()) {
                result = reducer(result, *it);
            }

            return result;
        }

        template <typename ResultType, typename Functor>
        ResultType aggregate(const ResultType& seed, const Functor& reducer) const
        {
            auto result = seed;
            auto it = begin();

            for (result = seed; it != end(); ++it) {
                result = reducer(result, *it);
            }

            return result;
        }

        template <typename ResultType, typename Functor, typename Selector>
        typename functor_retriver<decltype(&Selector::operator())>::type
            aggregate(const ResultType& seed, const Functor& reducer, const Selector& selector) const
        {
            return selector(aggregate(seed, reducer));
        }

        template <typename Functor>
        bool all(const Functor& predicate) const
        {
            return std::all_of(begin(), end(), predicate);
        }

        template <typename Functor>
        bool any(const Functor& predicate) const
        {
            return std::any_of(begin(), end(), predicate);
        }

        template <typename ResultType>
        ResultType average(void) const
        {
            if (empty()) {
                throw enumerable_exception("get a value from an empty collection");
            }

            ResultType total = 0;
            auto counter = 0;

            for (auto it = begin(); it != end(); ++it, ++counter) {
                total += *it;
            }

            return total / counter;
        }
        
        template <typename Iterator>
        Self concat(const Iterator& right_begin, const Iterator& right_end) const
        {
            return from (
                make_concat_iterator(begin(), end(), right_begin),
                make_concat_iterator(end(), end(), right_end)
                );
        }

        template <typename Container>
        Self concat(const Container& container) const
        {
            return concat(std::begin(container), std::end(container));
        }

        Self concat(const std::initializer_list<Type>& container) const 
        {
            return concat(std::begin(container), std::end(container));
        }
        
        bool contains(const Type& value) const
        {
            return std::find(begin(), end(), value) != end();
        }

        int count(void) const
        {
            return std::distance(begin(), end());
        }

        template <typename Functor>
        int count(const Functor& predicate) const
        {
            return static_cast<int>(std::count_if(begin(), end(), predicate));
        }

        Self default_if_empty(const Type& default_value) const
        {
            if (empty()) {
                return from_values({ default_value });
            }

            return *this;
        }

        Self default_if_empty(void) const
        {
            return default_if_empty(Type());
        }

        Self distinct(void) const 
        {
            auto set = std::make_shared<std::set<Type>>();

            for (auto it = begin(); it != end(); ++it) {
                set->insert(*it);
            }

            return from(
                make_storage_iterator(set, set->begin()), 
                make_storage_iterator(set, set->end())
                );
        }

        bool empty(void) const
        {
            return begin() == end();
        }

        template <typename Iterator>
        Self except_with(const Iterator& right_begin, const Iterator& right_end) const
        {
            auto values = std::make_shared<std::vector<Type>>();
            std::set<Type> set(right_begin, right_end);

            for (auto it = begin(); it != end(); ++it) {
                if (set.insert(*it).second) {
                    values->push_back(*it);
                }
            }

            return from(
                make_storage_iterator(values, values->begin()),
                make_storage_iterator(values, values->end())
                );
        }

        template <typename Container>
        Self except_with(const Container& container) const
        {
            return except_with(std::begin(container), std::end(container));
        }

        Self except_with(const std::initializer_list<Type>& container) const
        {
            return except_with(std::begin(container), std::end(container));
        }

        Type element_at(int index)const
        {
            if (index >= 0) {
                int counter = 0;
                for (auto it = begin(); it != end(); it++) {
                    if (counter == index) {
                        return *it;
                    }
                    counter++;
                }
            }

            throw enumerable_exception("argument out of range");
        }

        enumerable_iterator<Type> find(const Type& value) const 
        {
            return std::find(begin(), end(), value);
        }
        
        Type first(void) const
        {
            if (empty()) {
                throw enumerable_exception("get a value from an empty collection");
            }

            return *begin();
        }
        
        Type first_or_default(const Type& value) const
        {
            return empty() ? value : *begin();
        }

        template <typename InnerIterator, 
                  typename OuterKeyFunctor, 
                  typename InnerKeyFunctor,
                  typename KeyType = typename functor_retriver<decltype(&OuterKeyFunctor::operator())>::type,
                  typename OuterValueType = Type,
                  typename InnerValueType = typename recover_type<typename std::iterator_traits<InnerIterator>::value_type>::type>
        enumerable<std::pair<KeyType, std::pair<enumerable<OuterValueType>, enumerable<InnerValueType>>>>
        full_join(const InnerIterator& right_begin,
            const InnerIterator& right_end,
            const OuterKeyFunctor& outer_key_selector,
            const InnerKeyFunctor& inner_key_selector) const
        {
            std::multimap<KeyType, OuterValueType> outer_map;
            std::multimap<KeyType, InnerValueType> inner_map;

            for (auto it = begin(); it != end(); ++it) {
                auto value = *it;
                auto key = outer_key_selector(value);

                outer_map.insert(std::make_pair(key, value));
            }

            for (auto it = right_begin; it != right_end; ++it) {
                auto value = *it;
                auto key = inner_key_selector(value);

                inner_map.insert(std::make_pair(key, value));
            }

            auto map = std::make_shared<std::map<KeyType, std::pair<enumerable<OuterValueType>, enumerable<InnerValueType>>>>();
            auto outer_lower = outer_map.begin();
            auto inner_lower = inner_map.begin();

            while (outer_lower != outer_map.end() && inner_lower != inner_map.end()) {
                auto outer_key = outer_lower->first;
                auto inner_key = inner_lower->first;
                auto outer_upper = outer_map.upper_bound(outer_key);
                auto inner_upper = inner_map.upper_bound(inner_key);

                if (outer_key < inner_key) {
                    std::vector<OuterValueType> outers;

                    for (auto it = outer_lower; it != outer_upper; ++it) {
                        outers.push_back(it->second);
                    }

                    map->insert(std::make_pair(outer_key, std::make_pair(from_values(outers), enumerable<InnerValueType>())));
                    outer_lower = outer_upper;

                } else if (outer_key > inner_key) {
                    std::vector<InnerValueType> inners;

                    for (auto it = inner_lower; it != inner_upper; ++it) {
                        inners.push_back(it->second);
                    }

                    map->insert(std::make_pair(outer_key, std::make_pair(enumerable<OuterValueType>(), from_values(inners))));
                    inner_lower = inner_upper;

                } else {
                    std::vector<OuterValueType> outers;
                    std::vector<InnerValueType> inners;

                    for (auto it = outer_lower; it != outer_upper; ++it) {
                        outers.push_back(it->second);
                    }

                    for (auto it = inner_lower; it != inner_upper; ++it) {
                        inners.push_back(it->second);
                    }

                    map->insert(std::make_pair(outer_key, std::make_pair(from_values(outers), from_values(inners))));
                    outer_lower = outer_upper;
                    inner_lower = inner_upper;
                }
            }

            return enumerable<std::pair<KeyType, std::pair<enumerable<OuterValueType>, enumerable<InnerValueType>>>>(
                make_storage_iterator(map, map->begin()),
                make_storage_iterator(map, map->end())
                );
        }

        template <typename Container, 
                  typename OuterKeyFunctor,
                  typename InnerKeyFunctor,
                  typename KeyType = typename functor_retriver<decltype(&OuterKeyFunctor::operator())>::type,
                  typename OuterValueType = Type>
        auto full_join(const Container& container,
                       const OuterKeyFunctor& outer_key_selector,
                       const InnerKeyFunctor& inner_key_selector) const ->
            enumerable<std::pair<KeyType, std::pair<enumerable<OuterValueType>, enumerable<typename recover_type<typename std::iterator_traits<decltype(std::begin(container))>::value_type>::type>>>>
        {
            return full_join(std::begin(container), std::end(container), outer_key_selector, inner_key_selector);
        }

        template <typename InnerValueType,
                  typename OuterKeyFunctor,
                  typename InnerKeyFunctor,
                  typename KeyType = typename functor_retriver<decltype(&OuterKeyFunctor::operator())>::type,
                  typename OuterValueType = Type>
        auto full_join(const std::initializer_list<InnerValueType>& container,
                       const OuterKeyFunctor& outer_key_selector,
                       const InnerKeyFunctor& inner_key_selector) const ->
            enumerable<std::pair<KeyType, std::pair<enumerable<OuterValueType>, enumerable<InnerValueType>>>>
        {
            return full_join(std::begin(container), std::end(container), outer_key_selector, inner_key_selector);
        }

        template <typename KeyFunctor, 
                  typename ValueFunctor, 
                  typename KeyType = typename functor_retriver<decltype(&KeyFunctor::operator())>::type, 
                  typename ValueType = typename functor_retriver<decltype(&ValueFunctor::operator())>::type>
        enumerable<std::pair<KeyType, enumerable<ValueType>>> 
            group_by(const KeyFunctor& key_selector, const ValueFunctor& value_selector) const
        {
            std::map<KeyType, std::shared_ptr<std::vector<ValueType>>> group;

            for (auto it = begin(); it != end(); ++it) {
                auto value = value_selector(*it);
                auto key = key_selector(*it);
                auto hit = group.find(key);

                if (hit == group.end()) {
                    auto values = std::make_shared<std::vector<ValueType>>();
                    values->push_back(value);
                    group.insert(std::make_pair(key, values));

                } else {
                    hit->second->push_back(value);
                }
            }

            auto result = std::make_shared<std::map<KeyType, enumerable<ValueType>>>();

            for (auto pair : group) {
                result->insert(std::make_pair(pair.first, from_values(pair.second)));
            }

            return enumerable<std::pair<KeyType, enumerable<ValueType>>>(
                make_storage_iterator(result, result->begin()),
                make_storage_iterator(result, result->end())
                );
        }

        template <typename Functor>
        enumerable<std::pair<typename functor_retriver<decltype(&Functor::operator())>::type, enumerable<Type>>> 
            group_by(const Functor& key_selector) const
        {
            return group_by(key_selector, [](const Type& value) { return value;});
        }

        template <typename InnerIterator,
                  typename OuterKeyFunctor, 
                  typename InnerKeyFunctor,
                  typename KeyType = typename functor_retriver<decltype(&OuterKeyFunctor::operator())>::type,
                  typename OuterValueType = Type,
                  typename InnerValueType = typename recover_type<typename std::iterator_traits<InnerIterator>::value_type>::type>
        enumerable<std::pair<KeyType, std::pair<OuterValueType, enumerable<InnerValueType>>>>
            group_join(const InnerIterator& inner_begin,
                       const InnerIterator& inner_end, 
                       const OuterKeyFunctor& outer_key_selector, 
                       const InnerKeyFunctor& inner_key_selector) const
        {
            auto table = full_join(inner_begin, inner_end, outer_key_selector, inner_key_selector);
            auto map = std::make_shared<std::multimap<KeyType, std::pair<OuterValueType, enumerable<InnerValueType>>>>();

            for (auto pair : table) {
                for (auto outer_value : pair.second.first) {
                    map->insert(std::make_pair(pair.first, std::make_pair(outer_value, from_values(pair.second.second))));
                }
            }
            
            return enumerable<std::pair<KeyType, std::pair<OuterValueType, enumerable<InnerValueType>>>>(
                make_storage_iterator(map, map->begin()),
                make_storage_iterator(map, map->end())
                );
        }

        template <typename Container,
            typename OuterKeyFunctor,
            typename InnerKeyFunctor,
            typename KeyType = typename functor_retriver<decltype(&OuterKeyFunctor::operator())>::type,
            typename OuterValueType = Type>
        auto group_join(const Container& container,
                        const OuterKeyFunctor& outer_key_selector,
                        const InnerKeyFunctor& inner_key_selector) const ->
            enumerable<std::pair<KeyType, std::pair<OuterValueType, enumerable<typename recover_type<decltype(*std::begin(container))>::type>>>>
        {
            return group_join(std::begin(container), std::end(container), outer_key_selector, inner_key_selector);
        }
     
        template <typename InnerValueType,
                  typename OuterKeyFunctor,
                  typename InnerKeyFunctor,
                  typename KeyType = typename functor_retriver<decltype(&OuterKeyFunctor::operator())>::type,
                  typename OuterValueType = Type>
        auto group_join(const std::initializer_list<InnerValueType>& container,
                        const OuterKeyFunctor& outer_key_selector,
                        const InnerKeyFunctor& inner_key_selector) const ->
            enumerable<std::pair<KeyType, std::pair<OuterValueType, enumerable<InnerValueType>>>>
        {
            return group_join(std::begin(container), std::end(container), outer_key_selector, inner_key_selector);
        }

        template <typename Iterator>
        Self intersect_with(const Iterator& right_begin, const Iterator& right_end) const
        {
            std::set<Type> left, right(right_begin, right_end);
            auto values = std::make_shared<std::vector<Type>>();
            
            for (auto it = begin(); it != end(); ++it) {
                if (left.insert(*it).second && !right.insert(*it).second) {
                    values->push_back(*it);
                }
            }

            return from_values(values);
        }

        template <typename Container>
        Self intersect_with(const Container& container) const
        {
            return intersect_with(std::begin(container), std::end(container));
        }

        Self intersect_with(const std::initializer_list<Type>& container) const
        {
            return intersect_with(std::begin(container), std::end(container));
        }

        template <typename InnerIterator,
                  typename OuterKeyFunctor,
                  typename InnerKeyFunctor,
                  typename KeyType = typename functor_retriver<decltype(&OuterKeyFunctor::operator())>::type,
                  typename OuterValueType = Type,
                  typename InnerValueType = typename recover_type<typename std::iterator_traits<InnerIterator>::value_type>::type>
        enumerable<std::pair<KeyType, std::pair<OuterValueType, InnerValueType>>>
            join(const InnerIterator& inner_begin,
                 const InnerIterator& inner_end,
                 const OuterKeyFunctor& outer_key_selector,
                 const InnerKeyFunctor& inner_key_selector) const
        {
            auto table = group_join(inner_begin, inner_end, outer_key_selector, inner_key_selector);
            auto map = std::make_shared<std::multimap<KeyType, std::pair<OuterValueType, InnerValueType>>>();

            for (auto pair : table) {
                for (auto value : pair.second.second) {
                    map->insert(std::make_pair(pair.first, std::make_pair(pair.second.first, value)));
                }
            }

            return enumerable<std::pair<KeyType, std::pair<OuterValueType, InnerValueType>>>(
                make_storage_iterator(map, map->begin()),
                make_storage_iterator(map, map->end())
                );
        }

        template <typename Container,
                  typename OuterKeyFunctor,
                  typename InnerKeyFunctor,
                  typename KeyType = typename functor_retriver<decltype(&OuterKeyFunctor::operator())>::type,
                  typename OuterValueType = Type>
        auto join(const Container& container,
                  const OuterKeyFunctor& outer_key_selector,
                  const InnerKeyFunctor& inner_key_selector) const -> 
            enumerable<std::pair<KeyType, std::pair<OuterValueType, typename recover_type<decltype(*std::begin(container))>::type>>>
        {
            return join(std::begin(container), std::end(container), outer_key_selector, inner_key_selector);
        }

        template <typename InnerValueType,
                  typename OuterKeyFunctor,
                  typename InnerKeyFunctor,
                  typename KeyType = typename functor_retriver<decltype(&OuterKeyFunctor::operator())>::type,
                  typename OuterValueType = Type>
        auto join(const std::initializer_list<InnerValueType>& container,
                  const OuterKeyFunctor& outer_key_selector,
                  const InnerKeyFunctor& inner_key_selector) const ->
            enumerable<std::pair<KeyType, std::pair<OuterValueType, InnerValueType>>>
        {
            return join(std::begin(container), std::end(container), outer_key_selector, inner_key_selector);
        }

        Type last(void) const 
        {
            if (empty()) {
                throw enumerable_exception("get a value from an empty collection");
            }

            auto it = begin();
            auto result = *it;

            while (++it != end()) {
                result = *it;
            }

            return result;
        }

        Type last_or_default(const Type& value) const
        {
            auto result = value;

            for (auto it = begin(); it != end(); ++it) {
                result = *it;
            }

            return result;
        }

        Type max(void) const 
        {
            if (empty()) {
                throw enumerable_exception("get a value from an empty collection");
            }
           
            return *std::max_element(begin(), end());
        }

        Type min(void) const 
        {
            if (empty()) {
                throw enumerable_exception("get a value from an empty collection");
            }
           
            return *std::min_element(begin(), end());
        }

        template <typename Functor>
        enumerable<Type> order_by(const Functor& selector) const 
        {
            auto values = std::make_shared<std::vector<Type>>();
            
            for (auto it = begin(); it != end(); ++it) {
                values->push_back(*it);
                std::push_heap(values->begin(), values->end(), [&selector](const Type& lhs, const Type& rhs){return selector(lhs) < selector(rhs);});
            }

            std::sort_heap(values->begin(), values->end(), [&selector](const Type& lhs, const Type& rhs){return selector(lhs) < selector(rhs);});

            return enumerable<Type>(
                make_storage_iterator(values, values->begin()),
                make_storage_iterator(values, values->end())
                );
        }

        template <typename Functor>
        Self order_by_descending(const Functor& selector) const
        {
            auto values = std::make_shared<std::vector<Type>>();

            for (auto it = begin(); it != end(); ++it) {
                values->push_back(*it);
                std::push_heap(values->begin(), values->end(), [&selector](const Type& lhs, const Type& rhs){return selector(lhs) > selector(rhs); });
            }

            std::sort_heap(values->begin(), values->end(), [&selector](const Type& lhs, const Type& rhs){return selector(lhs) > selector(rhs); });

            return from (
                make_storage_iterator(values, values->begin()),
                make_storage_iterator(values, values->end())
                );
        }

        Self reverse(void) const 
        {
            auto values = std::make_shared<std::list<Type>>();

            for (auto it = begin(); it != end(); ++it) {
                values->push_front(*it);
            }

            return from(
                make_storage_iterator(values, values->begin()),
                make_storage_iterator(values, values->end())
                );
        }

        template <typename Functor, typename Result = enumerable<typename functor_retriver<decltype(&Functor::operator())>::type>>
        Result select(const Functor& selector) const
        {
            return Result (
                make_select_iterator(begin(), selector),
                make_select_iterator(end(), selector)
                );
        }

        template <typename Functor>
        auto select_many(const Functor& selector) const->enumerable<decltype(*((typename functor_retriver<decltype(&Functor::operator())>::type*)0)->begin())>
        {
            typedef decltype(*((typename functor_retriver<decltype(&Functor::operator())>::type*)0)->begin()) ValueType;
            typedef typename functor_retriver<decltype(&Functor::operator())>::type Enumerable;

            return select(selector).aggregate(
                [](const enumerable<ValueType>& left, const Enumerable& right) {
                return left.concat(right);
            });
        }

        template <typename Iterator>
        bool sequence_equal(const Iterator& right_begin, const Iterator& right_end) const
        {
            auto lbegin = begin();
            auto lend = end();
            Iterator rbegin = right_begin;
            Iterator rend = right_end;

            while (lbegin != lend && rbegin != rend) {
                if (*lbegin++ != *rbegin++) {
                    return false;
                }
            }

            return lbegin == lend && rbegin == rend;
        }

        template <typename Container>
        bool sequence_equal(const Container& container) const
        {
            return sequence_equal(std::begin(container), std::end(container));
        }

        bool sequence_equal(const std::initializer_list<Type>& container) const 
        {
            return sequence_equal(std::begin(container), std::end(container));
        }

        Self single(void) const 
        {
            auto it = begin();
            if (it == end()) {
                throw enumerable_exception("get a value from an empty collection");
            }

            if (++it != end()) {
                throw enumerable_exception("the collection does not contain exactly one element");
            }

            return *this;
        }

        Self single_or_default(const Type& value) const 
        {
            auto it = begin();
            if (it == end()) {
                return from_values({value});
            }

            if (++it != end()) {
                throw enumerable_exception("the collection does not contain exactly one element");
            }

            return *this;
        }

        Self skip(int count) const 
        {
            return from (
                make_skip_iterator(begin(), end(), count),
                make_skip_iterator(end(), end(), count)
                );
        }

        template <typename Functor>
        Self skip_while(const Functor& predicate) const 
        {
            return from (
                make_skip_while_iterator(begin(), end(), predicate),
                make_skip_while_iterator(end(), end(), predicate)
                );
        }

        Type sum(void) const 
        {
            return aggregate([](const Type& lhs, const Type& rhs) {return lhs + rhs;});
        }

        Self take(int count) const 
        {
            return from(
                make_take_iterator(begin(), end(), count),
                make_take_iterator(end(), end(), count)
                );
        }

        template <typename Functor>
        Self take_while(const Functor& predicate) const 
        {
            return from(
                make_take_while_iterator(begin(), end(), predicate),
                make_take_while_iterator(end(), end(), predicate)
                );
        }

        std::vector<Type> to_vector(void) const 
        {
            std::vector<Type> values; 

            for (auto it = begin(); it != end(); ++it) {
                values.push_back(*it);
            }

            return std::move(values);
        }

        std::list<Type> to_list(void) const 
        {
            std::list<Type> values;

            for (auto it = begin(); it != end(); ++it) {
                values.push_back(*it);
            }

            return std::move(values);
        }

        std::deque<Type> to_deque(void) const 
        {
            std::deque<Type> values;

            for (auto it = begin(); it != end(); ++it) {
                values.push_back(*it);
            }

            return std::move(values);
        }

        template <typename Functor> 
        std::map<typename functor_retriver<decltype(&Functor::operator())>::type, Type> to_map(const Functor& selector) const
        {
            std::map<typename functor_retriver<decltype(&Functor::operator())>::type, Type> values;

            for (auto it = begin(); it != end(); ++it) {
                auto value = *it;
                values.insert(std::make_pair(selector(value), value));
            }

            return std::move(values);
        }

        template <typename Functor>
        std::multimap<typename functor_retriver<decltype(&Functor::operator())>::type, Type> to_multimap(const Functor& selector) const
        {
            std::multimap<typename functor_retriver<decltype(&Functor::operator())>::type, Type> values;

            for (auto it = begin(); it != end(); ++it) {
                auto value = *it;
                values.insert(std::make_pair(selector(value), value));
            }

            return std::move(values);
        }

        template <typename Functor>
        std::unordered_map<typename functor_retriver<decltype(&Functor::operator())>::type, Type> to_unordered_map(const Functor& selector) const
        {
            std::unordered_map<typename functor_retriver<decltype(&Functor::operator())>::type, Type> values;

            for (auto it = begin(); it != end(); ++it) {
                auto value = *it;
                values.insert(std::make_pair(selector(value), value));
            }

            return std::move(values);
        }

        std::set<Type> to_set() const
        {
            std::set<Type> values;

            for (auto it = begin(); it != end(); ++it) {
                values.insert(*it);
            }

            return std::move(values);
        }

        std::multiset<Type> to_multiset() const
        {
            std::multiset<Type> values;

            for (auto it = begin(); it != end(); ++it) {
                values.insert(*it);
            }

            return std::move(values);
        }

        std::unordered_set<Type> to_unordered_set() const
        {
            std::unordered_set<Type> values;

            for (auto it = begin(); it != end(); ++it) {
                values.insert(*it);
            }

            return std::move(values);
        }

        template <typename Iterator>
        Self union_with(const Iterator& right_begin, const Iterator& right_end) const
        {
            return concat(right_begin, right_end).distinct();
        }

        template <typename Container>
        Self union_with(const Container& container) const
        {
            return union_with(std::begin(container), std::end(container));
        }

        Self union_with(const std::initializer_list<Type>& container) const
        {
            return union_with(std::begin(container), std::end(container));
        }

        template <typename Functor>
        Self where(const Functor& predicate) const 
        {
            return from (
                make_where_iterator(begin(), end(), predicate),
                make_where_iterator(end(), end(), predicate)
                );
        }

        template <typename RightIterator, 
                  typename LeftType = Type,
                  typename RightType = typename recover_type<typename std::iterator_traits<RightIterator>::value_type>::type>
        enumerable<std::pair<LeftType, RightType>> zip(const RightIterator& right_begin, const RightIterator& right_end) const
        {
            return from (
                make_zip_iterator(begin(), end(), right_begin, right_end),
                make_zip_iterator(end(), end(), right_end, right_end)
                );
        }

        template <typename Container>
        auto zip(const Container& container) const -> 
            enumerable<std::pair<Type, typename recover_type<decltype(*std::begin(container))>::type>>
        {
            return zip(std::begin(container), std::end(container));
        }

        template <typename RightType>
        enumerable<std::pair<Type, RightType>> zip(const std::initializer_list<RightType>& container) const
        {
            return zip(std::begin(container), std::end(container));
        }

        template <typename RightIterator,
                  typename Functor,
                  typename LeftType = typename functor_retriver<decltype(&Functor::operator())>::type,
                  typename RightType = typename recover_type<typename std::iterator_traits<RightIterator>::value_type>::type>
        enumerable<std::pair<LeftType, RightType>> zip(const RightIterator& right_begin, const RightIterator& right_end, const Functor& selector) const
        {
            return enumerable<std::pair<LeftType, RightType>> (
                make_zip_with_iterator(begin(), end(), right_begin, right_end, selector),
                make_zip_with_iterator(end(), end(), right_end, right_end, selector)
                );
        }

        template <typename Container, typename Functor>
        auto zip(const Container& container, const Functor& selector) ->
            enumerable<std::pair<typename functor_retriver<decltype(&Functor::operator())>::type, typename recover_type<decltype(*std::begin(container))>::type>>
        {
            return zip(std::begin(container), std::end(container), selector);
        }

        template <typename RightType, typename Functor>
        enumerable<std::pair<typename functor_retriver<decltype(&Functor::operator())>::type, RightType>> 
            zip(const std::initializer_list<RightType>& container, const Functor& selector) const
        {
            return zip(std::begin(container), std::end(container), selector);
        }
    };

    template <>
    class enumerable<void> {
    };
};

#endif
