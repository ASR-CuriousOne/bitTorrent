#include <logger/logger.hpp>
#include <map>
#include <string>
#include <variant>

namespace BTCore {
struct BNode;

using BInt = long long;
using BString = std::string;
using BList = std::vector<BNode>;
using BDict = std::map<std::string, BNode>;

using BValue = std::variant<BInt, BString, BList, BDict>;

struct BNode {
  BValue value;

  template <typename T> bool is() const {
    return std::holds_alternative<T>(value);
  }

  template <typename T> const T &get() const { return std::get<T>(value); }

  void printNode(int indent = 0) const{printNode(*this);};

private:
  void printNode(const BNode &node, int indent = 0) const;
};

class BDecoder {
public:
  static BNode decode(const std::string &source);

private:
  static BNode parseElement(const std::string &source, size_t &index);

  static BNode parseInt(const std::string &source, size_t &index);

  static BNode parseString(const std::string &source, size_t &index);

  static BNode parseList(const std::string &source, size_t &index);

  static BNode parseDict(const std::string &source, size_t &index);
};
} // namespace BTCore
