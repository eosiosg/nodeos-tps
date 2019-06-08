//
// Created by deadlock on 2019-06-08.
//

#ifndef NODEOS_TPS_DEMO_HPP
#define NODEOS_TPS_DEMO_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <tuple>

namespace demo {

class demo {

};

/*
 * data structure
 *
 * visitor
 *
 * serializer
 *
 * */
struct reference {
  std::string name;
  std::string link;
  char property;

  auto reflect() const
  {
      return std::tie(name, link, property);
  }
};

struct article {
  uint32_t page_num;
  std::string title;
  std::vector<std::string> authors;
  std::string content;

  auto reflect() const
  {
      return std::tie(page_num, title, authors, content);
  }
};

struct summary {
  std::string title;
  std::string content;
  std::optional<reference> article_reference;

  auto reflect() const
  {
      return std::tie(title, content, article_reference);
  }
};

template<std::size_t I = 0, typename Visitor, typename... Types>
inline typename std::enable_if<I==sizeof...(Types), void>::type
for_each(const std::tuple<Types...>&, Visitor) { }

template<std::size_t I = 0, typename Visitor, typename... Types>
inline typename std::enable_if<I<sizeof...(Types), void>::type
for_each(const std::tuple<Types...>& t, Visitor f)
{
    f->serialize(std::get<I>(t));
    for_each<I+1, Visitor, Types...>(t, f);
}

class text_serializer {
public:

    std::string get_result() { return result; }

    template<class T, typename std::enable_if_t<std::is_class<T>::value, int> = 1>
    void serialize(const T& obj)
    {
        for_each(obj.reflect(), this);
    }

    void serialize(const uint32_t value)
    {
        result += std::to_string(value);
    }

    void serialize(const std::string& value)
    {
        uint32_t size = value.size();
        serialize(size);
        result += value;
    }

    void serialize(const char value)
    {
        result += value;
    }

    void serialize(const std::vector<std::string>& values)
    {
        uint32_t size = values.size();
        serialize(size);
        for (const auto& v:values) {
            serialize(v);
        }
    }

    template<class T>
    void serialize(const std::optional<T>& obj)
    {
        uint32_t v = obj.has_value() ? 1 : 0;
        serialize(v);
        if (obj) {
            serialize(*obj);
        }
    }

private:
    std::string result;
};
}

#endif //NODEOS_TPS_DEMO_HPP
