//Here is main
#include "test_runner.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <deque>
#include <memory>
#include <set>
#include <utility>

using namespace std;

template<typename T>
class PriorityCollection {
public:
	using Id = size_t;

	Id Add(T object) {
		size_t pos = values.size();
		values.push_back({ move(object), 0 });
		priorities.insert({ 0, pos });
		valid.push_back(true);
		return pos;
	}

	template<typename ObjInputIt, typename IdOutputIt>
	void Add(ObjInputIt range_begin, ObjInputIt range_end,
		IdOutputIt ids_begin) {
		for (auto it = range_begin; it != range_end; ++it, ++ids_begin) {
			*ids_begin = Add(move(*it));
		}
	}

	bool IsValid(Id id) const {
		if (id < 0 || id >= values.size()) {
			return false;
		}
		return valid[id];
	}

	const T& Get(Id id) const {
		return values[id].first;
	}

	void Promote(Id id) {
		priorities.erase({ values[id].second, id });
		values[id].second++;
		priorities.insert({ values[id].second, id });
	}

	pair<const T&, int> GetMax() const {
		auto max_element = *priorities.rbegin();
		return { Get(max_element.second), max_element.first };
	}

	pair<T, int> PopMax() {
		auto max_element = *priorities.rbegin();
		Id id = max_element.second;
		valid[id] = false;
		int rate = max_element.first;

		priorities.erase(max_element);

		return { move(values[id].first), rate };
	}

private:
	set<pair<int, Id>> priorities;
	vector<pair<T, int>> values;
	vector<bool> valid;
};

class StringNonCopyable : public string {
public:
	using string::string;  // Позволяет использовать конструкторы строки
	StringNonCopyable(const StringNonCopyable&) = delete;
	StringNonCopyable(StringNonCopyable&&) = default;
	StringNonCopyable& operator=(const StringNonCopyable&) = delete;
	StringNonCopyable& operator=(StringNonCopyable&&) = default;
};

void TestNoCopy() {
	PriorityCollection<StringNonCopyable> strings;
	const auto white_id = strings.Add("white");
	const auto yellow_id = strings.Add("yellow");
	const auto red_id = strings.Add("red");

	strings.Promote(yellow_id);
	for (int i = 0; i < 2; ++i) {
		strings.Promote(red_id);
	}
	strings.Promote(yellow_id);
	{
		const auto item = strings.PopMax();
		ASSERT_EQUAL(item.first, "red");
		ASSERT_EQUAL(item.second, 2);
	}
	{
		const auto item = strings.PopMax();
		ASSERT_EQUAL(item.first, "yellow");
		ASSERT_EQUAL(item.second, 2);
	}
	{
		const auto item = strings.PopMax();
		ASSERT_EQUAL(item.first, "white");
		ASSERT_EQUAL(item.second, 0);
	}
}

void TheBestTest() {
	PriorityCollection<StringNonCopyable> contain;

	vector<StringNonCopyable> words;
	words.push_back("red");
	words.push_back("blue");
	words.push_back("green");
	words.push_back("yellow");
	words.push_back("white");

	vector<PriorityCollection<StringNonCopyable>::Id> check(words.size());

	contain.Add(words.begin(), words.end(), check.begin());

	ASSERT_EQUAL(contain.Get(check[0]), "red");
	ASSERT_EQUAL(contain.Get(check[4]), "white");

	PriorityCollection<StringNonCopyable> spy;
	auto bomb = spy.Add({ "joke" });

	//ASSERT(!contain.IsValid(bomb));
	ASSERT(contain.IsValid(check[0]));
	contain.Promote(check[0]);				//red++
	contain.PopMax();                     //del red
	ASSERT(!contain.IsValid(check[0]));

	contain.Promote(check[4]);				//white++
	auto check_max = contain.GetMax();
	ASSERT_EQUAL(check_max.first, StringNonCopyable({ "white" }));
	auto check_max_copy = contain.GetMax();
	ASSERT_EQUAL(check_max.first, check_max_copy.first);

	ASSERT_EQUAL(check_max.first, "white");
	auto item = contain.PopMax();			//del white
	ASSERT_EQUAL(item.first, StringNonCopyable({ "white" }));
	ASSERT_EQUAL(item.second, 1);
	ASSERT(!contain.IsValid(check[4]));

	auto new_item = contain.Add({ "new" });
	contain.Promote(check[3]);	// yellow++
	contain.Promote(new_item);
	contain.Promote(check[2]);	// green++
	auto x = contain.GetMax();
	ASSERT_EQUAL(x.second, 1);
	ASSERT_EQUAL(x.first, "new");
}


int main()
{
	TestRunner tr;
	RUN_TEST(tr, TestNoCopy);
	RUN_TEST(tr, TheBestTest);
}
