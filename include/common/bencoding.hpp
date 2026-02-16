#pragma once
#include <array>
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

  void printNode(int indent = 0) const { printNode(*this); };

private:
  void printNode(const BNode &node, int indent = 0) const;
};

class BEncoder {
public:
  static std::string bencode(const BNode &node);
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

std::array<uint32_t, 5> calculateInfoHash(const BNode &root);

uint64_t getTorrentSize(const BNode &root);

struct TrackerInfo {
  std::string hostname;
  std::string port;
  std::string protocol;
};

TrackerInfo getTrackerInfo(const BNode& root);

} // namespace BTCore
