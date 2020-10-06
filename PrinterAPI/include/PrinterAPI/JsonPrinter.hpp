/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see
             // LICENSE.md for rights.
#ifndef PRINTER_API_PRINTER_JSONPRINTER_HPP_
#define PRINTER_API_PRINTER_JSONPRINTER_HPP_

#include "VarAPI/Vector.hpp"

#include "Printer.hpp"

namespace printer {

class JsonPrinter : public Printer {
public:
  JsonPrinter();

private:
  enum class ContainerType { array, object };

  using Container = PrinterContainer<ContainerType>;
  var::Vector<Container> m_container_list;
  var::Vector<Container> &container_list() { return m_container_list; }
  const var::Vector<Container> &container_list() const {
    return m_container_list;
  }

  // re-implemented virtual functions from Printer
  void print_open_object(Level level, const var::StringView &key) override;
  void print_close_object() override;
  void print_open_array(Level level, const var::StringView &key) override;
  void print_close_array() override { return print_close_object(); }
  void print(
    Level level,
    const char *key,
    const char *value,
    Newline is_newline = Newline::yes) override;

  Container &container() { return m_container_list.back(); }

  const Container &container() const { return m_container_list.back(); }

  void insert_comma();
};

} // namespace printer

#endif // PRINTER_API_PRINTER_JSONPRINTER_HPP_
