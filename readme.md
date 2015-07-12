##Linq

Linq for list comprehension in `C++`, provides an implementation of linq for `C++`. Here's an example:

```
struct student_t
{
	std::string last_name;
	std::vector<int> scores;
};

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
```

The code above will outputs this:

```
Garcia score: 94
```

The methods supported are:

to linq expression

*   from(range)
*   from_random()
*   from_random(selector)
*   from_values(range)

linq methods

*   aggregate(reducer)
*   aggregate(seed, reducer)
*   aggregate(seed, reducer, selector)
*   all(predicate)
*   any(predicate)
*   average()
*   begin()
*   concat(range)
*   contains(element)
*   count()
*   count(predicat)
*   default_if_empty()
*   default_if_empty(default_value)
*   distinct()
*   element_at(index)
*   empty()
*   end()
*   except_with(range)
*   find(element)
*   first()
*   first_or_default(value)
*   full_join(range, outer_key_selector, inner_key_selector)
*   group_by(key_selector)
*   group_by(key_selector, element_selector)
*   group_join(range, outer_key_selector, inner_key_selector, result_selector)
*   intersect_with(range)
*   join(range, outer_key_selector, inner_key_selector, result_selector)
*   last()
*   last_or_default(value)
*   max()
*   min()
*   order_by(selector)
*   order_by_descending(selector)
*   reverse()
*   select(selector)
*   select_many(selector)
*   sequence_equal(range)
*   single()
*   single_or_default()
*   skip(count)
*   skip_while(predicate)
*   sum()
*   take(count)
*   take_while(predicate)
*   to_deque()
*   to_list()
*   to_map(selector)
*   to_multimap(selector)
*   to_multiset()
*   to_set()
*   to_unordered_map(selector)
*   to_unordered_set()
*   to_vector()
*   union(range)
*   where(predicate)
*   zip(range)
*   zip(range, selector)

## Build

####g++ 4.8.4
```
g++ -std=c++11 -o enumerable main.cpp
```

####clang++ 3.5
```
clang++-3.5 -std=c++11 -o enumerable main.cpp
```

####msvc 2013
see msvc folder
