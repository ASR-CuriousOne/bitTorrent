#include <common/bencoding.hpp>
#include <common/sha1.hpp>
#include <common/utils.hpp>
#include <iostream>

namespace BTCore {
BNode BDecoder::decode(const std::string &source) {
  size_t index = 0;
  return parseElement(source, index);
}

std::string BEncoder::bencode(const BNode &node) {
  if (node.is<BInt>()) {
    return "i" + std::to_string(node.get<BInt>()) + "e";
  } else if (node.is<BString>()) {
    const auto &str = node.get<BString>();
    return std::to_string(str.length()) + ":" + str;
  } else if (node.is<BList>()) {
    std::string result = "l";
    for (const auto &item : node.get<BList>()) {
      result += bencode(item);
    }
    return result + "e";
  } else if (node.is<BDict>()) {
    std::string result = "d";
    const auto &dict = node.get<BDict>();
    for (const auto &[key, value] : dict) {
      result += std::to_string(key.length()) + ":" + key;
      result += bencode(value);
    }
    return result + "e";
  }
  return "";
}

BNode BDecoder::parseElement(const std::string &source, size_t &index) {
  if (index >= source.length())
    Utils::logAndThrowFatal("Parse Element", "Unexpected end of source");

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
    Utils::logAndThrowFatal("Parse Element", "Invalid charactor at index " +
                                                 std::to_string(index));
  }
}

BNode BDecoder::parseInt(const std::string &source, size_t &index) {
  index++;
  size_t end = source.find('e', index);
  if (end == std::string::npos)
    Utils::logAndThrowFatal("Parse Int", "Unterminated Integer");

  std::string numStr = source.substr(index, end - index);
  long long val = std::stoll(numStr);

  index = end + 1;
  return BNode{val};
}

BNode BDecoder::parseString(const std::string &source, size_t &index) {
  size_t colon = source.find(':', index);
  if (colon == std::string::npos)
    Utils::logAndThrowFatal("Parse String", "Invalid String Format");

  long long len = std::stoll(source.substr(index, colon - index));
  size_t start = colon + 1;

  if (start + len > source.length())
    Utils::logAndThrowFatal("Parse String", "String out of bounds");

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
    Utils::logAndThrowFatal("Parse List", "Unterminated list");

  index++;
  return BNode{list};
}

BNode BDecoder::parseDict(const std::string &source, size_t &index) {
  index++;
  BDict dict;

  while (index < source.length() && source[index] != 'e') {
    BNode keyNode = parseString(source, index);
    if (!keyNode.is<BString>())
      Utils::logAndThrowFatal("Parse Dict", "Dictionary key must be string");

    std::string key = keyNode.get<BString>();
    BNode value = parseElement(source, index);

    dict[key] = value;
  }

  if (index >= source.length())
    Utils::logAndThrowFatal("Parse Dict", "Unterminated Dictionary");

  index++;
  return BNode{dict};
}
void BNode::printNode(const BNode &node, int indent) const {
  std::string padding(indent, ' ');

  if (node.is<BInt>()) {
    std::cout << node.get<BInt>();
  } else if (node.is<BString>()) {
    std::cout << "\"" << node.get<BString>() << "\"";
  } else if (node.is<BList>()) {
    const auto &list = node.get<BList>();
    std::cout << "[\n";
    for (size_t i = 0; i < list.size(); ++i) {
      std::cout << padding << "  ";
      printNode(list[i], indent + 2);
      if (i < list.size() - 1)
        std::cout << ",";
      std::cout << "\n";
    }
    std::cout << padding << "]";
  } else if (node.is<BDict>()) {
    const auto &dict = node.get<BDict>();
    std::cout << "{\n";
    size_t count = 0;
    for (const auto &pair : dict) {
      std::cout << padding << "  \"" << pair.first << "\": ";
      printNode(pair.second, indent + 2);
      if (count < dict.size() - 1)
        std::cout << ",";
      std::cout << "\n";
      count++;
    }
    std::cout << padding << "}";
  }
}

std::array<uint32_t, 5> calculateInfoHash(const BNode &root) {

  if (!root.is<BDict>()) {
    Utils::logAndThrowFatal("InfoHash",
                            "Invalid torrent file: Root is not a dictionary");
  }

  const auto &rootDict = root.get<BDict>();
  if (rootDict.find("info") == rootDict.end()) {
    Utils::logAndThrowFatal("InfoHash",
                            "Invalid torrent file: 'info' key not found");
  }

  const BNode &infoNode = rootDict.at("info");
  std::string infoBytes = BEncoder::bencode(infoNode);

  SHA1 sha1;
  sha1.update(infoBytes);
  sha1.finalize();

  return sha1.getDigest();
}

uint64_t getTorrentSize(const BNode &root) {
  if (!root.is<BDict>()) {
    throw std::runtime_error(
        "Invalid torrent structure: Root is not a dictionary");
  }

  const auto &rootDict = root.get<BDict>();
  if (!rootDict.contains("info")) {
    throw std::runtime_error(
        "Invalid torrent structure: Missing 'info' dictionary");
  }

  const BNode &infoNode = rootDict.at("info");
  if (!infoNode.is<BDict>()) {
    throw std::runtime_error(
        "Invalid torrent structure: 'info' is not a dictionary");
  }
  const auto &infoDict = infoNode.get<BDict>();

  if (infoDict.contains("files")) {
    long long totalSize = 0;
    const BNode &filesNode = infoDict.at("files");

    if (!filesNode.is<BList>()) {
      throw std::runtime_error(
          "Invalid torrent structure: 'files' is not a list");
    }

    const auto &filesList = filesNode.get<BList>();
    for (const auto &fileNode : filesList) {
      if (fileNode.is<BDict>()) {
        const auto &fileDict = fileNode.get<BDict>();
        if (fileDict.contains("length")) {
          totalSize += fileDict.at("length").get<BInt>();
        }
      }
    }
    return totalSize;
  }

  if (infoDict.contains("length")) {
    return infoDict.at("length").get<BInt>();
  }

  throw std::runtime_error(
      "Invalid torrent structure: No length or files found");
}

TrackerInfo getTrackerInfo(const BNode &root) {
  if (!root.is<BDict>()) {
    throw std::runtime_error(
        "Invalid torrent structure: Root is not a dictionary");
  }

  const auto &rootDict = root.get<BDict>();

  if (!rootDict.contains("announce")) {
    throw std::runtime_error("Invalid torrent: Missing 'announce' URL");
  }

  std::string url = rootDict.at("announce").get<BString>();

  TrackerInfo info;

  size_t schemeEnd = url.find("://");
  if (schemeEnd != std::string::npos) {
    info.protocol = url.substr(0, schemeEnd);
    url = url.substr(schemeEnd + 3);
  }

  size_t pathStart = url.find('/');
  std::string hostPort =
      (pathStart == std::string::npos) ? url : url.substr(0, pathStart);

  size_t colonPos = hostPort.find_last_of(':');

  if (colonPos != std::string::npos) {
    info.hostname = hostPort.substr(0, colonPos);
    info.port = hostPort.substr(colonPos + 1);
  } else {
    info.hostname = hostPort;
    if (info.protocol == "http")
      info.port = "80";
    else if (info.protocol == "https")
      info.port = "443";
    else
      info.port = "80";
  }

  return info;
}

} // namespace BTCore
