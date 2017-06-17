// generators 

#include <oxeng.hh>


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



static const std::string doc_extension {"doc"};
static const std::string def_extension {"def"};

void
builder_context::set_target(const char *t_target_path_argv)
{
  if (file_exists(t_target_path_argv))
    {// save the target file name and get the file content in a string, in addition the file format is fixed to ease parsing.
      std::string basename_argv { basename(const_cast<char*>(t_target_path_argv))};
      m_target = std::string(t_target_path_argv);
      /* const_cast can be used to remove or add const to a variable; no other C++ cast is capable of removing it (not even reinterpret_cast). 
         It is important to note that modifying a formerly const value is only undefined if the original variable is const; 
         if you use it to take the const off a reference to something that wasn't declared with const, it is safe. 
         This can be useful when overloading member functions based on const, for instance. It can also be used to add const to an object, such as to call a member function overload.
         const_cast also works similarly on volatile, though that's less common*/
      size_t npos = basename_argv.find_last_of('.');
      m_target_basename =  basename_argv.substr(0,npos);
    }
  else
    {;} //throw(errno);
}

static const std::vector<std::string> FldSTR { "FLD", "POS1", "POS2", "LEN", "CST" };
static const std::vector<std::string> FldAPP { "STR", "REF", "LEN", "CST" };
static const std::vector<std::string> FldAPPbis { "STR", "REF", "LEN", "NOCST" };

std::vector<std::string>
split_field(const std::string& input,int submatch ) {
  static const  std::regex& re{"(FLD|POS1|POS2|LEN|CST|REF|NOCST|STR)=([^|\\n# \\t]+)", std::regex::optimize};
  
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
builder_context::parse_target()
{
  std::string orig_str_file_content = get_file_contents(m_target);
  size_t   initial_hash {std::hash<std::string>{}(orig_str_file_content)};
  std::string fixed_str_file_content {};
  
  canonize_file_format(orig_str_file_content, fixed_str_file_content); // fix NO_CST dangling and remove comments.
  std::cout<<"Doc filename : " << m_target << " : " << initial_hash <<  std::endl;
  std::vector<std::string> docitems {};
  if (m_file_map[m_target_basename] == "STR")
    {
      docitems = std::move(split_docfile(fixed_str_file_content)); 
    }
  else
    {
      docitems = std::move(split_appfile(fixed_str_file_content));
    }
  for(const auto& i : docitems)
    {
      bool result { insert_record(i, m_string_map) };
      if (result)
        std::cout << "." ;
    }
  std::cout << std::endl ;
}

void
builder_context::set_configuration_directory(const char* t_configuration_directory)
{
  if (directory_exists(t_configuration_directory))
    {
      std::string path_lang {t_configuration_directory};
      std::string path_file {t_configuration_directory};  
      path_lang += "/lang.cnf";  
      path_file += "/file.cnf";
      {
        std::string lang_cnf { get_file_contents(path_lang.c_str())};
        static const  std::regex& rgx {R"raw((\w+)\s+\"([^\"]*)\")raw", std::regex::optimize};
        std::sregex_iterator endit;
        for (std::sregex_iterator it(lang_cnf.begin(), lang_cnf.end(), rgx); it != endit; ++it) {
          auto& m =  *it;
          m_lang_map[m[1]] = m[2];
        }
      }
      
      {
        std::string file_cnf { get_file_contents(path_file.c_str())};
        static const std::regex& rgx{R"raw((\d+)\s+([\w.]+)\s+(\w+))raw" ,std::regex::optimize};
        std::sregex_iterator endit;
        for (std::sregex_iterator it(file_cnf.begin(), file_cnf.end(), rgx); it != endit; ++it) {
          auto&& m = *it;
          std::string name{m[2]}; 
          size_t npos = name.find_last_of('.');
          name =  name.substr(0,npos);
          m_file_map[name]  = m[3];
        }
      }
    }
  else
    {;} // throw(errno);
}

void
builder_context::set_data_directory(const char* t_data_directory)
{
  if (directory_exists(t_data_directory))
    {
      m_data_directory = std::string{t_data_directory};
    }
  else
    { ; } // throw(errno);
      
} 

void
builder_context::set_source_directory(const char* t_source_directory)
{
  m_source_directory = std::string(t_source_directory);
}

void
builder_context::set_string_path(const char* t_string_path)
{
  m_string_path = std::string(t_string_path);
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
  cpp_guard = "#ifndef " + cpp_guard + "\n#define " + cpp_guard + "/*Generated file do not modify!.*/\n\n";
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
  
  canonize_file_format(orig_str_file_content, fixed_str_file_content); // fix NO_CST dangling and remove comments.
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
  for(const auto& file : m_file_map)
    {
      
      std::string target_path {file.first};
      target_path = m_string_path+ "/" +target_path+ "." + doc_extension;
      if  (file.second == "APP") continue; 
      if  (file_exists(target_path.c_str()))
        {
          std::cout <<"[" << file.first << "]" <<  std::flush;
          readers.push_back(std::async(std::launch::async,  load_doc_file_async, file.first, target_path));
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
  generate_ref();
}
 


void
builder_context::parse_tel_targets()
{
  for(const auto& file : m_file_map)
    {
      // std::cout << file.first << "/" << file.second << std::endl;
      std::string target_path {file.first};
      target_path = "../test/" +target_path+ "." + doc_extension;
      if  (file.second == "APP") continue; 
      if  (file_exists(target_path.c_str()))
        {
          string_map_t  current_string_map {} ;
          std::cout << "\033[4;40m\033[32m[" << file.first << "]\033[m" << "\t{" ;
          std::string orig_str_file_content {get_file_contents(target_path.c_str())};
          std::string fixed_str_file_content {};
          
          std::string item {};
          canonize_file_format(orig_str_file_content, fixed_str_file_content); // fix NO_CST dangling and remove comments.
          for(size_t p=0, q=0; p!=fixed_str_file_content.npos; p=q) {
            item = fixed_str_file_content.substr(p+(p!=0), (q=fixed_str_file_content.find('\033', p+1))-p-(p!=0)) ;
            bool result { insert_record(item, current_string_map) };
            if (result)
              std::cout << "\033[32m.\033[m" ;
            else
              std::cout << "\033[31m~\033[m" ;
          }
          std::cout <<"}" << std::endl ;

          generate_tel_includes_async(file.first, current_string_map);
          // merge the current entries in the member map.
          m_string_map.insert(current_string_map.begin(), current_string_map.end());
        }
      else
        std::cout << "\033[4;40m\033[31m" << target_path   << "\033[m"  << std::endl;
    }
   

}
      
void 
builder_context::load_all_tsl(const std::string language)
{

  for(const auto& file : m_file_map)
    {
      std::string lang_extension {language};
      std::transform(lang_extension.begin(), lang_extension.end(), lang_extension.begin(), ::tolower);

      std::string tsl_path {m_data_directory};
      tsl_path += "/" + file.first + "." + lang_extension ;

      if (file_exists(tsl_path.c_str()))
        {
          if (m_debug)
            std::cout << "\033[4;40m\033[32m" << file.first << " : " << tsl_path  << "\033[m" << std::endl;
          std::string tslfile_content {get_file_contents(tsl_path.c_str())};
          std::string item {};
        
          for(size_t p=0, q=0; p!=tslfile_content.npos; p=q) {
            item = tslfile_content.substr(p+(p!=0), (q=tslfile_content.find('\033', p+1))-p-(p!=0)) ;
            insert_tsl(language, item);
          }
          
        }
      else
        std::cout << "\033[4;40m\033[32m" << m_lang_map[language] << " : \033[m\033[31m#" << "{" << tsl_path << "}"  << "\033[m"  << std::endl;
    }
}

void
builder_context::insert_def(const std::string& input)
{
  static const std::regex& rgx {R"raw((\d+\.\d+)\n(.+))raw" , std::regex::optimize};
  static const std::sregex_iterator endit;

  for (std::sregex_iterator it(input.begin(), input.end(), rgx); it != endit; ++it) {
    auto&& m = *it;
    m_all_tsl_map[m[1]]["DEF"] = m[2];
  }
}

void 
builder_context::load_def()
{
  std::string def_path {m_data_directory};
  def_path += "/" + m_target_basename + "." + def_extension ;

  if (file_exists(def_path.c_str()))
    {
      if (m_debug)
        std::cout << "\033[4;40m\033[32m" <<  def_path  << "\033[m" << std::endl;
      // std::vector<std::string> def_vector { split_tslfile(get_file_contents(def_path.c_str()))};
          
      //     for(const auto& a_def_record : def_vector)
      //      insert_def(a_def_record);
    }
  else
    std::cerr << "Missing DEF file name in data directory : " <<def_path << std::endl;
    
} 




void
builder_context::generate_locate()
{
  std::ofstream locate("LOCATE",std::ios_base::out); //  std::ios_base::app | 
  for  (const auto& item : m_string_map) {
    locate << item.first << "|" << item.second.m_pos1 << "|" << item.second.m_pos2 << "|" << item.second.m_len << "|" << item.second.m_cst  << std::endl;
    // locate << item << "|" << m_string_map[item].m_pos1 << "|" << m_string_map[item].m_pos2 << "|" << m_string_map[item].m_len << "|" << m_string_map[item].m_cst  << std::endl;
  }
}
void
builder_context::generate_ref()
{
  std::ofstream ref_stream("REF_STR",  std::ios_base::app );//| std::ios_base::out
  for  (const auto& item : m_string_map) {
    for  (const auto& ref:item.second.m_refs)
      {
        ref_stream << std::left << ref.first << std::endl;
      }
    ref_stream << "\033*" << item.first <<"*"<< item.second.m_refs.size() << std::endl;
  }
}




void
builder_context::insert_tsl(const std::string& lang, const std::string& input)
{
  static const std::regex& rgx {R"raw((\d+\.\d+)\n(.+)\n)raw", std::regex::optimize };
  static const std::sregex_iterator endit;
  static const  std::string& dummy_ref {"1.0"} ; 

  for (std::sregex_iterator it(input.begin(), input.end(), rgx); it != endit; ++it) {
    auto&& m = *it;
    if (m[1] != dummy_ref)
      m_all_tsl_map[m[1]][lang] = m[2];
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
  static const  std::string& dummy_ref {"1.0"} ; 
  std::string lang_extension {language};
  bool error = false;
  std::transform(lang_extension.begin(), lang_extension.end(), lang_extension.begin(), ::tolower);
  std::map<std::string, std::string>  a_tsl_map {}; 

  for(const auto& file : m_file_map)
    {
      std::string tsl_path {m_data_directory};
      tsl_path += "/" + file.first + "." + lang_extension ;

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


