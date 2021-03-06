#include "allocator_with_count.hpp"
#include "catch2/catch.hpp"
#include "immutable_string/string.hpp"

#include <cstring>
#include <type_traits>

using namespace immutable_string;

using string_count_alloc =
    basic_string<char, std::char_traits<char>, allocator_with_count<char>>;

static_assert(std::is_nothrow_copy_constructible<string>::value,
              "string shall be nothrow copy-constructible");
static_assert(std::is_nothrow_copy_assignable<string>::value,
              "string shall be nothrow copy-assignable");
static_assert(std::is_nothrow_move_constructible<string>::value,
              "string shall be nothrow move-constructible");
static_assert(std::is_nothrow_move_assignable<string>::value,
              "string shall be nothrow move-assignable");

#define REQUIRE_EMPTY(STR)    \
  REQUIRE(STR.size() == 0);   \
  REQUIRE(STR.length() == 0); \
  REQUIRE(STR.empty());       \
  REQUIRE(std::strcmp(STR.c_str(), "") == 0);

SCENARIO("empty string construction", "[string]") {
  GIVEN("default-constructed string") {
    string str;
    REQUIRE_EMPTY(str);
  }
  GIVEN("string constructed from empty cstr") {
    string str{""};
    REQUIRE_EMPTY(str);
  }
  GIVEN("string constructed from non-empty cstr with count = 0") {
    string str{"test", 0};
    REQUIRE_EMPTY(str);
  }
  GIVEN("string constructed with same character repeated 0 times") {
    string str{0, '1'};
    REQUIRE_EMPTY(str);
  }
}

SCENARIO("non-empty string construction", "[string]") {
  GIVEN("string constructed from test cstr") {
    string str{"test"};

    REQUIRE(str.size() == 4);
    REQUIRE(std::strcmp(str.c_str(), "test") == 0);
  }
  GIVEN("string constructed from test cstr with count = 2") {
    string str{"test", 2};

    REQUIRE(str.size() == 2);
    REQUIRE(std::strcmp(str.c_str(), "te") == 0);
  }
  GIVEN("string constructed with same character repeated 5 times") {
    string str{5, '1'};

    REQUIRE(str.size() == 5);
    REQUIRE(std::strcmp(str.c_str(), "11111") == 0);
  }
}

SCENARIO("string is copyable without new allocations", "[string]") {
  GIVEN("some test string constructed with allocator with count") {
    int allocated_count = 0;
    auto allocator = allocator_with_count<char>{allocated_count};
    string_count_alloc test_str{"test", allocator};

    REQUIRE(allocated_count == 1);

    WHEN("new string is copy-constructed") {
      string_count_alloc new_str{test_str};

      THEN("they point to the same memory") {
        REQUIRE(test_str.data() == new_str.data());
      }
      THEN("they have the same size") {
        REQUIRE(test_str.size() == new_str.size());
      }
      THEN("allocated count is 1") { REQUIRE(allocated_count == 1); }
    }
    WHEN("new string is created and test string is assigned to it") {
      string_count_alloc new_str{allocator};
      REQUIRE(allocated_count == 2);

      new_str = test_str;

      THEN("they point to the same memory") {
        REQUIRE(test_str.data() == new_str.data());
      }
      THEN("they have the same size") {
        REQUIRE(test_str.size() == new_str.size());
      }
      THEN("allocated count is 2") { REQUIRE(allocated_count == 2); }
    }
  }
}

SCENARIO("string is movable", "[string]") {
  GIVEN("some test string") {
    int allocated_count = 0;
    auto allocator = allocator_with_count<char>{allocated_count};
    string_count_alloc test_str{"test", allocator};

    WHEN("new string is move-constructed") {
      string_count_alloc new_str{std::move(test_str)};

      THEN("new string has test string") {
        REQUIRE(std::strcmp(new_str.data(), "test") == 0);
      }
      THEN("test string is unusable and point to nullptr") {
        REQUIRE(test_str.data() == nullptr);
      }
      THEN("allocated count is 1") { REQUIRE(allocated_count == 1); }
    }
    WHEN("new string is creeated and test string is move-assigned to it") {
      string_count_alloc new_str{allocator};
      REQUIRE(allocated_count == 2);

      new_str = std::move(test_str);

      THEN("new string has test string") {
        REQUIRE(std::strcmp(new_str.data(), "test") == 0);
      }
      THEN("test string is unusable and point to nullptr") {
        REQUIRE(test_str.data() == nullptr);
      }
      THEN("allocated count is 2") { REQUIRE(allocated_count == 2); }
    }
  }
}

SCENARIO("string's element access", "[string]") {
  GIVEN("some test string") {
    string test_str{"abcd"};

    WHEN("accessed pos < size()") {
      THEN("valid character is returned") {
        REQUIRE(test_str.front() == 'a');
        REQUIRE(test_str.at(1) == 'b');
        REQUIRE(test_str[2] == 'c');
        REQUIRE(test_str.back() == 'd');
      }
    }
    WHEN("accessed pos == size()") {
      THEN("at throws") {
        REQUIRE_THROWS_AS(test_str.at(4), std::out_of_range);
      }
      THEN("operator[] returns 0") { REQUIRE(test_str[4] == 0); }
    }
    WHEN("accessed pos > size()") {
      THEN("at throws") {
        REQUIRE_THROWS_AS(test_str.at(5), std::out_of_range);
        REQUIRE_THROWS_AS(test_str.at(100), std::out_of_range);
      }
    }
  }
}

SCENARIO("iterators usage") {
  GIVEN("some test string") {
    string test_str{"abcd"};

    WHEN("compared to the same string by std::equal") {
      string test_str2{"abcd"};
      THEN("thy are equal by begin/end iterators") {
        REQUIRE(
            std::equal(test_str.begin(), test_str.end(), test_str2.begin()));
      }
      THEN("thy are not equal by rbegin/rend iterators") {
        REQUIRE(
            !std::equal(test_str.rbegin(), test_str.rend(), test_str2.begin()));
      }
    }
    WHEN("compared to reversed string by std::equal") {
      string test_str2{"dcba"};
      THEN("thy are not equal by begin/end iterators") {
        REQUIRE(
            !std::equal(test_str.begin(), test_str.end(), test_str2.begin()));
      }
      THEN("thy are equal by rbegin/rend iterators") {
        REQUIRE(
            std::equal(test_str.rbegin(), test_str.rend(), test_str2.begin()));
      }
    }
  }
}

SCENARIO("string comparison") {
  GIVEN("str1 == str2") {
    string str1{"abcd"};
    string str2{"abcd"};

    REQUIRE(str1 == str2);
    REQUIRE(str1 <= str2);
    REQUIRE(str1 >= str2);
    REQUIRE_FALSE(str1 != str2);
    REQUIRE_FALSE(str1 < str2);
    REQUIRE_FALSE(str1 > str2);
  }
  GIVEN("str1 < str2") {
    string str1{"abcd"};
    string str2{"abcde"};

    REQUIRE(str1 < str2);
    REQUIRE(str1 <= str2);
    REQUIRE(str1 != str2);
    REQUIRE_FALSE(str1 == str2);
    REQUIRE_FALSE(str1 > str2);
    REQUIRE_FALSE(str1 >= str2);
  }
  GIVEN("str1 > str2") {
    string str1{"abcd"};
    string str2{"abcc"};

    REQUIRE(str1 > str2);
    REQUIRE(str1 >= str2);
    REQUIRE(str1 != str2);
    REQUIRE_FALSE(str1 == str2);
    REQUIRE_FALSE(str1 < str2);
    REQUIRE_FALSE(str1 <= str2);
  }
}

SCENARIO("string vs const char* comparison") {
  GIVEN("str == abcd") {
    int allocated_count = 0;
    auto allocator = allocator_with_count<char>{allocated_count};
    string_count_alloc str{"abcd", allocator};

    REQUIRE(str.compare("abcd") == 0);
    REQUIRE(str == "abcd");
    REQUIRE(str <= "abcd");
    REQUIRE(str >= "abcd");
    REQUIRE_FALSE(str > "abcd");
    REQUIRE_FALSE(str < "abcd");
    REQUIRE_FALSE(str != "abcd");

    REQUIRE("abcd" == str);
    REQUIRE("abcd" <= str);
    REQUIRE("abcd" >= str);
    REQUIRE_FALSE("abcd" < str);
    REQUIRE_FALSE("abcd" > str);
    REQUIRE_FALSE("abcd" != str);

    REQUIRE(allocated_count == 1);
  }
  GIVEN("str < abcde") {
    int allocated_count = 0;
    auto allocator = allocator_with_count<char>{allocated_count};
    string_count_alloc str{"abcd", allocator};

    REQUIRE(str.compare("abcde") < 0);
    REQUIRE(str != "abcde");
    REQUIRE(str < "abcde");
    REQUIRE(str <= "abcde");
    REQUIRE_FALSE(str > "abcde");
    REQUIRE_FALSE(str >= "abcde");
    REQUIRE_FALSE(str == "abcde");

    REQUIRE("abcde" != str);
    REQUIRE("abcde" > str);
    REQUIRE("abcde" >= str);
    REQUIRE_FALSE("abcde" < str);
    REQUIRE_FALSE("abcde" <= str);
    REQUIRE_FALSE("abcde" == str);

    REQUIRE(allocated_count == 1);
  }
  GIVEN("str > abcc") {
    int allocated_count = 0;
    auto allocator = allocator_with_count<char>{allocated_count};
    string_count_alloc str{"abcd", allocator};

    REQUIRE(str.compare("abcc") > 0);
    REQUIRE(str != "abcc");
    REQUIRE(str > "abcc");
    REQUIRE(str >= "abcc");
    REQUIRE_FALSE(str < "abcc");
    REQUIRE_FALSE(str <= "abcc");
    REQUIRE_FALSE(str == "abcc");

    REQUIRE("abcc" != str);
    REQUIRE("abcc" < str);
    REQUIRE("abcc" <= str);
    REQUIRE_FALSE("abcc" > str);
    REQUIRE_FALSE("abcc" >= str);
    REQUIRE_FALSE("abcc" == str);

    REQUIRE(allocated_count == 1);
  }
}

SCENARIO("find substring in a string") {
  GIVEN("test string") {
    string test_str{"aaabbbcccddd"};

    WHEN("correct substring is given") {
      string test_substr{"cddd"};
      REQUIRE(test_str.find(test_substr) == 8);
      REQUIRE(test_str.find("bbbc") == 3);
      REQUIRE(test_str.find('a') == 0);
      REQUIRE(test_str.find('a', 1) == 1);
      REQUIRE(test_str.find("ad", 0, 1) == 0);
    }

    WHEN("incorrect substring is given") {
      string test_substr{"dc"};
      REQUIRE(test_str.find(test_substr) == string::npos);
      REQUIRE(test_str.find("aba") == string::npos);
      REQUIRE(test_str.find('e') == string::npos);
      REQUIRE(test_str.find('a', 3) == string::npos);
    }
  }
}
