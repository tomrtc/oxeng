#pragma once

#include <map>
#include <string>
#include <vector>


// define a 'always' lowercase string see: http://www.gotw.ca/gotw/029.htm
template<typename T>
struct lowercase_char_traits : std::char_traits<T>
{
    static T* copy(T* dest, const T* src, std::size_t count )
    {
         for(size_t i = 0 ; i < count ; ++i)
              dest[i] = std::tolower(src[i]);
         return dest;
    }
    static void assign(T & out, T in)
    {
       out = std::tolower(in);
    }
};
typedef std::basic_string<char, lowercase_char_traits<char>> case_insensitive_string;


typedef std::vector<std::pair<std::string, std::string> > refs_vector;
struct fldrecord {
  fldrecord() = default;
  fldrecord( int16_t  pos1, int16_t pos2, int16_t len, std::string fld, std::string cst)
    :  m_pos1(pos1), m_pos2(pos2), m_len(len), m_fld(fld), m_cst(cst) {}; 
  fldrecord(std::string fld , int16_t len, std::string cst, std::string ref)
    :  m_len(len), m_fld(fld), m_cst(cst) , m_refs{{ref,cst}}  {};


public: 
  int16_t           m_pos1 {0};
  int16_t m_pos2 {0};
  int16_t m_len {0};
  std::string m_fld{""} ;
  std::string m_cst{""} ;
  refs_vector m_refs {};

};
struct string_map_t_comparator
{
  bool operator() (const std::string& lhs, const std::string& rhs) const
   {
      return std::stoi(lhs) < std::stoi(rhs); 
   }
};
typedef std::map<std::string, fldrecord,string_map_t_comparator> string_map_t;


class builder_context{
 
  std::string                        m_data_directory{} ;
  std::string                        m_string_path{} ;
  std::string                        m_utl_directory{} ;
 
  string_map_t  m_string_map{} ;
  std::map<std::string, std::string> m_lang_map{} ;
  std::map<int, std::string> m_app_file_map{} ;
  std::map< int, std::string> m_tel_file_map{} ;
  bool                               m_debug {false};
  bool                               m_parallel {true};
 
  std::map<std::pair<int,int> , std::pair<std::string, std::string>> m_def_map{} ;
 

  bool insert_record(const std::string& input, string_map_t &t_stringmap);
  
  void insert_tsl(const std::string& lang, const std::string& input);
  void insert_def(const std::string& input);
  
public:
  void generate_tel_locate();
  void generate_stu(const std::string language,const std::map<std::string, std::string>  &t_tsl_map);
  void generate_tel_ref();
  void generate_tel_hlp();
  void generate_tel_includes(const std::string &t_name, const string_map_t  &t_string_map);
  void load_all_tsl(const std::string language);
  void load_def(const std::string t_target_basename);
  void generate_app_hlp(const std::string &t_name, const string_map_t  &t_string_map);
  void set_string_directory(const char* t_string_directory);
  void set_utl_directory(const char* t_utl_directory);

  void parse_target();
  void parse_tel_targets();
  void parse_tel_targets_async();
  void parse_all_applications();
  void set_debug(){m_debug = true; set_parallel(false);} ;
  void set_parallel(bool t_mode){m_parallel = t_mode;} ;

  void generate_tel_binary_tsl_async();
 
  bool load_tsl_async(const std::string& language);
  bool load_app_tsl_async(const std::string& language, const std::string &app_name,const string_map_t  &app_string_map );
  void generate_app_tsl(const std::string language,const std::string &app_name,
  const string_map_t  &app_string_map, const std::map<std::string, std::string>  &t_tsl_map);
  bool app_async(const std::string&app_file);
}; 




std::vector<std::string> split_docfile(const std::string& input)        ;

std::vector<std::string> split_appfile(const std::string& input);
bool directory_exists(const char *path);
bool file_exists(const char *path);
std::string get_file_contents(const std::string filename);
size_t string_byte_size(const std::string &input);
size_t string_glyph_length(const std::string &input);
void canonize_file_format( std::string& input,std::string& output, bool application);
void from_alecode2utf8(std::string &input);
