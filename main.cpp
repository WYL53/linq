

#include "enumerable.h"

#include <iostream>
#include <iterator>
#include <vector>
#include <functional>
#include <list>
#include <sstream> 
#include <algorithm>
#include <map>
#include <cassert>
#include <random>

void sample(void);

struct student_t
{
    std::string last_name;
    std::vector<int> scores;
};

int main(void)
{
    std::vector<student_t> students =
    {
        { "Omelchenko", { 97, 72, 81, 60 } },
        { "O'Donnell", { 75, 84, 91, 39 } },
        { "Mortensen", { 88, 94, 65, 85 } },
        { "Garcia", { 97, 99, 85, 98 } },
        { "Beebe", { 35, 72, 91, 70 } }
    };

    auto scores = sb::from(students).
        where([](const student_t& student) {
           return std::accumulate(student.scores.begin(), student.scores.end(), 0) / 4 > 90;
        }).select([](const student_t& student){
            return std::make_pair(student.last_name, std::accumulate(student.scores.begin(), student.scores.end(), 0) / 4); 
        });

    for (auto x : scores) {
        printf("%s score: %i\n", x.first.c_str(), x.second);
    }

    return 0;
}

void sample(void)
{
    {
        // test from_values 
        auto v = std::make_shared<std::vector<int>>();
        for (auto i = 0; i < 10; ++i) {
            v->push_back(i);
        }

        std::cout << "test from_values(container):" << std::endl;
        auto linq = sb::from_values(v);
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        std::cout << "test from_values(iterator, iterator):" << std::endl;
        linq = sb::from_values(v->begin(), v->end());
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        std::cout << "test from_values(initializer_list):" << std::endl;
        linq = sb::from_values({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test from 
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        std::cout << "test from(container):" << std::endl;
        std::copy(sb::from(v).begin(), sb::from(v).end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        std::cout << "test from(iterator, iterator):" << std::endl;
        std::copy(sb::from(v.begin(), v.end()).begin(), sb::from(v.begin(), v.end()).end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        std::cout << "test from(initializer_list):" << std::endl;
        for (auto value : sb::from({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 })) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }

    {
        // test aggregate
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test aggregate(reducer):" << std::endl;
        std::cout << sb::from(v).aggregate([](int x, int y){return x + y; }) << std::endl;

        std::cout << "test aggregate(seed, reducer):" << std::endl;
        std::cout << sb::from(v).aggregate(5, [](int x, int y){return x + y; }) << std::endl;

        std::cout << "test aggregate(seed, reducer, selector):" << std::endl;
        std::cout << sb::from(v).aggregate(5, [](int x, int y){return x + y; }, [](int x){return std::string(x, 'H'); }) << std::endl;
    }

    {
        // test all, any
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        std::cout << "test all(predicate):" << std::endl;
        std::cout << std::boolalpha << sb::from(v).all([](int x) {return x < 10; }) << std::endl;
        std::cout << std::boolalpha << sb::from(v).all([](int x) {return x < 5; }) << std::endl;

        std::cout << "test any(predicate):" << std::endl;
        std::cout << std::boolalpha << sb::from(v).any([](int x) {return x == 1; }) << std::endl;
        std::cout << std::boolalpha << sb::from(v).any([](int x) {return x == 10; }) << std::endl;
    }

    {
        // test average
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test average():" << std::endl;
        std::cout << sb::from(v).average<int>() << std::endl;
        std::cout << sb::from(v).average<double>() << std::endl;
    }

    {
        //test concat
        std::vector<int> v1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::vector<int> v2 = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
        std::list<int> v3 = { 11, 12, 13 };

        auto linq = sb::from(v1).concat(v2);
        std::cout << "test concat(continer):" << std::endl;
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        std::cout << "test concat(iterator, iterator):" << std::endl;
        linq = linq.concat(v3.begin(), v3.end());
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        std::cout << "test concat(initializer_list):" << std::endl;
        linq = linq.concat({ 14, 15, 16 });
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test default_if_empty
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        auto linq = sb::from(v).default_if_empty();
        std::cout << "test default_if_empty():" << std::endl;
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        linq = sb::from(std::vector<int>()).default_if_empty();
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        linq = sb::from(v).default_if_empty();
        std::cout << "test default_if_empty(default_value):" << std::endl;
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        linq = sb::from(std::vector<int>()).default_if_empty(1024);
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test distinct
        std::vector<int> v = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9 };
        auto linq = sb::from(v).distinct();
        std::cout << "test distinct():" << std::endl;
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test except
        std::vector<int> v1 = { 1, 2, 3, 4, 5, 6, 7, 8 };
        std::vector<int> v2 = { 5, 7, 9, 10 };

        auto linq = sb::from(v1).except_with(v2);
        std::cout << "test except(container):" << std::endl;
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        linq = sb::from(v1).except_with(v2.begin(), v2.end());
        std::cout << "test except(iterator, iterator):" << std::endl;
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        linq = sb::from(v1).except_with({ 5, 7, 9, 10 });
        std::cout << "test except(initializer_list):" << std::endl;
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test find
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        auto it = sb::from(v).find(4);
        std::cout << "test find(value):" << std::endl;
        std::cout << *it << std::endl;
    }

    {
        // test first
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test first()" << std::endl;
        std::cout << sb::from(v).first() << std::endl;
    }

    {
        // test first_or_default
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::vector<int> empty;

        std::cout << "test first_or_default(value):" << std::endl;
        std::cout << sb::from(v).first_or_default(1024) << std::endl;
        std::cout << sb::from(empty).first_or_default(1024) << std::endl;
    }

    {
        // test full_join
        std::vector<int> v1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::vector<int> v2 = { 9, 8, 7, 6, 5, 4, 3, 2, 1 };

        std::cout << "test full_join(outer_iterator, outer_iterator, outer_key_selector, inner_key_selector):" << std::endl;
        auto linq = sb::from(v1).full_join(v2.begin(), v2.end(), [](int x) {return x % 2; }, [](int x) {return x % 2; });

        for (auto pair : linq) {
            std::cout << "key: " << pair.first << std::endl;
            std::cout << "left join: "; std::copy(pair.second.first.begin(), pair.second.first.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
            std::cout << "right join: "; std::copy(pair.second.second.begin(), pair.second.second.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
        }

        std::cout << "test full_join(container, outer_key_selector, inner_key_selector):" << std::endl;
        linq = sb::from(v1).full_join(v2, [](int x) {return x % 2; }, [](int x) {return x % 2; });

        for (auto pair : linq) {
            std::cout << "key: " << pair.first << std::endl;
            std::cout << "left join: "; std::copy(pair.second.first.begin(), pair.second.first.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
            std::cout << "right join: "; std::copy(pair.second.second.begin(), pair.second.second.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
        }

        std::cout << "test full_join(initializer_list, outer_key_selector, inner_key_selector):" << std::endl;
        linq = sb::from(v1).full_join({ 9, 8, 7, 6, 5, 4, 3, 2, 1 }, [](int x) {return x % 2; }, [](int x) {return x % 2; });

        for (auto pair : linq) {
            std::cout << "key: " << pair.first << std::endl;
            std::cout << "left join: "; std::copy(pair.second.first.begin(), pair.second.first.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
            std::cout << "right join: "; std::copy(pair.second.second.begin(), pair.second.second.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
        }
    }

    {
        // test group_by(key_selector)
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test group_by(key_selector):" << std::endl;
        for (auto pair : sb::from(v).group_by([](int x) {return x % 2; })) {
            std::cout << "key: " << pair.first << " ";
            std::cout << "value: ";
            std::copy(pair.second.begin(), pair.second.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
        }
      
        std::cout << "test group_by(key_selector, value_selector):" << std::endl;
        for (auto pair : sb::from(v).group_by([](int x) {return x % 2; }, [](int x){return x * x;})) {
            std::cout << "key: " << pair.first << " ";
            std::cout << "value: ";
            std::copy(pair.second.begin(), pair.second.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
        }
    }

    {
        std::vector<int> v1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::vector<int> v2 = { 9, 8, 7, 6, 5, 4, 3, 2, 1 };

        std::cout << "test group_join(outer_iterator, outer_iterator, outer_key_selector, inner_key_selector):" << std::endl;
        auto linq = sb::from(v1).group_join(v2.begin(), v2.end(), [](int x){return x % 2; }, [](int x){return x % 2; });
        for (auto pair : linq) {
            std::cout << "key: " << pair.first << std::endl;
            std::cout << "outer_value: " << pair.second.first << std::endl;
            std::cout << "inner_values: "; std::copy(pair.second.second.begin(), pair.second.second.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
        }

        std::cout << "test group_join(container, outer_key_selector, inner_key_selector):" << std::endl;
        linq = sb::from(v1).group_join(v2, [](int x){return x % 2; }, [](int x){return x % 2; });
        for (auto pair : linq) {
            std::cout << "key: " << pair.first << std::endl;
            std::cout << "outer_value: " << pair.second.first << std::endl;
            std::cout << "inner_values: "; std::copy(pair.second.second.begin(), pair.second.second.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
        }

        std::cout << "test group_join(initializer_list, outer_key_selector, inner_key_selector):" << std::endl;
        linq = sb::from(v1).group_join({ 9, 8, 7, 6, 5, 4, 3, 2, 1 }, [](int x){return x % 2; }, [](int x){return x % 2; });
        for (auto pair : linq) {
            std::cout << "key: " << pair.first << std::endl;
            std::cout << "outer_value: " << pair.second.first << std::endl;
            std::cout << "inner_values: "; std::copy(pair.second.second.begin(), pair.second.second.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
        }
    }

    {
        // test intersect_with
        std::vector<int> v1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::vector<int> v2 = { 9, 7, 5, 3, 1 };

        auto linq = sb::from(v1).intersect_with(v2);
        std::cout << "test intersect_with(container):" << std::endl;
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        linq = sb::from(v1).intersect_with(v2.begin(), v2.end());
        std::cout << "test intersect_with(iterator, iterator):" << std::endl;
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        linq = sb::from(v1).intersect_with({ 9, 7, 5, 3, 1 });
        std::cout << "test intersect_with(iterator, iterator):" << std::endl;
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test join
        std::vector<int> v1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::vector<int> v2 = { 9, 7, 5, 3, 1 };

        std::cout << "test join(container, outer_key_selector, inner_key_selector):" << std::endl;
        auto linq = sb::from(v1).join(v2, [](int x){return x % 2; }, [](int x){return x % 2; });
        for (auto pair : linq) {
            std::cout << "key: " << pair.first << std::endl;
            std::cout << "outer join: " << pair.second.first << std::endl;
            std::cout << "inner join: " << pair.second.second << std::endl;
        }

        std::cout << "test join(inner_iterator, inner_iterator, outer_key_selector, inner_key_selector):" << std::endl;
        linq = sb::from(v1).join(v2.begin(), v2.end(), [](int x){return x % 2; }, [](int x){return x % 2; });
        for (auto pair : linq) {
            std::cout << "key: " << pair.first << std::endl;
            std::cout << "outer join: " << pair.second.first << std::endl;
            std::cout << "inner join: " << pair.second.second << std::endl;
        }

        std::cout << "test join(initializer_list, outer_key_selector, inner_key_selector):" << std::endl;
        linq = sb::from(v1).join({ 9, 7, 5, 3, 1 }, [](int x){return x % 2; }, [](int x){return x % 2; });
        for (auto pair : linq) {
            std::cout << "key: " << pair.first << std::endl;
            std::cout << "outer join: " << pair.second.first << std::endl;
            std::cout << "inner join: " << pair.second.second << std::endl;
        }
    }

    {
        // test last
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test last():" << std::endl;
        std::cout << sb::from(v).last() << std::endl;
    }

    {
        // test last_or_default
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test last_or_default(default_value):" << std::endl;
        std::cout << sb::from(v).last_or_default(1024) << std::endl;
        std::cout << sb::from(std::vector<int>()).last_or_default(1024) << std::endl;
    }

    {
        // test max
        std::vector<int> v = { 7, 3, 6, 8, 0, 9, 7, 4, 5 };
        std::cout << "test max():" << std::endl;
        std::cout << sb::from(v).max() << std::endl;
    }

    {
        // test min
        std::vector<int> v = { 7, 3, 6, 8, 0, 9, 7, 4, 5 };
        std::cout << "test min():" << std::endl;
        std::cout << sb::from(v).min() << std::endl;
    }

    {
        // test order
        std::vector<int> v = { 7, 3, 6, 8, 0, 9, 7, 4, 5 };

        std::cout << "test order(selector):" << std::endl;
        auto linq = sb::from(v).order_by([](int x){ return x; });
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;


    }

    {
        // test order_by_descending
        std::vector<int> v = { 7, 3, 6, 8, 0, 9, 7, 4, 5 };

        std::cout << "test order_by_descending(selector):" << std::endl;
        auto linq = sb::from(v).order_by_descending([](int x){ return x; });
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test reverse 
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        std::cout << "test reverse():" << std::endl;
        auto linq = sb::from(v).reverse();
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test select
        std::vector<int> v = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

        std::cout << "test select(selector):" << std::endl;
        auto linq = sb::from(v).select([](int x) { return std::string(x, 'H'); });
        for (auto v : linq) {
            std::cout << v << std::endl;
        }
    }

    {
        // test select_many
        std::vector<int> v = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

        std::cout << "test select_many(selector):" << std::endl;
        auto linq = sb::from(v).select_many([](int x) { return sb::from_values({ std::string(x, 'H'), std::string(x + 1, 'H'), std::string(x + 2, 'H') }); });
        for (auto xs : linq) {
            std::copy(xs.begin(), xs.end(), std::ostream_iterator<char>(std::cout));
            std::cout << std::endl;
        }
    }

    {
        // test sequence_equal
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        std::cout << "test sequence_equal(container):" << std::endl;
        std::cout << std::boolalpha << sb::from(v).sequence_equal(v) << std::endl;

        std::cout << "test sequence_equal(iterator, iterator):" << std::endl;
        std::cout << std::boolalpha << sb::from(v).sequence_equal(v.begin(), v.end()) << std::endl;

        std::cout << "test sequence_equal(initializer_list):" << std::endl;
        std::cout << std::boolalpha << sb::from(v).sequence_equal({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }) << std::endl;

        std::cout << "test sequence_equal(initializer_list):" << std::endl;
        std::cout << std::boolalpha << sb::from(v).sequence_equal({ 0, 1, 2, 3, 5, 6, 7, 8, 9 }) << std::endl;
    }

    {
        // test single
        std::vector<int> v = { 1 };

        std::cout << "test single():" << std::endl;
        auto linq = sb::from(v).single();
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        //test single_or_default

        std::vector<int> v = { 1 };

        std::cout << "test single_or_default(default_value):" << std::endl;
        auto linq = sb::from(v).single_or_default(1024);
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
        linq = sb::from(std::vector<int>()).single_or_default(1024);
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test skip
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test skip():" << std::endl;
        auto linq = sb::from(v).skip(3);
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        //test skip_while
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test skip_while(predicate):" << std::endl;
        auto linq = sb::from(v).skip_while([](int x) {return x < 5; });
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test sum
        std::vector<int> v = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        std::cout << "test sum():" << std::endl;
        std::cout << sb::from(v).sum() << std::endl;
    }

    {
        // test take
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test take(count):" << std::endl;
        auto linq = sb::from(v).take(3);
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test take_while
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test take_while(predicate):" << std::endl;
        auto linq = sb::from(v).take_while([](int x) {return x < 5; });
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test to_deque
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test to_deque():" << std::endl;
        auto results = sb::from(v).to_deque();
        std::copy(results.begin(), results.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test to_list
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test to_list():" << std::endl;
        auto results = sb::from(v).to_list();
        std::copy(results.begin(), results.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test to_map
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test to_map(selector):" << std::endl;
        auto results = sb::from(v).to_map([](int x) { return x*x; });
        for (auto v : results) {
            std::cout << "key: " << v.first << " values: " << v.second << std::endl;
        }
    }

    {
        // test to_multimap
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test to_multimap(selector):" << std::endl;
        auto results = sb::from(v).to_multimap([](int x) { return x / 2; });
        for (auto v : results) {
            std::cout << "key: " << v.first << " values: " << v.second << std::endl;
        }
    }

    {
        // test to_unordered_map
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test to_unordered_map(selector):" << std::endl;
        auto results = sb::from(v).to_unordered_map([](int x) { return x * 2; });
        for (auto v : results) {
            std::cout << "key: " << v.first << " values: " << v.second << std::endl;
        }
    }

    {
        // test to_set
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test to_set():" << std::endl;
        auto results = sb::from(v).to_set();
        std::copy(results.begin(), results.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test to_multiset
        std::vector<int> v = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9 };
        std::cout << "test to_multiset():" << std::endl;
        auto results = sb::from(v).to_multiset();
        std::copy(results.begin(), results.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test to_unordered_set
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test to_unordered_set():" << std::endl;
        auto results = sb::from(v).to_unordered_set();
        std::copy(results.begin(), results.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test to_vector
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test to_vector():" << std::endl;
        auto results = sb::from(v).to_vector();
        std::copy(results.begin(), results.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test union_with
        std::vector<int> v1 = { 0, 1, 2, 3, 4, 5, 6, 7 };
        std::vector<int> v2 = { 9, 8, 7, 6, 5, 4, 3 };

        std::cout << "test union_with(iterator, iterator):" << std::endl;
        auto linq = sb::from(v1).union_with(v2.begin(), v2.end());
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        std::cout << "test union_with(container):" << std::endl;
        linq = sb::from(v1).union_with(v2);
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        std::cout << "test union_with(initializer_list):" << std::endl;
        linq = sb::from(v1).union_with({ 9, 8, 7, 6, 5, 4, 3 });
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test where
        std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::cout << "test where(predicate):" << std::endl;
        auto linq = sb::from(v).where([](int x){return x % 2 == 0; });
        std::copy(linq.begin(), linq.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    {
        // test zip
        std::vector<int> v1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::vector<int> v2 = { 9, 8, 7, 6, 5, 4, 3, 2, 1 };

        std::cout << "test zip(container):" << std::endl;
        auto linq = sb::from(v1).zip(v2);
        for (auto pair : linq) {
            std::cout << "key: " << pair.first << " value: " << pair.second << std::endl;
        }

        std::cout << "test zip(iterator, iterator):" << std::endl;
        linq = sb::from(v1).zip(v2.begin(), v2.end());
        for (auto pair : linq) {
            std::cout << "key: " << pair.first << " value: " << pair.second << std::endl;
        }

        std::cout << "test zip(initializer_list):" << std::endl;
        linq = sb::from(v1).zip({ 9, 8, 7, 6, 5, 4, 3, 2, 1 });
        for (auto pair : linq) {
            std::cout << "key: " << pair.first << " value: " << pair.second << std::endl;
        }

        std::cout << "test zip(container, selector):" << std::endl;
        auto zip_linq = sb::from(v1).zip(v2, [](int x, int y){return std::string(x, 'H'); });
        for (auto pair : zip_linq) {
            std::cout << "key: " << pair.first << " value: " << pair.second << std::endl;
        }

        std::cout << "test zip(iterator, iterator, selector):" << std::endl;
        zip_linq = sb::from(v1).zip(v2.begin(), v2.end(), [](int x, int y){return std::string(x, 'H'); });
        for (auto pair : zip_linq) {
            std::cout << "key: " << pair.first << " value: " << pair.second << std::endl;
        }

        std::cout << "test zip(initializer_list, selector):" << std::endl;
        zip_linq = sb::from(v1).zip({ 9, 8, 7, 6, 5, 4, 3, 2, 1 }, [](int x, int y){return std::string(x, 'H'); });
        for (auto pair : zip_linq) {
            std::cout << "key: " << pair.first << " value: " << pair.second << std::endl;
        }
    }
}
