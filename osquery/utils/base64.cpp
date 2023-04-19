/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include "base64.h"

#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

#include <osquery/logger/logger.h>

namespace bai = boost::archive::iterators;

namespace osquery {

namespace base64 {

namespace {

typedef bai::binary_from_base64<const char*> base64_str;
typedef bai::transform_width<base64_str, 8, 6> base64_dec;
typedef bai::transform_width<std::string_view::const_iterator, 6, 8> base64_enc;
typedef bai::base64_from_binary<base64_enc> it_base64;

std::string decode_impl(std::string_view encoded) {
  if (encoded.empty()) {
    return std::string();
  }

  try {
    return std::string(base64_dec(encoded.data()),
                       base64_dec(encoded.data() + encoded.size()));
  } catch (const boost::archive::iterators::dataflow_exception& e) {
    LOG(INFO) << "Could not base64 decode string: " << e.what();
    return std::string();
  }
}

} // namespace

std::string decode(std::string&& encoded) {
  boost::erase_all(encoded, "\r\n");
  boost::erase_all(encoded, "\n");
  boost::trim_right_if(encoded, boost::is_any_of("="));

  return decode_impl(encoded);
}

std::string decode(std::string_view encoded) {
  if (encoded.find_first_of("\r\n=") == std::string_view::npos) {
    return decode_impl(encoded);
  }

  return decode(std::string(encoded));
}

std::string encode(std::string_view unencoded) {
  if (unencoded.empty()) {
    return std::string();
  }

  size_t writePaddChars = (3U - unencoded.length() % 3U) % 3U;
  try {
    auto encoded =
        std::string(it_base64(unencoded.begin()), it_base64(unencoded.end()));
    encoded.append(writePaddChars, '=');
    return encoded;
  } catch (const boost::archive::iterators::dataflow_exception& e) {
    LOG(INFO) << "Could not base64 encode string: " << e.what();
    return std::string();
  }
}

} // namespace base64
} // namespace osquery
