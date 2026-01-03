#include <common/bencoding.hpp>
#include <iostream>


[[noreturn]] inline void logAndThrowFatal(const std::string &origin,
                                          const std::string &error) {
  Logger::error(origin, error);
  throw std::runtime_error(error);
}

namespace BTCore {
BNode BDecoder::decode(const std::string &source){
    size_t index = 0;
    return parseElement(source, index);
  }

BNode BDecoder::parseElement(const std::string &source, size_t &index) {
  if (index >= source.length())
    logAndThrowFatal("Parse Element", "Unexpected end of source");

  char c = source[index];

  if (c == 'i') {
    return parseInt(source, index);
  } else if (c == 'l') {
    return parseList(source, index);
  } else if (c == 'd') {
    return parseDict(source, index);
  } else if (std::isdigit(c)) {
    return parseString(source, index);
  } else {
    logAndThrowFatal("Parse Element",
                     "Invalid charactor at index " + std::to_string(index));
  }
}

BNode BDecoder::parseInt(const std::string &source, size_t &index) {
  index++;
  size_t end = source.find('e', index);
  if (end == std::string::npos)
    logAndThrowFatal("Parse Int", "Unterminated Integer");

  std::string numStr = source.substr(index, end - index);
  long long val = std::stoll(numStr);

  index = end + 1;
  return BNode{val};
}

BNode BDecoder::parseString(const std::string &source, size_t &index) {
  size_t colon = source.find(':', index);
  if (colon == std::string::npos)
    logAndThrowFatal("Parse String", "Invalid String Format");

  long long len = std::stoll(source.substr(index, colon - index));
  size_t start = colon + 1;

  if (start + len > source.length())
    logAndThrowFatal("Parse String", "String out of bounds");

  std::string str = source.substr(start, len);
  index = start + len;
  return BNode{str};
}

BNode BDecoder::parseList(const std::string &source, size_t &index) {
  index++;
  BList list;

  while (index < source.length() && source[index] != 'e') {
    list.push_back(parseElement(source, index));
  }

  if (index >= source.length())
    logAndThrowFatal("Parse List", "Unterminated list");

  index++;
  return BNode{list};
}

BNode BDecoder::parseDict(const std::string &source, size_t &index) {
  index++;
  BDict dict;

  while (index < source.length() && source[index] != 'e') {
    BNode keyNode = parseString(source, index);
    if (!keyNode.is<BString>())
      logAndThrowFatal("Parse Dict", "Dictionary key must be string");

    std::string key = keyNode.get<BString>();
    BNode value = parseElement(source, index);

    dict[key] = value;
  }

  if (index >= source.length())
    logAndThrowFatal("Parse Dict", "Unterminated Dictionary");

  index++;
  return BNode{dict};
}
void BNode::printNode(const BNode& node, int indent) const{
    std::string padding(indent, ' ');

    if (node.is<BInt>()) {
        std::cout << node.get<BInt>();
    } 
    else if (node.is<BString>()) {
        std::cout << "\"" << node.get<BString>() << "\"";
    } 
    else if (node.is<BList>()) {
        const auto& list = node.get<BList>();
        std::cout << "[\n";
        for (size_t i = 0; i < list.size(); ++i) {
            std::cout << padding << "  ";
            printNode(list[i], indent + 2);
            if (i < list.size() - 1) std::cout << ",";
            std::cout << "\n";
        }
        std::cout << padding << "]";
    } 
    else if (node.is<BDict>()) {
        const auto& dict = node.get<BDict>();
        std::cout << "{\n";
        size_t count = 0;
        for (const auto& pair : dict) {
            std::cout << padding << "  \"" << pair.first << "\": ";
            printNode(pair.second, indent + 2);
            if (count < dict.size() - 1) std::cout << ",";
            std::cout << "\n";
            count++;
        }
        std::cout << padding << "}";
    }
}
} // namespace BTCore
