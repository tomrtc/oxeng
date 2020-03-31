// generators

#include "oxeng.hh"

#include <cstdlib>

#include <iostream>     // std::cout, std::ostream, std::ios
#include <fstream>      // std::filebuf
#include <sstream>
#include <iomanip>      // std::setw

#include <algorithm>

#include <functional>
#include <vector>
#include <list>
#include <regex>


#include <string>
#include <cerrno>

#include <chrono>
#include <future>
//#include <unordered_map>
//#include <utility>
#include <initializer_list>
// C/Posix/linux headers.
// no more necessary #define _XOPEN_SOURCE 500



//header only 3rd party libraries.
std::ostream& operator<< (std::ostream& os, const fldrecord& f);
#include "prettyprint.hh"



static const std::string              doc_extension {"doc"};
static const std::string              def_extension {"def"};
static const std::string              dummy_ref {"1.0"} ;
static const std::vector<std::string> FldSTR { "FLD", "POS1", "POS2", "LEN", "CST" };
static const std::vector<std::string> FldAPP { "STR", "REF", "LEN", "CST" };
static const std::vector<std::string> FldAPPbis { "STR", "REF", "LEN", "NOCST" };

std::vector<std::string>
split_field(const std::string& input,int submatch ) {
  static const std::regex& re{"(FLD|POS1|POS2|LEN|CST|REF|NOCST|STR)=([^|\\n# \\t]+)", std::regex::optimize};

  std::sregex_token_iterator
    first{input.begin(), input.end(), re, submatch},
    last;
    return {first, last};
}

bool
builder_context::insert_record(const std::string& input, string_map_t &t_string_map) {

  const std::vector<std::string> fieldvector { split_field(input,1)};
  if( fieldvector.size() >= FldSTR.size() && equal(FldSTR.begin(), FldSTR.end(), fieldvector.begin()) )
    {
      std::vector<std::string> fieldvalues { split_field(input, 2)};
      fldrecord a_record ( std::stoi(fieldvalues[1]),
                           std::stoi(fieldvalues[2]),
                           std::stoi(fieldvalues[3]),
                           fieldvalues[0],
                           fieldvalues[4]);


      fieldvalues.erase(fieldvalues.begin(), fieldvalues.begin() + 5);
      for (auto it = fieldvalues.begin() ; it < fieldvalues.end(); it += 2)
        a_record.m_refs.emplace_back(std::make_pair(*it, *(it+1)));

      auto result =
        t_string_map.insert(std::make_pair(a_record.m_fld, a_record));
      return result.second;
    }
  else  if( fieldvector.size() >= FldAPP.size()
            && (equal(FldAPP.begin(), FldAPP.end(), fieldvector.begin())
                ||  equal(FldAPPbis.begin(), FldAPPbis.end(), fieldvector.begin()) ))
    {
      std::vector<std::string> fieldvalues { split_field(input, 2)};
      fldrecord a_record (fieldvalues[0],
                          std::stoi(fieldvalues[2]),
                          fieldvalues[3],
                          fieldvalues[1]);
      auto result =
        t_string_map.insert(std::make_pair(a_record.m_fld, a_record));
      return result.second;
    }
  return false;
}



void
builder_context::set_string_directory(const char* t_string_directory)
{
  std::string data_directory {t_string_directory};
  data_directory += "/data";
  if (directory_exists(data_directory.c_str()))
    {
      m_data_directory = data_directory;
    }
  else
    throw(errno);

  if (directory_exists(t_string_directory))
    {
      std::string path_lang {t_string_directory};
      std::string path_file {t_string_directory};
      m_string_path =  path_lang; // keep the string path.
      path_lang += "/config/lang.cnf";
      path_file += "/config/file.cnf";
      {//load lang.cnf
        std::string lang_cnf { get_file_contents(path_lang.c_str())};
        static const  std::regex& rgx {R"raw((\w+)\s+\"([^\"]*)\")raw", std::regex::optimize};
        std::sregex_iterator endit;
        for (std::sregex_iterator it(lang_cnf.begin(), lang_cnf.end(), rgx); it != endit; ++it) {
          auto& m =  *it;
          m_lang_map[m[1]] = m[2];
        }
      }

      { // load file.cnf
        std::string file_cnf { get_file_contents(path_file.c_str())};
        static const std::regex& rgx{R"raw((\d+)\s+([\w.]+)\s+(\w+))raw" ,std::regex::optimize};
        std::sregex_iterator endit;
        for (std::sregex_iterator it(file_cnf.begin(), file_cnf.end(), rgx); it != endit; ++it) {
          auto&& m = *it;
          std::string name{m[2]};
          int index { std::stoi(m[1])};
          std::string type{m[3]}   ;
          size_t npos = name.find_last_of('.');
          name =  name.substr(0,npos);
          if (type == "STR")
            m_tel_file_map[index] = name;
          if (type == "APP")
            m_app_file_map[index] = name;

        }
      }
    }
  else
    throw(errno);
}



void
builder_context::set_utl_directory(const char* t_utl_directory)
{
  m_utl_directory = std::string(t_utl_directory);
}




void
insert_async(const std::string& input, string_map_t &t_string_map) {

  const std::vector<std::string> fieldvector { split_field(input,1)};
  if( fieldvector.size() >= FldSTR.size() && equal(FldSTR.begin(), FldSTR.end(), fieldvector.begin()) )
    {
      std::vector<std::string> fieldvalues { split_field(input, 2)};
      fldrecord a_record ( std::stoi(fieldvalues[1]),
                           std::stoi(fieldvalues[2]),
                           std::stoi(fieldvalues[3]),
                           fieldvalues[0],
                           fieldvalues[4]);


      fieldvalues.erase(fieldvalues.begin(), fieldvalues.begin() + 5);
      for (auto it = fieldvalues.begin() ; it < fieldvalues.end(); it += 2)
        a_record.m_refs.emplace_back(std::make_pair(*it, *(it+1)));


      t_string_map.insert(std::make_pair(a_record.m_fld, a_record));
      return ;
    }
  else  if( fieldvector.size() >= FldAPP.size()
            && (equal(FldAPP.begin(), FldAPP.end(), fieldvector.begin())
                ||  equal(FldAPPbis.begin(), FldAPPbis.end(), fieldvector.begin()) ))
    {
      std::vector<std::string> fieldvalues { split_field(input, 2)};
      fldrecord a_record (fieldvalues[0],
                          std::stoi(fieldvalues[2]),
                          fieldvalues[3],
                          fieldvalues[1]);

      t_string_map.insert(std::make_pair(a_record.m_fld, a_record));
      return ;
    }
  return ;
}
std::string get_include_filename(const std::string t_prefix, const std::string &name, const std::string t_suffix)
{
  std::string tmp {t_prefix};
  tmp = tmp + name + t_suffix;
  return tmp;
}
std::string header_helper(std::string name)
{
  std::string cpp_guard {name};
  size_t npos = cpp_guard.find_last_of('.');
  cpp_guard[npos] = '_';
  std::transform(cpp_guard.begin(), cpp_guard.end(), cpp_guard.begin(), ::toupper);
  cpp_guard = "#ifndef " + cpp_guard + "\n#define " + cpp_guard + "\n/*Generated file do not modify!.*/\n\n";
  return cpp_guard;
}

void
generate_tel_includes_async(const std::string &t_name, const string_map_t  &t_string_map)
{
  /**
   ** for every field (context) X:
   **  - one constant (NB_STR_CHAMPX) is defined.
   **  - one type (CHAMPX) is defined:  TYPE_ARRAY_1D(CHAMPX, 0, NB_STR_CHAMPX - 1, stringY);
   **   - one type "champ" is defined. It is one structure of many fields :
   **          contextX whose types are CHAMPX.
   **          The structure is like:
   **          typedef struct champ {
   **                CHAMP0    context0;
   **                CHAMP1    context1;
   **                ...
   **                CHAMPX    contextX;
   **                ...
   **          }
   */



  // Three files containing declaration of constants:
  // gencst_<string file>_1.hpp:
  // contains all constants that define for each context the number of strings of
  // the context (included in inc/cpu_dec/constants/string.hpp).
  std::string   gencst_1_name {get_include_filename("gencst_",t_name,  "_1.hpp")};
  std::ofstream gencst_1(gencst_1_name );
  gencst_1 << header_helper(gencst_1_name);
  // gencst_<string file>_2.hpp:
  // contains all constants that define the the constant identifying each context
  // (included in inc/cpu_dec/constants/string.hpp).
  std::string   gencst_2_name {get_include_filename("gencst_",t_name,  "_2.hpp")};
  std::ofstream gencst_2(gencst_2_name );
  gencst_2 << header_helper(gencst_2_name);
  // gencst_<string file>_3.hpp:
  // contains all constants that define the the constant identifying each string
  // in each context (included in inc/cpu_dec/constants/string.hpp).
  std::string   gencst_3_name {get_include_filename("gencst_",t_name,  "_3.hpp")};
  std::ofstream gencst_3(gencst_3_name );
  gencst_3 << header_helper(gencst_3_name);
  // Two files containing code:
  // gencod_<string file>_1.hpp:
  // contains the code of the loading of the strings in remanent.
  // (included in mao/code/initrem/inilang.C)
  std::string   gencod_1_name {get_include_filename("gencod_",t_name,  "_1.hpp")};
  std::ofstream gencod_1(gencod_1_name );
  gencod_1 << header_helper(gencod_1_name);
  // gencod_<string file>_2.hpp:
  // contains the code of the reading of the strings in remanent.
  // (included in utl/code/cpu_telutil/tl_aff_c.C)
  std::string   gencod_2_name {get_include_filename("gencod_",t_name,  "_2.hpp")};
  std::ofstream gencod_2(gencod_2_name );
  gencod_2 << header_helper(gencod_2_name);
  // Two files containing declaration of types:
  // gentyp_<string file>_1.hpp:
  // contains definitions of type associated to each context.
  // (included in inc/cpu_dec/types/typstring.hpp).
  std::string   gentyp_1_name {get_include_filename("gentyp_",t_name,  "_1.hpp")};
  std::ofstream gentyp_1(gentyp_1_name );
  gentyp_1 << header_helper(gentyp_1_name);
  // gentyp_<string file>_2.hpp:
  // contains one part of the type of the remanent table of contexts.
  // (included in inc/cpu_dec/types/typstring.hpp).
  std::string   gentyp_2_name {get_include_filename("gentyp_",t_name,  "_2.hpp")};
  std::ofstream gentyp_2(gentyp_2_name );
  gentyp_2 << header_helper(gentyp_2_name);

  // Hereafter is a magic C++14 macro define to enhance for-range loop on sequence with a indexed variable sub_index.
  // A little bit tricky but handy for our purpose to generate a index for each context. (source stackoverflow)
#define indexed(...) indexed_v(sub_index, __VA_ARGS__)
#define indexed_v(v, ...) (bool _i_ = true, _break_ = false; _i_;) for(size_t v = 0; _i_; _i_ = false) for(__VA_ARGS__) if(_break_) break; else for(bool _j_ = true; _j_;) for(_break_ = true; _j_; _j_ = false) for(bool _k_ = true; _k_; v++, _k_ = false, _break_ = false)
#define MAX_SIZE_BY_CHAR	6

  for  ( const auto& item : t_string_map) {
    gencst_2 << "#define " << item.second.m_cst <<"\t\t" << item.first<< std::endl;
    gencst_1 << "#define NB_STR_CHAMP" << item.first <<" " << item.second.m_refs.size() << "\t\t//" <<  item.second.m_cst << std::endl;
    gencst_3 << "/* defined constant for field "<< item.first<< " : " << item.second.m_cst <<"*/" << std::endl;
    gentyp_1 << "TYPE_ARRAY_1D(CHAMP" << item.first<< ", 0, NB_STR_CHAMP" << item.first<< "-1, uastring"<< item.second.m_len << ");"<< std::endl;
    gentyp_2 << "\tCHAMP" << item.first<< " context" << item.first << ";"<< std::endl;
    gencod_1 << "\tif (field_compare(last_champ, "<< item.first <<"))\n\t{\t" << std::endl;
    gencod_1 << "\t\tfor (i=0; i<NB_STR_CHAMP"<< item.first <<" +1; i++)\n\t\t{\n\t\t\tchain[0] = 0;" << std::endl;
    gencod_1 << "\t\t\tfgets(chain, "<< (item.second.m_len *  MAX_SIZE_BY_CHAR + 10) <<", fich);" << std::endl;
    gencod_1 << "\t\t\tif ((chain[0] == 27) || (chain[0] == 0)) {i++; break;};" << std::endl;
    gencod_1 << "\t\t\tif (i <NB_STR_CHAMP"<< item.first << ")" << std::endl;
    gencod_1 << "\t\t\t\tlang[vlangue].context"<< item.first << "[i]= str_filter(chain, KEYSET_UA);\n\t\t}"<< std::endl;
    gencod_1 << "\n\t\tlast_champ = verif_correct (chain, status, i-1, " << item.first<< ", NB_STR_CHAMP" << item.first<< ");\n\t}"<< std::endl<< std::endl;
    gencod_2 << "case "<< item.first << ":" << std::endl;
    gencod_2 << "\tif (nustr < NB_STR_CHAMP" << item.first << ")" <<  std::endl;
    gencod_2 << "\t\tresult = lang[vlangue].context"<< item.first << "[nustr];" << std::endl << "\tbreak;"<< std::endl;
    for indexed (const auto& ref:item.second.m_refs) {
        if (ref.second.find_first_of("-") != std::string::npos)
          gencst_3 << "/* no constant for idx " << sub_index << " " << ref.second << " */ "<< std::endl;
        else
          gencst_3 <<"#define " << ref.second <<"\t\t" << sub_index << std::endl;
      }
  }

  gencst_1<< std::endl<< "#endif" << std::endl;
  gencst_2<< std::endl<< "#endif" << std::endl;
  gencst_3<< std::endl<< "#endif" << std::endl;
  gencod_1<< std::endl<< "#endif" << std::endl;
  gencod_2<< std::endl << "#endif" << std::endl;
  gentyp_1<< std::endl << "#endif" << std::endl;
  gentyp_2<< std::endl << "#endif" << std::endl;

  gencst_1.close();
  gencst_2.close();
  gencst_3.close();
  gencod_1.close();
  gencod_2.close();
  gentyp_1.close();
  gentyp_2.close();
}


string_map_t load_doc_file_async(const std::string filename, const std::string target_path)
{
  string_map_t  current_string_map {} ;

  std::string orig_str_file_content {get_file_contents(target_path.c_str())};
  std::string fixed_str_file_content {};

  std::string item {};

  canonize_file_format(orig_str_file_content, fixed_str_file_content, false); // fix NO_CST dangling and remove comments.
  for(size_t p=0, q=0; p!=fixed_str_file_content.npos; p=q) {
    item = fixed_str_file_content.substr(p+(p!=0), (q=fixed_str_file_content.find('\033', p+1))-p-(p!=0)) ;
    insert_async(item, current_string_map);
  }
  generate_tel_includes_async(filename, current_string_map);
  return current_string_map;
}


void
builder_context::parse_tel_targets_async()
{
  std::vector<std::future<string_map_t> > readers;

  std::cout << "Parallel launch : \033[4;40m\033[32m" << std::flush;
  for(const auto& file : m_tel_file_map)
    {

      std::string target_path {file.second};
      target_path = m_utl_directory + "/" +target_path+ "." + doc_extension;

      if  (file_exists(target_path.c_str()))
        {
          std::cout <<"[" << file.first << "]" <<  std::flush;
          readers.push_back(std::async(std::launch::async,  load_doc_file_async, file.second, target_path));
        }
      else
        std::cout << "\033[4;40m\033[31m[" << target_path   << "]\033[m\033[4;40m\033[32m"  << std::endl;
    }
  std::cout <<"\033[m" << std::endl;
  std::cout << "Waiting tasks parallel: {\033[4;40m\033[32m" << std::flush;

  for(auto  &reader : readers) {

    reader.wait();

    string_map_t result { std::move(reader.get()) } ;
    std::cout << "." << std::flush;
    m_string_map.insert(result.begin(), result.end());
  }
  std::cout << "}\033[m" << std::endl;
  generate_tel_ref();
}



// Compute Levenshtein Distance
// Martin Ettl, 2012-10-05

size_t uiLevenshteinDistance(const std::string &s1, const std::string &s2)
{
  const size_t m(s1.size());
  const size_t n(s2.size());

  if( m==0 ) return n;
  if( n==0 ) return m;

  size_t *costs = new size_t[n + 1];

  for( size_t k=0; k<=n; k++ ) costs[k] = k;

  size_t i = 0;
  for ( std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i )
    {
      costs[0] = i+1;
      size_t corner = i;

      size_t j = 0;
      for ( std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j )
        {
          size_t upper = costs[j+1];
          if( *it1 == *it2 )
            {
              costs[j+1] = corner;
            }
          else
            {
              size_t t(upper<corner?upper:corner);
              costs[j+1] = (costs[j]<t?costs[j]:t)+1;
            }

          corner = upper;
        }
    }

  size_t result = costs[n];
  delete [] costs;

  return result;
}

std::pair<std::string, std::string>
parse_def_entry(const std::string &entry)
{
  static const std::regex& rgx {R"raw(@(\d+))raw" , std::regex::optimize};
  std::smatch entry_match;
  std::string variable_part;
  std::string static_part;
  
  std::vector<std::string> entry_lines{ split_appfile(entry) };
 
  for(std::vector<std::string>::size_type index = 0; index != entry_lines.size(); index++)
    {
      std::cout << entry_lines[index] << '\n';
      if (std::regex_match(entry_lines[index], entry_match, rgx))
        {
          // std::string fmt_s = entry_match.format(
          //                                           // $` means characters before the match
          //                                        "[$&]"  // $& means the matched characters
          //                                        );  // $' means characters following the match
          // std::cout << "matched : " << fmt_s << " sub[1] = " << entry_match[1] << '\n';
          int size {std::stoi(entry_match[1])};
          
          if ((index + size) > entry_lines.size())
            { // @[num] is bigger than possible in file!!
              return std::pair<std::string, std::string>{variable_part, static_part};
            }
            
          while (size--)
            variable_part += entry_lines[index++] + '\n';
          if (index < entry_lines.size())
            variable_part += entry_lines[index] + '\n';
          
          if (index >= entry_lines.size()) // for error in file!!
            break;
        }
      else
        {
          static_part += entry_lines[index] + '\n';
        }
    }
  // std::cout << "collected variable_part :%%\n" << variable_part << "%%\n";
  // std::cout << "collected static_part :%%\n" << static_part << "%%\n";
  return std::pair<std::string, std::string>{variable_part, static_part};
}
 // std::vector<std::vector<double>> split_ends(const std::vector<double>& source, const std::vector<int>& ends) {
 //    std::vector<std::vector<double>> result;
 //    result.reserve(ends.size());
 //    auto anchor_front = source.begin();
 //    for (auto one_end: ends) {
 //        auto anchor_end = std::next(source.begin(), one_end + 1);
 //        result.emplace_back(anchor_front, anchor_end);
 //        anchor_front = anchor_end;
 //    }


void
builder_context::load_def(const std::string t_target_basename)
{
  static const std::regex& rgx {R"raw((\d+)\.(\d+)\n([^\0]+))raw" , std::regex::optimize};
  static const std::sregex_iterator endit;
  std::string def_path {m_data_directory};
  def_path += "/" + t_target_basename + "." + def_extension ;
  
  if (file_exists(def_path.c_str()))
    {
      if (m_debug)
        std::cout << "\033[4;40m\033[32m" <<  def_path  << "\033[m" << std::endl;
      std::string deffile_content {get_file_contents(def_path.c_str())};

      for(size_t p=0, q=0; p!=deffile_content.npos; p=q) {
        std::string item = deffile_content.substr(p+(p!=0), (q=deffile_content.find('\033', p+1))-p-(p!=0)) ;
        
        for (std::sregex_iterator it(item.begin(), item.end(), rgx); it != endit; ++it) {
          auto&& m = *it;
          std::string payload {m[3]};
        
          std::pair<int,int> ref{ std::stoi(m[1]) ,std::stoi(m[2])};
          auto pit = m_def_map.find(ref);
          if (pit != m_def_map.end())
            {
              if (pit->second.first != payload) {
                std::string old_def {pit->second.first};
                std::string new_def {payload};
                replace(old_def.begin(), old_def.end(), '\n', ' ');
                replace(new_def.begin(), new_def.end(), '\n', ' ');
                replace(old_def.begin(), old_def.end(), '\t', ' ');
                replace(new_def.begin(), new_def.end(), '\t', ' ');
                std::cerr << "ref : " << ref.first << "." << ref.second << " levenshtein distance : "
                          << uiLevenshteinDistance(old_def,  new_def) << "edit distance "
                          << " already exists! duplicated def in : " << def_path << std::endl;
                std::cerr << "existing : %%"<< old_def << "%%" << std::endl;
                std::cerr << "new : %%"<< new_def << "%%"  << std::endl;
                if (pit->second.first.size() < payload.size() ){
                  // m_def_map [ref_int] = payload;
                  std::cerr << "ref : " << ref.first << "." << ref.second << " redefined!" << std::endl;
                }
              } else { // silently ignore identical definitions !
                std::cerr << "ref : " << ref.first << "." << ref.second << " already exists! but same content!" << std::endl;
              }
            }
          else {
            m_def_map [ref] = parse_def_entry(payload);
            break;
          }
        }
      }
    }
  else
    std::cerr << "Missing DEF file name in data directory : " <<def_path << std::endl;

}

void
builder_context::generate_tel_hlp()
{
  std::ofstream hlp_stream("HLP_STR",  std::ios_base::out );

  for(const auto& file : m_tel_file_map)
    {

      load_def(file.second);
    }

  for  (const auto& item : m_def_map) {
  
    if (item.first.first > 2) break;
    std::string ref{""};
    ref = std::to_string(item.first.first) + "." + std::to_string(item.first.second);
      // hlp_stream << "\033" << ref << std::endl;
      // for (const auto &line : def_item)
      //   hlp_stream << line<< std::endl;

      // hlp_stream << def_lenght << std::endl;
      // hlp_stream << def_lenght << std::endl;
      // hlp_stream << linked_ref<< std::endl;
  }
}




void
builder_context::generate_tel_locate()
{
  std::ofstream locate("LOCATE",std::ios_base::out); //  std::ios_base::app |
  for  (const auto& item : m_string_map) {
    locate << item.first << "|" << item.second.m_pos1 << "|" << item.second.m_pos2 << "|" << item.second.m_len << "|" << item.second.m_cst  << std::endl;
    // locate << item << "|" << m_string_map[item].m_pos1 << "|" << m_string_map[item].m_pos2 << "|" << m_string_map[item].m_len << "|" << m_string_map[item].m_cst  << std::endl;
  }
}

void
builder_context::generate_tel_ref()
{
  std::ofstream ref_stream("REF_STR",  std::ios_base::out );
  for  (const auto& item : m_string_map) {
    for  (const auto& ref:item.second.m_refs)
      {
        ref_stream << std::left << ref.first << std::endl;
      }
    ref_stream << "\033*" << item.first <<"*"<< item.second.m_refs.size() << std::endl;
  }
}

void
builder_context::generate_stu(const std::string language,const std::map<std::string, std::string>  &t_tsl_map)
{
  size_t string_glyph_length(const std::string &input);
  void   remy_scan(std::string &input);
  size_t string_byte_size(const std::string &input);

  std::string name{language};
  name += "_STU";
  std::ofstream stu(name,  std::ios_base::out);
  for  (const auto& item : m_string_map) {
    size_t len = item.second.m_len ;
    for  (const auto& ref:item.second.m_refs)
      {
        if (t_tsl_map.count(ref.first) > 0)
          {
            std::string locale_str {t_tsl_map.at(ref.first)};
	    from_alecode2utf8(locale_str);

            if (locale_str.find_first_of("\?") != std::string::npos)
              {; }
            else
              {
		if (string_glyph_length(locale_str) > len)
                  locale_str.erase(len, std::string::npos) ;
              }
            stu << std::left << std::setw(len) << locale_str << std::endl;
          }
        else
          {
            stu << std::left << std::setw(len) << "? " << std::endl;
          }
      }
    stu  << "\033*"<< item.first <<"*"  << item.second.m_refs.size() << std::endl;
  }
}

bool
builder_context::load_tsl_async(const std::string& language)
{
  static const std::regex& rgx {R"raw((\d+\.\d+)\n(.+)\n)raw", std::regex::optimize };
  static const std::sregex_iterator endit;

  std::string lang_extension {language};
  bool error = false;
  std::transform(lang_extension.begin(), lang_extension.end(), lang_extension.begin(), ::tolower);
  std::map<std::string, std::string>  a_tsl_map {};

  for(const auto& file : m_tel_file_map)
    {
      std::string tsl_path {m_data_directory};
      tsl_path += "/" + file.second + "." + lang_extension ;

      if (file_exists(tsl_path.c_str()))
        {
          std::string tslfile_content {get_file_contents(tsl_path.c_str())};
          std::string item {};

          for(size_t p=0, q=0; p!=tslfile_content.npos; p=q) {
            item = tslfile_content.substr(p+(p!=0), (q=tslfile_content.find('\033', p+1))-p-(p!=0)) ;

            for (std::sregex_iterator it(item.begin(), item.end(), rgx); it != endit; ++it) {
              auto&& m = *it;
              if (m[1] != dummy_ref)
                a_tsl_map[m[1]] = m[2];
            }
          }
        }
      else
        error  = true;
    }
  generate_stu(language, a_tsl_map);
  return error;
}


bool
static_call_load_tsl_async( builder_context *self, const std::string &lang)
{
  return  self->load_tsl_async(lang);
}



void
builder_context::generate_tel_binary_tsl_async()
{
  std::vector<std::future<bool> > readers;
  //auto  = std::bind(&builder_context::load_tsl_async, this, std::placeholders::_1);
  //auto static_call_load_tsl_async = [this] (const std::string lang)
  //   {
  //   this->load_tsl_async(lang);
  // };
  std::cout << "parallel launch : \033[4;40m\033[32m" << std::flush;
  for(const auto language : m_lang_map)
    {
      std::cout<< "[" << language.first<< "]" << std::flush;
      readers.push_back(std::async(std::launch::async,  static_call_load_tsl_async, this ,language.first));
    }
  std::cout <<"\033[m" << std::endl;
  generate_tel_locate();
  generate_tel_ref();
  generate_tel_hlp();
  std::cout << "Waiting tasks parallel: {\033[4;40m\033[32m" << std::flush;
  for(auto  &reader : readers) {

    reader.wait();
    bool result { std::move(reader.get()) } ;
    if (!result)
      std::cout << "\033[32m.\033[m" << std::flush;
    else
      std::cout << "\033[31m~\033[m" << std::flush;

  }
  std::cout << "}\033[m" << std::endl;
}

void
builder_context::generate_app_hlp(const std::string &t_name, const string_map_t  &t_string_map)
{
  std::string app_name{t_name};
  app_name.erase(app_name.begin() + app_name.find('_'), app_name.end());
  std::transform(app_name.begin(), app_name.end(), app_name.begin(), ::toupper);
  std::string hlp_app_name {"HLP_"};
  hlp_app_name = hlp_app_name + app_name;
  
  std::ofstream app_stream(app_name,  std::ios_base::out);
  std::ofstream hlp_app_stream(hlp_app_name,  std::ios_base::out );
  std::cout << "Generate HLP file for application :  " << app_name << " " << hlp_app_name  << "\t {";
  for  (const auto& item : t_string_map)
    {
      std::string ref_str {item.second.m_refs[0].first};
      size_t npos = ref_str.find_last_of('.');
      std::string ref_0 =  ref_str.substr(0,npos);
      std::string ref_1 =  ref_str.substr(npos+1);
      std::pair<int,int> ref{ std::stoi(ref_0) ,std::stoi(ref_1)};
      auto pit = m_def_map.find(ref);

      
      if (pit != m_def_map.end())
        {
          std::cout << ".";
          std::vector<std::string> static_entries{ split_appfile(pit->second.second) };
          hlp_app_stream  << "\033" << ref_str << std::endl;
          hlp_app_stream << pit->second.first;
          if (static_entries.size() >2)
            hlp_app_stream  << static_entries[1]  << std::endl;
          else
            hlp_app_stream  << item.second.m_len  << std::endl;
          hlp_app_stream  << item.second.m_len  << std::endl;
          //  hlp_app_stream  << "<<" << linked_ref << ">>" << std::endl;
           if (static_entries.size() > 3)
            hlp_app_stream  << static_entries[2]  << std::endl;
          else
            hlp_app_stream  << "NO" << std::endl;
        }
    }
  std::cout << "}" << std::endl;
  app_stream.close();
  hlp_app_stream.close();
}

void
generate_app_includes_async(const std::string &t_name, const string_map_t  &t_string_map)
{
  // Three files containing declaration of constants: 
  // gencst_<string file>_1.hpp:
  // contains all constants that define for each context the number of strings of
  // the context (included in inc/cpu_dec/constants/string.hpp).
  std::string   gencst_1_name {get_include_filename("gencst_",t_name,  "_1.hpp")};
  std::ofstream gencst_1(gencst_1_name );
  gencst_1 << header_helper(gencst_1_name);
  // gencst_<string file>_2.hpp:
  // contains all constants that define the the constant identifying each context
  // (included in inc/cpu_dec/constants/string.hpp).
  std::string   gencst_2_name {get_include_filename("gencst_",t_name,  "_2.hpp")};
  std::ofstream gencst_2(gencst_2_name );
  gencst_2 << header_helper(gencst_2_name);

  std::string app_define{t_name} ;
  app_define.erase(app_define.begin() + app_define.find('_'), app_define.end());
  std::transform(app_define.begin(), app_define.end(), app_define.begin(), ::toupper);

  std::string  ref_app_file { app_define};
  ref_app_file = "REF_" + ref_app_file;
  std::ofstream app_ref(ref_app_file);


  gencst_2 << "#define SIZE_STR_FIC_" << app_define<< "\t" << t_string_map.size() <<std::endl;
  for  ( const auto& item : t_string_map) {
    if (item.second.m_cst == "-undefined")
      gencst_1 << "/* no constant for idx " << item.first << " */" << std::endl;
    else
      gencst_1 << "#define " << item.second.m_cst << "\t" << item.first << std::endl;
    app_ref << item.second.m_refs[0].first << std::endl;
  }

  gencst_1<< std::endl<< "#endif" << std::endl;
  gencst_2<< std::endl<< "#endif" << std::endl;
  gencst_1.close();
  gencst_2.close();
  app_ref.close();
}


void
builder_context::generate_app_tsl(const std::string language,const std::string &app_name,const string_map_t  &app_string_map,
                                  const std::map<std::string, std::string>  &t_tsl_map)
{
  std::string name{app_name};

  name.erase(name.begin() + name.find('_'), name.end());
  name = language + "_" + name;
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);

  std::ofstream str_app(name,  std::ios_base::out);
  for  (const auto& item : app_string_map) {
    size_t len = item.second.m_len ;
    for  (const auto& ref:item.second.m_refs)
      {
        if (t_tsl_map.count(ref.first) > 0)
          {
            std::string locale_str {t_tsl_map.at(ref.first)};

            str_app << std::left << std::setw(len) << locale_str << std::endl;
          }
        else
          {
            str_app << std::left << std::setw(len) << "? " << std::endl;
          }
      }

  }
}

bool
builder_context::load_app_tsl_async(const std::string& language, const std::string &app_name,const string_map_t  &app_string_map )
{
  static const std::regex& rgx {R"raw((\d+\.\d+)\n(.+)\n)raw", std::regex::optimize };
  static const std::sregex_iterator endit;

  std::string lang_extension {language};
  bool error = false;
  std::transform(lang_extension.begin(), lang_extension.end(), lang_extension.begin(), ::tolower);
  std::map<std::string, std::string>  a_tsl_map {};


  std::string tsl_path {m_data_directory};
  tsl_path += "/" + app_name + "." + lang_extension ;


  if (file_exists(tsl_path.c_str()))
    {
      std::string tslfile_content {get_file_contents(tsl_path.c_str())};
      std::string item {};

      for(size_t p=0, q=0; p!=tslfile_content.npos; p=q) {
        item = tslfile_content.substr(p+(p!=0), (q=tslfile_content.find('\033', p+1))-p-(p!=0)) ;

        for (std::sregex_iterator it(item.begin(), item.end(), rgx); it != endit; ++it) {
          auto&& m = *it;
          if (m[1] != dummy_ref)
            a_tsl_map[m[1]] = m[2];
        }
      }
    }
  else
    error  = true;
  
  generate_app_tsl(language, app_name, app_string_map, a_tsl_map);
  return error;
}


void
builder_context::parse_all_applications()
{
  for(const auto& file : m_app_file_map)
    {
      std::cout << file.first << "/" << file.second << std::endl;
      std::string target_path {file.second};
      target_path = m_utl_directory + "/" +target_path+ "." + doc_extension;
      if  (file.second == "STR") continue; // skip tel.
      if  (file_exists(target_path.c_str()))
        {
          string_map_t  current_string_map {} ;
          std::cout << "\033[4;40m\033[32m[" << file.first << "]\033[m" << "\t{" ;
          std::string orig_str_file_content {get_file_contents(target_path.c_str())};
          std::string fixed_str_file_content {};

          std::string item {};
          canonize_file_format(orig_str_file_content, fixed_str_file_content, true); // fix NO_CST dangling and remove comments.
          for(size_t p=0, q=0; p!=fixed_str_file_content.npos; p=q) {
            item = fixed_str_file_content.substr(p+(p!=0), (q=fixed_str_file_content.find('\n', p+1))-p-(p!=0)) ;
            if (item.size() > 0) {
              bool result { insert_record(item, current_string_map) };
              if (result)
                std::cout << "\033[32m.\033[m" ;
              else
                std::cout << "\033[31m~\033[m" ;
            }

          }
          std::cout <<"}" << std::endl ;
          generate_app_includes_async(file.second, current_string_map);
          load_def(file.second);
          generate_app_hlp(file.second, current_string_map);
          for(const auto language : m_lang_map)
            {
              std::cout<< "[" << language.first<< "]" << std::flush;
              load_app_tsl_async(language.first, file.second, current_string_map);
            }
        }
      else
        std::cout << "\033[4;40m\033[31m" << target_path   << "\033[m"  << std::endl;
    }
}
