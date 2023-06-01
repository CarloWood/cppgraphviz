#include "sys.h"
#include "TableNode.hpp"
#include <iostream>
#include <map>

namespace cppgraphviz::dot {

namespace {

std::string html_escape(std::string_view input)
{
  std::map<char, std::string> html_entities{
    {'&', "&amp;"},
    {'<', "&lt;"},
    {'>', "&gt;"},
    {'"', "&quot;"},
    {'\'', "&#39;"}
  };

  std::string escaped_string;
  for (char c : input)
  {
    auto it = html_entities.find(c);
    if (it != html_entities.end())
      escaped_string += it->second;
    else
      escaped_string += c;
  }
  return escaped_string;
}

} // namespace

void TableNodeItem::write_html_to(std::ostream& os, std::string const& indentation) const
{
  bool table_has_bgcolor = attribute_list().has_key("bgcolor");
  bool table_has_color = attribute_list().has_key("color");
  bool table_has_fontname = attribute_list().has_key("fontname");
  bool table_has_fontsize = attribute_list().has_key("fontsize");
  bool table_has_fontcolor = attribute_list().has_key("fontcolor");
  bool table_has_font = table_has_fontname || table_has_fontsize || table_has_fontcolor;

  os << indentation << dot_id() << " [shape=none, margin=0, label=<\n" <<
        indentation << "  <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\"";
  if (table_has_color)
    os << " COLOR=\"" << attribute_list().get_value("color") << '"';
  os << ">\n";
  //os << indentation << "    <TR><TD BORDER=\"0\"></TD></TR>\n";
  size_t size = container_size_();
  for (size_t port = 0; port < size; ++port)
  {
    TableElement table_element = container_reference_(port);
    AttributeList const& eal = table_element.attribute_list();
    os << indentation << "    <TR><TD PORT=\"" << port << "\"";
    if (eal.has_key("bgcolor"))
      os << " BGCOLOR=\"" << eal.get_value("bgcolor") << '"';
    else if (table_has_bgcolor)
      os << " BGCOLOR=\"" << attribute_list().get_value("bgcolor") << '"';
    if (eal.has_key("color"))
      os << " COLOR=\"" << eal.get_value("color") << '"';
    os << '>';
    bool has_fontname = eal.has_key("fontname");
    bool has_fontsize = eal.has_key("fontsize");
    bool has_fontcolor = eal.has_key("fontcolor");
    bool has_font = table_has_font || has_fontname || has_fontsize || has_fontcolor;
    if (has_font)
    {
      os << "<FONT";
      if (has_fontname)
        os << " FACE=\"" << eal.get_value("fontname") << "\"";
      else if (table_has_fontname)
        os << " FACE=\"" << attribute_list().get_value("fontname") << "\"";
      if (has_fontsize)
        os << " POINT-SIZE=\"" << eal.get_value("fontsize") << "\"";
      else if (table_has_fontsize)
        os << " POINT-SIZE=\"" << attribute_list().get_value("fontsize") << "\"";
      if (has_fontcolor)
        os << " COLOR=\"" << eal.get_value("fontcolor") << "\"";
      else if (table_has_fontcolor)
        os << " COLOR=\"" << attribute_list().get_value("fontcolor") << "\"";
      os << '>';
    }
    os << html_escape(table_element.label());
    if (has_font)
      os << "</FONT>";
    os << "</TD></TR>\n";
  }
  os << indentation << "  </TABLE>\n" <<
        indentation << ">]\n";
}

void TableNodeItem::write_dot_to(std::ostream& os, std::string& indentation) const
{
  write_html_to(os, indentation);
}

} // namespace cppgraphviz::dot
