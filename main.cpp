#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <string_view>
#include <vector>

#include <cassert>
#include <random>
#include "log_duration.h"

using namespace std;

class Domain {
public:
    // Конструктор позволяет конструирование из string
    Domain(const string& domain) : domain_(domain.rbegin(), domain.rend()) {
    }

    bool operator== (const Domain& other) const {
        return other.domain_ == domain_;
    }

    // Метод IsSubdomain, принимающий другой домен и возвращающий true, если this его поддомен
    bool IsSubdomain(Domain domain) const {
        size_t size = domain_.size();
        if (size < domain.domain_.size()) {
            return ((domain_ + '.') == domain.domain_.substr(0, size + 1));
        }
        return false;
    }

    // Возращает строку с доменом во внутреннем формате хранения (реверсивная строка)
    string_view GetRaw() const {
        return {domain_};
    }

    // Возвращает строку с доменом в нормальном формате
    string Get() const {
        return {domain_.rbegin(), domain_.rend()};
    }

private:
    string domain_;
};

template <typename InputIt>
class DomainChecker {
public:
    // Конструктор принимает список запрещённых доменов через пару итераторов
    DomainChecker(InputIt begin, InputIt end) {
        for (InputIt it = begin; it != end; ++it) {
            domains_.emplace_back(*it);
        }

        sort(domains_.begin(), domains_.end(), 
            [](const Domain& left, const Domain& right){
                return lexicographical_compare(left.GetRaw().begin(), left.GetRaw().end(), 
                                               right.GetRaw().begin(), right.GetRaw().end());
            });
        
        auto last = unique(domains_.begin(), domains_.end(), 
            [](const Domain& left, const Domain& right){
                return left.IsSubdomain(right);
            });
        domains_.erase(last, domains_.end());
    }

    // Метод IsForbidden, возвращает true, если домен запрещён
    bool IsForbidden(Domain domain) const {

        auto upper = upper_bound(domains_.begin(), domains_.end(), domain, 
                        [](const Domain& left, const Domain& right){
                            return left.GetRaw() < right.GetRaw();
                        });
        if (upper != domains_.begin()) {
            upper = prev(upper);
            return ((domain == *upper) || (*upper).IsSubdomain(domain));
        }

        return false;
    }

private:
    vector<Domain> domains_;
};

const vector<Domain> ReadDomains(istream& input, int num) {
    vector<Domain> result;
    for (int i = 0; i < num; ++i) {
        string s;
        if (getline(input, s)) {
            result.push_back(Domain{s});
        }
    }
    return result;
}

template <typename Number>
Number ReadNumberOnLine(istream& input) {
    string line;
    getline(input, line);

    Number num;
    istringstream(line) >> num;

    return num;
}

namespace test {

void TestDomainClass() {
    {
        const string test_domain{"com"s};
        Domain domain{"com"s};
        assert(test_domain == domain.Get());
    }
    {
        const string test_domain{"maps.me"s};
        Domain domain{"maps.me"s};
        assert(test_domain == domain.Get());
    }
    {
        const string test_domain{"a.b.c.m.gdz.ru"s};
        Domain domain{"a.b.c.m.gdz.ru"s};
        assert(test_domain == domain.Get());
    }

    {
        Domain domain{"com"s};
        assert(domain.IsSubdomain(Domain{"yandex.com"s}));
        assert(domain.IsSubdomain(Domain{"agr4.yandex.com"s}));
        assert(!domain.IsSubdomain(Domain{"com"s}));
        assert(!domain.IsSubdomain(Domain{"com.ru"s}));
        assert(!domain.IsSubdomain(Domain{"agr4.com.ru"s}));
    }
}

void TestReadDomains() {
    istringstream test_input{
R"(gdz.ru
maps.me
m.gdz.ru
a.m.gdz.ru
a.b.c.m.gdz.ru
com
)"
    };
    const vector<Domain> test_domains{"gdz.ru"s,
                                      "maps.me"s,
                                      "m.gdz.ru"s,
                                      "a.m.gdz.ru"s,
                                      "a.b.c.m.gdz.ru"s,
                                      "com"s};

    const vector<Domain> forbidden_domains = ReadDomains(test_input, 6);
    assert(equal(forbidden_domains.cbegin(), forbidden_domains.cend(),
                 test_domains.cbegin()));
}

void TestDomainChekerClass() {
    const vector<Domain> forbidden_domains{"gdz.ru"s,
                                           "maps.me"s,
                                           "m.gdz.ru"s,
                                           "a.m.gdz.ru"s,
                                           "a.b.c.m.gdz.ru"s,
                                           "com"s};
    DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());

    {
        Domain test_domain{"gdz.ru"s};
        assert(checker.IsForbidden(test_domain));
    }
    {
        Domain test_domain{"gdz.com"s};
        assert(checker.IsForbidden(test_domain));
    }
    {
        Domain test_domain{"m.maps.me"s};
        assert(checker.IsForbidden(test_domain));
    }
    {
        Domain test_domain{"alg.m.gdz.ru"s};
        assert(checker.IsForbidden(test_domain));
    }
    {
        Domain test_domain{"maps.com"s};
        assert(checker.IsForbidden(test_domain));
    }
    {
        Domain test_domain{"maps.ru"s};
        assert(!checker.IsForbidden(test_domain));
    }
    {
        Domain test_domain{"gdz.ua"s};
        assert(!checker.IsForbidden(test_domain));
    }
}

class DomainGenerator {
    char GenerateChar() {
        uniform_int_distribution<short> char_gen{0, static_cast<short>(possible_chars_.size() - 1)};
        return possible_chars_[char_gen(generator_)];
    }

public:
    Domain Generate() {
        static const int max_length = 100'000;
        const int length = uniform_int_distribution(0, max_length)(generator_);

        string domain;
        for (int i = 0; i < length; ++i) {
            domain += GenerateChar();
        }
        return Domain(domain);
    }

private:
    mt19937 generator_;

    // допустимые значения сохраним в static переменных
    // они объявлены inline, чтобы их определение не надо было выносить вне класса
    // постфикс s у литерала тут недопустим, он приведёт к неопределённому поведению
    inline static const string_view possible_chars_ = "abcdefghijklmnopqrstuvwxyz."sv;
};

void TestBenchmark() {
    static const int N = 1000;

    DomainGenerator generator;
    vector<Domain> forbidden_domains;
    vector<Domain> test_domains;

    generate_n(back_inserter(forbidden_domains), N, 
                [&]() { return generator.Generate(); });
    generate_n(back_inserter(test_domains), N, 
                [&]() { return generator.Generate(); });

    {
        LOG_DURATION("Set forbidden");
        DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());
    }

    {
        DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());
        LOG_DURATION("Check domains");
    }
}

void Test() {
    TestDomainClass();
    TestReadDomains();
    TestDomainChekerClass();
    TestBenchmark();
    cout << "Tests passed. Ready to read from input." << endl << endl;
}

} // namespace test

int main() {
    
    test::Test();

    const std::vector<Domain> forbidden_domains = ReadDomains(cin, ReadNumberOnLine<size_t>(cin));
    DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());

    const std::vector<Domain> test_domains = ReadDomains(cin, ReadNumberOnLine<size_t>(cin));
    for (const Domain& domain : test_domains) {
        cout << (checker.IsForbidden(domain) ? "Bad"sv : "Good"sv) << endl;
    }
}