/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see
             // LICENSE.md for rights.
#include "PrinterAPI/YamlPrinter.hpp"

using namespace printer;

YamlPrinter::YamlPrinter() {
  container_list().push_back(Container(Level::fatal, ContainerType::array));
}

void YamlPrinter::print(
  Level level,
  const char *key,
  const char *value,
  Newline is_newline) {

  if (level > verbose_level()) {
    return;
  }

  for (auto const &c : container_list()) {
    if (c.verbose_level() > verbose_level()) {
      return;
    }
  }

  for (size_t i = 0; i < container_list().count() - 1; i++) {
    print_final("   ");
  }

  if (container().type() == ContainerType::array) {
    print_final("- ");
  }

  Printer::print(
    level,
    key,
    value,
    (value != nullptr) ? Newline::yes : Newline::no);
}

void YamlPrinter::print_open_object(Level level, const var::StringView &key) {

  if (verbose_level() >= level) {
    if (o_flags() & print_bold_objects) {
      set_format_code(FormatType::bold);
    }
    print(level, key.cstring(), " ");
    if (o_flags() & print_bold_objects) {
      clear_format_code(FormatType::bold);
    }
  }

  container_list().push_back(Container(level, ContainerType::object));
}

void YamlPrinter::print_open_array(Level level, const var::StringView &key) {
  if (verbose_level() >= level) {
    if (o_flags() & print_bold_objects) {
      set_format_code(FormatType::bold);
    }
    print(level, key.cstring(), " ");
    if (o_flags() & print_bold_objects) {
      clear_format_code(FormatType::bold);
    }
  }
  container_list().push_back(Container(level, ContainerType::array));
}

void YamlPrinter::print_close_object() {
  if (container_list().count() > 1) {
    container_list().pop_back();
  }
}
